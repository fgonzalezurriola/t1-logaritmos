#include <algorithm>
#include <chrono>
#include <ctime>
#include <fstream>
#include <iostream>
#include <limits>
#include <queue>
#include <random>
#include <string>

using namespace std;

/**
 * *@BLOCK_SIZE: 4096. Use `stat -f /` in Unix
 * *@INTS_PER_BLOCK: 4096/8 = 512. Number of Ints64 in one block
 * *@bin_dir: Directory name with the binaries files
 * *@M_SIZE: 50*1024 * 1024 = 50 Megabytes. Value of M
 * *@BLOCKS_PER_M: M_SIZE//BLOCK_SIZE = 12800
 */
const int64_t BLOCK_SIZE = 4096;
const int64_t INTS_PER_BLOCK = BLOCK_SIZE / sizeof(int64_t);
const string bin_dir = "dist";
const int64_t M_SIZE = 50 * 1024 * 1024;
const int64_t BLOCKS_PER_M = M_SIZE / 4096;

void create_and_write_M(int64_t m_mult);
std::vector<int64_t> random_vector_int64(int64_t n);
void write_vector_to_file(
    const std::string &filename, int64_t block_index, const std::vector<int64_t> &vec
);

/** create_and_write_M
 * @brief Calls the random_vector_int64() and write_vector_to_file() functions
 * @param n: Number of int64_t to populate
 * @returns void
 * @note the for ends after m_mult*BLOCKS_PER_M loops
 */
void create_and_write_M(int64_t m_mult, int64_t n_secuences) {
    for (int j = 0; j < n_secuences; j++) {
        string index = to_string(j + 1);
        string filename = bin_dir + "/m_" + to_string(m_mult) + "/" + "secuence" + "_" + index + ".bin";
        // * BLOCKS_PER_M multiplied per m_mult to get 4M, 8M... etc.
        for (int i = 0; i < m_mult * BLOCKS_PER_M; i++) {
            vector<int64_t> vector = random_vector_int64(INTS_PER_BLOCK);
            write_vector_to_file(filename, i, vector);
        }
    }
}

/** random_vector_int64
 * @brief Create a vector of size n with random int64_t values
 * @details using numeric_limits of int64_t to avoid writing the values of -2^63 and 2^63-1
 * @param n: Number of int64_t to populate
 * @returns Vector created
 * @note static keyword makes that the seed and the distribution are calculated just the first time
 */
vector<int64_t> random_vector_int64(int64_t n) {
    vector<int64_t> vec(n);
    // *Static keyword to avoid re-calculations of seed and uniform_int_distribution
    static mt19937_64 rng(random_device{}());
    static uniform_int_distribution<int64_t>
        dist(numeric_limits<int64_t>::min(), numeric_limits<int64_t>::max());
    for (int i = 0; i < n; i++) {
        vec[i] = dist(rng);
    }
    return vec;
}

/** write_vector_to_file
 * @details If the file it's already created it continues writing at EndOfFIle
 * @param filename file to write/overwrite
 * @return Void
 * @warning If the files doesn't exists, the program exits with error
 */
void write_vector_to_file(const string &filename, int64_t block_index, const vector<int64_t> &vec) {
    // If the file it's already created ios::app interface continues writing at EoF
    ofstream out(filename, ios::binary | ios::app);
    if (!out) {
        cerr << "Error opening file " << filename << endl;
        exit(EXIT_FAILURE);
    }
    out.seekp(block_index * BLOCK_SIZE);
    out.write(reinterpret_cast<const char *>(vec.data()), BLOCK_SIZE);
    if (!out) {
        cerr << "Error writing file " << filename << endl;
    }
    out.close();
    return;
}

#ifdef CREATE_SECUENCES_MAIN
int main(int argc, char *argv[]) {
    (void)argc;
    int64_t m_mult = stoi(argv[1]);
    int64_t n_secuences = stoi(argv[2]);
    cout << "Creating secuences" << endl;
    const auto start_create{chrono::steady_clock::now()};
    create_and_write_M(m_mult, n_secuences);
    const auto finish_create{chrono::steady_clock::now()};
    const chrono::duration<double> elapsed_seconds_create{finish_create - start_create};
    cout << "Time used for M=60: " << elapsed_seconds_create.count() << "seconds" << endl;
    return 0;
}
#endif