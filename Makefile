# Makefile para o Trabalho de Computação Gráfica - Espada na Pedra

# Compilador
CXX = g++

# Flags de compilação (com OpenMP para paralelização)
CXXFLAGS = -std=c++17 -Wall -O2 -I./include -fopenmp

# Flags do linker para FreeGLUT (Windows) + OpenMP + GLEW (GPU)
LDFLAGS = -lglew32 -lfreeglut -lopengl32 -lglu32 -fopenmp

# Diretórios
SRC_DIR = src
INCLUDE_DIR = include
BUILD_DIR = build

# Arquivos fonte (incluindo GPU renderer)
SOURCES = $(SRC_DIR)/main.cpp $(SRC_DIR)/globals.cpp $(SRC_DIR)/scene_setup.cpp $(SRC_DIR)/renderer.cpp $(SRC_DIR)/input_handlers.cpp $(SRC_DIR)/stb_impl.cpp $(SRC_DIR)/gui/gui_manager.cpp $(SRC_DIR)/gui/gui_primitives.cpp $(SRC_DIR)/gui/gui_render.cpp $(SRC_DIR)/gui/gui_input.cpp $(SRC_DIR)/gpu_renderer.cpp

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
