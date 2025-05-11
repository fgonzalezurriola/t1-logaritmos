CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -Iinclude

# Regla para correr todo
# ! Se queda sim memoria al usar -O2 con 50MB, usar prepare-all y simple-all para ese caso
run-all:
	@echo "=== Running main.cpp ==="
	make build-main
	./bin/main 1
	@echo "=== Success ==="

# Regla para preparar todo antes de limitar la memoria
prepare-all:
	@echo "=== Working ==="
	make prepare
	make build
	./bin/create_secuences 60 1
	@echo "=== Success ==="

# Regla asumiendo que se usó prepare-all para correr todo, CON EXPERIMENTO ARIDAD
simple-all:
	@echo  "=== Running main.cpp ==="
	./bin/main 1
	@echo "=== Success ==="

# Regla asumiendo que se usó prepare-all para correr todo, SIN EXPERIMENTO ARIDAD
simple-all-no-experiment:
	@echo  "=== Running main.cpp ==="
	rm dist/m_60/secuence_1.bin
	./bin/main 0	
	@echo "=== Success ==="

run-all-no-experiment:
	@echo "=== Running main.cpp ==="
	make prepare
	make build-main
	./bin/main 0
	@echo "=== Success ==="

run-create_secuences:
	@echo "Compiling and running create_secuences"
	make prepare
	make build-create_secuences
	@./bin/create_secuences 60 5

# Limpiar caché de la memoria (usar SUDO)
clean-cache:
	@echo "Sync..."
	@sync
	@echo "Cleaning CACHÉ... (SUDO REQUIRED)"
	@sudo sh -c "echo 3 > /proc/sys/vm/drop_caches"
	@echo "Success."
	@sleep 1

# Regenera el archivo de entrada para el experimento
regenerate-input:
	@mkdir -p dist/m_60
	make build-create_secuences
	./bin/create_secuences 60 1

# Ejecuta el proceso completo con información detallada
run-arity:
	make clean
	@echo "=== Experiment for Arity ==="
	@echo "1. cleaning orphan files..."
	make clean-arity
	@echo "2. building directories and code..."
	make prepare
	make build-calculate_arity
	@echo "3. CLEANING CACHÉ"
	make clean-cache
	@echo "4. ..."
	make regenerate-input
	@echo "5. Ejecutando experimento de aridad..."
	@sleep 2
	./bin/calculate_arity
	@echo "=== Success ==="
	@echo "Result file in: results/arity_results.txt"
	
# Función auxiliar para comprobar cosas
read:
	./bin/read dist/arity_exp
	./bin/read dist/arity_exp/sorted_24.bin 391216000 393216000

# Targets para compilar cada programa por separado
build-main:
	@mkdir -p bin
	$(CXX) $(CXXFLAGS) src/main.cpp src/calculate_arity.cpp src/create_secuences.cpp src/external_mergesort.cpp src/external_quicksort.cpp -o bin/main

build-create_secuences:
	@mkdir -p bin
	$(CXX) $(CXXFLAGS) -DCREATE_SECUENCES_MAIN src/create_secuences.cpp -o bin/create_secuences

build-read:
	@mkdir -p bin
	$(CXX) $(CXXFLAGS) utils/read.cpp -o bin/read

build-calculate_arity:
	@mkdir -p bin
	$(CXX) $(CXXFLAGS) -DCALCULATE_ARITY_MAIN src/calculate_arity.cpp src/external_mergesort.cpp -o bin/calculate_arity

# Para bibliotecas compartidas
build-libs:
	@mkdir -p obj
	$(CXX) $(CXXFLAGS) -c src/calculate_arity.cpp -o obj/calculate_arity.o
	$(CXX) $(CXXFLAGS) -c src/create_secuences.cpp -o obj/create_secuences.o
	$(CXX) $(CXXFLAGS) -c src/external_mergesort.cpp -o obj/external_mergesort.o
	$(CXX) $(CXXFLAGS) -c src/external_quicksort.cpp -o obj/external_quicksort.o

# Compilar el código cpp
build: build-main build-create_secuences build-read build-calculate_arity

# Construir las carpetas para los archivos binarios desde 4 hasta 60
prepare:
	@echo "=== Creating directories ==="
	mkdir -p bin/ obj/ dist/m_4 dist/m_8 dist/m_12 dist/m_16 dist/m_20 \
	         dist/m_24 dist/m_28 dist/m_32 dist/m_36 dist/m_40 \
	         dist/m_44 dist/m_48 dist/m_52 dist/m_56 dist/m_60 \
	         results dist/arity_exp
	@echo "=== Success ==="

# Eliminar todos los archivos temporales
clean:
	make clean-bin
	make clean-build
	make clean-arity
	rm -rf obj/*

# Eliminar todos los archivos de bin/
clean-bin:
	rm -rf bin/*

# Eliminar todos los archivos de dist/
clean-build:
	rm -rf dist/*

# Limpiar los archivos temporales y resultados de experimentos de aridad
clean-arity:
	rm -rf results/*
	rm -rf dist/arity_exp/*
	rm -rf temp_*

clean-temp:
	rm -rf temp_*
	rm -rf results/*

# Regla para generar solamente los gráficos
graphics:
	@echo "=== Generating plots ==="
	python3 plot_experiment.py
	@echo "=== Plots saved in results/graficos/ ==="

# Las reglas dentro de PHONY se tratan como reglas de makefile en vez de archivos-directorios
.PHONY: clean run prepare read-test test clean-cache regenerate-input run-arity build-main \
        build-create_secuences build-read build-calculate_arity prepare-all simple-all