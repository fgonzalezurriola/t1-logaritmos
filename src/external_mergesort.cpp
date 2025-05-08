#include <algorithm>
#include <calculate_arity.h>
#include <chrono>
#include <external_mergesort.h>
#include <fstream>
#include <iostream>
#include <limits>
#include <queue>
#include <string>
#include <vector>

using namespace std;

// Using constants defined in calculate_arity.cpp
// These are not exported with extern, so we need to define them here
const int64_t BLOCK_SIZE = 4096;
const int64_t INTS_PER_BLOCK = BLOCK_SIZE / sizeof(int64_t);
const int64_t TOTAL_MEMORY_RAM = 50 * 1024 * 1024;

// Forward declarations for functions from calculate_arity.cpp that we'll use
vector<int64_t>
read_multiple_blocks(const string &filename, int64_t start_block, int64_t num_blocks_to_read);
void sort_in_memory(vector<int64_t> &data);
void create_directories(const string &dir);
void remove_directory(const string &dir);
void copy_file(const string &src, const string &dst);

/** k_way_merge
 * @brief Performs a k-way merge of multiple sorted files.
 * @param input_files Vector with paths of input files.
 * @param output_file Path of the merged output file.
 * @param arity The maximum number of files to merge at once.
 * @return Total number of I/O operations performed.
 */
int64_t k_way_merge(const vector<string> &input_files, const string &output_file, int64_t arity) {
    int64_t actual_arity = min((int64_t)(input_files.size()), arity);
    if (actual_arity == input_files.size()) {
        cout << "DEBUG: Using all input files for merging." << endl;
    }
    int64_t total_io_operations = 0;
    int64_t total_seeks = 0;
    const int64_t buffer_size_per_file = TOTAL_MEMORY_RAM / (actual_arity + 1);
    int64_t blocks_per_buffer = buffer_size_per_file / BLOCK_SIZE;
    if (blocks_per_buffer == 0)
        blocks_per_buffer = 1;
    const int64_t BLOCKS_PER_READ = blocks_per_buffer;

    /* K-way Merge Algorithm:
     * 1. Memory Management:
     *    - Divide available memory among input files and one output buffer
     *    - Each input file gets a buffer to hold blocks_per_buffer blocks
     *    - Larger arity means smaller buffers per file
     *
     * 2. Data Structures:
     *    - input_buffers: Array of buffers, one per input file
     *    - min_heap: Priority queue that keeps track of the smallest element across all files
     *      - Each heap node contains: value, file_index, block_index, element_index
     *    - output_buffer: Buffer for accumulating output before writing to disk
     *
     * 3. Algorithm Flow:
     *    - Initially fill buffers from each input file
     *    - Insert the first element from each buffer into the min heap
     *    - While the heap is not empty:
     *      a. Extract minimum element and add to output buffer
     *      b. When output buffer is full, write to disk
     *      c. Get the next element from the buffer that provided the min element
     *      d. If buffer is exhausted, refill it from the corresponding file
     *      e. Insert the new element into the heap
     *
     * 4. I/O Efficiency:
     *    - Reads and writes are performed in blocks to amortize I/O costs
     *    - Only required seeks are performed when a buffer is exhausted
     *    - Tracks total I/O operations for performance analysis
     */
    vector<vector<int64_t>> input_buffers(actual_arity);
    priority_queue<HeapNode, vector<HeapNode>, greater<HeapNode>> min_heap;
    vector<int64_t> output_buffer;
    output_buffer.reserve(blocks_per_buffer * INTS_PER_BLOCK);

    ofstream out_file(output_file, ios::binary);
    if (!out_file)
        exit(EXIT_FAILURE);

    // Initial filling of buffers and heap
    for (size_t i = 0; i < actual_arity; i++) {
        input_buffers[i] = read_multiple_blocks(input_files[i], 0, BLOCKS_PER_READ);
        total_io_operations += (input_buffers[i].size() + INTS_PER_BLOCK - 1) / INTS_PER_BLOCK;
        total_seeks++;
        if (!input_buffers[i].empty()) {
            min_heap.push({input_buffers[i][0], static_cast<int64_t>(i), 0, 0});
        }
    }

    // Main merge loop: extract minimum element, add to output, get next element
    while (!min_heap.empty()) {
        HeapNode min_node = min_heap.top();
        min_heap.pop();
        output_buffer.push_back(min_node.value);

        // If output buffer is full, write to disk
        if ((int64_t)output_buffer.size() == blocks_per_buffer * INTS_PER_BLOCK) {
            out_file.write(
                reinterpret_cast<const char *>(output_buffer.data()), blocks_per_buffer * BLOCK_SIZE
            );
            output_buffer.clear();
            total_io_operations += blocks_per_buffer;
        }

        min_node.element_index++;

        // If we've exhausted the current buffer, read more data from disk
        if (min_node.element_index >= static_cast<int64_t>(input_buffers[min_node.file_index].size())) {
            min_node.block_index += BLOCKS_PER_READ;
            min_node.element_index = 0;
            input_buffers[min_node.file_index] = read_multiple_blocks(
                input_files[min_node.file_index], min_node.block_index, BLOCKS_PER_READ
            );
            int64_t blocks_read =
                (input_buffers[min_node.file_index].size() + INTS_PER_BLOCK - 1) / INTS_PER_BLOCK;
            total_io_operations += blocks_read;
            if (blocks_read > 0)
                total_seeks++;
            if (!input_buffers[min_node.file_index].empty()) {
                min_node.value = input_buffers[min_node.file_index][0];
                min_heap.push(min_node);
            }
        } else {
            // Get next element from current buffer
            min_node.value = input_buffers[min_node.file_index][min_node.element_index];
            min_heap.push(min_node);
        }
    }

    // Write any remaining data in output buffer
    if (!output_buffer.empty()) {
        out_file.write(
            reinterpret_cast<const char *>(output_buffer.data()), output_buffer.size() * sizeof(int64_t)
        );
        total_io_operations += (output_buffer.size() + INTS_PER_BLOCK - 1) / INTS_PER_BLOCK;
    }

    out_file.close();
    return total_io_operations + total_seeks;
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

    int64_t num_blocks = (file_size + BLOCK_SIZE - 1) / BLOCK_SIZE;
    int64_t blocks_per_run = TOTAL_MEMORY_RAM / BLOCK_SIZE;
    if (blocks_per_run == 0)
        blocks_per_run = 1;
    int64_t estimated_runs = num_blocks / blocks_per_run;
    vector<string> run_files;

    cout << "  File size: " << file_size << " bytes" << endl;
    cout << "  Num blocks to process: " << num_blocks << endl;
    cout << "  Using " << blocks_per_run << " blocks per initial run" << endl;
    cout << "  Runs: " << estimated_runs << endl;
    cout << "  Phase 1: Sorting blocks in memory..." << endl;

    // This for:
    // Process the input file in chunks that fit into memory (blocks_per_run)
    // For each chunk:
    //  - Read multiple blocks sequentially into memory
    //  - Sort the entire chunk
    //  - Writes the chunk in a temp file
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
        total_io_operations += (large_block.size() + INTS_PER_BLOCK - 1) / INTS_PER_BLOCK;

        run_files.push_back(run_file);
    }

    cout << "  Generated " << run_files.size() << " run files." << endl;
    input.close();

    cout << "  Phase 2: Performing k-way merge..." << endl;

    // This while:
    // Merges runs in groups of 'arity' files
    // In each pass:
    //  - Process runs in groups of 'arity' files
    //  - For each group, merge all runs into a single sorted output file
    //  - After merging, delete the input run files to save disk space
    //  - The new merged runs become input for the next pass
    // 3. Continue until only one run file remains (the fully sorted file)
    // 4. The number of passes required is approximately log_arity(num_runs)
    // 5. The arity parameter controls the trade-off between:
    //    - Number of passes (fewer with higher arity)
    //    - I/O operations per pass (more with higher arity)
    //
    int64_t pass_number = 0;
    while (run_files.size() > 1) {
        pass_number++;
        vector<string> new_run_files;
        cout << "    Pass " << pass_number << ": Merging " << run_files.size() << " files with arity "
             << arity << endl;

        for (size_t i = 0; i < run_files.size(); i += arity) {
            vector<string> files_to_merge;
            for (size_t j = i; j < i + arity && j < run_files.size(); j++) {
                files_to_merge.push_back(run_files[j]);
            }
            if (files_to_merge.size() == 1) {
                string new_file = temp_dir + "pass_" + to_string(pass_number) + "_" +
                                  to_string(new_run_files.size()) + ".bin";
                copy_file(files_to_merge[0], new_file);
                new_run_files.push_back(new_file);
                if (files_to_merge[0] != new_file) {
                    remove(files_to_merge[0].c_str());
                }
            } else {
                string merged_file = temp_dir + "pass_" + to_string(pass_number) + "_" +
                                     to_string(new_run_files.size()) + ".bin";
                int64_t merge_io = k_way_merge(files_to_merge, merged_file, files_to_merge.size());
                total_io_operations += merge_io;
                new_run_files.push_back(merged_file);
                for (const string &file : files_to_merge) {
                    remove(file.c_str());
                }
            }
        }

        cout << "      Merged into " << new_run_files.size() << " files" << endl;
        run_files = new_run_files;
    }

    if (!run_files.empty()) {
        cout << "  Copying final file to output location..." << endl;
        copy_file(run_files[0], output_file);
        ifstream final_file(run_files[0], ios::binary | ios::ate);
        if (final_file) {
            int64_t size = final_file.tellg();
            total_io_operations += (size + BLOCK_SIZE - 1) / BLOCK_SIZE * 2;
            final_file.close();
        }
    }

    cout << "  Clean temporary files..." << endl;
    remove_directory(temp_dir);

    return total_io_operations;
}
