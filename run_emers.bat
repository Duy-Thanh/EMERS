@echo off
echo Running Emergency Market Event Response System (EMERS)

REM ========= CONFIGURATION SETTINGS =========
REM Replace these API keys with your valid, working keys
set TIINGO_API_KEY=replace_with_your_tiingo_api_key
set MARKETAUX_API_KEY=replace_with_your_marketaux_api_key

REM Set the symbols to analyze (comma-separated)
set SYMBOLS=AAPL,MSFT,GOOG,AMZN,TSLA,SPY

REM Set date range (optional, will use defaults if not specified)
REM Format: YYYY-MM-DD
set START_DATE=2023-01-01
set END_DATE=

REM ========= EXECUTION =========
echo Verifying API keys...
if "%TIINGO_API_KEY%"=="replace_with_your_tiingo_api_key" (
    echo WARNING: You need to edit this batch file and add your actual Tiingo API key
    echo The placeholder value is still being used
    pause
    exit /b 1
)

if "%MARKETAUX_API_KEY%"=="replace_with_your_marketaux_api_key" (
    echo WARNING: You need to edit this batch file and add your actual MarketAux API key
    echo The placeholder value is still being used
    pause
    exit /b 1
)

REM Create data directory if it doesn't exist
if not exist "data" mkdir data

REM Run the EMERS program with the API keys, symbols and date range
echo Executing EMERS with the following settings:
echo - Symbols: %SYMBOLS%
echo - Date Range: %START_DATE% to %END_DATE% (current date if blank)
echo.
echo Running analysis...

REM Execute with date range if specified
if not "%START_DATE%"=="" (
    if not "%END_DATE%"=="" (
        bin\emers.exe -k %TIINGO_API_KEY% -m %MARKETAUX_API_KEY% -s %SYMBOLS% --start-date %START_DATE% --end-date %END_DATE%
    ) else (
        bin\emers.exe -k %TIINGO_API_KEY% -m %MARKETAUX_API_KEY% -s %SYMBOLS% --start-date %START_DATE%
    )
) else (
    bin\emers.exe -k %TIINGO_API_KEY% -m %MARKETAUX_API_KEY% -s %SYMBOLS%
)

echo.
echo Execution complete. Check emers_log.txt for details.
echo.
echo If you encountered errors:
echo - Verify your API keys have the proper permissions
echo - Consider using a shorter date range (1-2 years instead of 10)
echo - Check the symbols are valid
pause