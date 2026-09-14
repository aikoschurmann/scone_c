# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -Iinclude -Iout/generated

# Directories
SRC_DIR = src
OUT_DIR = out
GEN_DIR = $(OUT_DIR)/generated
VENDOR_DIR = vendor
SCHEMA_DIR = schemas

# Files
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(OUT_DIR)/%.o, $(SRCS))

# Dynamically find all schemas and map them to generated headers
SCHEMAS = $(wildcard $(SCHEMA_DIR)/*.schema)
GEN_HEADERS = $(patsubst $(SCHEMA_DIR)/%.schema, $(GEN_DIR)/%.h, $(SCHEMAS))

TARGET = $(OUT_DIR)/scone

# Tools
CFG_GEN = $(VENDOR_DIR)/cfgsafe/tools/out/cfg-gen

.PHONY: all clean tools run help

# Default target
help:
	@echo "Available commands:"
	@echo "  make        - Build the project and tools (alias for 'make all')"
	@echo "  make all    - Build the cfgsafe tools and the scone executable"
	@echo "  make run    - Build the project and execute it"
	@echo "  make clean  - Remove all built files (objects, generated headers, binaries)"
	@echo "  make tools  - Force rebuild the cfgsafe code generator"

all: tools $(GEN_HEADERS) $(TARGET)

# Main executable depends on object files
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

# Object files depend on ALL generated headers to ensure they exist before compiling
$(OUT_DIR)/%.o: $(SRC_DIR)/%.c $(GEN_HEADERS) | $(OUT_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Create output directories
$(OUT_DIR):
	mkdir -p $(OUT_DIR)
$(GEN_DIR):
	mkdir -p $(GEN_DIR)

# Tool building
tools:
	$(MAKE) -C $(VENDOR_DIR)/cfgsafe/tools

# Universal pattern rule to generate a .h file for any .schema file
$(GEN_DIR)/%.h: $(SCHEMA_DIR)/%.schema tools | $(GEN_DIR)
	$(CFG_GEN) $< -o $@

# Run the executable
run: all
	./$(TARGET)

clean:
	rm -rf $(OUT_DIR)/*
	$(MAKE) -C $(VENDOR_DIR)/cfgsafe/tools clean
