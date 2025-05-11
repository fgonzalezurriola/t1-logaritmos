# Tarea 1 Logaritmos

## Ejecución de la Tarea

Con docker instalado utilizar el siguiente comando en la raiz del proyecto para ejecutar el contenedor de docker:

```sh
docker run --rm -it -m 50m -v "$PWD":/workspace pabloskewes/cc4102-cpp-env bash
```

```sh
make prepare-all
make simple-all
```

Al terminar simple-all se guardan dos archivos CSV en results/, luego para los gráficos:

```sh
python3 -m venv venv
source venv/bin/activate
pip install pandas matplotlib numpy os
make graphics
```

Para graficar

Tener instalado python o python3

```sh
# Crear venv
python3 -m venv venv
source venv/bin/activate
# Instalar librerías
pip install pandas matplotlib numpy
# usar la regla para los gráficos
make graphics
```

## Descripción del experimento

El flujo será el siguiente:

1. Se crean las carpetas y se compila el código cpp
2. Se ejecuta el main.cpp, se generan csv en results/ al inicio se ejecuta el experimento de la busqueda ternaria en \[2,62] (\*ver justificación en informe) con cada paso del experimento (qs con M=4, qs con M=8... qs con M=60, análogo para mergesort)
3. Se grafican los csv, y se guardan en results/graphics/

En caso de rehacer el experimento, usar make clean, seguido de make prepare-all.
Como está explicado en el informe, la aridad a=62 está hardcodeada, por lo que se puede usar

```sh
make simple-all-no-experiment
```

Para evitar la ejecución lenta del experimento.

Explicación de main.cpp

1. Se crea una secuencia de 60M y ejecuta el experimento de búsqueda ternaria obtener la mejor aridad entre (2, 62)
2. Para cada M se ejecuta el experimento con Quicksort (evitando colapsar la memoria secundaria generando y borrando las secuencias), guardando los resultados en un csv.
3. Para cada M se ejecuta el experimento con Mergesort, guardando los resultados en un csv.
4. Se toman los resultados y se grafican con python.
5. Sugerencia: usar make clean para eliminar csv y archivos temporales, para correr el experimento de nuevo.

## Requisitos

Docker o Docker Desktop
Python o Python3

Debido a que el experimento se corre en docker, no es necesario tener más dependencias.

## Requisitos Dev

Esto fue usado para desarrollar la tarea, no es necesario tenerlo para correr el experimento.

````sh
Nota: la tarea fue desarollada en debian 12 bookworm, en cierto punto se probó en windows.
de todas formas utilice docker para correr el experimento.
Nota: eliminar los .csv a mano si se quiere volver a correr el experimento.

### Para Windows

Instalar [gestor de paquetes Chocolatey](https://chocolatey.org/install#individual) y usar en powershell o cmd con parmisos de administrador

```sh
choco install mingw make llvm msys2 -y
refreshenv
````

Comprobar con:

```sh
g++ --version
make --version
clang-format --version
```

Nota: usar la terminal de msys2 para ejecutar los comandos make, ya que msys2 trae los comandos unix que usa el Makefile.

Alternativamente usar [docker desktop](https://docs.docker.com/desktop/setup/install/windows-install/) y seguir las instrucciones para instalar docker en Windows.

Alternativamente usar WSL (Windows Subsystem for Linux) y seguir las instrucciones para Linux.

### Para Linux Debian-based

En una terminal:

```sh
sudo apt update && \
sudo apt install gcc && \
sudo apt install make && \
sudo apt install clang-format
sudo apt install docker.io
sudo systemctl start docker
sudo systemctl enable docker
sudo usermod -aG docker $USER
```

### Para Mac

```sh
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
brew --version
brew install gcc
brew install make
brew install clang-format
brew install --cask docker
open /Applications/Docker.app
```
