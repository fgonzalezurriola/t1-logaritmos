#include "calculate_arity.h"
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
