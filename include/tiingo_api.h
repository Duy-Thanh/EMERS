/**
 * Tiingo API Integration Module
 * Header file for Tiingo API functions
 */

#ifndef TIINGO_API_H
#define TIINGO_API_H

#include "emers.h"

/* Tiingo API specific constants */
#define TIINGO_API_BASE_URL "https://api.tiingo.com"
#define TIINGO_API_DAILY_URL "/tiingo/daily"
#define TIINGO_API_NEWS_URL "/tiingo/news"
#define CSV_DATA_DIRECTORY "./data/"
#define MAX_PATH_LENGTH 256

/* NewsAPI.ai specific constants */
#define NEWSAPI_BASE_URL "https://api.newsapi.ai"
#define NEWSAPI_ARTICLES_URL "/api/v1/article/getArticles"
#define NEWSAPI_DEFAULT_FIELDS "title,body,sourceUri,publishDate,source.title"
#define NEWSAPI_DEFAULT_RESULT_TYPE "articles"
#define NEWSAPI_DEFAULT_ARTICLE_LIMIT 50

/* API and HTTP helper typedefs */
typedef struct {
    char* data;
    size_t size;
} Memory;

/* Function prototypes */

/* API initialization and setup */
int initializeTiingoAPI(const char* apiKey);
void setTiingoAPIKey(const char* apiKey);
const char* getTiingoAPIKey(void);

/* HTTP and API request functions */
char* buildAPIUrl(const char* endpoint, const char* params);
int performAPIRequest(const char* url, Memory* response);

/* Data retrieval functions */
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

/* Error handling */
void logAPIError(const char* message, const char* url, int statusCode);

#endif /* TIINGO_API_H */