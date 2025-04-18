/**
 * Emergency Market Event Response System (EMERS)
 * Main application file
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../include/emers.h"
#include "../include/tiingo_api.h"
#include "../include/technical_analysis.h"
#include "../include/error_handling.h"

#define MAX_STOCKS 100
#define MAX_SYMBOL_LENGTH 16
#define MAX_DETECTED_EVENTS 20
#define DEFAULT_LOOKBACK_DAYS 3650  // 10 years of historical data (approximately)

/* Function prototypes */
void printUsage(const char* programName);
void printStock(const Stock* stock);
void printTechnicalIndicators(const TechnicalIndicators* indicators);
void printExtendedTechnicalIndicators(const ExtendedTechnicalIndicators* indicators);
void analyzeStock(const Stock* stock);

int main(int argc, char* argv[]) {
    char apiKey[MAX_API_KEY_LENGTH] = "";
    char symbols[MAX_STOCKS][MAX_SYMBOL_LENGTH];
    int symbolCount = 0;
    char startDate[MAX_DATE_LENGTH] = "";
    char endDate[MAX_DATE_LENGTH] = "";
    int i;

    /* Initialize error handling */
    initErrorHandling("emers_log.txt", LOG_DEBUG, LOG_INFO);
    
    printf("Emergency Market Event Response System (EMERS)\n");
    
    /* Parse command line arguments */
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-k") == 0 || strcmp(argv[i], "--api-key") == 0) {
            if (i + 1 < argc) {
                strncpy(apiKey, argv[i + 1], MAX_API_KEY_LENGTH - 1);
                apiKey[MAX_API_KEY_LENGTH - 1] = '\0';
                i++;
            }
        } else if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--symbols") == 0) {
            if (i + 1 < argc) {
                char* token = strtok(argv[i + 1], ",");
                while (token != NULL && symbolCount < MAX_STOCKS) {
                    strncpy(symbols[symbolCount], token, MAX_SYMBOL_LENGTH - 1);
                    symbols[symbolCount][MAX_SYMBOL_LENGTH - 1] = '\0';
                    symbolCount++;
                    token = strtok(NULL, ",");
                }
                i++;
            }
        } else if (strcmp(argv[i], "--start-date") == 0) {
            if (i + 1 < argc) {
                strncpy(startDate, argv[i + 1], MAX_DATE_LENGTH - 1);
                startDate[MAX_DATE_LENGTH - 1] = '\0';
                i++;
            }
        } else if (strcmp(argv[i], "--end-date") == 0) {
            if (i + 1 < argc) {
                strncpy(endDate, argv[i + 1], MAX_DATE_LENGTH - 1);
                endDate[MAX_DATE_LENGTH - 1] = '\0';
                i++;
            }
        } else if (strcmp(argv[i], "-m") == 0 || strcmp(argv[i], "--marketaux-key") == 0) {
            /* Skip this parameter and its value - news functionality is now in Java GUI */
            if (i + 1 < argc) {
                i++; /* Skip the value */
            }
            printf("Note: News functionality is now handled by the Java GUI. The -m parameter is ignored.\n");
        } else {
            printf("Unknown option: %s\n", argv[i]);
            printUsage(argv[0]);
            return 1;
        }
    }
    
    /* Validate required parameters */
    if (strlen(apiKey) == 0 || symbolCount == 0) {
        printf("Error: API key and at least one symbol are required.\n");
        printUsage(argv[0]);
        return 1;
    }
    
    /* Use default dates if not provided */
    if (strlen(endDate) == 0) {
        char buffer[MAX_DATE_LENGTH];
        getCurrentDate(buffer);
        strncpy(endDate, buffer, MAX_DATE_LENGTH - 1);
        endDate[MAX_DATE_LENGTH - 1] = '\0';
    }
    
    if (strlen(startDate) == 0) {
        char buffer[MAX_DATE_LENGTH];
        getPastDate(buffer, DEFAULT_LOOKBACK_DAYS);  /* Default: last 10 years */
        strncpy(startDate, buffer, MAX_DATE_LENGTH - 1);
        startDate[MAX_DATE_LENGTH - 1] = '\0';
    }
    
    /* Initialize API */
    if (!initializeTiingoAPI(apiKey)) {
        return 1;
    }
    
    /* Initialize stocks */
    Stock stocks[MAX_STOCKS];
    
    /* Process each stock */
    for (i = 0; i < symbolCount; i++) {
        printf("\nAnalyzing stock: %s\n", symbols[i]);
        
        /* Initialize stock structure */
        initializeStock(&stocks[i], symbols[i]);
        
        /* Fetch historical data */
        if (!fetchHistoricalDataWithCache(symbols[i], startDate, endDate, &stocks[i])) {
            printf("Error: Failed to fetch data for %s.\n", symbols[i]);
            continue;
        }
        
        printf("Retrieved %d data points for %s.\n", stocks[i].dataSize, symbols[i]);
        
        /* Analyze the stock */
        analyzeStock(&stocks[i]);
    }
    
    /* Clean up */
    for (i = 0; i < symbolCount; i++) {
        freeStock(&stocks[i]);
    }
    
    /* Cleanup error handling */
    cleanupErrorHandling();
    
    return 0;
}

void analyzeStock(const Stock* stock) {
    if (!stock) {
        return;
    }
    
    printf("\nAnalyzing stock: %s\n", stock->symbol);
    printf("Retrieved %d data points for %s.\n", stock->dataSize, stock->symbol);
    
    /* Calculate technical indicators */
    ExtendedTechnicalIndicators* indicators = (ExtendedTechnicalIndicators*)malloc(sizeof(ExtendedTechnicalIndicators));
    if (!indicators) {
        printf("Error: Memory allocation failed for technical indicators\n");
        return;
    }
    
    /* Most recent date in the stock data */
    const char* latestDate = stock->data[stock->dataSize - 1].date;
    
    /* Calculate all extended technical indicators */
    calculateExtendedIndicators(stock->data, stock->dataSize, indicators);
    
    printf("\nTechnical indicators for %s (%s):\n", 
           stock->symbol, latestDate);
    printExtendedTechnicalIndicators(indicators);
    
    free(indicators);
}

void printUsage(const char* programName) {
    printf("Usage: %s -k API_KEY -s SYMBOLS [options]\n\n", programName);
    printf("Options:\n");
    printf("  -k, --api-key KEY       Tiingo API key (for market data)\n");
    printf("  -s, --symbols SYM1,SYM2 Comma-separated list of stock symbols\n");
    printf("  --start-date DATE       Start date (YYYY-MM-DD), default is 10 years ago\n");
    printf("  --end-date DATE         End date (YYYY-MM-DD), default is today\n");
    printf("\nNote: News analysis and data mining are now handled by the Java GUI.\n");
    printf("      Use run_gui.bat to access these features.\n");
}

void printStock(const Stock* stock) {
    printf("Symbol: %s\n", stock->symbol);
    printf("Data points: %d\n", stock->dataSize);
    
    if (stock->dataSize > 0) {
        printf("Latest price data (%s):\n", stock->data[stock->dataSize - 1].date);
        printf("  Open:  %.2f\n", stock->data[stock->dataSize - 1].open);
        printf("  High:  %.2f\n", stock->data[stock->dataSize - 1].high);
        printf("  Low:   %.2f\n", stock->data[stock->dataSize - 1].low);
        printf("  Close: %.2f\n", stock->data[stock->dataSize - 1].close);
        printf("  Volume: %.0f\n", stock->data[stock->dataSize - 1].volume);
    }
}

void printTechnicalIndicators(const TechnicalIndicators* indicators) {
    if (!indicators) {
        return;
    }
    
    printf("SMA(20): %.2f\n", indicators->sma);
    printf("EMA(14): %.2f\n", indicators->ema);
    printf("RSI(14): %.2f\n", indicators->rsi);
    printf("MACD: %.2f Signal: %.2f Histogram: %.2f\n", 
           indicators->macd, indicators->macdSignal, indicators->macdHistogram);
    printf("Bollinger Bands: Upper %.2f Middle %.2f Lower %.2f\n", 
           indicators->bollingerUpper, indicators->bollingerMiddle, indicators->bollingerLower);
    printf("ATR: %.2f\n", indicators->atr);
}

/* Print extended technical indicators */
void printExtendedTechnicalIndicators(const ExtendedTechnicalIndicators* indicators) {
    if (!indicators) {
        return;
    }
    
    /* Print basic indicators */
    printf("=== BASIC INDICATORS ===\n");
    printf("SMA(20): %.2f\n", indicators->sma);
    printf("EMA(14): %.2f\n", indicators->ema);
    printf("RSI(14): %.2f\n", indicators->rsi);
    printf("MACD: %.2f Signal: %.2f Histogram: %.2f\n", 
           indicators->macd, indicators->macdSignal, indicators->macdHistogram);
    printf("Bollinger Bands: Upper %.2f Middle %.2f Lower %.2f\n", 
           indicators->bollingerUpper, indicators->bollingerMiddle, indicators->bollingerLower);
    printf("ATR: %.2f\n\n", indicators->atr);
    
    /* Print advanced indicators */
    printf("=== ADVANCED INDICATORS ===\n");
    printf("ADX: %.2f +DI: %.2f -DI: %.2f\n", 
           indicators->adx, indicators->diPlus, indicators->diMinus);
    printf("Stochastic %%K: %.2f %%D: %.2f\n", 
           indicators->stochasticK, indicators->stochasticD);
    printf("Money Flow Index: %.2f\n", indicators->mfi);
    printf("Parabolic SAR: %.2f\n\n", indicators->psar);
}