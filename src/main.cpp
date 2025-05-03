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

/**
 * Función Main
 * Luego, usar reglas makefile para limpiar y crear archivos
 */
int main() {
    // int64_t target = 0;
    // vector<int64_t> vec = random_vector_int64(512);
    // const auto start_random{chrono::steady_clock::now()};
    // int64_t res = binary_search(target, vec);
    // const auto finish_random{chrono::steady_clock::now()};
    // const chrono::duration<double> elapsed_seconds_random{finish_random - start_random};
    // cout << "Tiempo binsort: " << elapsed_seconds_random.count() << "s" << endl;
    // cout << res << endl;

    cout << "Creando secuencias M=60..." << endl;
    const auto start_create{chrono::steady_clock::now()};
    create_and_write_M(4);
    const auto finish_create{chrono::steady_clock::now()};
    const chrono::duration<double> elapsed_seconds_create{finish_create - start_create};
    cout << "Tiempo creando secuencias M=60: " << elapsed_seconds_create.count() << "s" << endl;
    // Tiempo creando secuencias M=60: 260.588s -> notebook Pancho, usó 55mb en promedio
    // Tiempo después optimización de I/O's M=60: 94.6277s
    return 0;
}

// ? Idea de main
// ? Calcular la aridad (VER SI SE PUEDE CALCULAR LA ARIDAD SOLO UNA VEZ)
// ? usar M y "quicksort" o "mergesort"
// ? If "quicksort" Llamar main con el M y qsort
// ? If "mergesort" Llamar main con el M y msort
// ?
