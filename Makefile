# ============================================================
#  Lumis Mini Compiler — Makefile
# ============================================================
#  Builds the Lumis compiler using Flex (lexer) and Bison (parser).
#
#  Targets:
#     make         - build the compiler
#     make clean   - remove generated files
#     make test    - run all sample programs
# ============================================================

# Compiler and flags
CC      = gcc
CFLAGS  = -Wall -g -std=c99 -D_POSIX_C_SOURCE=200809L -D_DEFAULT_SOURCE
LDFLAGS =

# Tools
FLEX  = flex
BISON = bison

# Source layout
SRC_DIR = src
BUILD_DIR = build

# Generated files (created by flex/bison)
LEXER_C   = $(BUILD_DIR)/lex.yy.c
PARSER_C  = $(BUILD_DIR)/parser.tab.c
PARSER_H  = $(BUILD_DIR)/parser.tab.h

# Our own C sources (exclude the generated ones)
SRCS  = $(SRC_DIR)/main.c \
        $(SRC_DIR)/ast.c \
        $(SRC_DIR)/symtab.c \
        $(SRC_DIR)/semantic.c \
        $(SRC_DIR)/interp.c

OBJS  = $(SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
# The parser/lexer .c files live in build/, so handle them too
PARSER_OBJ = $(BUILD_DIR)/parser.tab.o
LEXER_OBJ  = $(BUILD_DIR)/lex.yy.o

# Output executable
TARGET = lumis

# Default rule
all: $(TARGET)

# Link the final binary
$(TARGET): $(OBJS) $(PARSER_OBJ) $(LEXER_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo ""
	@echo "✓ Build complete. Run: ./$(TARGET) <file.lum>"
	@echo ""

# Compile each .c into build/
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c $(PARSER_H) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -I$(BUILD_DIR) -I$(SRC_DIR) -c $< -o $@

# Compile the generated parser.c
$(PARSER_OBJ): $(PARSER_C) $(PARSER_H) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -I$(BUILD_DIR) -I$(SRC_DIR) -c $< -o $@

# Compile the generated lexer.c
$(LEXER_OBJ): $(LEXER_C) $(PARSER_H) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -I$(BUILD_DIR) -I$(SRC_DIR) -c $< -o $@

# Run Bison to generate the parser
$(PARSER_C) $(PARSER_H): $(SRC_DIR)/parser.y | $(BUILD_DIR)
	$(BISON) -d -o $(PARSER_C) --defines=$(PARSER_H) $<

# Run Flex to generate the lexer
$(LEXER_C): $(SRC_DIR)/lexer.l $(PARSER_H) | $(BUILD_DIR)
	$(FLEX) -o $@ $<

# Make sure build/ exists
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Clean up generated files
clean:
	rm -rf $(BUILD_DIR) $(TARGET)
	@echo "✓ Clean complete."

# Run all sample programs
test: $(TARGET)
	@echo ""
	@echo "=== VALID PROGRAMS ==="
	@for f in tests/valid/*.lum; do \
		echo ""; \
		echo "--- $$f ---"; \
		./$(TARGET) $$f || echo "(failed)"; \
	done
	@echo ""
	@echo "=== INVALID PROGRAMS (expect errors) ==="
	@for f in tests/invalid/*.lum; do \
		echo ""; \
		echo "--- $$f ---"; \
		./$(TARGET) $$f || true; \
	done

.PHONY: all clean test
