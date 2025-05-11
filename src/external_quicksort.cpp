#include <algorithm>
#include <calculate_arity.h>
#include <chrono>
#include <cstring>
#include <external_quicksort.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <random>
#include <string>
#include <sys/stat.h>
#include <vector>

using namespace std;

// Use the constants from calculate_arity.h
const int64_t BLOCK_SIZE = 4096;
const int64_t INTS_PER_BLOCK = BLOCK_SIZE / sizeof(int64_t);
// Use 80% of available memory as recommended
const int64_t TOTAL_MEMORY_RAM = 50 * 1024 * 1024;
const int64_t EFFECTIVE_MEMORY = TOTAL_MEMORY_RAM * 0.8;

int64_t total_io_operations = 0;

// Forward declarations of external functions
// These functions are implemented in calculate_arity.cpp
vector<int64_t>
read_multiple_blocks(const string &filename, int64_t start_block, int64_t num_blocks_to_read);
void sort_in_memory(vector<int64_t> &data);
void create_directories(const string &dir);
void remove_directory(const string &dir);
void copy_file(const string &src, const string &dst);

/**
 * @brief Improved pivot selection for better balanced partitions
 * @param input_file Path to the input binary file
 * @param arity Number of partitions to create (a-way partitioning)
 * @return Vector of arity-1 sorted pivot values
 */
vector<int64_t> select_pivots(const string &input_file, int64_t arity) {
    if (arity <= 1)
        return {};

    ifstream in(input_file, ios::binary | ios::ate);
    if (!in) {
        cerr << "Error opening file " << input_file << endl;
        exit(EXIT_FAILURE);
    }
    int64_t file_size = in.tellg();
    in.close();
    total_io_operations++;

    int64_t num_blocks = file_size / BLOCK_SIZE;
    if (num_blocks == 0)
        return {};

    // Determine sample size: more samples for higher arity, but respect memory limits
    int64_t sample_blocks = min(int64_t(20), num_blocks);
    // Use more samples for higher arities to get better pivots
    if (arity > 64) {
        sample_blocks = min(int64_t(30), num_blocks);
    }

    vector<int64_t> samples;
    // Reserve space based on number of blocks to be sampled
    samples.reserve(sample_blocks * INTS_PER_BLOCK / 4);

    std::mt19937 rng{std::random_device{}()};

    // Sample from beginning, end, and evenly distributed blocks throughout the file
    vector<int64_t> positions;
    // Always include first and last blocks
    positions.push_back(0);
    if (num_blocks > 1)
        positions.push_back(num_blocks - 1);

    // Add evenly distributed blocks
    for (int i = 1; i < sample_blocks - 1; i++) {
        positions.push_back((i * num_blocks) / sample_blocks);
    }

    // Add some randomness for variety
    std::shuffle(positions.begin() + 2, positions.end(), rng);

    // Process unique positions only
    std::sort(positions.begin(), positions.end());
    auto last = std::unique(positions.begin(), positions.end());
    positions.erase(last, positions.end());

    /**
     * This for:
     * For each selected block position:
     *  - Read the block from disk
     *  - Take every n-th element as a sample
     *  - Add samples to the samples vector
     * This approach ensures we get a representative sample across the entire file
     * without loading too much data into memory
     */
    for (int64_t block_idx : positions) {
        vector<int64_t> block = read_multiple_blocks(input_file, block_idx, 1);
        total_io_operations++;

        // Take every n-th element to avoid bias
        int take_every = 4; // Take 25% of values

        for (size_t j = 0; j < block.size(); j += take_every) {
            samples.push_back(block[j]);
        }
    }

    // Ensure we have enough samples
    if (static_cast<int64_t>(samples.size()) < arity * 3) {
        if (!samples.empty()) {
            // If we don't have enough samples, duplicate existing ones
            // This should rarely happen but ensures we have enough values
            int original_size = samples.size();
            for (int i = 0; i < original_size && static_cast<int64_t>(samples.size()) < arity * 3; i++) {
                samples.push_back(samples[i]);
            }
        } else {
            // Handle empty file case
            samples = {0};
        }
    }

    // Sort samples to select representative pivots
    std::sort(samples.begin(), samples.end());

    // Select arity-1 equidistant pivots for balanced partitions
    vector<int64_t> pivots;
    pivots.reserve(arity - 1);

    // Calculate quantiles for better distribution
    for (int64_t i = 1; i < arity; i++) {
        // Use exact position calculation to prevent over/underflow
        int64_t idx = (static_cast<int64_t>(samples.size()) * i) / arity;
        if (idx < static_cast<int64_t>(samples.size())) {
            pivots.push_back(samples[idx]);
        }
    }

    // Remove duplicates to ensure unique pivots
    auto last_unique = std::unique(pivots.begin(), pivots.end());
    pivots.erase(last_unique, pivots.end());

    // If we have duplicate pivots, use a different strategy
    if (static_cast<int64_t>(pivots.size()) < arity - 1 && samples.size() >= 2) {
        // Find min and max values in samples
        int64_t min_val = samples.front();
        int64_t max_val = samples.back();
        int64_t range = max_val - min_val;

        // Only regenerate if we have a valid range
        if (range > 0) {
            pivots.clear();
            pivots.reserve(arity - 1);
            for (int64_t i = 1; i < arity; i++) {
                // Generate evenly spaced pivots
                pivots.push_back(min_val + (range * i) / arity);
            }
        }
    }

    // Free sample memory immediately
    vector<int64_t>().swap(samples);

    return pivots;
}

/**
 * @brief Memory-efficient concatenation of partition files
 * @param sorted_files Vector of paths to sorted files
 * @param output_file Path to the output file
 * @return Number of I/O operations performed
 */
int64_t concatenate_partitions(const vector<string> &sorted_files, const string &output_file) {
    int64_t io_count = 0;
    ofstream output(output_file, ios::binary);

    if (!output) {
        cerr << "Error opening output file: " << output_file << endl;
        exit(EXIT_FAILURE);
    }

    // Use an efficient buffer size based on available memory
    // For very large arities, use a smaller buffer to avoid memory pressure
    const int64_t files_to_merge = sorted_files.size();
    const int64_t MAX_BUFFER_SIZE = 4 * 1024 * 1024; // 4MB maximum
    const int64_t MIN_BUFFER_SIZE = 256 * 1024;      // 256KB minimum

    // Calculate buffer size inversely proportional to number of files
    int64_t buffer_size = MAX_BUFFER_SIZE / (1 + files_to_merge / 100);
    buffer_size = max(buffer_size, MIN_BUFFER_SIZE);
    buffer_size = min(buffer_size, MAX_BUFFER_SIZE);

    vector<char> buffer(buffer_size);

    /**
     * This for:
     * Process each sorted file sequentially:
     *  - Open each file one at a time
     *  - Copy file contents to the output file using a fixed-size buffer
     *  - Count I/O operations for each read/write
     *  - Close the file before processing the next one
     * This approach minimizes memory usage by only having one file open at a time
     * and using a single buffer for all copy operations
     */
    for (const string &file : sorted_files) {
        ifstream input(file, ios::binary);

        if (!input) {
            continue;
        }

        // Get file size to avoid unnecessary operations
        input.seekg(0, ios::end);
        int64_t file_size = input.tellg();
        input.seekg(0, ios::beg);

        if (file_size <= 0) {
            input.close();
            continue;
        }

        while (input) {
            input.read(buffer.data(), buffer.size());
            std::streamsize bytes_read = input.gcount();

            if (bytes_read > 0) {
                output.write(buffer.data(), bytes_read);
                io_count += 2;
            }
        }

        input.close();
    }

    output.close();
    return io_count;
}

/**
 * @brief Dynamically calculates optimal buffer sizes based on available memory and arity
 * @param arity Number of partitions
 * @param file_size Size of the input file
 * @return Pair containing read buffer size and partition buffer size (in bytes)
 */
pair<int64_t, int64_t> calculate_buffer_sizes(int64_t arity, int64_t file_size) {
    // Minimum buffer sizes to ensure efficient I/O
    const int64_t MIN_READ_BUFFER = BLOCK_SIZE * 2;  // 8KB minimum
    const int64_t MIN_PARTITION_BUFFER = BLOCK_SIZE; // 4KB minimum (one block)

    // Base buffer size allocations
    int64_t read_buffer_bytes;
    int64_t partition_buffer_bytes;

    // Adjust buffer sizes based on arity
    // Large arity
    read_buffer_bytes = EFFECTIVE_MEMORY * 0.15;
    partition_buffer_bytes = (EFFECTIVE_MEMORY * 0.85) / arity;

    // Ensure minimum buffer sizes
    read_buffer_bytes = max(read_buffer_bytes, MIN_READ_BUFFER);
    partition_buffer_bytes = max(partition_buffer_bytes, MIN_PARTITION_BUFFER);

    // Adjust if file is small compared to buffer sizes
    if (file_size < EFFECTIVE_MEMORY && arity > 2) {
        // For small files with multiple partitions, optimize for fewer I/O operations
        int64_t target_read_size = min(file_size / 4, EFFECTIVE_MEMORY / 4);
        read_buffer_bytes = min(read_buffer_bytes, target_read_size);
    }

    // Final check to ensure we're within memory limits
    int64_t total_buffer_memory = read_buffer_bytes + (partition_buffer_bytes * arity);
    if (total_buffer_memory > EFFECTIVE_MEMORY) {
        // Scale down partition buffers if needed
        double scale_factor = EFFECTIVE_MEMORY / static_cast<double>(total_buffer_memory);
        partition_buffer_bytes = static_cast<int64_t>(partition_buffer_bytes * scale_factor * 0.95);
        partition_buffer_bytes = max(partition_buffer_bytes, MIN_PARTITION_BUFFER);
    }

    // Round buffer sizes to multiples of BLOCK_SIZE for more efficient I/O
    read_buffer_bytes = (read_buffer_bytes / BLOCK_SIZE) * BLOCK_SIZE;
    if (read_buffer_bytes < MIN_READ_BUFFER)
        read_buffer_bytes = MIN_READ_BUFFER;

    return {read_buffer_bytes, partition_buffer_bytes};
}

/**
 * @brief Optimized recursive external quicksort implementation
 * @param input_file File to sort
 * @param output_file File where the sorted result will be saved
 * @param arity Number of partitions to create
 * @param temp_dir Directory for temporary files
 * @param depth Recursion depth (for naming temporary files)
 * @return Number of I/O operations performed
 */
int64_t recursive_external_quicksort(
    const string &input_file, const string &output_file, int64_t arity, const string &temp_dir,
    int64_t depth
) {
    int64_t io_operations = 0;

    // Get file size
    ifstream in_size(input_file, ios::binary | ios::ate);
    if (!in_size) {
        cerr << "Error opening input file: " << input_file << endl;
        exit(EXIT_FAILURE);
    }
    int64_t file_size = in_size.tellg();
    in_size.close();
    io_operations++;

    // If file is empty, create empty output file and return
    if (file_size == 0) {
        ofstream(output_file, ios::binary).close();
        return io_operations;
    }

    // If file is small enough, sort in memory
    if (file_size <= EFFECTIVE_MEMORY / 2) {
        vector<int64_t> data(file_size / sizeof(int64_t));
        ifstream in(input_file, ios::binary);
        in.read(reinterpret_cast<char *>(data.data()), file_size);
        in.close();
        io_operations++;

        sort_in_memory(data);

        ofstream out(output_file, ios::binary);
        out.write(reinterpret_cast<const char *>(data.data()), file_size);
        out.close();
        io_operations++;

        // Clear memory
        vector<int64_t>().swap(data);

        return io_operations;
    }

    // Select pivots for partitioning
    vector<int64_t> pivots = select_pivots(input_file, arity);
    io_operations += 2; // Count pivot selection as 2 I/O operations

    // Calculate optimal buffer sizes based on arity and file size
    auto [read_buffer_bytes, partition_buffer_bytes] = calculate_buffer_sizes(arity, file_size);

    const int64_t READ_BUFFER_SIZE = read_buffer_bytes / sizeof(int64_t);
    const int64_t PARTITION_BUFFER_SIZE = partition_buffer_bytes / sizeof(int64_t);

    if (depth <= 1) { // Only log buffer sizes for top-level calls
        cout << "  Read buffer: " << read_buffer_bytes / 1024
             << " KB, Partition buffers: " << partition_buffer_bytes / 1024 << " KB x " << arity << " = "
             << (partition_buffer_bytes * arity) / 1024 << " KB" << endl;
    }

    vector<vector<int64_t>> partition_buffers(arity);
    vector<int64_t> partition_sizes(arity, 0);

    for (int64_t i = 0; i < arity; i++) {
        partition_buffers[i].resize(PARTITION_BUFFER_SIZE);
    }

    vector<string> partition_files(arity);
    vector<ofstream> partition_streams(arity);

    for (int64_t i = 0; i < arity; i++) {
        partition_files[i] = temp_dir + "partition_" + to_string(depth) + "_" + to_string(i) + ".bin";
        partition_streams[i].open(partition_files[i], ios::binary);

        if (!partition_streams[i]) {
            cerr << "Error creating partition file: " << partition_files[i] << endl;
            exit(EXIT_FAILURE);
        }
    }

    ifstream input(input_file, ios::binary);
    vector<int64_t> read_buffer(READ_BUFFER_SIZE);
    vector<int64_t> total_partition_elements(arity, 0);

    /**
     * This while:
     * Reads and processes the input file in fixed-size chunks (read_buffer):
     *  - Read a block of data from the input file
     *  - For each element in the block:
     *    - Find the appropriate partition using binary search on pivots
     *    - Add the element to the corresponding partition buffer
     *    - If a partition buffer is full, flush it to disk
     *    - Track total elements in each partition for balance reporting
     *  - Continue until the entire file is processed
     * This approach ensures we keep memory usage under control by:
     *  - Processing the file sequentially with fixed buffer size
     *  - Flushing partition buffers as needed
     */
    while (input) {
        input.read(reinterpret_cast<char *>(read_buffer.data()), read_buffer_bytes);
        int64_t elems_read = input.gcount() / sizeof(int64_t);

        if (elems_read == 0) {
            break;
        }

        io_operations++;

        for (int64_t i = 0; i < elems_read; i++) {
            int64_t val = read_buffer[i];

            int64_t partition_idx = 0;
            if (!pivots.empty()) {
                auto it = upper_bound(pivots.begin(), pivots.end(), val);
                partition_idx = distance(pivots.begin(), it);
            }

            // Add to the buffer
            partition_buffers[partition_idx][partition_sizes[partition_idx]++] = val;
            total_partition_elements[partition_idx]++;

            // Flush if buffer is full (balatro reference)
            if (static_cast<int64_t>(partition_sizes[partition_idx]) >= PARTITION_BUFFER_SIZE) {
                partition_streams[partition_idx].write(
                    reinterpret_cast<const char *>(partition_buffers[partition_idx].data()),
                    partition_sizes[partition_idx] * sizeof(int64_t)
                );
                io_operations++;
                partition_sizes[partition_idx] = 0;
            }
        }
    }

    input.close();

    // Flush any remaining data in buffers
    for (int64_t i = 0; i < arity; i++) {
        if (partition_sizes[i] > 0) {
            partition_streams[i].write(
                reinterpret_cast<const char *>(partition_buffers[i].data()),
                partition_sizes[i] * sizeof(int64_t)
            );
            io_operations++;
        }
        partition_streams[i].close();
    }

    // Clear memory
    for (int64_t i = 0; i < arity; i++) {
        vector<int64_t>().swap(partition_buffers[i]);
    }
    vector<int64_t>().swap(pivots);
    vector<int64_t>().swap(read_buffer);

    if (depth <= 1) {
        int64_t total_elements = 0;
        for (int64_t count : total_partition_elements) {
            total_elements += count;
        }
        cout << endl;
    }

    vector<string> sorted_partition_files;
    sorted_partition_files.reserve(arity);

    /**
     * This for:
     * Process each partition file one at a time:
     *  - Check if the partition is empty or very small
     *  - For very small partitions, sort directly in memory
     *  - For larger partitions, recursively apply external quicksort
     *  - Adjust arity for deeper recursion levels based on partition size
     *  - Add the sorted partition file to the list of files to concatenate
     *  - Remove intermediate files immediately after processing
     * This sequential processing minimizes memory usage by handling
     * one partition at a time and cleaning up temporary files as we go
     */
    for (int64_t i = 0; i < arity; i++) {
        ifstream check_file(partition_files[i], ios::binary | ios::ate);
        int64_t partition_size = 0;

        if (check_file) {
            partition_size = check_file.tellg();
            check_file.close();
        }

        // Skip empty partitions
        if (partition_size <= 0) {
            sorted_partition_files.push_back(partition_files[i]);
            continue;
        }

        // For very small partitions, sort in memory directly
        if (partition_size <= BLOCK_SIZE * 2) {
            vector<int64_t> small_data(partition_size / sizeof(int64_t));
            ifstream small_in(partition_files[i], ios::binary);
            small_in.read(reinterpret_cast<char *>(small_data.data()), partition_size);
            small_in.close();
            io_operations++;

            sort_in_memory(small_data);

            string sorted_file = temp_dir + "sorted_" + to_string(depth) + "_" + to_string(i) + ".bin";
            ofstream small_out(sorted_file, ios::binary);
            small_out.write(reinterpret_cast<const char *>(small_data.data()), partition_size);
            small_out.close();
            io_operations++;

            sorted_partition_files.push_back(sorted_file);

            remove(partition_files[i].c_str());
            continue;
        }

        // Sort partition recursively
        int64_t next_arity = arity;
        if (depth >= 2) {
            next_arity = min(arity, max(int64_t(8), arity / 2));

            if (partition_size < file_size / 10) {
                next_arity = min(next_arity, int64_t(16));
            }
        }

        string sorted_file = temp_dir + "sorted_" + to_string(depth) + "_" + to_string(i) + ".bin";
        io_operations += recursive_external_quicksort(
            partition_files[i], sorted_file, next_arity, temp_dir, depth + 1
        );

        sorted_partition_files.push_back(sorted_file);

        if (partition_files[i] != sorted_file) {
            remove(partition_files[i].c_str());
        }
    }

    io_operations += concatenate_partitions(sorted_partition_files, output_file);

    for (const string &file : sorted_partition_files) {
        remove(file.c_str());
    }

    return io_operations;
}

/**
 * @brief Implements the External Quick Sort algorithm with configurable arity.
 * @param input_file Path of the input file to sort.
 * @param output_file Path of the sorted output file.
 * @param arity Partitioning arity (number of partitions to create).
 * @return Total number of I/O operations performed.
 */
int64_t external_quicksort(const string &input_file, const string &output_file, int64_t arity) {
    total_io_operations = 0;

    cout << "  Input file: " << input_file << endl;
    cout << "  Output file: " << output_file << endl;

    string timestamp = to_string(chrono::system_clock::now().time_since_epoch().count());
    string temp_dir = "temp_quick_" + to_string(arity) + "_" + timestamp + "/";
    create_directories(temp_dir);

    ifstream input(input_file, ios::binary | ios::ate);
    if (!input) {
        cerr << "Error opening input file: " << input_file << endl;
        exit(EXIT_FAILURE);
    }
    int64_t file_size = input.tellg();
    input.close();
    int64_t num_blocks = (file_size + BLOCK_SIZE - 1) / BLOCK_SIZE;

    cout << "  File size: " << file_size << " bytes (" << file_size / (1024 * 1024) << " MB)" << endl;
    cout << "  Num blocks to process: " << num_blocks << endl;
    cout << "  Memory available: " << EFFECTIVE_MEMORY / (1024 * 1024) << " MB (80% of "
         << TOTAL_MEMORY_RAM / (1024 * 1024) << " MB)" << endl;
    cout << "  Block size: " << BLOCK_SIZE << " bytes" << endl;
    cout << "  Arity: " << arity << endl;
    cout << "  Phase 1: Running External Quicksort..." << endl;

    auto start_time = chrono::high_resolution_clock::now();
    total_io_operations = recursive_external_quicksort(input_file, output_file, arity, temp_dir, 0);
    auto end_time = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(end_time - start_time).count();

    cout << "  Clean temporary files..." << endl;
    remove_directory(temp_dir);
    cout << "  Total I/O Operations: " << total_io_operations << endl;
    cout << "  Total time: " << duration / 1000.0 << " seconds" << endl;

    return total_io_operations;
}
