# Tarea 1 Logaritmos

## Requisitos

Nota: eliminar los .csv a mano si se quiere volver a correr el experimento.

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

Nota: usar la terminal de msys2 para ejecutar los comandos make, ya que msys2 trae los comandos unix que usa el Makefile.

Alternativamente usar [docker desktop](https://docs.docker.com/desktop/setup/install/windows-install/) y seguir las instrucciones para instalar docker en Windows.

Alternativamente usar WSL (Windows Subsystem for Linux) y seguir las instrucciones para Linux.

### Para Linux Debian-based

En una terminal

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

````sh
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
brew --version
brew install gcc
brew install make
brew install clang-format
brew install --cask docker
open /Applications/Docker.app


```sh
docker run --rm -it -m 50m -v "$PWD":/workspace pabloskewes/cc4102-cpp-env bash
````

### Requisitos para Gráficos

Tener instalado python o python3

```sh
# Crear venv
python3 -m venv venv
source venv/bin/activate
# Instalar librerías
pip install pandas matplotlib
```

## Ejecución de la Tarea

Utilizar el siguiente comando en la raiz del proyecto para ejecutar el contenedor de docker:

```sh
docker run --rm -it -m 50m -v "$PWD":/workspace pabloskewes/cc4102-cpp-env bash
```

Ejecución de la experimentación cpp y el graficado con python:

```sh
make run-all
```

El flujo será el siguiente:

1. Se crean las carpetas
2. Se compila el código cpp
3. Se ejecuta el main.cpp

Dentro de main.cpp

1. Se ejecuta el experimento para obtener la mejor aridad (creando una secuencia de 60M)
2. Para cada M se ejecuta el experimento con Mergesort, (evitando colapsar la memoria secundaria).
3. Para cada M se ejecuta el experimento con Quicksort.
4. Se toman los resultados y se grafican con python.
5. Se eliminan los archivos temporales.
