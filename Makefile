CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -Iinclude

# Construye carpetas y ejecuta el código compilado en bin/
# run:
# 	make prepare
# 	./bin/create_secuences 4

run:
	make prepare
	./bin/program 60

run-arity:
	make prepare
	make build
	make test

test:
	make calculate_arity
	./bin/calculate_arity

# 187500000 187600000 187532953
# ./bin/read temp_merge_172/merged_0.bin 374029760 375029760
# ./bin/read dist/m_60/secuence_1.bin 374029760 375029760
read:
	./bin/read dist/m_60/secuence_1.bin 374029760 375029760
	./bin/read temp_merge_172/merged_0.bin 374029760 375029760


# Compilar el código c++
build:
	$(CXX) $(CXXFLAGS) src/main.cpp src/create_secuences.cpp -o bin/program
	$(CXX) $(CXXFLAGS) utils/read.cpp -o bin/read
	$(CXX) $(CXXFLAGS) src/calculate_arity.cpp -o bin/calculate_arity

# Construir las carpetas para los archivos binarios desde 4 hasta 60
prepare:
	mkdir -p bin/ dist/m_4 dist/m_8 dist/m_12 dist/m_16 dist/m_20 \
	         dist/m_24 dist/m_28 dist/m_32 dist/m_36 dist/m_40 \
	         dist/m_44 dist/m_48 dist/m_52 dist/m_56 dist/m_60 \
	         results dist/arity_exp
	make build

# Si da error usar `sudo make (regla)`

# Eliminar todos los archivos de bin/ y dist/
clean:
	rm -rf bin/*
	rm -rf dist/*

# Eliminar todos los archivos de bin/
clean-bin:
	rm -rf bin/*

# Eliminar todos los archivos de dist/
clean-build:
	rm -rf dist/*

clean-arity:
	rm -rf temp_merge_*
	rm -rf results/*
	rm -rf dist/arity_exp

# Las reglas dentro de PHONY se tratarán como reglas de makefile en vez de archivos-directorios
.PHONY: clean run prepare read test calculate_arity