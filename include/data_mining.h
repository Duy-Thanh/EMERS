/**
 * Data Mining Module
 * Header file for pattern recognition and data mining functions
 */

#ifndef DATA_MINING_H
#define DATA_MINING_H

#include "emers.h"
#include "event_database.h"
#include "event_analysis.h"

/* Constants */
#define MAX_PATTERNS 10
#define MAX_ANOMALIES 10
#define MAX_SIMILAR_EVENTS 10
#define MAX_STOCKS 20
#define MAX_HISTORICAL_YEARS 10

/* Pattern types */
typedef enum {
    PATTERN_DOUBLE_TOP,
    PATTERN_DOUBLE_BOTTOM,
    PATTERN_HEAD_AND_SHOULDERS,
    PATTERN_TRIANGLE,
    PATTERN_FLAG,
    PATTERN_WEDGE,
    PATTERN_CHANNEL,
    PATTERN_UNKNOWN
} PatternType;

/* Pattern detection result */
typedef struct {
    PatternType type;
    char description[128];
    int startIndex;
    int endIndex;
    double confidence;
    double expectedMove;  /* As percentage of current price */
} PatternResult;

/* Similar historical event with outcome data */
typedef struct {
    EventData eventData;
    double similarityScore;
    double priceChangeAfterEvent;
    int daysToRecovery;
} SimilarHistoricalEvent;

/* Historical data mining result */
typedef struct {
    char symbol[MAX_SYMBOL_LENGTH];
    double meanReturn;           /* Average daily return */
    double annualizedReturn;     /* Annualized return */
    double volatility;           /* Historical volatility */
    double maxDrawdown;          /* Maximum drawdown */
    double sharpeRatio;          /* Sharpe ratio */
    double correlationWithMarket; /* Correlation with market index */
    int totalTradingDays;        /* Total data points analyzed */
    int patterns[8];             /* Count of each pattern type */
    double avgPatternReturn;     /* Average return following pattern formation */
    double bestDay;              /* Best single-day return */
    double worstDay;             /* Worst single-day return */
    char bestDayDate[MAX_DATE_LENGTH];
    char worstDayDate[MAX_DATE_LENGTH];
} HistoricalAnalysis;

/* Statistical significance result */
typedef struct {
    double pValue;
    int significantAt95pct;
    int significantAt99pct;
    double effectSize;
    double confidenceInterval[2];
} StatisticalResult;

/* Pattern recognition functions */
int detectPricePatterns(const StockData* data, int dataSize, PatternResult* patterns, int maxPatterns);

/* Time series similarity measures */
double calculateEuclideanDistance(const double* series1, const double* series2, int length);
double calculatePearsonCorrelation(const double* series1, const double* series2, int length);
double calculateDTW(const double* series1, int length1, const double* series2, int length2);

/* Volatility prediction functions */
double predictVolatility(const StockData* data, int dataSize, int horizon);
double predictVolatilityEWMA(const StockData* data, int dataSize, int lookback);
double predictVolatilityGARCH(const StockData* data, int dataSize, int horizon);

/* Anomaly detection functions */
double calculateAnomalyScore(const StockData* data, int dataSize);
int detectAnomalies(const StockData* data, int dataSize, int* anomalyIndices, int maxAnomalies);

/* Event similarity analysis functions */
int findSimilarHistoricalEvents(const EventData* currentEvent, const EventDatabase* historicalEvents, 
                     SimilarHistoricalEvent* similarEvents, int maxResults);
double predictEventOutcome(const EventData* event, const SimilarHistoricalEvent* similarEvents, int count);

/* Historical data mining with CSV cache */
int fetchAndAnalyzeHistoricalData(const char* symbol, const char* startDate, const char* endDate, HistoricalAnalysis* result);
int batchAnalyzeHistoricalData(const char** symbols, int symbolCount, const char* startDate, const char* endDate, HistoricalAnalysis* results);
int calculateCorrelationMatrix(const char** symbols, int symbolCount, const char* startDate, const char* endDate, double** correlationMatrix);
int calculateStatisticalSignificance(const StockData* data1, int dataSize1, const StockData* data2, int dataSize2, StatisticalResult* result);
int findSeasonalPatterns(const StockData* data, int dataSize, PatternResult* patterns, int maxPatterns);
int testTradingStrategy(const StockData* data, int dataSize, const char* strategyParams, double* returnRate, double* sharpeRatio);

#endif /* DATA_MINING_H */