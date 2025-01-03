CXX := g++
CXXFLAGS := -Wall -std=c++20 -Iinc $(shell pkg-config --cflags gtk4)
CXXLIBS := $(shell pkg-config --libs gtk4)
CSHADER := glslc

SRC_DIR := src
SHADER_DIR := shaders
ifeq ($(DEBUG), 1)
	BUILD_DIR := build/debug
	CXXFLAGS += -g -Og -DDEBUG
else
	BUILD_DIR := build/release
	CXXFLAGS += -O2 -DNDEBUG
endif

SRCS := $(shell find $(SRC_DIR) -name '*.cpp')
OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/$(SRC_DIR)/%.o, $(SRCS))
DEPS := $(OBJS:.o=.d)
SHADERS := $(shell find $(SHADER_DIR) -name '*.comp')
SPIRVS := $(patsubst $(SHADER_DIR)/%.comp, $(BUILD_DIR)/%_comp.spv, $(SHADERS))
TARGET := foldscape

all: $(TARGET) $(SPIRVS)

$(TARGET): $(OBJS)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(CXXLIBS) -o $(BUILD_DIR)/$@ $^

$(BUILD_DIR)/$(SRC_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@
	
$(BUILD_DIR)/%_comp.spv: $(SHADER_DIR)/%.comp
	@mkdir -p $(dir $@)
	$(CSHADER) $< -o $@

print:
	@echo $(BUILD_DIR)
	@echo $(SHADER_DIR)
	@echo $(SHADERS)
	@echo $(SPIRVS)

-include $(DEPS)

clean:
	rm -rf $(BUILD_DIR)

Makefile: ;

.PHONY: all clean

