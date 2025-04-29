# Cálculos

## Resumen

El archivo más pesado pesa 3GB
En el peor caso sin abstraer y ejecutar en orden serían 120GB (que no tengo) por ALGORITMO
En el mejor caso se necesitan 3GB. Actualmente solo he abstraído para peor caso 15GB-30GB

5 secuencias de 1M
M = 50 MB

int64 = 8 bytes
4096/8 = 512 elementos puede tener cada página

M = 50 MB = 50 \* 10^6 = 50.000.000 bytes
50.000.000/4096 = 12207.031, podemos usar 12207 bloques por simplificación
12207 bloques x 512 elementos/bloque = 6.249.984 elementos con 1M

Para el caso 60M
60\*50MB = 3.000.000.000 bytes 3GB
x 5 secuencias = 15GB

Para crear 1M:
for BLOCKS_PER_M
crear el bloque con enteros
guardar el bloque con enteros
Después de 12208 veces hecho este proceso debería tener 1M

4\*50MB = 200.000.000
