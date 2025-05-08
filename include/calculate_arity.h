#ifndef CALCULATE_ARITY_H
#define CALCULATE_ARITY_H

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <limits>
#include <queue>
#include <string>
#include <vector>

/**
 * @brief Reads multiple blocks from a binary file.
 * @param filename Name of the file to read.
 * @param start_block Index of the first block to read.
 * @param num_blocks_to_read Number of blocks to read.
 * @return Vector with integers read from the blocks.
 */
std::vector<int64_t>
read_multiple_blocks(const std::string &filename, int64_t start_block, int64_t num_blocks_to_read);

/**
 * @brief Writes a block of data to a binary file.
 * @param filename Name of the file to write to.
 * @param block_index Index of the block to write.
 * @param buffer Vector of data to write.
 */
void write_block(const std::string &filename, int64_t block_index, const std::vector<int64_t> &buffer);

/**
 * @brief Sorts an integer vector in memory using the standard sort algorithm.
 * @param data Vector of integers to sort.
 */
void sort_in_memory(std::vector<int64_t> &data);

/**
 * @brief Recursively creates directories in the specified path.
 * @param dir Path of the directory to create.
 */
void create_directories(const std::string &dir);

/**
 * @brief Recursively removes a directory and its contents.
 * @param dir Path of the directory to remove.
 */
void remove_directory(const std::string &dir);

/**
 * @brief Copies a file from source to destination.
 * @param src Path of the source file.
 * @param dst Path of the destination file.
 */
void copy_file(const std::string &src, const std::string &dst);

/**
 * @brief Performs a k-way merge of multiple sorted files.
 * @param input_files Vector with paths of input files.
 * @param output_file Path of the merged output file.
 * @param arity The maximum number of files to merge at once.
 * @return Total number of I/O operations performed.
 */
int64_t
k_way_merge(const std::vector<std::string> &input_files, const std::string &output_file, int64_t arity);

/**
 * @brief Implements the External Merge Sort algorithm with configurable arity.
 * @param input_file Path of the input file to sort.
 * @param output_file Path of the sorted output file.
 * @param arity Merge arity (number of files to merge simultaneously).
 * @return Total number of I/O operations performed.
 */
int64_t external_mergesort(const std::string &input_file, const std::string &output_file, int64_t arity);

/**
 * @brief Performs a ternary search to find the optimal arity.
 * @param left Lower limit of the search range.
 * @param right Upper limit of the search range.
 * @return The optimal arity found.
 */
int64_t ternary_search_optimal_arity(int64_t left, int64_t right);

/**
 * @brief Runs the experiment to find the optimal arity in a specified range.
 * @param min_arity Minimum arity to test.
 * @param max_arity Maximum arity to test.
 * @return The optimal arity found.
 */
int64_t run_arity_experiment(int64_t min_arity, int64_t max_arity);

#endif
