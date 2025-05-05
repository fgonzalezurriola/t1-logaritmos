#ifndef CALCULATE_ARITY_H
#define CALCULATE_ARITY_H

#include <cstdint>
#include <string>
#include <vector>

std::vector<int64_t> read_block(const std::string &filename, int64_t block_index);
void write_block(
    const std::string &filename, int64_t block_index, const std::vector<int64_t> &buffer
);

void sort_in_memory(std::vector<int64_t> &data);

int64_t k_way_merge(const std::vector<std::string> &input_files, const std::string &output_file);

int64_t
external_mergesort(const std::string &input_file, const std::string &output_file, int64_t arity);

void run_arity_experiment(int64_t min_arity, int64_t max_arity);
struct ArityResult {
    int64_t arity;
    double time_seconds;
};

int64_t find_best_arity(const std::string &results_file);

#endif