# StockPredict GUI

A Java-based graphical user interface for visualizing StockPredict data mining results.

## Overview

This GUI provides a user-friendly interface to:

- Display stock price charts with OHLC data
- Visualize detected price patterns and trading signals
- Show technical indicators (SMA, EMA, MACD, RSI, etc.)
- Display detected anomalies in the data
- Export results for further analysis

## Architecture

The GUI is implemented using Java with Swing and JFreeChart for visualization. It connects to the StockPredict C codebase via JNI (Java Native Interface) for data processing and analysis.

The system consists of:

1. **Java GUI Layer (`src/gui/`)**
   - `StockPredictGUI.java`: Main GUI implementation with Swing components
   - `StockPredictJNIBridge.java`: JNI interface to the C code

2. **JNI Bridge (`src/jni/`)**
   - `stockpredict_jni.c`: C implementation of the JNI interface
   - `Makefile`: Compiles the JNI shared library

## Requirements

- Java JDK 8 or higher
- JFreeChart library (automatically downloaded by build scripts)
- C compiler (GCC or MinGW on Windows) for compiling the JNI bridge
- StockPredict C codebase (for data mining functionality)

## Building and Running

### On Windows:

1. Ensure Java JDK is installed and in your PATH
2. Run the build script:
   ```
   build_gui.bat
   ```

### On Linux/Unix:

1. Ensure Java JDK and GCC are installed
2. Make the build script executable:
   ```
   chmod +x build_gui.sh
   ```
3. Run the build script:
   ```
   ./build_gui.sh
   ```

## Usage

1. **Loading Data**
   - Click File → Open Data File to load historical stock data in CSV format
   - The data should have columns: Date, Open, High, Low, Close, Volume

2. **Running Analysis**
   - Click Analysis → Run Analysis to perform data mining on the loaded data
   - This will detect patterns, signals, and anomalies

3. **Viewing Results**
   - Use the tabs to switch between different views:
     - Price Chart: View the stock price chart
     - Price Patterns: View detected patterns
     - Trading Signals: View buy/sell signals
     - Anomalies: View detected anomalies
     - Technical Indicators: View and configure indicators

4. **Exporting Results**
   - Click File → Export Results to save analysis results

## Extending the GUI

To add new functionality to the GUI:

1. Add new methods to the JNI bridge for any new C functions
2. Update the Java GUI to utilize the new functionality
3. Rebuild the JNI bridge and recompile the Java code

## Notes for Developers

- The `convertStockDataToArrays` method in `StockPredictJNIBridge.java` handles conversion between Java objects and C data structures
- Error handling is implemented throughout the JNI bridge to prevent crashes
- Memory management in the JNI C code uses proper allocation and deallocation to prevent memory leaks 