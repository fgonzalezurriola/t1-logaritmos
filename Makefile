CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -Iinclude

run-dist:
	@echo "Compiling and running create_secuences"
	make prepare
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
	@./bin/create_secuences 60 1

# Ejecuta el proceso completo con información detallada
run-arity:
	make clean
	@echo "=== Experiment for Arity ==="
	@echo "1. cleaning orphan files..."
	make clean-arity
	@echo "2. building directories and code..."
	make prepare
	make build
	@echo "3. CLEANING CACHÉ"
	make clean-cache
	@echo "4. ..."
	make regenerate-input
	@echo "5. Ejecutando experimento de aridad..."
	@sleep 2
	make test
	@echo "=== Success ==="
	@echo "Result file in: results/arity_results.txt"

test:
	@./bin/calculate_arity

read-test:
	./bin/read dist/m_60/secuence_1.bin 374029760 375029760
	./bin/read temp_merge_172/merged_0.bin 374029760 375029760

# Compilar el código c++
build:
	@$(CXX) $(CXXFLAGS) src/main.cpp src/create_secuences.cpp -o bin/create_secuences
	@$(CXX) $(CXXFLAGS) utils/read.cpp -o bin/read
	@$(CXX) $(CXXFLAGS) src/calculate_arity.cpp src/external_mergesort.cpp -o bin/calculate_arity

# Construir las carpetas para los archivos binarios desde 4 hasta 60
prepare:
	@echo "=== Creating directories ==="
	@mkdir -p bin/ dist/m_4 dist/m_8 dist/m_12 dist/m_16 dist/m_20 \
	         dist/m_24 dist/m_28 dist/m_32 dist/m_36 dist/m_40 \
	         dist/m_44 dist/m_48 dist/m_52 dist/m_56 dist/m_60 \
	         results dist/arity_exp
	@echo "=== Success ==="

# Eliminar todos los archivos temporales
clean:
	make clean-bin
	make clean-build
	make clean-arity

# Eliminar todos los archivos de bin/
clean-bin:
	@rm -rf bin/*

# Eliminar todos los archivos de dist/
clean-build:
	@rm -rf dist/*

# Limpiar los archivos temporales y resultados de experimentos de aridad
clean-arity:
	@rm -rf results/*
	@rm -rf dist/arity_exp
	@rm -rf temp_*

# Las reglas dentro de PHONY se tratarán como reglas de makefile en vez de archivos-directorios
.PHONY: clean run prepare read-test test clean-cache regenerate-input run-arity