#ifndef CREATE_SECUENCES_H
#define CREATE_SECUENCES_H

#include <ctime>
#include <fstream>
#include <iostream>
#include <limits>
#include <random>
#include <string>

void create_and_write_M(int64_t m_mult);
std::vector<int64_t> random_vector_int64(int64_t n);
void write_vector_to_file(const std::string &filename, const std::vector<int64_t> vec);

#endif