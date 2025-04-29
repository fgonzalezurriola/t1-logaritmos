CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -Iinclude
TARGET = create_secuences

run:
	make prepare
	./bin/$(TARGET)

read:
	./bin/read


prepare:
	mkdir -p bin/ dist/m_4 dist/m_8 dist/m_12 dist/m_16 dist/m_20 \
	         dist/m_24 dist/m_28 dist/m_32 dist/m_36 dist/m_40 \
	         dist/m_44 dist/m_48 dist/m_52 dist/m_56 dist/m_60

build:
	$(CXX) $(CXXFLAGS) utils/read.cpp -o bin/read 
	$(CXX) $(CXXFLAGS) src/create_secuences.cpp -o bin/$(TARGET)


clean:
	rm -rf bin/*
	rm -rf dist/*

clean-bin:
	rm -rf bin/*

clean-build:
	rm -rf dist/*

.PHONY: clean run prepare read