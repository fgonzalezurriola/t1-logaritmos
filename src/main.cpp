#include "create_secuences.h"
#include "external_mergesort.h"
#include "external_quicksort.h"

#include <chrono>
#include <iostream>
#include <limits>
#include <random>
#include <string>
#include <vector>

using namespace std;

int main(int argc, char *argv[]) {
    // int64_t target = 0;
    // vector<int64_t> vec = random_vector_int64(512);
    // const auto start_random{chrono::steady_clock::now()};
    // int64_t res = binary_search(target, vec);
    // const auto finish_random{chrono::steady_clock::now()};
    // const chrono::duration<double> elapsed_seconds_random{finish_random - start_random};
    // cout << "Tiempo binsort: " << elapsed_seconds_random.count() << "s" << endl;
    // cout << res << endl;
    int64_t m_mult = stoi(argv[1]);
    cout << "Creando secuencias M=" << m_mult << "." << endl;
    const auto start_create{chrono::steady_clock::now()};
    create_and_write_M(m_mult);
    const auto finish_create{chrono::steady_clock::now()};
    const chrono::duration<double> elapsed_seconds_create{finish_create - start_create};
    cout << "Tiempo creando secuencias M=60: " << elapsed_seconds_create.count() << "s" << endl;
    // Tiempo creando secuencias M=60: 260.588s -> notebook Pancho, usó 55mb en promedio
    // Tiempo después optimización de I/O's M=60: 94.6277s
    return 0;
}
