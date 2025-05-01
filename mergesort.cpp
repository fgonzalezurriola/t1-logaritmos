#include <iostream>
using namespace std;

// Function to merge two sorted subarrays
void Merge(int arr[], int left, int mid, int right) {
    // Calculate the sizes of the two subarrays
    int n1 = mid - left + 1; // Size of left subarray
    int n2 = right - mid;    // Size of right subarray

    // Create temporary arrays for left and right subarrays
    int *L = new int[n1]; // Left subarray
    int *R = new int[n2]; // Right subarray

    // Copy data to temporary arrays L[] and R[]
    for (int i = 0; i < n1; i++)
        L[i] = arr[left + i];
    for (int j = 0; j < n2; j++)
        R[j] = arr[mid + 1 + j];

    // Merge the temporary arrays back into arr[left..right]
    int i = 0;    // Initial index of first subarray
    int j = 0;    // Initial index of second subarray
    int k = left; // Initial index of merged array

    while (i < n1 && j < n2) {
        if (L[i] <= R[j]) {
            arr[k] = L[i];
            i++;
        } else {
            arr[k] = R[j];
            j++;
        }
        k++;
    }

    // Copy remaining elements of L[] if any
    while (i < n1) {
        arr[k] = L[i];
        i++;
        k++;
    }

    // Copy remaining elements of R[] if any
    while (j < n2) {
        arr[k] = R[j];
        j++;
        k++;
    }

    // Free dynamically allocated memory
    delete[] L;
    delete[] R;
}

// Function to implement merge sort
void MergeSort(int arr[], int left, int right) {
    if (left < right) {
        // Find the middle point
        int mid = left + (right - left) / 2;

        // Recursively sort first and second halves
        MergeSort(arr, left, mid);
        MergeSort(arr, mid + 1, right);

        // Merge the sorted halves
        Merge(arr, left, mid, right);
    }
}

void MergeSort_N_ario(int N[], position, int arity){
  int position_after_puntero = (len(N)-1)/arity + 1;

  for (int i = 0; i < len(N); i += position_after_puntero){

    int puntero = i + (len(N)-1)/arity;
    MergeSort_N_ario(N, N[i:puntero], arity);

    // el left en este caso sería la poisicón de i, el mid la posición de
    // puntero y el right la posición justo despues de puntero
    Merge(arr, i, puntero, position_after_puntero);

    }    
  }

// Main function to execute the program
int main() {
    // Example array to be sorted
    int arr[] = {38, 27, 43, 3, 9, 82, 10};
    int n = sizeof(arr) / sizeof(arr[0]); // Calculate number of elements

    cout << "Original Array: ";
    for (int i = 0; i < n; i++)
        cout << arr[i] << " ";

    cout << endl;

    MergeSort(arr, 0, n - 1); // Call merge sort on the array

    cout << "Sorted Array: ";
    for (int i = 0; i < n; i++)
        cout << arr[i] << " ";

    cout << endl;

    return 0;
}