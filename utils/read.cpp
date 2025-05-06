#include <cstdint>
#include <ctime>
#include <fstream>
#include <iostream>
#include <limits>
#include <random>
#include <string>

using namespace std;

void read_n_lines(const string &filename, int ini, int end) {
    ifstream in(filename, ios::binary);
    if (!in) {
        cerr << "Error opening file: " << filename << endl;
        exit(EXIT_FAILURE);
    }
    vector<int64_t> vector;
    for (int i = ini; i < end; i++) {
        unsigned long position = i * sizeof(int64_t);
        in.seekg(position, ios::beg);
        int64_t y;
        in.read(reinterpret_cast<char *>(&y), sizeof(y));
        cout << "Número " << i + 1 << " : " << y << endl;
    }
    return;
}

int main(int argc, char *argv[]) {
    string path = argv[1];
    int ini = stoi(argv[2]);
    int end = stoi(argv[3]);
    read_n_lines(path, ini, end);
    return 0;
}
