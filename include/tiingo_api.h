/**
 * Tiingo API Integration Module
 * Header file for Tiingo API functions
 */

#ifndef TIINGO_API_H
#define TIINGO_API_H

#include "emers.h"
#include "event_database.h"

/* Tiingo API buffer sizes - don't redefine macros from emers.h */
#define MAX_BUFFER_SIZE     4096
#define MAX_PATH_LENGTH      512

/* Tiingo API URLs */
#define TIINGO_API_BASE_URL      "https://api.tiingo.com/"
#define TIINGO_API_DAILY_URL     "tiingo/daily"

/* MarketAux API definitions */
#define MARKETAUX_API_URL        "https://api.marketaux.com/v1/news/all"
#define MARKETAUX_API_KEY_LENGTH 64

/* Data storage */
#define CSV_DATA_DIRECTORY       "./data/"

/* Callback data structure for curl */
typedef struct {
    char *data;
    size_t size;
} Memory;

/* Function prototypes */

/* Initialization and configuration */
int initializeTiingoAPI(const char* apiKey);
void setTiingoAPIKey(const char* key);
const char* getTiingoAPIKey(void);

/* MarketAux API configuration */
void setMarketAuxAPIKey(const char* key);
const char* getMarketAuxAPIKey(void);

/* API request helpers */
char* buildAPIUrl(const char* endpoint, const char* params);
int performAPIRequest(const char* url, Memory* response);

/* API data fetching */
int fetchStockData(const char* symbol, const char* startDate, const char* endDate, Stock* stock);
int fetchNewsFeed(const char* symbols, EventDatabase* events);

/* CSV cache functions */
int saveStockDataToCSV(const Stock* stock, const char* startDate, const char* endDate);
int loadStockDataFromCSV(const char* symbol, const char* startDate, const char* endDate, Stock* stock);
int checkCSVDataExists(const char* symbol, const char* startDate, const char* endDate);
int fetchHistoricalDataWithCache(const char* symbol, const char* startDate, const char* endDate, Stock* stock);
char* generateCSVFilename(const char* symbol, const char* startDate, const char* endDate);

/* JSON parsing functions */
int parseStockDataJSON(const char* jsonData, Stock* stock);
int parseNewsDataJSON(const char* jsonData, EventDatabase* events);

/* Sentiment analysis and impact scoring functions */
double calculateSentiment(const char* title, const char* description);
double calculateImpactScore(const EventData* event);

/* Error handling */
void logAPIError(const char* message, const char* url, int statusCode);

#endif /* TIINGO_API_H */