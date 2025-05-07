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

  //imprimir arreglo resultante
  /*for (int numero : ind_pivotes) {
      std::cout << numero << " ";
  }
  std::cout << std::endl;*/

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
int64_t file_size = A.tellg();

void quicksort_N_ario(std::vector<int> &N, int puntero, int arity){

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
    char lol;
    read_block(&lol, bloque_aleatorio);

    // elegir 𝑎 − 1 de sus elementos al azar y ordenarlos
    partition(N, arity);

    // pivots es el arreglo de índices ordenado
    // pivots_arreglo son los números N[pivots[i]] ordenados

    // hay que asignar en las posiciones ordenadas de los índices (pivots)
    // los valores ordenados del arreglo (pivots_arreglo)

    for (int i = 0; i <= arity - 1; i++){
      N[pivots[i]] = pivots_arreglo[i];
    }
    // esta wea hacerla en el archivo binario dios sepa como D:
    // luego de hacer esto, se supone que en ela rreglo N en las posiciones
    // pivots (N[pivots]) estarán los valores ordenados de esas posiciones del arreglo

    // la cantidad de numeros que contiene el arreglo A será de
    int cant_ints = file_size/8;

    // Particionar el arreglo 𝐴 en 𝑎 subarreglos, de tal forma de que estos
    // subarreglos sean separados por los pivotes elegidos

    // creamos los a arreglos vacios primero
    std::vector<int> vector_de_vectores;
    
    for(int i = 0; i <= arity; i++){
        vector_de_vectores.push_back(std::vector<int>()); // agrega un vector<int> vacío
    }

    for (int i = 0; i<= arity; i++){
      std::vector<int> sub_arreglo_i;
      for (int j = 0; j <= cant_ints; j++){
        // devuelve true si el índice que esta viendo en el arreglo es el mismo
        // indice que el del pivot, si es ese el caso, ++
        if (std::find(pivots.begin(), pivots.end(), j) != pivots.end()){
          continue;
        }

        else{
          // pivots posee arity - 1 elementos
          // el int posicion marcará el índice dentro de pivots y a que vector <arreglo_i> se debe añadir
          int posicion = 0;
          while(posicion <= arity - 1){
            if(N[j] <= N[pivots[posicion]]){
              vector_de_vectores[posicion].push_back(N[j]);
            }
            else{
              posicion++;
            }
          }

          // cuando posición > arity - 1, ya se termino de recorrer el arreglo de pivots, entonces el elemento
          // N[j] pertenece al último arreglo (<arreglo_a>)
          vector_de_vectores[arity].push_back(N[j]);
          }
          
      }}}

    // bubblesort para poder ordenar los pivotes en el arreglo A y
    // poder preguntar dato a dato sobre su valor con respecto al pivote mas
    // cercano de forma iterativa para poder definir su posicion final en 
    // los subarreglos
        // for(int i=0; i <= indices.len; i++){
        //  if A[ind[i]] > A[ind[i+1]]{
        //    (A[i], A[i+1]) = (A[i+1], A[i])
        //  }
      //  }
      }
    }

}}

int main() {
  int a[] = {2, 6, 5, 1, 3, 4};
  int n = sizeof(a) / sizeof(a[0]);
  quickSort(a, 0, n - 1);
  for (int i = 0; i < n; i++)
    cout << a[i] << " ";
  return 0;
}
