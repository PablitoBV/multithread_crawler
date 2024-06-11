# Makefile for multithreaded web crawler using libcurl and Gumbo

# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -Wall -Wextra -std=c++11

# Include directory for Gumbo Parser
INCLUDES = -I/opt/homebrew/opt/gumbo-parser/include

# Linker flags
LDFLAGS = -lcurl -L/opt/homebrew/opt/gumbo-parser/lib

# Gumbo Parser library
LDLIBS = -lgumbo

# Target executable
TARGET = web

# Source files
SRC = main.cpp

# Header files
HDRS = ConcurrentQueue.h StripedHashSet.h

# Object files
OBJ = $(SRC:.cpp=.o)

# Default rule
all: $(TARGET)

# Rule to link the target executable
$(TARGET): $(OBJ)
	$(CXX) $(OBJ) -o $(TARGET) $(LDFLAGS) $(LDLIBS)

# Rule to compile source files into object files
%.o: %.cpp $(HDRS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Clean rule to remove generated files
clean:
	rm -f $(OBJ) $(TARGET)

# Phony targets
.PHONY: all clean