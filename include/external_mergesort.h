#ifndef EXTERNAL_MERGESORT_H
#define EXTERNAL_MERGESORT_H

#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <queue>
#include <string>
#include <vector>

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
 * @param arity The maximum number of files to merge at once.
 * @return Total number of I/O operations performed.
 */
int64_t
k_way_merge(const std::vector<std::string> &input_files, const std::string &output_file, int64_t arity);

/** external_mergesort
 * @brief Implements the External Merge Sort algorithm with configurable arity.
 * @param input_file Path of the input file to sort.
 * @param output_file Path of the sorted output file.
 * @param arity Merge arity (number of files to merge simultaneously).
 * @return Total number of I/O operations performed.
 */
int64_t external_mergesort(const std::string &input_file, const std::string &output_file, int64_t arity);

#endif
