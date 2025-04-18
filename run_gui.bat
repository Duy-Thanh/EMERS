@echo off
setlocal enabledelayedexpansion

REM Check if Java is installed
where java >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo Error: Java not found. Please install Java and try again.
    pause
    exit /b 1
)

REM Create bin directory if it doesn't exist
if not exist bin mkdir bin
if not exist bin\gui mkdir bin\gui

REM Set classpath for compilation and running
set CLASSPATH=bin;lib\*

REM Note: We have consolidated all data structure classes in DataUtils.java
REM This prevents duplicate class definitions and compatibility issues
REM between StockPredictGUI.java and DataMining.java

REM Compile the Java GUI files if needed
if not exist bin\gui\StockPredictGUI.class (
    echo Compiling GUI files...
    javac -d bin -cp %CLASSPATH% src\gui\DataUtils.java
    if %ERRORLEVEL% NEQ 0 (
        echo Failed to compile DataUtils.java. Please check the error messages above.
        pause
        exit /b 1
    )
    
    javac -d bin -cp %CLASSPATH% src\gui\DataMining.java
    if %ERRORLEVEL% NEQ 0 (
        echo Failed to compile DataMining.java. Please check the error messages above.
        pause
        exit /b 1
    )
    
    javac -d bin -cp %CLASSPATH% src\gui\StockPredictGUI.java
    if %ERRORLEVEL% NEQ 0 (
        echo Failed to compile StockPredictGUI.java. Please check the error messages above.
        pause
        exit /b 1
    )
    
    echo Compilation successful!
)

REM Run the application
echo Starting StockPredict GUI...
java -cp %CLASSPATH% gui.StockPredictGUI

REM Keep console window open
pause 