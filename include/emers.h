/**
 * @file emers.h
 * @brief Main header file for Emergency Market Event Response System
 */

#ifndef EMERS_H
#define EMERS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <stdbool.h>

/* Constants */
#define MAX_URL_LENGTH 256
#define MAX_BUFFER_SIZE 4096
#define MAX_SYMBOL_LENGTH 16
#define MAX_DATE_LENGTH 20
#define MAX_API_KEY_LENGTH 64

/* Data Structures */
typedef struct {
    char date[MAX_DATE_LENGTH];
    double open;
    double high;
    double low;
    double close;
    double volume;
    double adjClose;
} StockData;

typedef struct {
    char symbol[MAX_SYMBOL_LENGTH];
    StockData* data;
    int dataSize;
    int dataCapacity;
} Stock;

typedef struct {
    char title[MAX_BUFFER_SIZE];
    char description[MAX_BUFFER_SIZE];
    char date[MAX_DATE_LENGTH];
    double sentiment;
    int impactScore;
} EventData;

typedef struct {
    EventData* events;
    int eventCount;
    int eventCapacity;
} EventDatabase;

typedef struct {
    double sma;       // Simple Moving Average
    double ema;       // Exponential Moving Average
    double rsi;       // Relative Strength Index
    double macd;      // Moving Average Convergence Divergence
    double macdSignal;
    double macdHistogram;
    double bollingerUpper;  // Bollinger Bands
    double bollingerMiddle;
    double bollingerLower;
    double atr;       // Average True Range
} TechnicalIndicators;

/**
 * @enum EventType
 * @brief Types of market events that can be detected and analyzed
 */
typedef enum {
    MERGER_ACQUISITION,
    EARNINGS_REPORT,  
    CORPORATE_SCANDAL,
    LEADERSHIP_CHANGE,
    STOCK_SPLIT,
    IPO,
    LAYOFFS,
    PRODUCT_LAUNCH,
    PARTNERSHIP,
    REGULATORY_CHANGE,
    UNKNOWN_EVENT
} EventType;

/**
 * @struct MarketEvent
 * @brief Structure to represent a detected market event
 */
typedef struct {
    EventType type;
    char description[256];
    time_t timestamp;
    char affectedStocks[10][16]; // Up to 10 stock symbols
    int affectedStockCount;
    int impactScore;          // -10 to 10 scale
    char source[64];          // Information source
} MarketEvent;

/**
 * @struct AlertConfig
 * @brief Configuration for alerts
 */
typedef struct {
    double priceChangeThreshold;    // Percentage
    int volumeChangeThreshold;      // Percentage
    bool alertOnEarnings;
    bool alertOnMergers;
    int severityThreshold;          // 1-10 scale
    char targetStocks[20][16];      // Up to 20 stock symbols
    int targetStockCount;
} AlertConfig;

/* Function prototypes */

// API Functions
int initializeTiingoAPI(const char* apiKey);
int fetchStockData(const char* symbol, const char* startDate, const char* endDate, Stock* stock);
int fetchNewsFeed(const char* symbols, EventDatabase* events);

// Technical Analysis
void calculateSMA(const StockData* data, int dataSize, int period, double* output);
void calculateEMA(const StockData* data, int dataSize, int period, double* output);
void calculateRSI(const StockData* data, int dataSize, int period, double* output);
void calculateMACD(const StockData* data, int dataSize, int fastPeriod, int slowPeriod, int signalPeriod, 
                   double* macdLine, double* signalLine, double* histogram);
void calculateBollingerBands(const StockData* data, int dataSize, int period, double stdDevMultiplier,
                            double* upperBand, double* middleBand, double* lowerBand);
void calculateATR(const StockData* data, int dataSize, int period, double* output);
void calculateAllIndicators(const StockData* data, int dataSize, TechnicalIndicators* indicators);

// Event Detection and Analysis
int detectMarketEvents(const Stock* stocks, int stockCount, const EventDatabase* newsEvents, EventData* detectedEvents);
double calculateEventImpact(const EventData* event, const Stock* stocks, int stockCount);
EventType classifyEvent(const EventData* event);

// Risk Management
double calculateValueAtRisk(const Stock* stock, double confidenceLevel, int timeHorizon);
double calculateMaxDrawdown(const StockData* data, int dataSize);
void generateDefensiveStrategy(const Stock* stocks, int stockCount, const EventData* event, char* strategyOutput);

// Utility Functions
void initializeStock(Stock* stock, const char* symbol);
void freeStock(Stock* stock);
void initializeEventDatabase(EventDatabase* db);
void freeEventDatabase(EventDatabase* db);
char* getCurrentDate(char* buffer);
char* getPastDate(char* buffer, int daysBack);

/**
 * @brief Initializes the EMERS system
 * 
 * @param configFile Path to the configuration file
 * @return true if initialization succeeded, false otherwise
 */
bool initEMERS(const char *configFile);

/**
 * @brief Cleans up and shuts down the EMERS system
 */
void cleanupEMERS(void);

/**
 * @brief Process a new market event
 * 
 * @param event The event to process
 * @return true if processing succeeded, false otherwise
 */
bool processEvent(const MarketEvent *event);

/**
 * @brief Generate alerts based on current market events and configurations
 * 
 * @param config Alert configuration
 * @param events Array of events to consider
 * @param eventCount Number of events
 * @return int Number of alerts generated
 */
int generateAlerts(const AlertConfig *config, const MarketEvent *events, int eventCount);

/**
 * @brief Update internal market data with new stock information
 * 
 * @param data Array of stock data
 * @param dataCount Number of stock data entries
 * @return true if update succeeded, false otherwise
 */
bool updateMarketData(const StockData *data, int dataCount);

// Include subsystem headers that actually exist
#include "event_analysis.h"
#include "tiingo_api.h"
#include "model_validation.h"
#include "text_analysis.h"
#include "error_handling.h"
#include "event_database.h"
#include "technical_analysis.h"
#include "asm_optimize.h"

#endif /* EMERS_H */ 