#ifndef EXTERNAL_QUICKSORT_H
#define EXTERNAL_QUICKSORT_H

#include <string>
#include <vector>

/** external_quicksort
 * @brief Implements the External Quick Sort algorithm with configurable arity.
 * @param input_file Path of the input file to sort.
 * @param output_file Path of the sorted output file.
 * @param arity Partitioning arity (number of partitions to create).
 * @return Total number of I/O operations performed.
 */
int64_t external_quicksort(const std::string &input_file, const std::string &output_file, int64_t arity);

/** recursive_external_quicksort
 * @brief Recursive function that implements the External Quicksort algorithm
 * @param input_file File to sort
 * @param output_file File where the sorted result will be saved
 * @param arity Number of partitions to create
 * @param temp_dir Directory for temporary files
 * @param depth Recursion depth (for naming temporary files)
 * @return Number of I/O operations performed
 */
int64_t recursive_external_quicksort(
    const std::string &input_file, const std::string &output_file, int64_t arity,
    const std::string &temp_dir, int64_t depth
);

#endif