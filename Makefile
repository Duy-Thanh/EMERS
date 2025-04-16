CC = gcc
CFLAGS = -Wall -Wextra -g -std=c99
LDFLAGS = -lm

# Directories
SRC_DIR = src
INCLUDE_DIR = include
OBJ_DIR = obj
BIN_DIR = bin
TEST_DIR = test

# Files
SRC_FILES = $(wildcard $(SRC_DIR)/*.c)
OBJ_FILES = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC_FILES))
BIN_FILE = $(BIN_DIR)/emers
TEST_FILES = $(wildcard $(TEST_DIR)/test_*.c)
TEST_EXECUTABLES = $(patsubst $(TEST_DIR)/%.c,$(TEST_DIR)/%,$(TEST_FILES))

# Define specific test file targets
TECHNICAL_ANALYSIS_TEST = $(TEST_DIR)/test_technical_analysis
EXTENDED_INDICATORS_TEST = $(TEST_DIR)/test_extended_indicators
DATA_MINING_TEST = $(TEST_DIR)/test_data_mining
MODEL_VALIDATION_TEST = $(TEST_DIR)/test_model_validation

# Default target
all: $(BIN_DIR) $(OBJ_DIR) $(BIN_FILE)

# Create directories if they don't exist
$(BIN_DIR) $(OBJ_DIR):
	mkdir -p $@

# Compile source files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -I$(INCLUDE_DIR) -c $< -o $@

# Link the final executable
$(BIN_FILE): $(OBJ_FILES)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

# Build the test framework
$(TEST_DIR)/test_framework.o: $(TEST_DIR)/test_framework.c
	$(CC) $(CFLAGS) -I$(INCLUDE_DIR) -c $< -o $@

# Tests
test: test_technical test_extended_indicators test_mining test_model

# Technical Analysis Test
test_technical: $(TECHNICAL_ANALYSIS_TEST)
	$(TECHNICAL_ANALYSIS_TEST)

$(TECHNICAL_ANALYSIS_TEST): $(TEST_DIR)/test_technical_analysis.c $(TEST_DIR)/test_framework.o $(OBJ_DIR)/technical_analysis.o $(OBJ_DIR)/error_handling.o
	$(CC) $(CFLAGS) -I$(INCLUDE_DIR) $^ -o $@ $(LDFLAGS)

# Extended Indicators Test
test_extended_indicators: $(EXTENDED_INDICATORS_TEST)
	$(EXTENDED_INDICATORS_TEST)

$(EXTENDED_INDICATORS_TEST): $(TEST_DIR)/test_extended_indicators.c $(TEST_DIR)/test_framework.o $(OBJ_DIR)/technical_analysis.o $(OBJ_DIR)/error_handling.o
	$(CC) $(CFLAGS) -I$(INCLUDE_DIR) $^ -o $@ $(LDFLAGS)

# Data Mining Test
test_mining: $(DATA_MINING_TEST)
	$(DATA_MINING_TEST)

$(DATA_MINING_TEST): $(TEST_DIR)/test_data_mining.c $(TEST_DIR)/test_framework.o $(OBJ_DIR)/data_mining.o $(OBJ_DIR)/technical_analysis.o $(OBJ_DIR)/error_handling.o $(OBJ_DIR)/event_analysis.o
	$(CC) $(CFLAGS) -I$(INCLUDE_DIR) $^ -o $@ $(LDFLAGS)

# Model Validation Test
test_model: $(MODEL_VALIDATION_TEST)
	$(MODEL_VALIDATION_TEST)

$(MODEL_VALIDATION_TEST): $(TEST_DIR)/test_model_validation.c $(TEST_DIR)/test_framework.o $(OBJ_DIR)/model_validation.o $(OBJ_DIR)/technical_analysis.o $(OBJ_DIR)/error_handling.o
	$(CC) $(CFLAGS) -I$(INCLUDE_DIR) $^ -o $@ $(LDFLAGS)

# Run all tests
run_tests: test

# Clean up
clean:
	rm -f $(OBJ_FILES) $(BIN_FILE) $(TEST_EXECUTABLES) $(TEST_DIR)/*.o

# Clean and rebuild
rebuild: clean all

# Debug build
debug: CFLAGS += -DDEBUG
debug: all

# Optimized build
release: CFLAGS += -O3 -DNDEBUG
release: all

# Install
install: all
	mkdir -p /usr/local/bin
	cp $(BIN_FILE) /usr/local/bin/

# Uninstall
uninstall:
	rm -f /usr/local/bin/emers

.PHONY: all clean rebuild debug release install uninstall test test_technical test_extended_indicators test_mining test_model run_tests