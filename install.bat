@echo off
REM EMERS Installation Script for Windows

echo Emergency Market Event Response System (EMERS) Installation
echo ==========================================================

REM Default installation settings
set INSTALL_PREFIX=C:\Program Files\EMERS
set BUILD_TYPE=Release
set LOCAL_INSTALL=0

REM Process command line arguments
:parse_args
if "%~1"=="" goto :end_parse_args
if /i "%~1"=="--local" (
    set INSTALL_PREFIX=%USERPROFILE%\EMERS
    set LOCAL_INSTALL=1
    shift
    goto :parse_args
)
if /i "%~1"=="--debug" (
    set BUILD_TYPE=Debug
    shift
    goto :parse_args
)
if /i "%~1:~0,9%"=="--prefix=" (
    set INSTALL_PREFIX=%~1:~9%
    shift
    goto :parse_args
)
echo Unknown argument: %~1
echo Usage: install.bat [--local] [--debug] [--prefix=^<path^>]
exit /b 1

:end_parse_args

REM Check for administrative privileges if not local install
if %LOCAL_INSTALL%==0 (
    net session >nul 2>&1
    if %ERRORLEVEL% NEQ 0 (
        echo Please run as administrator for a system-wide installation, or use --local for a local installation
        echo Usage: Right-click install.bat and select "Run as administrator" or install.bat --local
        exit /b 1
    )
)

REM Check dependencies
echo Checking dependencies...
set MISSING_DEPS=0

where gcc >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo  - gcc not found!
    set MISSING_DEPS=1
) else (
    where gcc
)

where make >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo  - make not found!
    set MISSING_DEPS=1
) else (
    where make
)

where curl >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo  - curl not found!
    set MISSING_DEPS=1
) else (
    where curl
)

if %MISSING_DEPS%==1 (
    echo Please install the missing dependencies and try again.
    echo For gcc and make: Install MinGW or MSYS2
    echo For curl: Install curl from https://curl.se/windows/ or via choco install curl
    exit /b 1
)

REM Create build directory
echo Creating build directory...
if not exist build mkdir build
cd build

REM Build the project
echo Building EMERS...
make clean
set CFLAGS=-DEMERS_%BUILD_TYPE%
make

if %ERRORLEVEL% NEQ 0 (
    echo Build failed! Please check the error messages.
    exit /b 1
)

REM Create installation directories
echo Creating installation directories...
if not exist "%INSTALL_PREFIX%\bin" mkdir "%INSTALL_PREFIX%\bin"
if not exist "%INSTALL_PREFIX%\share\emers" mkdir "%INSTALL_PREFIX%\share\emers"
if not exist "%INSTALL_PREFIX%\share\doc\emers" mkdir "%INSTALL_PREFIX%\share\doc\emers"

REM Install binary
echo Installing EMERS binary...
copy /Y bin\emers.exe "%INSTALL_PREFIX%\bin\"

REM Install documentation
echo Installing documentation...
copy /Y ..\README.md "%INSTALL_PREFIX%\share\doc\emers\"

REM Create configuration directory
if %LOCAL_INSTALL%==1 (
    set CONFIG_DIR=%USERPROFILE%\AppData\Local\EMERS
) else (
    set CONFIG_DIR=%ProgramData%\EMERS
)

if not exist "%CONFIG_DIR%" mkdir "%CONFIG_DIR%"

REM Create default configuration file
echo Creating default configuration...
(
    echo # EMERS Configuration File
    echo.
    echo # API Settings
    echo api_key = 
    echo api_timeout = 30
    echo.
    echo # Data Settings
    echo default_lookback_days = 180
    echo cache_directory = %CONFIG_DIR%\cache
    echo.
    echo # Logging Settings
    echo log_level = info
    echo log_file = %CONFIG_DIR%\emers.log
    echo.
    echo # Model Settings
    echo event_detection_threshold = 0.75
    echo risk_confidence_level = 0.95
) > "%CONFIG_DIR%\emers.conf"

REM Create cache directory
if not exist "%CONFIG_DIR%\cache" mkdir "%CONFIG_DIR%\cache"

REM Create uninstall script
(
    echo @echo off
    echo REM EMERS Uninstall Script
    echo.
    echo echo Uninstalling EMERS...
    echo rmdir /s /q "%INSTALL_PREFIX%"
    echo.
    echo if exist "%CONFIG_DIR%" (
    echo     echo Configuration directory found at: %CONFIG_DIR%
    echo     set /p REMOVE=Do you want to remove it? [Y/N] 
    echo     if /i "%%REMOVE%%" == "Y" (
    echo         rmdir /s /q "%CONFIG_DIR%"
    echo         echo Configuration directory removed.
    echo     ^) else (
    echo         echo Configuration directory preserved.
    echo     ^)
    echo ^)
    echo.
    echo echo Uninstallation complete!
) > uninstall.bat

echo Installation complete!
echo EMERS binary installed to: %INSTALL_PREFIX%\bin\emers.exe
echo Configuration file created at: %CONFIG_DIR%\emers.conf
echo Please edit the configuration file to set your Tiingo API key.
echo.
echo Uninstall script created at: %CD%\uninstall.bat

REM Add to PATH if local install
if %LOCAL_INSTALL%==1 (
    echo Would you like to add EMERS to your PATH? [Y/N]
    set /p ADD_PATH=
    if /i "%ADD_PATH%"=="Y" (
        setx PATH "%PATH%;%INSTALL_PREFIX%\bin"
        echo EMERS added to your PATH.
    )
) 