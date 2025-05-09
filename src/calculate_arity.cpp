#include <algorithm>
#include <calculate_arity.h>
#include <chrono>
#include <external_mergesort.h>
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
// docker run --rm -it -m 50m -v "$PWD":/workspace pabloskewes/cc4102-cpp-env bash

/**
 * @BLOCK_SIZE: 4096 bytes. Size of a disk block.
 * @INTS_PER_BLOCK: 512. Number of int64_t that fit in a block.
 * @TOTAL_MEMORY_RAM: 50MB. Memory available for in-memory sorting.
 * @results_file: File where experiment results will be written.
 */
const int64_t BLOCK_SIZE = 4096;
const int64_t INTS_PER_BLOCK = BLOCK_SIZE / sizeof(int64_t);
const int64_t TOTAL_MEMORY_RAM = 50 * 1024 * 1024;
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

        cout << "\n  Testing arity: " << m2 << endl;
        string output_file_m2 = "dist/arity_exp/sorted_" + to_string(m2) + ".bin";
        int64_t io_m2 = external_mergesort(input_file, output_file_m2, m2);
        cout << "  I/O Operations for arity " << m2 << ": " << io_m2 << endl;
        results_out << m2 << "," << io_m2 << endl;

        cout << "\n  Testing arity: " << m1 << endl;
        string output_file_m1 = "dist/arity_exp/sorted_" + to_string(m1) + ".bin";
        int64_t io_m1 = external_mergesort(input_file, output_file_m1, m1);
        cout << "  I/O Operations for arity " << m1 << ": " << io_m1 << endl;
        results_out << m1 << "," << io_m1 << endl;

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

    cout << "\n=========================================================" << endl;
    cout << "Final linear search in range [" << left << ", " << right << "]" << endl;
    cout << "=========================================================" << endl;

    int64_t optimal_arity = left;
    int64_t min_io = numeric_limits<int64_t>::max();

    for (int64_t arity = right; arity >= left; arity--) {
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
    ofstream("results/best_arity.txt") << optimal_arity << endl;
    return;
}

#ifdef CALCULATE_ARITY_MAIN
/**
 * @brief Main function of the program.
 */
int main() {
    int64_t min_arity = 2;
    int64_t max_arity = 512;

    cout << "Running arity experiment with range [" << min_arity << ", " << max_arity << "]" << endl;

    run_arity_experiment(min_arity, max_arity);

    return 0;
}
#endif
