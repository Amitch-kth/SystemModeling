# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -Wall -std=c++11

# Executable name
EXEC = dm_cache

# Source files
SRCS = cache_test.cc

# Object files (automatically replace .cpp/.cc with .o)
OBJS = $(SRCS:.cpp=.o)
OBJS := $(OBJS:.cc=.o)

HEADERS = cache.h

# Target to build the executable
all: $(EXEC)

# Rule to link object files and create the executable
$(EXEC): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(EXEC) $(OBJS)


# Rule to compile .cc files into object files
%.o: %.cc $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Rule to run the program
run: $(EXEC)
	./$(EXEC)

# Rule to clean up object files and the executable
clean:
	rm -f $(OBJS) $(EXEC)

# Phony targets (to avoid confusion with files named 'all', 'clean', 'run')
.PHONY: all clean run
