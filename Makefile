CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -Iinclude
TARGET = create_secuences

# Construye carpetas y ejecuta el código compilado en bin/
run:
	make prepare
	./bin/$(TARGET) 4

read:
	./bin/read

# Construir las carpetas para los archivos binarios desde 4 hasta 60
prepare:
	mkdir -p bin/ dist/m_4 dist/m_8 dist/m_12 dist/m_16 dist/m_20 \
	         dist/m_24 dist/m_28 dist/m_32 dist/m_36 dist/m_40 \
	         dist/m_44 dist/m_48 dist/m_52 dist/m_56 dist/m_60

# Compilar el código c++
build:
	$(CXX) $(CXXFLAGS) utils/read.cpp -o bin/read 
	$(CXX) $(CXXFLAGS) src/create_secuences.cpp -o bin/$(TARGET)

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

# Las reglas dentro de PHONY se tratarán como reglas de makefile en vez de archivos-directorios
.PHONY: clean run prepare read