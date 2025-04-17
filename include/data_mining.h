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
#define PATTERN_UPTREND       7
#define PATTERN_DOWNTREND     8
#define PATTERN_HEAD_AND_SHOULDERS 9

/* Signal types for trading signal detection */
#define SIGNAL_UNKNOWN        0
#define SIGNAL_BUY            1
#define SIGNAL_SELL           2
#define SIGNAL_HOLD           3
#define SIGNAL_STOP_LOSS      4

/* Cluster analysis parameters */
#define MAX_CLUSTERS          10
#define DEFAULT_CLUSTER_COUNT 3
#define MAX_ITERATIONS        100
#define CONVERGENCE_THRESHOLD 0.001

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
 * Structure to hold a market pattern
 * Simplified version for pattern detection
 */
typedef struct {
    int type;                     /* Pattern type (see defines above) */
    int startIndex;               /* Start index in the data array */
    int endIndex;                 /* End index in the data array */
    double confidence;            /* Confidence level (0.0-1.0) */
} MarketPattern;

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
 * Structure to hold a cluster from k-means clustering
 */
typedef struct {
    double centroid[5];           /* Centroid coordinates (OHLCV) */
    int* pointIndices;            /* Indices of points in this cluster */
    int pointCount;               /* Number of points in cluster */
    double averageDistance;       /* Average distance to centroid */
    char label[32];               /* Cluster label */
} Cluster;

/* Preprocessing Functions */

/**
 * Normalize price data using min-max scaling
 * 
 * @param data Input stock data array
 * @param dataSize Number of data points
 * @param normalizedData Output array for normalized data
 * @return 0 on success, negative on failure
 */
int normalizeStockData(const StockData* data, int dataSize, StockData* normalizedData);

/**
 * Remove outliers using z-score method
 * Points with z-score above threshold are replaced with the mean value
 * 
 * @param data Input/output stock data array
 * @param dataSize Number of data points
 * @param threshold Z-score threshold for outlier detection (typically 3.0)
 * @return Number of outliers detected and fixed
 */
int removeOutliers(StockData* data, int dataSize, double threshold);

/**
 * Fill missing data in stock data array
 * Uses linear interpolation for missing values
 * 
 * @param data Input/output stock data array
 * @param dataSize Number of data points
 * @return Number of missing values filled
 */
int fillMissingData(StockData* data, int dataSize);

/**
 * Prepare input data for the data mining algorithms
 * - Removes outliers
 * - Fills missing data
 * - Normalizes data
 * 
 * @param inputData Input stock data array
 * @param inputSize Number of input data points
 * @param outputData Output prepared data array (must be pre-allocated)
 * @param shouldNormalize Whether to normalize the data
 * @return 0 on success, negative on failure
 */
int prepareDataForMining(const StockData* inputData, int inputSize, 
                         StockData* outputData, int shouldNormalize);

/* Core Data Mining Functions */

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
int detectPricePatterns(const StockData* data, int dataSize, MarketPattern* patterns, int maxPatterns);

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
 * IMPROVED: Detect EMA crossover signals
 * Detects trading signals based on EMA crossovers with configurable periods
 * 
 * @param data Pointer to stock data array
 * @param dataSize Number of elements in data array
 * @param shortPeriod Period for short EMA
 * @param longPeriod Period for long EMA
 * @param signals Array to store detected signals
 * @param maxSignals Maximum number of signals to detect
 * @return Number of signals detected
 */
int detectEMACrossoverSignals(const StockData* data, int dataSize, int shortPeriod, int longPeriod, 
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
 * Calculate anomaly scores based on price and volume movements
 * 
 * @param data Pointer to stock data array
 * @param dataSize Number of elements in data array
 * @return Anomaly score (higher is more anomalous)
 */
double calculateAnomalyScore(const StockData* data, int dataSize);

/**
 * IMPROVED: Detect anomalies using statistical methods
 * Combines z-score and moving average deviation
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
 * NEW: Perform K-means clustering on stock data
 * Groups similar trading days based on price/volume patterns
 * 
 * @param data Pointer to stock data array
 * @param dataSize Number of elements in data array
 * @param clusters Array to store resulting clusters
 * @param k Number of clusters to create
 * @return 0 on success, negative value on error
 */
int performKMeansClustering(const StockData* data, int dataSize, Cluster* clusters, int k);

/**
 * NEW: Detect seasonality patterns in time series data
 * Identifies regular cyclical patterns in price data
 * 
 * @param data Pointer to stock data array
 * @param dataSize Number of elements in data array
 * @param periods Array of period lengths to check
 * @param periodCount Number of periods to check
 * @param results Array to store correlation scores (must be pre-allocated)
 * @return Dominant period length or 0 if no seasonality detected
 */
int detectSeasonality(const StockData* data, int dataSize, const int* periods, int periodCount, double* results);

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

/**
 * Calculate historical volatility using simple window method
 * 
 * @param data Stock data array
 * @param dataSize Number of data points
 * @param window Window size for volatility calculation
 * @return Historical volatility (annualized)
 */
double calculateHistoricalVolatility(const StockData* data, int dataSize, int window);

/**
 * Detect head and shoulders pattern in price data
 * This pattern consists of three peaks with the middle peak (head) being the highest
 * and the two outer peaks (shoulders) being at similar levels
 * 
 * @param data Pointer to stock data array
 * @param dataSize Number of elements in data array
 * @param patterns Array to store detected patterns
 * @param maxPatterns Maximum number of patterns that can be detected
 * @return Number of patterns detected
 */
int detectHeadAndShouldersPattern(const StockData* data, int dataSize, 
                                 MarketPattern* patterns, int maxPatterns);

// Helper functions for pattern detection
int findLocalMaximum(const StockData* data, int startIdx, int endIdx);
int findLocalMinimum(const StockData* data, int startIdx, int endIdx);
int findHighestHigh(const StockData* data, int dataSize, int startIdx, int endIdx);
int findLowestLow(const StockData* data, int dataSize, int startIdx, int endIdx);

#endif /* DATA_MINING_H */