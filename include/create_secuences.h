#ifndef CREATE_SECUENCES_H
#define CREATE_SECUENCES_H

#include <algorithm>
#include <cstdint>
#include <ctime>
#include <fstream>
#include <iostream>
#include <limits>
#include <queue>
#include <random>
#include <string>
#include <vector>

/**
 * @brief Calls the random_vector_int64() and write_vector_to_file() functions
 * @param m_mult Size multiplier for M
 * @param n_secuences Number of sequences to create
 * @returns void
 * @note The for loop ends after m_mult*BLOCKS_PER_M iterations
 */
void create_and_write_M(int64_t m_mult, int64_t n_secuences);

/**
 * @brief Create a vector of size n with random int64_t values
 * @details using numeric_limits of int64_t to avoid writing the values of -2^63 and 2^63-1
 * @param n Number of int64_t to populate
 * @returns Vector created
 * @note static keyword makes that the seed and the distribution are calculated just the first time
 */
std::vector<int64_t> random_vector_int64(int64_t n);

/**
 * @brief Write a vector to a file at specific block position
 * @details If the file it's already created it continues writing at specific position
 * @param filename File to write/overwrite
 * @param block_index The index of the block to write to
 * @param vec The vector of data to write
 * @return Void
 * @warning If the files doesn't exist or can't be opened, the program exits with error
 */
void write_vector_to_file(
    const std::string &filename, int64_t block_index, const std::vector<int64_t> &vec
);

#endif
