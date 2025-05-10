#include <iostream>
#include <vector>
#include <unordered_set>
#include <algorithm>
#include <random>
#include "create_secuence.h"

std::vector<int> generar_pivotes(int cant_pivotes, int max) {

  std::unordered_set<int> num; //numeros unicos
  std::vector<int> ind_pivotes; //arreglo de numeros

  //generador de números aleatorios
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> distribucion(0, max);

  //generar números aleatorios únicos
  while (ind_pivotes.size() < cant_pivotes) {
      int indices = distribucion(gen); //numeros
      if (num.find(indices) == num.end()) { //si no está repetido se guarda
          num.insert(indices);
          ind_pivotes.push_back(indices);
      }
  }
  return ind_pivotes;
}

void partition(std::vector<int> &N, int arity){
  int n_pivots = arity - 1;
  int max = N.size() - 1;

  std::vector<int> pivots = generar_pivotes(n_pivots, max);
  // ahora pivots estará ordenado
  std::sort(pivots.begin(), pivots.end());
  std::vector<int> pivots_arreglo;
  
  for(int i = 0; i < pivots.size(); i++){
    pivots_arreglo.push_back(N[pivots[i]]);
  }
  
  // pivotes_arreglo ahora estará ordenado
  std::sort(pivots_arreglo.begin(), pivots_arreglo.end());
  
  //n_particiones = arity;

  //for(i = 0; i<= len(N); i++){
    //if(N[i]<)
  //}
  for(int i = 0; i < pivots_arreglo.size(); i++){
      std::cout << pivots_arreglo[i] << std::endl;  
}}

const int64_t M = 50 * 1024 * 1024;
int64_t file_size = N.tellg();

void quicksort_N_ario(std::vector<int> &N, int arity){

  partition(N, arity);
  
  if (file_size <= M){
    std::sort(N.begin(), N.end());
  }

  else{
    int cantidad_bloques;
    // seleccionar un bloque aleatorio de A para leerlo
    if (file_size%M != 0){
      cantidad_bloques = file_size/M + 1;
    }
    else{
      cantidad_bloques = file_size/M;
    }

    // se obtiene un valor aleatorio del bloque de A al cual se accederá
    int64_t bloque_aleatorio;

    static mt19937_64 rng(random_device{}());
    static uniform_int_distribution<int64_t>
    dist(0, cantidad_bloques);
    bloque_aleatorio = dist(rng);
    
    // se lee el bloque aleatorio
    // es read_n_lines y tiene que recibir un número n de lineas a leer
    char lol;
    read_block(&lol, bloque_aleatorio);

    // pivots es el arreglo de índices ordenado
    // pivots_arreglo son los números N[pivots[i]] ordenados que ya se generó

    // la cantidad de numeros que contiene el arreglo A será de
    int cant_ints = file_size/8;

    // Particionar el arreglo 𝐴 en 𝑎 subarreglos, de tal forma de que estos
    // subarreglos sean separados por los pivotes elegidos

    // creamos los a arreglos vacios primero
    std::vector<std::vector<int>> vector_de_vectores;
    
    for(int i = 0; i <= arity; i++){
        vector_de_vectores.push_back(std::vector<int>()); // agrega un vector<int> vacío
    }

    // for para ver uno a uno los elementos del bloque N
    for (int i = 0; i <= cant_ints - 1; i++){
      // for para poder moverse entre el arreglo pivots_arreglo y comparar
      for (int j = 0; j <= arity - 2; j++){
        if(N[i] <= pivots_arreglo[j]){  
          vector_de_vectores[0].push_back(N[i]);
        }
        else{
          continue;
        }
      vector_de_vectores[arity-1].push_back(N[i]);
      } 
    }

  std::vector<int> resultado;
  // Este for se encarga de, si el vector_de_vectores de la iteración tiene
  // largo 1 (posee solo 1 elemento), ya esta redy y tiene que pasar al siguiente,
  // donde sino posee largo uno llama iterativamente a quicksort_N_ario
    for (int k = 0; k <= arity - 1; k++){
      if (vector_de_vectores[k].size() > 1){
        quicksort_N_ario(vector_de_vectores[k], vector_de_vectores[k].size());
      } 
      // no se pueden asegurar el generar arity sub-arreglos en cada sub-arreglo
      // depende del tamaño de cada uno
      resultado.insert(resultado.end(), vector_de_vectores[k].begin(), vector_de_vectores[k].end());
      
      for (const auto& subvector : vector_de_vectores) {
        resultado.insert(resultado.end(), subvector.begin(), subvector.end());
        
      }

      vector_de_vectores = resultado;

      N = resultado;
    }
    }

}

    // En este punto cada vector dentro del arreglo vector_de_vectores esta lleno
    // ahora falta la recursión
  }}

int main() {
  int a[] = {2, 6, 5, 1, 3, 4};
  int n = sizeof(a) / sizeof(a[0]);
  quickSort(a, 0, n - 1);
  for (int i = 0; i < n; i++)
    cout << a[i] << " ";
  return 0;
}
