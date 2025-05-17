#include <algorithm>
#include <calculate_arity.h>
#include <chrono>
#include <external_quicksort.h>
#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include <sys/stat.h>
#include <vector>

using namespace std;

const int64_t BLOCK_SIZE = 4096;
const int64_t INTS_PER_BLOCK = BLOCK_SIZE / sizeof(int64_t);
const int64_t TOTAL_MEMORY_RAM = 40 * 1024 * 1024;
const int64_t CONCAT_BUFFER_SIZE = 256 * 1024;

int64_t total_io_operations = 0;

/**
 * @brief List of vector methods used and their purpose:
 *
 * - reserve(n): Pre-allocates space for n elements to avoid reallocations.
 * - push_back(x): Adds x to the end of the vector (used to fill buffers).
 * - clear(): Empties the vector but keeps the allocated memory.
 * - swap(vec): Frees the memory by swapping with an empty vector.
 */

vector<int64_t>
read_multiple_blocks(const string &filename, int64_t start_block, int64_t num_blocks_to_read);
void sort_in_memory(vector<int64_t> &data);
void create_directories(const string &dir);
void remove_directory(const string &dir);
void copy_file(const string &src, const string &dst);

/**
 * @brief Simplified pivot selection optimized for aridades entre 20-70
 * @param input_file Path to the input binary file
 * @param arity Number of partitions to create (a-way partitioning)
 * @return Vector of arity-1 sorted pivot values
 */
vector<int64_t> select_pivots(const string &input_file, int64_t arity) {
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

    int64_t sample_blocks = min(int64_t(10), num_blocks);

    vector<int64_t> samples;
    samples.reserve(sample_blocks * INTS_PER_BLOCK / 2);

    vector<int64_t> positions;
    positions.reserve(sample_blocks);
    positions.push_back(0);
    if (num_blocks > 1)
        positions.push_back(num_blocks - 1);

    for (int i = 1; i < sample_blocks - 1; i++) {
        positions.push_back((i * num_blocks) / sample_blocks);
    }

    std::sort(positions.begin(), positions.end());
    auto last = std::unique(positions.begin(), positions.end());
    positions.erase(last, positions.end());

    for (int64_t block_idx : positions) {
        vector<int64_t> block = read_multiple_blocks(input_file, block_idx, 1);
        total_io_operations++;

        for (size_t j = 0; j < block.size(); j += 2) {
            samples.push_back(block[j]);
        }
    }

    std::sort(samples.begin(), samples.end());

    vector<int64_t> pivots;
    pivots.reserve(arity - 1);

    for (int64_t i = 1; i < arity; i++) {
        int64_t idx = ((int64_t)samples.size() * i) / arity;
        if (idx < (int64_t)samples.size()) {
            pivots.push_back(samples[idx]);
        }
    }

    vector<int64_t>().swap(samples);

    return pivots;
}

/**
 * @brief Simple concatenation of partition files
 * @param sorted_files Vector of paths to sorted files
 * @param output_file Path to the output file
 * @return Number of I/O operations performed
 */
int64_t
concatenate_partitions(const vector<string> &sorted_files, const string &output_file, int64_t arity) {
    int64_t io_count = 0;
    ofstream output(output_file, ios::binary);

    if (!output) {
        cerr << "Error opening output file: " << output_file << endl;
        exit(EXIT_FAILURE);
    }

    // !
    int64_t buffer_size = CONCAT_BUFFER_SIZE;
    if (arity >= 32) {
        buffer_size = CONCAT_BUFFER_SIZE / 2;
    }
    if (arity >= 64) {
        buffer_size = CONCAT_BUFFER_SIZE / 4;
    }

    vector<char> buffer(CONCAT_BUFFER_SIZE);

    // This for:
    // Iterates over each sorted partition file.
    // For each file:
    //  - Opens and reads it in blocks.
    //  - Writes each block to the final output file.
    //  - Counts each read and write as an I/O operation.
    for (const string &file : sorted_files) {
        ifstream input(file, ios::binary);
        if (!input)
            continue;

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

    ifstream in_size(input_file, ios::binary | ios::ate);
    if (!in_size) {
        cerr << "Error opening input file: " << input_file << endl;
        exit(EXIT_FAILURE);
    }
    int64_t file_size = in_size.tellg();
    in_size.close();
    io_operations++;

    if (file_size == 0) {
        ofstream(output_file, ios::binary).close();
        return io_operations;
    }

    if (file_size <= TOTAL_MEMORY_RAM / 2) {
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

        vector<int64_t>().swap(data);

        return io_operations;
    }

    vector<int64_t> pivots = select_pivots(input_file, arity);
    io_operations += 2;
    const int64_t READ_BUFFER_BYTES = TOTAL_MEMORY_RAM * 0.2;
    const int64_t PARTITION_BUFFER_BYTES = (TOTAL_MEMORY_RAM * 0.7) / arity;

    const int64_t MIN_PARTITION_BUFFER = BLOCK_SIZE;
    const int64_t PARTITION_BUFFER_SIZE =
        max(PARTITION_BUFFER_BYTES, MIN_PARTITION_BUFFER) / sizeof(int64_t);
    const int64_t READ_BUFFER_SIZE = READ_BUFFER_BYTES / sizeof(int64_t);

    vector<vector<int64_t>> partition_buffers(arity);
    for (int64_t i = 0; i < arity; i++) {
        partition_buffers[i].reserve(PARTITION_BUFFER_SIZE);
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

    // This while:
    // Reads the input file in fixed-size blocks (READ_BUFFER_BYTES).
    // For each block read:
    //  - For each element in the block:
    //    - Determines which partition it belongs to using the pivots.
    //    - Adds the element to the corresponding partition buffer (push_back).
    //    - If the partition buffer is full, writes it to disk and clears it.
    // Continues until the entire file has been read and partitioned.
    while (input) {
        input.read(reinterpret_cast<char *>(read_buffer.data()), READ_BUFFER_BYTES);
        int64_t elems_read = input.gcount() / sizeof(int64_t);

        if (elems_read == 0) {
            break;
        }

        io_operations++;

        // This for:
        // Iterates over all elements read from the current block.
        // For each element:
        //  - Determines the partition using binary search on the pivots.
        //  - Adds the element to the corresponding partition buffer.
        //  - If the buffer is full, writes it to disk and clears it.
        for (int64_t i = 0; i < elems_read; i++) {
            int64_t val = read_buffer[i];

            int64_t partition_idx = 0;
            if (!pivots.empty()) {
                auto it = upper_bound(pivots.begin(), pivots.end(), val);
                partition_idx = distance(pivots.begin(), it);
            }

            partition_buffers[partition_idx].push_back(val);
            total_partition_elements[partition_idx]++;

            if ((int64_t)partition_buffers[partition_idx].size() >= PARTITION_BUFFER_SIZE) {
                partition_streams[partition_idx].write(
                    reinterpret_cast<const char *>(partition_buffers[partition_idx].data()),
                    partition_buffers[partition_idx].size() * sizeof(int64_t)
                );
                io_operations++;
                partition_buffers[partition_idx].clear();
                partition_buffers[partition_idx].reserve(PARTITION_BUFFER_SIZE);
            }
        }
    }

    input.close();

    for (int64_t i = 0; i < arity; i++) {
        if (!partition_buffers[i].empty()) {
            partition_streams[i].write(
                reinterpret_cast<const char *>(partition_buffers[i].data()),
                partition_buffers[i].size() * sizeof(int64_t)
            );
            io_operations++;
        }
        partition_streams[i].close();

        vector<int64_t>().swap(partition_buffers[i]);
    }

    vector<int64_t>().swap(pivots);
    vector<int64_t>().swap(read_buffer);

    vector<string> sorted_partition_files;
    sorted_partition_files.reserve(arity);

    // This for:
    // Iterates over each generated partition file.
    // For each partition:
    //  - If it is empty, skips.
    //  - If it is very small, sorts it in memory and write.
    //  - If it is large, recursively calls quicksort.
    //  - Removes temporary files after processing.
    for (int64_t i = 0; i < arity; i++) {
        ifstream file(partition_files[i], ios::binary | ios::ate);
        int64_t partition_size = 0;
        if (file) {
            partition_size = file.tellg();
            file.close();
        }

        if (partition_size <= 0) {
            sorted_partition_files.push_back(partition_files[i]);
            continue;
        }

        if (partition_size <= BLOCK_SIZE * 2) {
            vector<int64_t> sdata(partition_size / sizeof(int64_t));
            ifstream s_file_in(partition_files[i], ios::binary);
            s_file_in.read(reinterpret_cast<char *>(sdata.data()), partition_size);
            s_file_in.close();
            io_operations++;
            sort_in_memory(sdata);
            string sorted_file = temp_dir + "sorted_" + to_string(depth) + "_" + to_string(i) + ".bin";
            ofstream s_file_out(sorted_file, ios::binary);
            s_file_out.write(reinterpret_cast<const char *>(sdata.data()), partition_size);
            s_file_out.close();
            io_operations++;
            sorted_partition_files.push_back(sorted_file);
            remove(partition_files[i].c_str());
            continue;
        }

        string sorted_file = temp_dir + "sorted_" + to_string(depth) + "_" + to_string(i) + ".bin";
        io_operations +=
            recursive_external_quicksort(partition_files[i], sorted_file, arity, temp_dir, depth + 1);

        sorted_partition_files.push_back(sorted_file);
        if (partition_files[i] != sorted_file) {
            remove(partition_files[i].c_str());
        }
    }

    io_operations += concatenate_partitions(sorted_partition_files, output_file, arity);

    for (const string &file : sorted_partition_files) {
        remove(file.c_str());
    }

    return io_operations;
}

/**
 * @brief Implementa el algoritmo External Quick Sort con aridad configurable
 * @param input_file Path del archivo de entrada a ordenar
 * @param output_file Path del archivo de salida ordenado
 * @param arity Aridad de particionamiento (número de particiones)
 * @return Número total de operaciones de E/S realizadas
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
    cout << "  Memory available: " << TOTAL_MEMORY_RAM / (1024 * 1024) << " MB" << endl;
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
