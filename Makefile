# Compiler settings
CXX = g++
CXXFLAGS = -std=c++17 -g -Wall -Wextra -Iinclude
LIBS = -lSDL2 -lGLEW -lGL

# Source files from your src directory
SRC = src/main.cpp src/app.cpp src/Grid.cpp src/Renderer.cpp src/Simulator.cpp
OBJ = $(SRC:.cpp=.o)
TARGET = WaveSim

# Default build target
all: $(TARGET)

# Rule to link the final executable
$(TARGET): $(OBJ)
	$(CXX) $(OBJ) -o $(TARGET) $(LIBS)

# Rule to compile individual source files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up build files
clean:
	rm -f src/*.o $(TARGET)