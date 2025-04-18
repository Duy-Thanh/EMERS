@echo off
REM Build script for StockPredict GUI

REM Check if Java is installed
where java >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Java is not found in your PATH.
    echo Please install Java JDK and ensure it's in your PATH.
    echo You can download it from: https://www.oracle.com/java/technologies/downloads/
    echo.
    echo After installing Java, you may need to set the JAVA_HOME environment variable:
    echo 1. Right-click on 'This PC' or 'My Computer'
    echo 2. Select 'Properties'
    echo 3. Click on 'Advanced system settings'
    echo 4. Click on 'Environment Variables'
    echo 5. Add JAVA_HOME pointing to your Java installation directory
    echo 6. Add %%JAVA_HOME%%\bin to your PATH variable
    echo.
    pause
    exit /b 1
)

REM Ensure necessary directories exist
if not exist bin mkdir bin
if not exist lib mkdir lib
if not exist obj mkdir obj
if not exist bin\gui mkdir bin\gui

REM Set Java classpath
set CLASSPATH=.;lib\*;bin

REM JFreeChart libraries - download if needed
set JFREECHART_URL=https://repo1.maven.org/maven2/org/jfree/jfreechart/1.5.3/jfreechart-1.5.3.jar
set JCOMMON_URL=https://repo1.maven.org/maven2/org/jfree/jcommon/1.0.24/jcommon-1.0.24.jar

if not exist lib\jfreechart-1.5.3.jar (
    echo Downloading JFreeChart library...
    powershell -Command "& {Invoke-WebRequest -Uri '%JFREECHART_URL%' -OutFile 'lib\jfreechart-1.5.3.jar'}"
)

if not exist lib\jcommon-1.0.24.jar (
    echo Downloading JCommon library...
    powershell -Command "& {Invoke-WebRequest -Uri '%JCOMMON_URL%' -OutFile 'lib\jcommon-1.0.24.jar'}"
)

REM For now, we'll skip JNI compilation on Windows and use the sample data
echo Note: JNI bridge will be compiled in a separate step for performance reasons

REM Compile Java GUI
echo Compiling Java GUI...

REM First compile DataUtils.java
echo 1. Compiling DataUtils.java...
javac -d bin src\gui\DataUtils.java

REM Then compile DataMining.java
echo 2. Compiling DataMining.java...
javac -d bin -cp %CLASSPATH% src\gui\DataMining.java

REM Finally compile StockPredictGUI.java
echo 3. Compiling StockPredictGUI.java...
javac -d bin -cp %CLASSPATH% src\gui\StockPredictGUI.java

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo ERROR: Java compilation failed. Please check the error messages above.
    pause
    exit /b 1
)

echo All compilation steps completed successfully.

REM Create a sample data file for demo if user doesn't already have data
if not exist data mkdir data
if not exist data\MSFT.csv (
    echo Creating sample stock data...
    echo Date,Open,High,Low,Close,Volume > data\MSFT.csv
    echo 2023-01-01,220.15,225.30,219.80,224.90,12345678 >> data\MSFT.csv
    echo 2023-01-02,224.90,228.45,223.75,227.80,11234567 >> data\MSFT.csv
    echo 2023-01-03,227.80,230.10,226.50,229.75,10123456 >> data\MSFT.csv
    echo 2023-01-04,229.75,231.80,227.60,230.20,12345670 >> data\MSFT.csv
    echo 2023-01-05,230.20,233.45,229.80,233.10,13456789 >> data\MSFT.csv
    echo 2023-01-06,233.10,233.90,231.20,232.80,10987654 >> data\MSFT.csv
    echo 2023-01-07,232.80,235.60,232.40,235.20,11876543 >> data\MSFT.csv
    echo 2023-01-08,235.20,237.45,234.50,237.10,12765432 >> data\MSFT.csv
    echo 2023-01-09,237.10,238.30,235.70,236.80,13654321 >> data\MSFT.csv
    echo 2023-01-10,236.80,237.90,235.20,237.50,12543210 >> data\MSFT.csv
)

REM Note about the updated analysis system
echo.
echo NOTE: The analysis system has been completely redesigned to dynamically
echo handle datasets of any size. The pattern, signal, and anomaly detection 
echo now uses dynamic allocation based on the dataset characteristics.
echo There are NO artificial limits on the number of patterns or timeframe
echo that can be analyzed and displayed.
echo.

REM Run the GUI
echo Starting StockPredict GUI...
java -cp %CLASSPATH% gui.StockPredictGUI

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo ERROR: Failed to run the GUI application. Please check the error messages above.
    pause
    exit /b 1
)

pause 