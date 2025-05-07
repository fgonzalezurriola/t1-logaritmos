#include <algorithm>
#include <calculate_arity.h>
#include <chrono>
#include <fstream>
#include <iostream>
#include <limits>
#include <map>
#include <queue>
#include <string>
#include <sys/stat.h>
#include <vector>

// Windows support
#ifdef _WIN32
#include <direct.h>
#define MKDIR(dir) _mkdir(dir)
#else
#define MKDIR(dir) mkdir(dir, 0755)
#endif

using namespace std;

// create_secuences crea el archivo de tamaño 60M = 3GB con los que se hace el test

/**
 * @BLOCK_SIZE: 4096 bytes. Size of a disk block.
 * @INTS_PER_BLOCK: 512. Number of 64-bit integers that fit in a block.
 * @MAX_INITIAL_RUNS: 1000. Maximum number of initial runs.
 * @TOTAL_MEMORY_RAM: 50MB. Memory available for in-memory sorting.
 * @files_arity: Directory where sequence files are located.
 * @results_file: File where experiment results will be written.
 */
const int64_t BLOCK_SIZE = 4096;
const int64_t INTS_PER_BLOCK = BLOCK_SIZE / sizeof(int64_t); // 512
const int64_t MAX_INITIAL_RUNS = 1000;                       // Todo: revisar
const int64_t TOTAL_MEMORY_RAM =
    500 * 1024 *
    1024; // docker run --rm -it -m 500m -v "$PWD":/workspace pabloskewes/cc4102-cpp-env bash
const string files_arity = "dist/arity_exp/";
const string results_file = "results/arity_results.txt";

/** sort_in_memory
 * @brief Sorts an integer vector in memory using the standard sort algorithm.
 * @param data Vector of integers to sort.
 */
void sort_in_memory(vector<int64_t> &data) {
    sort(data.begin(), data.end());
}

/** create_directories
 * @brief Recursively creates directories in the specified path.
 * @param dir Path of the directory to create.
 */
void create_directories(const string &dir) {
    size_t pos = 0;
    string path;
    while ((pos = dir.find('/', pos)) != string::npos) {
        path = dir.substr(0, pos++);
        if (path.length() > 0) {
            MKDIR(path.c_str());
        }
    }
    MKDIR(dir.c_str());
}

/** remove_directory
 * @brief Recursively removes a directory and its contents.
 * @param dir Path of the directory to remove.
 */
void remove_directory(const string &dir) {
    string cmd = "rm -rf " + dir;
    int result = system(cmd.c_str());
    if (result != 0) {
        cerr << "Error removing directory: " << dir << endl;
    }
}

/** copy_file
 * @brief Copies a file from source to destination.
 * @param src Path of the source file.
 * @param dst Path of the destination file.
 */
void copy_file(const string &src, const string &dst) {
    ifstream source(src, ios::binary);
    ofstream dest(dst, ios::binary);
    dest << source.rdbuf();
    source.close();
    dest.close();
}

/** write_block
 * @brief Writes a block of data to a binary file.
 * @param filename Name of the file to write to.
 * @param block_index Index of the block to write.
 * @param buffer Vector of data to write.
 */
void write_block(const string &filename, int64_t block_index, const vector<int64_t> &buffer) {
    fstream out(filename, ios::in | ios::out | ios::binary);
    if (!out) {
        ofstream create(filename, ios::binary);
        create.close();
        out.open(filename, ios::in | ios::out | ios::binary);
        if (!out) {
            cerr << "Error opening file " << filename << endl;
            exit(EXIT_FAILURE);
        }
    }
    out.seekp(block_index * BLOCK_SIZE);
    out.write(reinterpret_cast<const char *>(buffer.data()), buffer.size() * sizeof(int64_t));
    if (!out) {
        cerr << "Error writing to block in file " << filename << endl;
    }
    out.close();
}

/** read_multiple_blocks
 * @brief Reads multiple blocks from a binary file.
 * @param filename Name of the file to read.
 * @param start_block Index of the first block to read.
 * @param num_blocks_to_read Number of blocks to read.
 * @return Vector with integers read from the blocks.
 */
vector<int64_t>
read_multiple_blocks(const string &filename, int64_t start_block, int64_t num_blocks_to_read) {
    vector<int64_t> buffer;
    buffer.reserve(num_blocks_to_read * INTS_PER_BLOCK);

    ifstream in(filename, ios::binary);
    if (!in) {
        cerr << "Error opening file " << filename << endl;
        exit(EXIT_FAILURE);
    }

    in.seekg(start_block * BLOCK_SIZE);

    for (int64_t i = 0; i < num_blocks_to_read; i++) {
        vector<int64_t> temp_buffer(INTS_PER_BLOCK);
        in.read(reinterpret_cast<char *>(temp_buffer.data()), BLOCK_SIZE);

        if (in.gcount() == 0) {
            break;
        }

        if (in.gcount() != BLOCK_SIZE) {
            temp_buffer.resize(in.gcount() / sizeof(int64_t));
        }

        buffer.insert(buffer.end(), temp_buffer.begin(), temp_buffer.end());

        if (in.gcount() != BLOCK_SIZE) {
            break;
        }
    }

    in.close();
    return buffer;
}

/**
 * @brief Structure for nodes used in the heap during k-way merge.
 * @details Stores a value, file index, block index, and position in the block.
 */
struct HeapNode {
    int64_t value;
    int64_t file_index;
    int64_t block_index;
    int64_t element_index;

    bool operator>(const HeapNode &other) const {
        return value > other.value;
    }
};

/** k_way_merge
 * @brief Performs a k-way merge of multiple sorted files.
 * @param input_files Vector with paths of input files.
 * @param output_file Path of the merged output file.
 * @return Total number of I/O operations performed.
 */
int64_t k_way_merge(const vector<string> &input_files, const string &output_file) {
    int64_t total_io_operations = 0;

    // Calculate memory limit per file
    const int64_t TOTAL_MEMORY_LIMIT = 500 * 1024 * 1024;
    const int64_t MAX_BLOCKS_PER_FILE = (TOTAL_MEMORY_LIMIT / input_files.size()) / BLOCK_SIZE;
    const int64_t BLOCKS_PER_READ = max((int64_t)1, min((int64_t)50, MAX_BLOCKS_PER_FILE));

    cout << "  K-way merge: Reading " << BLOCKS_PER_READ << " blocks at once per file" << endl;

    ofstream out_file(output_file, ios::binary);
    if (!out_file) {
        cerr << "Error opening output file: " << output_file << endl;
        exit(EXIT_FAILURE);
    }

    vector<vector<int64_t>> input_buffers(input_files.size());
    priority_queue<HeapNode, vector<HeapNode>, greater<HeapNode>> min_heap;

    // Load the first set of blocks for each file
    for (size_t i = 0; i < input_files.size(); i++) {
        input_buffers[i] = read_multiple_blocks(input_files[i], 0, BLOCKS_PER_READ);

        // Count as one I/O operation per block we tried to read
        // total_io_operations +=
        //     BLOCKS_PER_READ > 0
        //         ? min(BLOCKS_PER_READ,
        //               (int64_t)((input_buffers[i].size() + INTS_PER_BLOCK - 1) / INTS_PER_BLOCK))
        //         : 0;
        total_io_operations += input_buffers[i].size() / INTS_PER_BLOCK;

        if (!input_buffers[i].empty()) {
            min_heap.push({input_buffers[i][0], static_cast<int64_t>(i), 0, 0});
        }
    }

    // Output buffer
    vector<int64_t> output_buffer;
    output_buffer.reserve(INTS_PER_BLOCK);

    while (!min_heap.empty()) {
        HeapNode min_node = min_heap.top();
        min_heap.pop();

        // Add the minimum element to the output buffer
        output_buffer.push_back(min_node.value);

        // If output buffer is full, write it to disk
        if (output_buffer.size() == INTS_PER_BLOCK) {
            out_file.write(reinterpret_cast<const char *>(output_buffer.data()), BLOCK_SIZE);
            output_buffer.clear();
            total_io_operations++;
        }

        // Move to the next element in the current buffer
        min_node.element_index++;

        // If we've exhausted the current buffer
        if (min_node.element_index >= static_cast<int64_t>(input_buffers[min_node.file_index].size())) {
            // Load more blocks from the file
            min_node.block_index += BLOCKS_PER_READ;
            min_node.element_index = 0;

            input_buffers[min_node.file_index] = read_multiple_blocks(
                input_files[min_node.file_index], min_node.block_index, BLOCKS_PER_READ
            );

            // Count I/O for each block read
            int64_t blocks_read =
                (input_buffers[min_node.file_index].size() + INTS_PER_BLOCK - 1) / INTS_PER_BLOCK;

            total_io_operations += blocks_read > 0 ? blocks_read : 0;

            if (!input_buffers[min_node.file_index].empty()) {
                min_node.value = input_buffers[min_node.file_index][0];
                min_heap.push(min_node);
            }
        } else {
            // Move to the next element in the current buffer
            min_node.value = input_buffers[min_node.file_index][min_node.element_index];
            min_heap.push(min_node);
        }
    }

    // Write any remaining data in the output buffer
    if (!output_buffer.empty()) {
        out_file.write(
            reinterpret_cast<const char *>(output_buffer.data()), output_buffer.size() * sizeof(int64_t)
        );
        total_io_operations++;
    }

    out_file.close();
    return total_io_operations;
}

/** external_mergesort
 * @brief Implements the External Merge Sort algorithm with configurable arity.
 * @param input_file Path of the input file to sort.
 * @param output_file Path of the sorted output file.
 * @param arity Merge arity (number of files to merge simultaneously).
 * @return Total number of I/O operations performed.
 */
int64_t external_mergesort(const string &input_file, const string &output_file, int64_t arity) {
    int64_t total_io_operations = 0;

    cout << "  Input file: " << input_file << endl;
    cout << "  Output file: " << output_file << endl;

    string temp_dir = "temp_merge_" + to_string(arity) + "/";
    create_directories(temp_dir);

    ifstream input(input_file, ios::binary | ios::ate);
    if (!input) {
        cerr << "Error opening input file: " << input_file << endl;
        exit(EXIT_FAILURE);
    }

    int64_t file_size = input.tellg();
    input.seekg(0);

    // Todo: Revisar
    int64_t num_blocks = (file_size + BLOCK_SIZE - 1) / BLOCK_SIZE;
    cout << "  File size: " << file_size << " bytes" << endl;
    cout << "  Num blocks to process: " << num_blocks << endl;

    // Todo: SIMPLIFICAR
    int64_t blocks_per_run = 1;
    if (num_blocks > MAX_INITIAL_RUNS) {
        blocks_per_run = (num_blocks + MAX_INITIAL_RUNS - 1) / MAX_INITIAL_RUNS;
        cout << "  Using " << blocks_per_run << " blocks per initial run" << endl;
    }

    int64_t run_size = blocks_per_run * BLOCK_SIZE;
    // Todo: simplificar
    int64_t estimated_runs = (file_size + run_size - 1) / run_size;
    cout << "  Estimated initial runs: " << estimated_runs << endl;

    vector<string> run_files;

    cout << "  Phase 1: Sorting blocks in memory..." << endl;

    for (int64_t i = 0; i < num_blocks; i += blocks_per_run) {
        int64_t blocks_to_read = min(blocks_per_run, num_blocks - i);
        vector<int64_t> large_block;
        large_block.reserve(blocks_to_read * INTS_PER_BLOCK);

        for (int64_t j = 0; j < blocks_to_read; j++) {
            vector<int64_t> block(INTS_PER_BLOCK);
            input.read(reinterpret_cast<char *>(block.data()), BLOCK_SIZE);
            int64_t elements_read = input.gcount() / sizeof(int64_t);
            if (elements_read > 0) {
                block.resize(elements_read);
                large_block.insert(large_block.end(), block.begin(), block.end());
            }
            total_io_operations++;
        }

        sort_in_memory(large_block);

        string run_file = temp_dir + "run_" + to_string(run_files.size()) + ".bin";
        ofstream run_out(run_file, ios::binary);
        run_out.write(
            reinterpret_cast<const char *>(large_block.data()), large_block.size() * sizeof(int64_t)
        );
        run_out.close();
        total_io_operations++;

        run_files.push_back(run_file);
    }

    cout << "  Phase 1 completed. Generated " << run_files.size() << " run files." << endl;
    input.close();

    cout << "  Phase 2: Performing k-way merge..." << endl;

    while (run_files.size() > 1) {
        vector<string> new_run_files;
        int64_t num_groups = (run_files.size() + arity - 1) / arity;

        for (size_t i = 0; i < run_files.size(); i += arity) {
            if (i % (arity * 5) == 0 || i == 0) {
                int64_t current_group = i / arity + 1;
                cout << "      Processing group " << current_group << " of " << num_groups << " ("
                     << (current_group * 100 / num_groups) << "%)" << endl;
            }

            vector<string> merge_files;
            size_t files_in_group = 0;

            for (size_t j = i; j < i + arity && j < run_files.size(); j++) {
                merge_files.push_back(run_files[j]);
                files_in_group++;
            }

            if (files_in_group > 1) {
                string merged_file = temp_dir + "merged_" + to_string(new_run_files.size()) + ".bin";
                total_io_operations += k_way_merge(merge_files, merged_file);
                new_run_files.push_back(merged_file);

                for (const string &file : merge_files) {
                    remove(file.c_str());
                }
            } else if (files_in_group == 1) {
                new_run_files.push_back(merge_files[0]);
            }
        }

        cout << "      Merged into " << new_run_files.size() << " files" << endl;
        run_files = new_run_files;
    }

    if (!run_files.empty()) {
        cout << "  Copying final file to output location..." << endl;
        copy_file(run_files[0], output_file);
    }

    cout << "  Clean temporary files..." << endl;
    remove_directory(temp_dir);

    return total_io_operations;
}

/** ternary_search_optimal_arity
 * @brief Performs a ternary search to find the optimal arity.
 * @param left Lower limit of the search range.
 * @param right Upper limit of the search range.
 * @return The optimal arity found.
 */
int64_t ternary_search_optimal_arity(int64_t left, int64_t right) {
    cout << "\n=========================================================" << endl;
    cout << "Starting ternary search for optimal arity in range [" << left << ", " << right << "]"
         << endl;
    cout << "=========================================================" << endl;

    create_directories("dist/arity_exp");
    create_directories("results");
    string input_file = "dist/m_60/secuence_1.bin";

    ofstream results_out(results_file);
    results_out << "Arity,I/O's" << endl;

    while (right - left > 4) {
        cout << "Current search interval: [" << left << ", " << right << "]" << endl;

        int64_t m1 = left + (right - left) / 3;
        int64_t m2 = right - (right - left) / 3;

        if (m1 >= m2) {
            m1 = left + (right - left) / 2 - 1;
            m2 = left + (right - left) / 2 + 1;
        }

        cout << "\n  Testing arity: " << m1 << endl;
        string output_file_m1 = "dist/arity_exp/sorted_" + to_string(m1) + ".bin";
        int64_t io_m1 = external_mergesort(input_file, output_file_m1, m1);
        cout << "  I/O Operations for arity " << m1 << ": " << io_m1 << endl;
        results_out << m1 << "," << io_m1 << endl;

        cout << "\n  Testing arity: " << m2 << endl;
        string output_file_m2 = "dist/arity_exp/sorted_" + to_string(m2) + ".bin";
        int64_t io_m2 = external_mergesort(input_file, output_file_m2, m2);
        cout << "  I/O Operations for arity " << m2 << ": " << io_m2 << endl;
        results_out << m2 << "," << io_m2 << endl;

        if (io_m1 < io_m2) {
            // Optimal value is in the left part [left, m2]
            right = m2;
            cout << "I/O(" << m1 << ")=" << io_m1 << " < I/O(" << m2 << ")=" << io_m2
                 << ", updating range to [" << left << ", " << right << "]" << endl;
        } else if (io_m2 < io_m1) {
            // Optimal value is in the right part [m1, right]
            left = m1;
            cout << "I/O(" << m2 << ")=" << io_m2 << " < I/O(" << m1 << ")=" << io_m1
                 << ", updating range to [" << left << ", " << right << "]" << endl;
        } else {
            // Equal I/Os, reduce to [m1, m2]
            left = m1;
            right = m2;
            cout << "I/O(" << m1 << ")=" << io_m1 << " = I/O(" << m2 << ")=" << io_m2
                 << ", updating range to [" << left << ", " << right << "]" << endl;
        }
    }

    // Final linear search within the small remaining interval
    cout << "\n=========================================================" << endl;
    cout << "Final linear search in range [" << left << ", " << right << "]" << endl;
    cout << "=========================================================" << endl;

    int64_t optimal_arity = left;
    int64_t min_io = numeric_limits<int64_t>::max();

    for (int64_t arity = left; arity <= right; arity++) {
        string output_file = "dist/arity_exp/sorted_" + to_string(arity) + ".bin";
        int64_t io_operations = external_mergesort(input_file, output_file, arity);
        cout << "  I/O Operations for arity " << arity << ": " << io_operations << endl;
        results_out << arity << "," << io_operations << endl;
        if (io_operations < min_io) {
            min_io = io_operations;
            optimal_arity = arity;
        }
    }

    results_out.close();

    return optimal_arity;
}

/** run_arity_experiment
 * @brief Runs the experiment to find the optimal arity in a specified range.
 * @param min_arity Minimum arity to test.
 * @param max_arity Maximum arity to test.
 */
void run_arity_experiment(int64_t min_arity, int64_t max_arity) {
    cout << "\n=========================================================" << endl;
    cout << "Starting arity experiment with range [" << min_arity << ", " << max_arity << "]" << endl;
    cout << "Current time: " << chrono::system_clock::now().time_since_epoch().count() << endl;
    cout << "=========================================================" << endl;

    int64_t optimal_arity = ternary_search_optimal_arity(min_arity, max_arity);

    cout << "\n=========================================================" << endl;
    cout << "Optimal arity found: " << optimal_arity << endl;
    cout << "Results saved to " << results_file << endl;
    cout << "=========================================================" << endl;
}

/**
 * @brief Main function of the program.
 * @return Program exit code.
 */
int main() {
    int64_t min_arity = 2;
    int64_t max_arity = 512;

    cout << "Running arity experiment with range [" << min_arity << ", " << max_arity << "]" << endl;

    run_arity_experiment(min_arity, max_arity);

    return 0;
}
