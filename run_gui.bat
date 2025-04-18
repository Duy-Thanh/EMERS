@echo off
REM Run script for StockPredict GUI

REM Check if Java is installed
where java >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Java is not found in your PATH.
    echo Please install Java and ensure it's in your PATH.
    echo You can download it from: https://www.oracle.com/java/technologies/downloads/
    pause
    exit /b 1
)

REM Check if we need to build
if not exist bin\gui\StockPredictGUI.class (
    echo GUI not built yet. Building first...
    call build_gui.bat
    if %ERRORLEVEL% NEQ 0 (
        echo Build failed. Please fix errors and try again.
        pause
        exit /b 1
    )
)

REM Set Java classpath
set CLASSPATH=bin;lib\*

REM Run the application
echo Starting StockPredict GUI...
java -cp %CLASSPATH% gui.StockPredictGUI

pause 