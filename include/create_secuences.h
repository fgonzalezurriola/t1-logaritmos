#ifndef CREATE_SECUENCES_H
#define CREATE_SECUENCES_H

#include <cstdint>
#include <string>
#include <vector>

extern const int64_t BLOCK_SIZE;
extern const int64_t INTS_PER_BLOCK;
extern const std::string bin_dir;
extern const int64_t M_SIZE;
extern const int64_t BLOCKS_PER_M;
extern const int64_t NUMBER_OF_SECUENCES;

void create_and_write_M(int64_t m_mult, int64_t n_secuences);
std::vector<int64_t> random_vector_int64(int64_t n);
void write_vector_to_file(
    const std::string &filename, int64_t block_index, const std::vector<int64_t> &vec
);
#endif