# Makefile for StockPredict JNI Bridge

# Java and JNI settings
JAVA_HOME := $(shell javac -version 2>&1 | grep -oP '(?<=javac\s)(\d+)' | xargs -I{} echo /usr/lib/jvm/java-{}-openjdk)
JNI_INCLUDES := -I$(JAVA_HOME)/include -I$(JAVA_HOME)/include/linux

# C compiler settings
CC := gcc
CFLAGS := -Wall -Wextra -fPIC -O2 -g
LDFLAGS := -shared

# Source files
JNI_SRC := stockpredict_jni.c
TARGET := libstockpredict_jni.so

# StockPredict library paths
STOCKPREDICT_SRC := ../../src
STOCKPREDICT_INCLUDE := ../../include
STOCKPREDICT_OBJ := ../../obj

# Include paths
INCLUDES := -I$(STOCKPREDICT_INCLUDE) $(JNI_INCLUDES)

# Source files from StockPredict library
SRC_FILES := $(wildcard $(STOCKPREDICT_SRC)/*.c)
OBJS := $(patsubst $(STOCKPREDICT_SRC)/%.c,$(STOCKPREDICT_OBJ)/%.o,$(SRC_FILES))

# Default target
all: header $(TARGET)

# Generate JNI header file
header:
	@echo "Generating JNI header file..."
	@if [ ! -d ../../bin/gui ]; then \
		mkdir -p ../../bin/gui; \
	fi
	javac -d ../../bin ../gui/StockPredictJNIBridge.java
	javah -o gui_StockPredictJNIBridge.h -classpath ../../bin gui.StockPredictJNIBridge

# Build the shared library
$(TARGET): $(JNI_SRC) $(OBJS)
	@echo "Building JNI shared library..."
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^ $(LDFLAGS)

# Clean up
clean:
	rm -f $(TARGET) gui_StockPredictJNIBridge.h

# Install the shared library to a standard location
install: all
	@echo "Installing JNI shared library..."
	mkdir -p $(HOME)/lib
	cp $(TARGET) $(HOME)/lib/

.PHONY: all clean install header 