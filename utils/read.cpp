#include <cstdint>
#include <ctime>
#include <fstream>
#include <iostream>
#include <limits>
#include <random>
#include <string>

using namespace std;

void read_n_lines(const string &filename, int n) {
    ifstream in(filename, ios::binary);
    if (!in) {
        cerr << "Error opening file: " << filename << endl;
        exit(EXIT_FAILURE);
    }
    vector<int64_t> vector;

    for (int i = 0; i < n; i++) {
        unsigned long position = i * sizeof(int64_t);
        in.seekg(position, ios::beg);
        int64_t y;
        in.read(reinterpret_cast<char *>(&y), sizeof(y));
        cout << "Número " << i << " : " << y << endl;
    }
    return;
}

int main() {
    string filename = "dist/m_4/secuence_1.bin";
    int n = 100;
    read_n_lines(filename, n);
}