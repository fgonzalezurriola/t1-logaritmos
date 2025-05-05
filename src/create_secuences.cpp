#include <algorithm>
#include <ctime>
#include <fstream>
#include <iostream>
#include <limits>
#include <queue>
#include <random>
#include <string>

using namespace std;

/**
 * *@BLOCK_SIZE: 4096. Use `stat -f /` in Linux/Mac
 * *@INTS_PER_BLOCK: 4096/8 = 512. Number of Ints64 in one block
 * *@bin_dir: Directory name with the binaries files
 * *@M_SIZE: 50*100.000 = 50 Megabytes. Value of M
 * *@BLOCKS_PER_M: 12207//4096 = 12207, using +1 for well ¿rounded? numbers
 * *@NUMBER_OF_SECUENCES: times of secuences = 5
 */
const int64_t BLOCK_SIZE = 4096;
const int64_t INTS_PER_BLOCK = BLOCK_SIZE / sizeof(int64_t);
const string bin_dir = "dist";
const int64_t M_SIZE = 50 * 1e6;
const int64_t BLOCKS_PER_M = M_SIZE / 4096 + 1;
const int64_t NUMBER_OF_SECUENCES = 5;

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
void create_and_write_M(int64_t m_mult) {
    for (int j = 0; j < NUMBER_OF_SECUENCES; j++) {
        string index = to_string(j + 1);
        string filename =
            bin_dir + "/m_" + to_string(m_mult) + "/" + "secuence" + "_" + index + ".bin";
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

// // TODO: todo

// // Devuelve un vector con los 512 enteros leídos, de momento si no tiene 512 enteros retorna
// error vector<int64_t> read_block(const string &filename, int64_t block_index) {
//     vector<int64_t> buffer(INTS_PER_BLOCK);
//     ifstream in(filename, ios::binary);
//     if (!in) {
//         cerr << "Error opening file " << filename << endl;
//         exit(EXIT_FAILURE);
//     }
//     in.seekg(block_index * BLOCK_SIZE);
//     in.read(reinterpret_cast<char *>(buffer.data()), BLOCK_SIZE);
//     return buffer;
// }

// void write_block(const string &filename, int64_t block_index, const vector<int64_t> &buffer) {
//     fstream out(filename, ios::in | ios::out | ios::binary);
//     if (!out) {
//         cerr << "Error opening file " << filename << endl;
//         exit(EXIT_FAILURE);
//     }
//     streampos file_offset = block_index * sizeof(block_index);
//     out.seekp(block_index * BLOCK_SIZE);
//     out.write(reinterpret_cast<const char *>(buffer.data()), BLOCK_SIZE);
//     if (!out) {
//         cerr << "Error writing to block in file " << filename << endl;
//     }
//     return;
// }

// void sort_in_memory(vector<int64_t> &data) {
//     sort(data.begin(), data.end());
// }
