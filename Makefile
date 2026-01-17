# Makefile para o Trabalho de Computação Gráfica - Espada na Pedra

# Compilador
CXX = g++

# Flags de compilação
CXXFLAGS = -std=c++17 -Wall -O2 -I./include

# Flags do linker para FreeGLUT (Windows)
LDFLAGS = -lfreeglut -lopengl32 -lglu32

# Diretórios
SRC_DIR = src
INCLUDE_DIR = include
BUILD_DIR = build

# Arquivos fonte
SOURCES = $(SRC_DIR)/main.cpp

# Nome do executável
TARGET = raytracer.exe

# Regra principal
all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

# Compilar e executar
run: $(TARGET)
	./$(TARGET)

# Limpar arquivos gerados
clean:
	rm -f $(TARGET)
	rm -rf $(BUILD_DIR)

# Verificar includes
check:
	@echo "Verificando estrutura de arquivos..."
	@dir /B include
	@dir /B src

.PHONY: all run clean check
