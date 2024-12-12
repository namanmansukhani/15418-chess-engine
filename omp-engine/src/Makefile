# Makefile for Chess Engine Project

# Compiler and flags
CXX = g++-14
CXXFLAGS = -O3 -std=c++17 -fopenmp

# Target executable
TARGET = chess-engine

# Source files
SRCS = main.cpp serial-engine.cpp thc.cpp

# Object files (replace .cpp with .o)
OBJS = $(SRCS:.cpp=.o)

# Default rule
all: $(TARGET)

# Linking the executable
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

# Compiling source files into object files
%.o: %.cpp %.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up build files
clean:
	rm -f $(TARGET) $(OBJS)


