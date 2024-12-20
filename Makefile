CXX := g++
CXXFLAGS := -Wall -std=c++20 -Iinc $(shell pkg-config --cflags gtk4 epoxy)
CXXLIBS := $(shell pkg-config --libs gtk4 epoxy)
CSHADER := glslc

SRC_DIR := src
ifeq ($(DEBUG), 1)
	BUILD_DIR := build/debug
	CXXFLAGS += -g -Og
else
	BUILD_DIR := build/release
	CXXFLAGS += -O2
endif

SRCS := $(shell find $(SRC_DIR) -name '*.cpp')
OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/$(SRC_DIR)/%.o, $(SRCS))
DEPS := $(OBJS:.o=.d)
SPIRVS := $(patsubst $(SHADER_DIR)/%.vert, $(BUILD_DIR)/%_vert.spv, $(VERT_SHADERS)) $(patsubst $(SHADER_DIR)/%.frag, $(BUILD_DIR)/%_frag.spv, $(FRAG_SHADERS))
TARGET := foldscape

all: $(TARGET)

$(TARGET): $(OBJS)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(CXXLIBS) -o $(BUILD_DIR)/$@ $^

$(BUILD_DIR)/$(SRC_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

-include $(DEPS)

clean:
	rm -rf $(BUILD_DIR)

Makefile: ;

.PHONY: all clean

