CXX = g++
CXXFLAGS = -std=c++23 -Wall -Wextra -O2 -MMD -MP
TARGET = zura

# Find all source files

# This shell command COULD be dangerous but because the file structure of Zura
# is so out of whack anyways that this command hardly does it lol
SRC_FILES := $(shell find src -name '*.cpp')
OBJ_FILES := $(SRC_FILES:.cpp=.o)
DEP_FILES := $(OBJ_FILES:.o=.d)

# Final target
$(TARGET): $(OBJ_FILES)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Compile each .cpp to .o for parallelization
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Include generated dependency files
-include $(DEP_FILES)

# Clean rule
.PHONY: clean
clean:
	rm -f $(OBJ_FILES) $(DEP_FILES)
