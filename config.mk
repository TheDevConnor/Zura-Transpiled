CXX ?= g++
BUILD ?= debug
INSTALL_DIR ?= /usr/local/bin

ifeq ($(BUILD),debug)
	CXXFLAGS := -std=c++20 -g -O0 -DDEBUG \
		-Wall -Wpedantic -Wextra -Wundef -Wwrite-strings -Wredundant-decls \
		-Wdisabled-optimization -Wdouble-promotion -Wmissing-declarations -Wconversion \
		-Wstrict-overflow=2 -fstack-protector-all -Wvla
	BUILD_DIR := debug
else ifeq ($(BUILD),release)
	CXXFLAGS := -std=c++20 -O2 -DNDEBUG \
		-Wall -Wpedantic -Wextra -Wundef -Wwrite-strings -Wredundant-decls \
		-Wdisabled-optimization -Wdouble-promotion -Wmissing-declarations -Wconversion \
		-Wstrict-overflow=2 -fstack-protector-all -Wvla
	BUILD_DIR := release
else
$(error Unknown build type "$(BUILD)", must be 'debug' or 'release')
endif

LDFLAGS := -lstdc++ -Wl,-E

SRC_DIR := src
OBJ_DIR := $(BUILD_DIR)/obj

EXEC := $(BUILD_DIR)/zura

CPP_FILES := $(shell find $(SRC_DIR) -name '*.cpp')
OBJECTS := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(CPP_FILES))
