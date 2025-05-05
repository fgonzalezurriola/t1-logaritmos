# Tarea 1 Logaritmos

## Lenguaje de programación

C++

Para ejecutar el código usar en la raiz del proyecto

```docker
docker run --rm -it -m 500m -v "$PWD":/workspace pabloskewes/cc4102-cpp-env bash
```

```sh
# Preparar los directorios para los archivos binarios
make prepare
```

```sh
# Compilar el código cpp
make build
```

```sh
# El código se ejecutará segun lo escrito en make run
# (ejecutará create_secuences pasandole un M=4, creando las 5 secuencias en dist/m_4)
make run
```

## Requisitos

### Para Windows

Instalar [gestor de paquetes Chocolatey](https://chocolatey.org/install#individual) y usar en powershell o cmd con parmisos de administrador

```sh
choco install mingw make llvm msys2 -y
refreshenv
```

Comprobar con:

```sh
g++ --version
make --version
clang-format --version
```

### Para Linux debian-based

En una terminal

```sh
sudo apt update && \
sudo apt install gcc && \
sudo apt install make && \
sudo apt install clang-format
```

### Para Mac

```sh
// No estoy seguro de si se instala así brew (gestor de paquetes):
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
brew --version
brew install gcc
brew install make
brew install clang-format
```

### g++

Para compilar usar

```sh
# usar el compilador con c++17, sobre el archivo nombre.cpp y que el archivo de output sea binario.extension
g++ -std=c++17 nombreArchivo.cpp -o nombreArchivoSalida
```

```sh
# La flag O2 optimiza el código. Wall añade advertencias comunes, Wextra añade aún más advertencias
g++ -std=c++17 -O2 -Wall nombreArchivo.cpp -o nombreArchivoSalida
```

```sh
# Ni idea que hace pero uso esto en progcomp cada vez que algo salía mal
g++ -std=c++17 -Wall -Wextra -pedantic -O2 -Wshadow \
-Wformat=2 -Wfloat-equal -Wconversion -Wlogical-op -Wshift-overflow=2 \
-Wduplicated-cond -Wcast-qual -Wcast-align -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC \
-D_FORTIFY_SOURCE=2 -fsanitize=address -fsanitize=undefined -fno-sanitize-recover \
-fstack-protector nombreArchivo.cpp -o nombreArchivoSalida
```

Y para correr el código usar

```sh
/binario.extension
```

### Docker

Para simplificar, docker levanta en una especie de "Maquina virtual" (contenedor) con lo mínimo para correr programas.
En las instrucciones se pide crear un contenedor con 500MB de ram.

[Instrucciones](https://hub.docker.com/r/pabloskewes/cc4102-cpp-env)

### Extensiones

1. C/C++ Microsoft (aceptar el extension pack) (Esta extensión fue quitada de los forks de vscode por si usan alguno)
2. Clang-Format
3. Makefile Tools Microsoft
4. Better Comments
5. Path Intellisense
6. Error Lens (Desde esta son opcionales)
7. vscode-pdf

Podríamos llegar a usar la extensión Live Share para codear en conjunto
