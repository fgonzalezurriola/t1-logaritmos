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

using namespace std;

const int64_t BLOCK_SIZE = 4096;
const int64_t INTS_PER_BLOCK = BLOCK_SIZE / sizeof(int64_t);
const int64_t BLOCKS_PER_M = 12208; // 12207 + 1
const int64_t MAX_INITIAL_RUNS = 1000;

void create_directory(const string &dir) {
    mkdir(dir.c_str(), 0755);
}

void sort_in_memory(vector<int64_t> &data) {
    sort(data.begin(), data.end());
}

void create_directories(const string &dir) {
    size_t pos = 0;
    string path;
    while ((pos = dir.find('/', pos)) != string::npos) {
        path = dir.substr(0, pos++);
        if (path.length() > 0) {
            mkdir(path.c_str(), 0755);
        }
    }
    mkdir(dir.c_str(), 0755);
}

void copy_file(const string &src, const string &dst) {
    ifstream source(src, ios::binary);
    ofstream dest(dst, ios::binary);
    dest << source.rdbuf();
    source.close();
    dest.close();
}

void remove_directory(const string &dir) {
    string cmd = "rm -rf " + dir;
    int result = system(cmd.c_str());
    if (result != 0) {
        cerr << "Error removing directory: " << dir << endl;
    }
}

const string files_arity = "dist/arity_exp/";
const string results_file = "results/arity_results.txt";

struct HeapNode {
    int64_t value;
    int64_t file_index;
    int64_t block_index;
    int64_t element_index;

    bool operator>(const HeapNode &other) const {
        return value > other.value;
    }
};

vector<int64_t> read_block(const string &filename, int64_t block_index) {
    vector<int64_t> buffer(INTS_PER_BLOCK);
    ifstream in(filename, ios::binary);
    if (!in) {
        cerr << "Error opening file " << filename << endl;
        exit(EXIT_FAILURE);
    }
    in.seekg(block_index * BLOCK_SIZE);
    in.read(reinterpret_cast<char *>(buffer.data()), BLOCK_SIZE);

    // If EOF is reached, adjust vector size without printing message
    if (in.gcount() != BLOCK_SIZE) {
        buffer.resize(in.gcount() / sizeof(int64_t));
    }

    return buffer;
}

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

// Function to read multiple blocks from a file into memory
vector<int64_t>
read_multiple_blocks(const string &filename, int64_t start_block, int64_t num_blocks_to_read) {
    vector<int64_t> buffer;
    buffer.reserve(num_blocks_to_read * INTS_PER_BLOCK); // Pre-allocate space

    ifstream in(filename, ios::binary);
    if (!in) {
        cerr << "Error opening file " << filename << endl;
        exit(EXIT_FAILURE);
    }

    in.seekg(start_block * BLOCK_SIZE);

    for (int64_t i = 0; i < num_blocks_to_read; i++) {
        vector<int64_t> temp_buffer(INTS_PER_BLOCK);
        in.read(reinterpret_cast<char *>(temp_buffer.data()), BLOCK_SIZE);

        // If we reach end of file or read less than a complete block
        if (in.gcount() == 0) {
            break; // Nothing was read, we've reached the end of file
        }

        // Adjust size if we read a partial block
        if (in.gcount() != BLOCK_SIZE) {
            temp_buffer.resize(in.gcount() / sizeof(int64_t));
        }

        // Add the block to the main buffer
        buffer.insert(buffer.end(), temp_buffer.begin(), temp_buffer.end());

        // If we read less than a full block, we've reached the end
        if (in.gcount() != BLOCK_SIZE) {
            break;
        }
    }

    in.close();
    return buffer;
}

int64_t k_way_merge(const vector<string> &input_files, const string &output_file) {
    int64_t total_io_operations = 0;

    // Determine how much memory we can use per file
    // Assuming we want to use approximately 50MB total (adjust as needed)
    const int64_t TOTAL_MEMORY_LIMIT = 50 * 1024 * 1024; // 50MB in bytes
    const int64_t MAX_BLOCKS_PER_FILE = (TOTAL_MEMORY_LIMIT / input_files.size()) / BLOCK_SIZE;
    const int64_t BLOCKS_PER_READ =
        max((int64_t)1, min((int64_t)50, MAX_BLOCKS_PER_FILE)); // Between 1 and 50 blocks per read

    cout << "  K-way merge: Reading " << BLOCKS_PER_READ << " blocks at once per file" << endl;

    ofstream out_file(output_file, ios::binary);
    if (!out_file) {
        cerr << "Error opening output file: " << output_file << endl;
        exit(EXIT_FAILURE);
    }

    // For each input file: buffer, current position in buffer, and current block
    vector<vector<int64_t>> input_buffers(input_files.size());
    vector<int64_t> current_positions(input_files.size(), 0);
    vector<int64_t> current_block_indices(input_files.size(), 0);

    priority_queue<HeapNode, vector<HeapNode>, greater<HeapNode>> min_heap;

    // Load the first set of blocks for each file
    for (size_t i = 0; i < input_files.size(); ++i) {
        input_buffers[i] = read_multiple_blocks(input_files[i], 0, BLOCKS_PER_READ);
        // Count as one I/O operation per block we tried to read
        total_io_operations +=
            BLOCKS_PER_READ > 0
                ? min(BLOCKS_PER_READ,
                      (int64_t)((input_buffers[i].size() + INTS_PER_BLOCK - 1) / INTS_PER_BLOCK))
                : 0;

        if (!input_buffers[i].empty()) {
            min_heap.push({input_buffers[i][0], static_cast<int64_t>(i), 0, 0});
        }
    }

    // Buffer for output
    vector<int64_t> output_buffer;
    output_buffer.reserve(INTS_PER_BLOCK);
    int64_t output_block_index = 0;

    while (!min_heap.empty()) {
        HeapNode min_node = min_heap.top();
        min_heap.pop();

        // Add the minimum element to the output buffer
        output_buffer.push_back(min_node.value);

        // If the output buffer is full, write it to disk
        if (output_buffer.size() == INTS_PER_BLOCK) {
            out_file.write(reinterpret_cast<const char *>(output_buffer.data()), BLOCK_SIZE);
            output_buffer.clear();
            output_block_index++;
            total_io_operations++;
        }

        // Advance to the next element in the current buffer
        min_node.element_index++;

        // If we've exhausted the current buffer
        if (min_node.element_index >=
            static_cast<int64_t>(input_buffers[min_node.file_index].size())) {
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
            // Advance to the next element in the current buffer
            min_node.value = input_buffers[min_node.file_index][min_node.element_index];
            min_heap.push(min_node);
        }
    }

    // Write any remaining data in the output buffer
    if (!output_buffer.empty()) {
        out_file.write(
            reinterpret_cast<const char *>(output_buffer.data()),
            output_buffer.size() * sizeof(int64_t)
        );
        total_io_operations++;
    }

    out_file.close();
    return total_io_operations;
}

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
    cout << "  File size: " << file_size << " bytes" << endl;
    cout << "  Num blocks to process: " << num_blocks << endl;

    int64_t blocks_per_run = 1;
    if (num_blocks > MAX_INITIAL_RUNS) {
        blocks_per_run = (num_blocks + MAX_INITIAL_RUNS - 1) / MAX_INITIAL_RUNS;
        cout << "  Optimizing: Using " << blocks_per_run << " blocks per initial run" << endl;
    }

    int64_t run_size = blocks_per_run * BLOCK_SIZE;
    int64_t estimated_runs = (file_size + run_size - 1) / run_size;
    cout << "  Estimated initial runs: " << estimated_runs << endl;

    vector<string> run_files;

    cout << "  Phase 1: Sorting blocks in memory..." << endl;
    int64_t last_percentage = -1;

    for (int64_t i = 0; i < num_blocks; i += blocks_per_run) {
        int64_t current_percentage = (i * 100) / num_blocks;
        if (current_percentage % 5 == 0 && current_percentage != last_percentage) {
            cout << "    Progress: " << current_percentage << "% (" << i << "/" << num_blocks
                 << " blocks)" << endl;
            last_percentage = current_percentage;
        }

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
    int merge_iteration = 0;

    while (run_files.size() > 1) {
        cout << "    Merge iteration " << merge_iteration++ << ": merging " << run_files.size()
             << " files" << endl;

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

            for (size_t j = i; j < i + arity && j < run_files.size(); ++j) {
                merge_files.push_back(run_files[j]);
                files_in_group++;
            }

            if (files_in_group > 1) {
                string merged_file =
                    temp_dir + "merged_" + to_string(new_run_files.size()) + ".bin";
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
        remove(run_files[0].c_str());
    }

    cout << "  Clean temporary files..." << endl;
    remove_directory(temp_dir);

    return total_io_operations;
}

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

    // Map to store already computed values to avoid redundant calculations
    map<int64_t, int64_t> memoized_results;

    // Function to evaluate I/O operations for a given arity
    auto evaluate_arity = [&](int64_t arity) -> int64_t {
        // Check if we've already computed this value
        if (memoized_results.find(arity) != memoized_results.end()) {
            cout << "  Using cached result for arity " << arity << ": " << memoized_results[arity]
                 << " I/Os" << endl;
            return memoized_results[arity];
        }

        cout << "\n  Testing arity: " << arity << endl;

        string output_file = "dist/arity_exp/sorted_" + to_string(arity) + ".bin";

        int64_t io_operations = external_mergesort(input_file, output_file, arity);

        cout << "  I/O Operations for arity " << arity << ": " << io_operations << endl;

        // Record result to file
        results_out << arity << "," << io_operations << endl;

        // Cache the result
        memoized_results[arity] = io_operations;

        return io_operations;
    };

    // Perform ternary search
    while (right - left > 4) {
        cout << "Current search interval: [" << left << ", " << right << "]" << endl;

        // Calculate two middle points
        int64_t m1 = left + (right - left) / 3;
        int64_t m2 = right - (right - left) / 3;

        // Make sure m1 < m2 even for small intervals
        if (m1 >= m2) {
            m1 = left + (right - left) / 2 - 1;
            m2 = left + (right - left) / 2 + 1;
        }

        cout << "Evaluating middle points m1=" << m1 << " and m2=" << m2 << endl;

        // Evaluate at middle points
        int64_t io_m1 = evaluate_arity(m1);
        int64_t io_m2 = evaluate_arity(m2);

        // Update the search range based on the comparison
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
            // Equal I/Os, narrow to [m1, m2]
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

    for (int64_t arity = left; arity <= right; ++arity) {
        int64_t io_operations = evaluate_arity(arity);

        if (io_operations < min_io) {
            min_io = io_operations;
            optimal_arity = arity;
        }
    }

    results_out.close();

    return optimal_arity;
}

void run_arity_experiment(int64_t min_arity, int64_t max_arity) {
    cout << "\n=========================================================" << endl;
    cout << "Starting arity experiment with range [" << min_arity << ", " << max_arity << "]"
         << endl;
    cout << "Current time: " << chrono::system_clock::now().time_since_epoch().count() << endl;
    cout << "=========================================================" << endl;

    // Apply ternary search to find the optimal arity
    int64_t optimal_arity = ternary_search_optimal_arity(min_arity, max_arity);

    cout << "\n=========================================================" << endl;
    cout << "Experiment completed!" << endl;
    cout << "Optimal arity found: " << optimal_arity << endl;
    cout << "Results saved to " << results_file << endl;
    cout << "=========================================================" << endl;
}

int main(int argc, char *argv[]) {
    // Modificando los límites para buscar en el rango [500, 512]
    int64_t min_arity = 500; // Nuevo valor mínimo
    int64_t max_arity = 512;

    cout << "Running arity experiment with range [" << min_arity << ", " << max_arity << "]"
         << endl;

    // Llamamos a run_arity_experiment con el nuevo valor mínimo
    run_arity_experiment(min_arity, max_arity);
    return 0;
}
