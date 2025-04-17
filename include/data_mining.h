/**
 * Data Mining Module Header
 * Core pattern recognition and data mining functions
 * FOCUSED VERSION - Core Data Mining Functionality
 */

#ifndef DATA_MINING_H
#define DATA_MINING_H

#include "emers.h"

/* Pattern types for price pattern recognition */
#define PATTERN_UNKNOWN       0
#define PATTERN_SUPPORT       1
#define PATTERN_RESISTANCE    2
#define PATTERN_TREND_CHANGE  3
#define PATTERN_DOUBLE_TOP    4
#define PATTERN_DOUBLE_BOTTOM 5
#define PATTERN_HEAD_SHOULDER 6

/* Signal types for trading signal detection */
#define SIGNAL_UNKNOWN        0
#define SIGNAL_BUY            1
#define SIGNAL_SELL           2
#define SIGNAL_HOLD           3
#define SIGNAL_STOP_LOSS      4

/**
 * Structure to hold a detected price pattern
 */
typedef struct {
    char name[32];                /* Pattern name */
    int type;                     /* Pattern type (see defines above) */
    char description[128];        /* Human-readable description */
    int startIndex;               /* Start index in the data array */
    int endIndex;                 /* End index in the data array */
    double confidence;            /* Confidence level (0.0-1.0) */
    double expectedMove;          /* Expected price move (% change) */
} PatternResult;

/**
 * Structure to hold a trading signal
 */
typedef struct {
    int type;                     /* Signal type (see defines above) */
    char description[128];        /* Signal description */
    int signalIndex;              /* Index in data where signal occurs */
    double confidence;            /* Confidence level (0.0-1.0) */
    double entryPrice;            /* Suggested entry price */
    double targetPrice;           /* Target price for take profit */
    double stopLossPrice;         /* Suggested stop loss price */
    double riskRewardRatio;       /* Risk/reward ratio */
} TradingSignal;

/**
 * Structure to hold anomaly detection results
 */
typedef struct {
    int index;                    /* Index in data where anomaly occurs */
    double score;                 /* Anomaly score (higher is more anomalous) */
    double priceDeviation;        /* Price deviation in standard deviations */
    double volumeDeviation;       /* Volume deviation in standard deviations */
    char description[128];        /* Human-readable description */
} AnomalyResult;

/**
 * Structure to hold historical analysis results
 */
typedef struct {
    char symbol[16];              /* Stock symbol */
    double meanReturn;            /* Mean daily return */
    double annualizedReturn;      /* Annualized return */
    double volatility;            /* Annualized volatility */
    double maxDrawdown;           /* Maximum drawdown */
    double sharpeRatio;           /* Sharpe ratio */
    int totalTradingDays;         /* Total trading days analyzed */
    double bestDay;               /* Best daily return */
    double worstDay;              /* Worst daily return */
    char bestDayDate[16];         /* Date of best day */
    char worstDayDate[16];        /* Date of worst day */
} HistoricalAnalysis;

/**
 * CORE ALGORITHM 1: Detect price patterns using pattern recognition
 * Focuses on support/resistance levels, trend changes, and double tops/bottoms
 * 
 * @param data Pointer to stock data array
 * @param dataSize Number of elements in data array
 * @param patterns Array to store detected patterns
 * @param maxPatterns Maximum number of patterns to detect
 * @return Number of patterns detected
 */
int detectPricePatterns(const StockData* data, int dataSize, PatternResult* patterns, int maxPatterns);

/**
 * CORE ALGORITHM 2: SMA Crossover Signal Detection
 * Detects trading signals based on SMA crossovers
 * 
 * @param data Pointer to stock data array
 * @param dataSize Number of elements in data array
 * @param shortPeriod Period for short SMA
 * @param longPeriod Period for long SMA
 * @param signals Array to store detected signals
 * @param maxSignals Maximum number of signals to detect
 * @return Number of signals detected
 */
int detectSMACrossoverSignals(const StockData* data, int dataSize, int shortPeriod, int longPeriod, 
                              TradingSignal* signals, int maxSignals);

/**
 * Calculate simple volatility for a lookback period
 * Annualized standard deviation of returns
 * 
 * @param data Pointer to stock data array
 * @param dataSize Number of elements in data array
 * @param lookback Number of days to look back
 * @return Annualized volatility
 */
double calculateSimpleVolatility(const StockData* data, int dataSize, int lookback);

/**
 * CORE ALGORITHM 3: Anomaly Detection
 * Calculate a simple anomaly score based on price and volume movements
 * 
 * @param data Pointer to stock data array
 * @param dataSize Number of elements in data array
 * @return Anomaly score (higher is more anomalous)
 */
double calculateAnomalyScore(const StockData* data, int dataSize);

/**
 * Detect anomalies in price and volume data
 * Enhanced to provide more detailed anomaly information
 * 
 * @param data Pointer to stock data array
 * @param dataSize Number of elements in data array
 * @param anomalies Array to store detected anomalies
 * @param maxAnomalies Maximum number of anomalies to detect
 * @return Number of anomalies detected
 */
int detectAnomalies(const StockData* data, int dataSize, AnomalyResult* anomalies, int maxAnomalies);

/**
 * Calculate the Euclidean distance between two time series
 * 
 * @param series1 First time series
 * @param series2 Second time series
 * @param length Length of time series
 * @return Euclidean distance
 */
double calculateEuclideanDistance(const double* series1, const double* series2, int length);

/**
 * Calculate the Pearson correlation coefficient between two time series
 * 
 * @param series1 First time series
 * @param series2 Second time series
 * @param length Length of time series
 * @return Pearson correlation coefficient
 */
double calculatePearsonCorrelation(const double* series1, const double* series2, int length);

/**
 * Analyze price momentum to detect overbought/oversold conditions
 * 
 * @param data Pointer to stock data array
 * @param dataSize Number of elements in data array
 * @param period Period for momentum calculation
 * @param threshold Threshold for overbought/oversold
 * @return 1 for overbought, -1 for oversold, 0 for neutral
 */
int analyzePriceMomentum(const StockData* data, int dataSize, int period, double threshold);

/**
 * Fetch and analyze historical data for a stock symbol
 * 
 * @param symbol Stock symbol
 * @param startDate Start date (YYYY-MM-DD)
 * @param endDate End date (YYYY-MM-DD)
 * @param result Pointer to store analysis results
 * @return 0 on success, negative on error
 */
int fetchAndAnalyzeHistoricalData(const char* symbol, const char* startDate, const char* endDate, HistoricalAnalysis* result);

#endif /* DATA_MINING_H */