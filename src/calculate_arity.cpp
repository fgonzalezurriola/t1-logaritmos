#include <chrono>
#include <create_secuences.h>
#include <ctime>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

void function();

/** function
 * @brief
 * @details
 * @param
 * @return
 * @note
 * @warning
 */
void function() {
    return;
}

/**
    Idea: Experimento
    Hacer binary search sobre arreglos de 512 de largo
    Busqueda binaria de [2,512]
    Calcular IO/s y tiempo, el mejor resultado definir ese como #DEFINE a o const int64_t a = ...;
        Primero: probar con un arreglo de 512
        Luego: tratar de abstraer a 60M
*/

/**
 *
 *
 */

/**
 * @brief Reads a vector of 512 int64_t values from a binary file at a specific offset.
 * @param offset The starting offset (in number of int64_t values) from the beginning of the file.
 * @param filename The name of the binary file to read from.
 * @return A vector containing the 512 int64_t values read from the file.
 */
vector<int64_t> read_vector_from_file(int64_t offset, const string &filename) {
    ifstream in(filename, ios::binary);
    if (!in) {
        cerr << "Error opening file " << filename << " in read_vector_from_file()" << endl;
        exit(EXIT_FAILURE);
    }
    int64_t position = offset * sizeof(int64_t);
    in.seekg(position, ios::beg);
    if (!in) {
        cerr << "Error seeking to position " << position << " (offset " << offset << ") in file "
             << filename << endl;
        exit(EXIT_FAILURE);
    }
    vector<int64_t> result(INTS_PER_BLOCK);
    in.read(reinterpret_cast<char *>(result.data()), INTS_PER_BLOCK * sizeof(int64_t));
    // if (!in || in.gcount() != INTS_PER_BLOCK * sizeof(int64_t)) {
    //     cerr << "Error reading " << INTS_PER_BLOCK << " int64_t values from offset " << offset
    //          << " in file " << filename << endl;
    //     exit(EXIT_FAILURE);
    // }
    return result;
}

void write_vector_to_file(const string &filename, const vector<int64_t> &vec) {
    ofstream out(filename, ios::binary);
    if (!out) {
        cerr << "Error opening file " << filename << endl;
        exit(EXIT_FAILURE);
    }
    for (auto val : vec) {
        out.write(reinterpret_cast<const char *>(&val), sizeof(val));
    }
    return;
}
