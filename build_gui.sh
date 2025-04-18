#!/bin/bash

# Build script for StockPredict GUI

# Ensure necessary directories exist
mkdir -p bin
mkdir -p lib
mkdir -p obj

# Set Java classpath
CLASSPATH=".:lib/*:bin"

# JFreeChart libraries - download if needed
JFREECHART_URL="https://repo1.maven.org/maven2/org/jfree/jfreechart/1.5.3/jfreechart-1.5.3.jar"
JCOMMON_URL="https://repo1.maven.org/maven2/org/jfree/jcommon/1.0.24/jcommon-1.0.24.jar"

if [ ! -f "lib/jfreechart-1.5.3.jar" ]; then
    echo "Downloading JFreeChart library..."
    curl -L -o lib/jfreechart-1.5.3.jar $JFREECHART_URL
fi

if [ ! -f "lib/jcommon-1.0.24.jar" ]; then
    echo "Downloading JCommon library..."
    curl -L -o lib/jcommon-1.0.24.jar $JCOMMON_URL
fi

# Compile C code if needed
echo "Building C library..."
cd src && make clean && make && cd ..

# Build JNI bridge
echo "Building JNI bridge..."
cd src/jni && make clean && make && make install && cd ../..

# Compile Java GUI
echo "Compiling Java GUI..."
javac -d bin -cp $CLASSPATH src/gui/*.java

# Run the GUI
echo "Starting StockPredict GUI..."
java -cp $CLASSPATH -Djava.library.path=$HOME/lib gui.StockPredictGUI 