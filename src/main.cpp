#include "calculate_arity.h"
#include "create_secuences.h"
#include "external_mergesort.h"
#include "external_quicksort.h"
using namespace std;

// * Al usarlo con makefile se puede usar /nombrebinario 1000 (el int argc) argumentostring (el char
// *argv)
// * Se buscará hacer en el makefile
// * ./main 4 quicksort -- se creará los binarios para m=4, se usa quicksort
// * make clean // para no matar ningún disco
// * ./main 4 mergesort
// * ...
// * ./main 60 mergesort
// * Ver resultados escritos en results/mergesort|quicksort/m_{numero}.txt
int main(int argc, char *argv[]) {

    return 0;
}

// ? Idea de main
// ? Calcular la aridad (VER SI SE PUEDE CALCULAR LA ARIDAD SOLO UNA VEZ)
// ? usar M y "quicksort" o "mergesort"
// ? If "quicksort" Llamar main con el M y qsort
// ? If "mergesort" Llamar main con el M y msort
// ?

// ? Idea de makefile
// ?