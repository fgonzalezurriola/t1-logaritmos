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
 * @brief Writes the results of the sorting experiment to a CSV file.
 * @param algorithm Name of the sorting algorithm ("mergesort"/"quicksort")
 * @param m m_mult of the sequence
 * @param sequence_number Number of the sequence
 * @param total_io Total number of I/O operations performed
 * @param time_seconds Time used for the sorting
 */
void write_sort_results(
    const string &algorithm, int64_t m, int64_t sequence_number, int64_t total_io, double time_seconds
) {
    const string results_file = "results/" + algorithm + "_results.csv";

    bool file_exists = false;
    ifstream check_file(results_file);
    if (check_file.good()) {
        file_exists = true;
    }
    check_file.close();

    ofstream results_out(results_file, ios::app);
    if (!results_out) {
        cerr << "Error opening results file: " << results_file << endl;
        exit(EXIT_FAILURE);
    }
    if (!file_exists) {
        results_out << "m,sequence,IO_operations,time_seconds" << endl;
    }

    results_out << m << "," << sequence_number << "," << total_io << "," << time_seconds << endl;
    results_out.close();

    cout << "   Results written in " << results_file << endl;
}

void run_sorting_experiment(
    const string &algorithm, int64_t arity, const vector<int64_t> &m_mults, int64_t n_secuences
) {
    for (int64_t m : m_mults) {
        cout << "==========================================" << endl;
        cout << "Running: create_and_write_M with m_mult = " << m << endl;
        const auto start_create{chrono::steady_clock::now()};
        create_and_write_M(m, n_secuences);
        const auto finish_create{chrono::steady_clock::now()};
        const chrono::duration<double> elapsed_seconds_create{finish_create - start_create};
        cout << "Time used for M= " << m << " " << elapsed_seconds_create.count() << " seconds" << endl;

        for (int64_t i = 0; i < n_secuences; i++) {
            string input_file = "dist/m_" + to_string(m) + "/secuence_" + to_string(i + 1) + ".bin";
            string output_file = "dist/m_" + to_string(m) + "/sorted_" + to_string(i + 1) + ".bin";

            cout << "       Running external " << algorithm << endl;
            cout << "       Using m_mult = " << m
                 << (algorithm == "mergesort" ? " and arity = " + to_string(arity) : "") << endl;

            const auto start_sort{chrono::steady_clock::now()};
            int64_t total_io = 0;

            if (algorithm == "mergesort") {
                total_io = external_mergesort(input_file, output_file, arity);
            } else if (algorithm == "quicksort") {
                // Todo: quicksort
                // total_io = external_quicksort(input_file, output_file);
                cout << "Quicksort not implemented yet" << endl;
            }

            const auto finish_sort{chrono::steady_clock::now()};
            const chrono::duration<double> elapsed_seconds_sort{finish_sort - start_sort};

            double time_seconds = elapsed_seconds_sort.count();
            cout << "       Time: " << time_seconds << " seconds, I/O: " << total_io << endl;

            write_sort_results(algorithm, m, i + 1, total_io, time_seconds);
        }

        // Limpiamos los archivos temporales después de procesar cada tamaño m
        string temp_dir = "dist/m_" + to_string(m) + "/";
        remove_directory(temp_dir);
        create_directories(temp_dir);
        cout << "Cleaned temporary files for m = " << m << endl;
        cout << "==========================================" << endl;
    }
}

int main(int argc, char *argv[]) {
    (void)argc;
    int experiment = stoi(argv[1]);
    // There is a rule to skip the experiment
    // if argv[1] is 1, run the experiment
    if (experiment == 1) {
        cout << "==========================================" << endl;
        cout << "Running: calculate_arity between [2, 512]" << endl;
        const auto start_create{chrono::steady_clock::now()};
        run_arity_experiment(2, 512);
        const auto finish_create{chrono::steady_clock::now()};
        const chrono::duration<double> elapsed_seconds_create{finish_create - start_create};
        cout << "Time used for calculate_arity: " << elapsed_seconds_create.count() << "seconds" << endl;
        cout << "==========================================" << endl;
    }

    int64_t n_secuences = 5;
    const vector<int64_t> m_mults{4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 48, 52, 56, 60};

    // Read the best arity stored in results/best_arity.txt

    // Todo: Esperar que respondan los auxiliares sino dejarlo en 29 o 62
    // Todo: y explicar en el informe
    int64_t arity = 62;
    ifstream best_arity_file("results/best_arity.txt");
    if (best_arity_file) {
        best_arity_file >> arity;
        best_arity_file.close();
    }

    // Run quicksort experiment
    // Todo: quicksort
    // ! Mergesort deletes the m_secuences, use quicksort first
    // run_sorting_experiment("quicksort", 0, m_mults, n_secuences);

    // Run mergesort experiment
    run_sorting_experiment("mergesort", arity, m_mults, n_secuences);

    return 0;
}
