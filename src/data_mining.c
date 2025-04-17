/**
 * Data Mining Module
 * Implementation of pattern recognition and data mining functions
 * FOCUSED VERSION - Core Data Mining Functionality
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include "../include/emers.h"
#include "../include/data_mining.h"
#include "../include/technical_analysis.h"
#include "../include/error_handling.h"
#include <float.h>

/* Helper functions for pattern detection */

/**
 * Find local maximum in a data range
 * 
 * @param data Stock data array
 * @param startIdx Start index to search
 * @param endIdx End index to search
 * @return Index of local maximum or -1 if not found
 */
int findLocalMaximum(const StockData* data, int startIdx, int endIdx) {
    if (!data || startIdx < 0 || endIdx <= startIdx) {
        return -1;
    }
    
    for (int i = startIdx + 1; i < endIdx; i++) {
        /* Check if this point is higher than neighbors */
        if (data[i].high > data[i-1].high && data[i].high > data[i+1].high) {
            return i;
        }
    }
    
    return -1;
}

/**
 * Find local minimum in a data range
 * 
 * @param data Stock data array
 * @param startIdx Start index to search
 * @param endIdx End index to search
 * @return Index of local minimum or -1 if not found
 */
int findLocalMinimum(const StockData* data, int startIdx, int endIdx) {
    if (!data || startIdx < 0 || endIdx <= startIdx) {
        return -1;
    }
    
    for (int i = startIdx + 1; i < endIdx; i++) {
        /* Check if this point is lower than neighbors */
        if (data[i].low < data[i-1].low && data[i].low < data[i+1].low) {
            return i;
        }
    }
    
    return -1;
}

/**
 * Find the highest high in a data range
 * 
 * @param data Stock data array
 * @param dataSize Total size of data array
 * @param startIdx Start index to search
 * @param endIdx End index to search
 * @return Index of highest high
 */
int findHighestHigh(const StockData* data, int dataSize, int startIdx, int endIdx) {
    if (!data || dataSize <= 0 || startIdx < 0 || endIdx >= dataSize || endIdx < startIdx) {
        return -1;
    }
    
    int highestIdx = startIdx;
    double highestVal = data[startIdx].high;
    
    for (int i = startIdx + 1; i <= endIdx; i++) {
        if (data[i].high > highestVal) {
            highestVal = data[i].high;
            highestIdx = i;
        }
    }
    
    return highestIdx;
}

/**
 * Find the lowest low in a data range
 * 
 * @param data Stock data array
 * @param dataSize Total size of data array
 * @param startIdx Start index to search
 * @param endIdx End index to search
 * @return Index of lowest low
 */
int findLowestLow(const StockData* data, int dataSize, int startIdx, int endIdx) {
    if (!data || dataSize <= 0 || startIdx < 0 || endIdx >= dataSize || endIdx < startIdx) {
        return -1;
    }
    
    int lowestIdx = startIdx;
    double lowestVal = data[startIdx].low;
    
    for (int i = startIdx + 1; i <= endIdx; i++) {
        if (data[i].low < lowestVal) {
            lowestVal = data[i].low;
            lowestIdx = i;
        }
    }
    
    return lowestIdx;
}

/* Data Preprocessing Functions */

/**
 * Normalize price data using min-max scaling
 * 
 * @param data Input stock data array
 * @param dataSize Number of data points
 * @param normalizedData Output array for normalized data
 * @return 0 on success, negative on failure
 */
int normalizeStockData(const StockData* data, int dataSize, StockData* normalizedData) {
    if (!data || !normalizedData || dataSize <= 0) {
        return ERR_INVALID_PARAMETER;
    }
    
    /* Find min and max values for each dimension */
    double minOpen = data[0].open;
    double maxOpen = data[0].open;
    double minHigh = data[0].high;
    double maxHigh = data[0].high;
    double minLow = data[0].low;
    double maxLow = data[0].low;
    double minClose = data[0].close;
    double maxClose = data[0].close;
    double minVolume = data[0].volume;
    double maxVolume = data[0].volume;
    
    for (int i = 1; i < dataSize; i++) {
        /* Update min values */
        if (data[i].open < minOpen) minOpen = data[i].open;
        if (data[i].high < minHigh) minHigh = data[i].high;
        if (data[i].low < minLow) minLow = data[i].low;
        if (data[i].close < minClose) minClose = data[i].close;
        if (data[i].volume < minVolume) minVolume = data[i].volume;
        
        /* Update max values */
        if (data[i].open > maxOpen) maxOpen = data[i].open;
        if (data[i].high > maxHigh) maxHigh = data[i].high;
        if (data[i].low > maxLow) maxLow = data[i].low;
        if (data[i].close > maxClose) maxClose = data[i].close;
        if (data[i].volume > maxVolume) maxVolume = data[i].volume;
    }
    
    /* Perform min-max normalization to [0,1] range */
    for (int i = 0; i < dataSize; i++) {
        /* Copy date and other fields */
        strcpy(normalizedData[i].date, data[i].date);
        normalizedData[i].adjClose = data[i].adjClose;
        
        /* Normalize price data */
        if (maxOpen != minOpen) {
            normalizedData[i].open = (data[i].open - minOpen) / (maxOpen - minOpen);
        } else {
            normalizedData[i].open = 0.5; /* Default for constant data */
        }
        
        if (maxHigh != minHigh) {
            normalizedData[i].high = (data[i].high - minHigh) / (maxHigh - minHigh);
        } else {
            normalizedData[i].high = 0.5;
        }
        
        if (maxLow != minLow) {
            normalizedData[i].low = (data[i].low - minLow) / (maxLow - minLow);
        } else {
            normalizedData[i].low = 0.5;
        }
        
        if (maxClose != minClose) {
            normalizedData[i].close = (data[i].close - minClose) / (maxClose - minClose);
        } else {
            normalizedData[i].close = 0.5;
        }
        
        if (maxVolume != minVolume) {
            normalizedData[i].volume = (data[i].volume - minVolume) / (maxVolume - minVolume);
        } else {
            normalizedData[i].volume = 0.5;
        }
    }
    
    return 0;
}

/**
 * Remove outliers using z-score method
 * Points with z-score above threshold are replaced with the mean value
 * 
 * @param data Input/output stock data array
 * @param dataSize Number of data points
 * @param threshold Z-score threshold for outlier detection (typically 3.0)
 * @return Number of outliers detected and fixed
 */
int removeOutliers(StockData* data, int dataSize, double threshold) {
    if (!data || dataSize <= 0) {
        return ERR_INVALID_PARAMETER;
    }
    
    int outlierCount = 0;
    
    /* Process each data dimension separately */
    /* Close prices */
    double sum = 0.0, mean = 0.0, stdDev = 0.0;
    
    /* Calculate mean */
    for (int i = 0; i < dataSize; i++) {
        sum += data[i].close;
    }
    mean = sum / dataSize;
    
    /* Calculate standard deviation */
    sum = 0.0;
    for (int i = 0; i < dataSize; i++) {
        sum += (data[i].close - mean) * (data[i].close - mean);
    }
    stdDev = sqrt(sum / dataSize);
    
    /* Detect and replace outliers */
    for (int i = 0; i < dataSize; i++) {
        double zScore = fabs((data[i].close - mean) / stdDev);
        if (zScore > threshold) {
            /* Replace with mean or adjacent values */
            if (i > 0 && i < dataSize - 1) {
                /* Use average of adjacent points */
                data[i].close = (data[i-1].close + data[i+1].close) / 2.0;
            } else {
                /* Use mean */
                data[i].close = mean;
            }
            outlierCount++;
        }
    }
    
    /* Similar processing for volume data */
    sum = 0.0, mean = 0.0, stdDev = 0.0;
    
    /* Calculate mean */
    for (int i = 0; i < dataSize; i++) {
        sum += data[i].volume;
    }
    mean = sum / dataSize;
    
    /* Calculate standard deviation */
    sum = 0.0;
    for (int i = 0; i < dataSize; i++) {
        sum += (data[i].volume - mean) * (data[i].volume - mean);
    }
    stdDev = sqrt(sum / dataSize);
    
    /* Detect and replace outliers */
    for (int i = 0; i < dataSize; i++) {
        double zScore = fabs((data[i].volume - mean) / stdDev);
        if (zScore > threshold) {
            /* Replace with mean or adjacent values */
            if (i > 0 && i < dataSize - 1) {
                /* Use average of adjacent points */
                data[i].volume = (data[i-1].volume + data[i+1].volume) / 2.0;
            } else {
                /* Use mean */
                data[i].volume = mean;
            }
            outlierCount++;
        }
    }
    
    return outlierCount;
}

/**
 * Fill missing data in stock data array
 * Uses linear interpolation for missing values
 * 
 * @param data Input/output stock data array
 * @param dataSize Number of data points
 * @return Number of missing values filled
 */
int fillMissingData(StockData* data, int dataSize) {
    if (!data || dataSize <= 0) {
        return ERR_INVALID_PARAMETER;
    }
    
    int filledCount = 0;
    
    /* Check for zero values in price data (assuming these are missing) */
    for (int i = 1; i < dataSize - 1; i++) {
        /* Check close price */
        if (data[i].close == 0.0) {
            /* Use linear interpolation between prev and next */
            data[i].close = (data[i-1].close + data[i+1].close) / 2.0;
            filledCount++;
        }
        
        /* Check other OHLC values */
        if (data[i].open == 0.0) {
            data[i].open = (data[i-1].open + data[i+1].open) / 2.0;
            filledCount++;
        }
        
        if (data[i].high == 0.0) {
            data[i].high = (data[i-1].high + data[i+1].high) / 2.0;
            filledCount++;
        }
        
        if (data[i].low == 0.0) {
            data[i].low = (data[i-1].low + data[i+1].low) / 2.0;
            filledCount++;
        }
        
        /* Check volume */
        if (data[i].volume == 0.0) {
            data[i].volume = (data[i-1].volume + data[i+1].volume) / 2.0;
            filledCount++;
        }
    }
    
    /* Handle edge cases (first and last element) */
    if (data[0].close == 0.0 && dataSize > 1) {
        data[0].close = data[1].close;
        filledCount++;
    }
    
    if (data[dataSize-1].close == 0.0 && dataSize > 1) {
        data[dataSize-1].close = data[dataSize-2].close;
        filledCount++;
    }
    
    return filledCount;
}

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
                         StockData* outputData, int shouldNormalize) {
    if (!inputData || !outputData || inputSize <= 0) {
        return ERR_INVALID_PARAMETER;
    }
    
    /* Copy input data to output */
    memcpy(outputData, inputData, inputSize * sizeof(StockData));
    
    /* Fill missing data */
    int filledCount = fillMissingData(outputData, inputSize);
    (void)filledCount; /* Prevent unused variable warning */
    
    /* Remove outliers */
    int outlierCount = removeOutliers(outputData, inputSize, 3.0); /* 3 sigma rule */
    (void)outlierCount; /* Prevent unused variable warning */
    
    /* Normalize data if requested */
    if (shouldNormalize) {
        StockData* tempData = (StockData*)malloc(inputSize * sizeof(StockData));
        if (!tempData) {
            return ERR_MEMORY_ALLOCATION;
        }
        
        int result = normalizeStockData(outputData, inputSize, tempData);
        if (result < 0) {
            free(tempData);
            return result;
        }
        
        /* Copy normalized data back to output */
        memcpy(outputData, tempData, inputSize * sizeof(StockData));
        free(tempData);
    }
    
    return 0;
}

/**
 * CORE ALGORITHM 1: Detect price patterns using pattern recognition
 * Focuses on support/resistance levels, trend changes, and double tops/bottoms
 */
int detectPricePatterns(const StockData* data, int dataSize, MarketPattern* patterns, int maxPatterns) {
    if (!data || !patterns || dataSize <= 0 || maxPatterns <= 0) {
        return ERR_INVALID_PARAMETER;
    }
    
    int patternCount = 0;
    
    /* Detect simple patterns first */
    int i;
    for (i = 2; i < dataSize && patternCount < maxPatterns; i++) {
        /* Higher high with higher low */
        if (data[i].high > data[i-1].high && data[i].low > data[i-1].low) {
            patterns[patternCount].type = PATTERN_UPTREND;
            patterns[patternCount].startIndex = i-1;
            patterns[patternCount].endIndex = i;
            patterns[patternCount].confidence = 0.7;
            patternCount++;
        }
        
        /* Lower low with lower high */
        if (data[i].low < data[i-1].low && data[i].high < data[i-1].high) {
            patterns[patternCount].type = PATTERN_DOWNTREND;
            patterns[patternCount].startIndex = i-1;
            patterns[patternCount].endIndex = i;
            patterns[patternCount].confidence = 0.7;
            patternCount++;
        }
    }
    
    /* Detect complex patterns */
    int highIndex = findHighestHigh(data, dataSize, 0, dataSize - 1);
    (void)highIndex; /* Prevent unused variable warning */
    
    int lowIndex = findLowestLow(data, dataSize, 0, dataSize - 1);
    (void)lowIndex; /* Prevent unused variable warning */
    
    /* Detect head and shoulders pattern */
    patternCount += detectHeadAndShouldersPattern(data, dataSize, &patterns[patternCount], 
                                               maxPatterns - patternCount);
    
    return patternCount;
}

/**
 * Detect head and shoulders pattern in price data
 * This pattern consists of three peaks with the middle peak (head) being the highest
 * and the two outer peaks (shoulders) being at similar levels
 * 
 * @param data Pointer to stock data array
 * @param dataSize Number of elements in data array
 * @param patterns Array to store detected patterns
 * @param patternCount Current count of detected patterns
 * @param maxPatterns Maximum number of patterns that can be detected
 * @return Updated pattern count
 */
int detectHeadAndShouldersPattern(const StockData* data, int dataSize, 
                                 MarketPattern* patterns, int maxPatterns) {
    if (!data || !patterns || dataSize < 50 || maxPatterns <= 0) {
        return ERR_INVALID_PARAMETER;
    }
    
    int patternCount = 0;
    
    /* Window size for pattern detection */
    int window = 30;
    
    /* Minimum required peaks and troughs */
    int i;
    for (i = window; i < dataSize - window && patternCount < maxPatterns; i++) {
        /* Look for a sequence of 5 points: shoulder, trough, head, trough, shoulder */
        int leftShoulderIdx = findLocalMaximum(data, i - window, i);
        if (leftShoulderIdx < 0) continue;
        
        int leftTroughIdx = findLocalMinimum(data, leftShoulderIdx, i);
        if (leftTroughIdx < 0) continue;
        
        int headIdx = findLocalMaximum(data, leftTroughIdx, i + window/2);
        if (headIdx < 0) continue;
        
        int rightTroughIdx = findLocalMinimum(data, headIdx, i + window);
        if (rightTroughIdx < 0) continue;
        
        int rightShoulderIdx = findLocalMaximum(data, rightTroughIdx, i + window);
        if (rightShoulderIdx < 0) continue;
        
        double leftShoulderVal = data[leftShoulderIdx].high;
        double headVal = data[headIdx].high;
        double rightShoulderVal = data[rightShoulderIdx].high;
        (void)rightShoulderVal; /* Prevent unused variable warning */
        
        /* Check if head is higher than shoulders */
        if (headVal > leftShoulderVal && 
            headVal > data[rightShoulderIdx].high && 
            fabs(data[leftShoulderIdx].high - data[rightShoulderIdx].high) < (0.1 * headVal)) {
            
            patterns[patternCount].type = PATTERN_HEAD_AND_SHOULDERS;
            patterns[patternCount].startIndex = leftShoulderIdx;
            patterns[patternCount].endIndex = rightShoulderIdx;
            patterns[patternCount].confidence = 0.8;
            patternCount++;
        }
    }
    
    return patternCount;
}

/**
 * CORE ALGORITHM 2: SMA Crossover Signal Detection
 * Detects trading signals based on SMA crossovers
 */
int detectSMACrossoverSignals(const StockData* data, int dataSize, int shortPeriod, int longPeriod, 
                              TradingSignal* signals, int maxSignals) {
    if (!data || dataSize < longPeriod || !signals || maxSignals <= 0) {
        return 0;
    }
    
    int signalCount = 0;
    
    /* Calculate short and long SMAs */
    double* shortSMA = (double*)malloc(dataSize * sizeof(double));
    double* longSMA = (double*)malloc(dataSize * sizeof(double));
    
    if (!shortSMA || !longSMA) {
        free(shortSMA);
        free(longSMA);
        return 0;
    }
    
    /* Calculate SMAs for each day */
    for (int i = shortPeriod - 1; i < dataSize; i++) {
        double sum = 0.0;
        for (int j = i - shortPeriod + 1; j <= i; j++) {
            sum += data[j].close;
        }
        shortSMA[i] = sum / shortPeriod;
    }
    
    for (int i = longPeriod - 1; i < dataSize; i++) {
        double sum = 0.0;
        for (int j = i - longPeriod + 1; j <= i; j++) {
            sum += data[j].close;
        }
        longSMA[i] = sum / longPeriod;
    }
    
    /* Look for crossovers in the most recent data first */
    for (int i = dataSize - 2; i >= longPeriod && signalCount < maxSignals; i--) {
        /* Check for bullish crossover (short SMA crosses above long SMA) */
        if (shortSMA[i] <= longSMA[i] && shortSMA[i+1] > longSMA[i+1]) {
            signals[signalCount].type = SIGNAL_BUY;
            strcpy(signals[signalCount].description, "Bullish SMA Crossover");
            signals[signalCount].signalIndex = i+1;
            signals[signalCount].confidence = 0.7;
            signals[signalCount].entryPrice = data[i+1].close;
            signals[signalCount].targetPrice = data[i+1].close * 1.05; /* 5% profit target */
            signals[signalCount].stopLossPrice = data[i+1].close * 0.97; /* 3% stop loss */
            signals[signalCount].riskRewardRatio = (0.05 / 0.03);
            signalCount++;
        }
        /* Check for bearish crossover (short SMA crosses below long SMA) */
        else if (shortSMA[i] >= longSMA[i] && shortSMA[i+1] < longSMA[i+1] && signalCount < maxSignals) {
            signals[signalCount].type = SIGNAL_SELL;
            strcpy(signals[signalCount].description, "Bearish SMA Crossover");
            signals[signalCount].signalIndex = i+1;
            signals[signalCount].confidence = 0.7;
            signals[signalCount].entryPrice = data[i+1].close;
            signals[signalCount].targetPrice = data[i+1].close * 0.95; /* 5% profit target */
            signals[signalCount].stopLossPrice = data[i+1].close * 1.03; /* 3% stop loss */
            signals[signalCount].riskRewardRatio = (0.05 / 0.03);
            signalCount++;
        }
    }
    
    free(shortSMA);
    free(longSMA);
    return signalCount;
}

/**
 * Calculate simple volatility for a lookback period
 * Annualized standard deviation of returns
 */
double calculateSimpleVolatility(const StockData* data, int dataSize, int lookback) {
    if (!data || dataSize <= lookback) {
        return 0.0;
    }
    
    /* Constrain lookback to available data */
    if (lookback <= 0) {
        lookback = dataSize - 1;
    }
    
    double* returns = (double*)malloc((lookback) * sizeof(double));
    if (!returns) {
        return 0.0;
    }
    
    for (int i = 0; i < lookback; i++) {
        int idx = dataSize - lookback + i;
        returns[i] = (data[idx].close - data[idx-1].close) / data[idx-1].close;
    }
    
    /* Calculate mean return */
    double sumReturns = 0.0;
    for (int i = 0; i < lookback; i++) {
        sumReturns += returns[i];
    }
    double meanReturn = sumReturns / lookback;
    
    /* Calculate variance */
    double sumSquaredDiff = 0.0;
    for (int i = 0; i < lookback; i++) {
        double diff = returns[i] - meanReturn;
        sumSquaredDiff += diff * diff;
    }
    
    double variance = sumSquaredDiff / lookback;
    double stdDev = sqrt(variance);
    
    /* Annualize the daily volatility (approximately) */
    double annualizedVol = stdDev * sqrt(252.0); /* 252 trading days in a year */
    
    free(returns);
    return annualizedVol;
}

/**
 * CORE ALGORITHM 3: Anomaly Detection
 * Calculate a simple anomaly score based on price and volume movements
 */
double calculateAnomalyScore(const StockData* data, int dataSize) {
    if (!data || dataSize < 30) {
        return 0.0;
    }
    
    /* Calculate price and volume z-scores */
    double priceSum = 0.0;
    double volumeSum = 0.0;
    double priceSumSq = 0.0;
    double volumeSumSq = 0.0;
    
    /* Calculate means first */
    for (int i = dataSize - 30; i < dataSize - 1; i++) {
        double priceChange = (data[i].close - data[i-1].close) / data[i-1].close;
        priceSum += priceChange;
        priceSumSq += priceChange * priceChange;
        
        volumeSum += data[i].volume;
        volumeSumSq += data[i].volume * data[i].volume;
    }
    
    double priceMean = priceSum / 29.0;
    double volumeMean = volumeSum / 29.0;
    
    double priceStdDev = sqrt(priceSumSq / 29.0 - priceMean * priceMean);
    double volumeStdDev = sqrt(volumeSumSq / 29.0 - volumeMean * volumeMean);
    
    /* Calculate z-scores for most recent day */
    double latestPriceChange = (data[dataSize-1].close - data[dataSize-2].close) / data[dataSize-2].close;
    double latestVolume = data[dataSize-1].volume;
    
    double priceZScore = (latestPriceChange - priceMean) / priceStdDev;
    double volumeZScore = (latestVolume - volumeMean) / volumeStdDev;
    
    /* Combine the z-scores */
    double combinedScore = sqrt(priceZScore * priceZScore + volumeZScore * volumeZScore);
    
    return combinedScore;
}

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
int detectAnomalies(const StockData* data, int dataSize, AnomalyResult* anomalies, int maxAnomalies) {
    if (!data || dataSize < 30 || !anomalies || maxAnomalies <= 0) {
        return 0;
    }
    
    int anomalyCount = 0;
    
    /* Calculate moving average and standard deviation of prices */
    const int maWindow = 20;
    double* movingAvg = (double*)malloc(dataSize * sizeof(double));
    double* stdDevs = (double*)malloc(dataSize * sizeof(double));
    
    if (!movingAvg || !stdDevs) {
        if (movingAvg) free(movingAvg);
        if (stdDevs) free(stdDevs);
        return 0;
    }
    
    /* Calculate moving average */
    for (int i = 0; i < dataSize; i++) {
        if (i < maWindow) {
            /* Not enough data for full window */
            double sum = 0.0;
            for (int j = 0; j <= i; j++) {
                sum += data[j].close;
            }
            movingAvg[i] = sum / (i + 1);
        } else {
            double sum = 0.0;
            for (int j = i - maWindow + 1; j <= i; j++) {
                sum += data[j].close;
            }
            movingAvg[i] = sum / maWindow;
        }
    }
    
    /* Calculate standard deviations */
    for (int i = 0; i < dataSize; i++) {
        if (i < maWindow) {
            double sumSquares = 0.0;
            for (int j = 0; j <= i; j++) {
                sumSquares += (data[j].close - movingAvg[i]) * (data[j].close - movingAvg[i]);
            }
            stdDevs[i] = sqrt(sumSquares / (i + 1));
        } else {
            double sumSquares = 0.0;
            for (int j = i - maWindow + 1; j <= i; j++) {
                sumSquares += (data[j].close - movingAvg[i]) * (data[j].close - movingAvg[i]);
            }
            stdDevs[i] = sqrt(sumSquares / maWindow);
        }
    }
    
    /* Calculate volume moving average */
    double* volumeMA = (double*)malloc(dataSize * sizeof(double));
    double* volumeStdDevs = (double*)malloc(dataSize * sizeof(double));
    
    if (!volumeMA || !volumeStdDevs) {
        free(movingAvg);
        free(stdDevs);
        if (volumeMA) free(volumeMA);
        if (volumeStdDevs) free(volumeStdDevs);
        return 0;
    }
    
    /* Calculate volume moving average */
    for (int i = 0; i < dataSize; i++) {
        if (i < maWindow) {
            double sum = 0.0;
            for (int j = 0; j <= i; j++) {
                sum += data[j].volume;
            }
            volumeMA[i] = sum / (i + 1);
        } else {
            double sum = 0.0;
            for (int j = i - maWindow + 1; j <= i; j++) {
                sum += data[j].volume;
            }
            volumeMA[i] = sum / maWindow;
        }
    }
    
    /* Calculate volume standard deviations */
    for (int i = 0; i < dataSize; i++) {
        if (i < maWindow) {
            double sumSquares = 0.0;
            for (int j = 0; j <= i; j++) {
                sumSquares += (data[j].volume - volumeMA[i]) * (data[j].volume - volumeMA[i]);
            }
            volumeStdDevs[i] = sqrt(sumSquares / (i + 1));
        } else {
            double sumSquares = 0.0;
            for (int j = i - maWindow + 1; j <= i; j++) {
                sumSquares += (data[j].volume - volumeMA[i]) * (data[j].volume - volumeMA[i]);
            }
            volumeStdDevs[i] = sqrt(sumSquares / maWindow);
        }
    }
    
    /* Detect anomalies using Z-score */
    const double priceThreshold = 2.5;  /* Z-score threshold for price anomalies */
    const double volumeThreshold = 3.0; /* Z-score threshold for volume anomalies */
    
    for (int i = maWindow; i < dataSize; i++) {
        double priceZScore = fabs((data[i].close - movingAvg[i]) / stdDevs[i]);
        double volumeZScore = fabs((data[i].volume - volumeMA[i]) / volumeStdDevs[i]);
        
        /* Check if either price or volume (or both) are anomalous */
        if (priceZScore > priceThreshold || volumeZScore > volumeThreshold) {
            if (anomalyCount < maxAnomalies) {
                anomalies[anomalyCount].index = i;
                anomalies[anomalyCount].priceDeviation = priceZScore;
                anomalies[anomalyCount].volumeDeviation = volumeZScore;
                
                /* Calculate combined anomaly score */
                anomalies[anomalyCount].score = sqrt(priceZScore * priceZScore + volumeZScore * volumeZScore);
                
                /* Create description */
                if (priceZScore > priceThreshold && volumeZScore > volumeThreshold) {
                    sprintf(anomalies[anomalyCount].description, 
                            "Price (%.2f σ) and volume (%.2f σ) anomaly detected", 
                            priceZScore, volumeZScore);
                } else if (priceZScore > priceThreshold) {
                    sprintf(anomalies[anomalyCount].description, 
                            "Price anomaly detected (%.2f σ from mean)", 
                            priceZScore);
                } else {
                    sprintf(anomalies[anomalyCount].description, 
                            "Volume anomaly detected (%.2f σ from mean)", 
                            volumeZScore);
                }
                
                anomalyCount++;
            }
        }
    }
    
    /* Sort anomalies by score (descending) */
    for (int i = 0; i < anomalyCount - 1; i++) {
        for (int j = i + 1; j < anomalyCount; j++) {
            if (anomalies[i].score < anomalies[j].score) {
                /* Swap */
                AnomalyResult temp = anomalies[i];
                anomalies[i] = anomalies[j];
                anomalies[j] = temp;
            }
        }
    }
    
    /* Clean up */
    free(movingAvg);
    free(stdDevs);
    free(volumeMA);
    free(volumeStdDevs);
    
    return anomalyCount;
}

/**
 * Calculate the Euclidean distance between two time series
 */
double calculateEuclideanDistance(const double* series1, const double* series2, int length) {
    if (!series1 || !series2 || length <= 0) {
        return 0.0;
    }
    
    double sumSquaredDiff = 0.0;
    for (int i = 0; i < length; i++) {
        double diff = series1[i] - series2[i];
        sumSquaredDiff += diff * diff;
    }
    
    return sqrt(sumSquaredDiff);
}

/**
 * Calculate the Pearson correlation coefficient between two time series
 */
double calculatePearsonCorrelation(const double* series1, const double* series2, int length) {
    if (!series1 || !series2 || length <= 1) {
        return 0.0;
    }
    
    double sum1 = 0.0, sum2 = 0.0;
    double sumSq1 = 0.0, sumSq2 = 0.0;
    double pSum = 0.0;
    
    for (int i = 0; i < length; i++) {
        sum1 += series1[i];
        sum2 += series2[i];
        sumSq1 += series1[i] * series1[i];
        sumSq2 += series2[i] * series2[i];
        pSum += series1[i] * series2[i];
    }
    
    double num = pSum - (sum1 * sum2 / length);
    double den = sqrt((sumSq1 - (sum1 * sum1) / length) * (sumSq2 - (sum2 * sum2) / length));
    
    if (fabs(den) < 0.000001) {
        return 0.0;
    }
    
    return num / den;
}

/**
 * Analyze price momentum to detect overbought/oversold conditions
 */
int analyzePriceMomentum(const StockData* data, int dataSize, int period, double threshold) {
    if (!data || dataSize < period + 1) {
        return 0; /* Neutral */
    }
    
    /* Calculate simple momentum as percentage change */
    double current = data[dataSize - 1].close;
    double past = data[dataSize - period - 1].close;
    double momentumPct = (current - past) / past * 100.0;
    
    /* Return overbought(1), oversold(-1), or neutral(0) */
    if (momentumPct > threshold) {
        return 1; /* Overbought */
    } else if (momentumPct < -threshold) {
        return -1; /* Oversold */
    } else {
        return 0; /* Neutral */
    }
}

/**
 * Fetch and analyze historical data for a stock symbol
 */
int fetchAndAnalyzeHistoricalData(const char* symbol, const char* startDate, const char* endDate, HistoricalAnalysis* result) {
    if (!symbol || !startDate || !endDate || !result) {
        return -1;
    }
    
    /* Note: In a real implementation, this would fetch data from a database or API */
    /* For now, we'll just return some example values */
    strcpy(result->symbol, symbol);
    result->meanReturn = 0.0005;           /* 0.05% daily return */
    result->annualizedReturn = 0.12;       /* 12% annual return */
    result->volatility = 0.18;             /* 18% annualized volatility */
    result->maxDrawdown = 0.25;            /* 25% maximum drawdown */
    result->sharpeRatio = 0.7;             /* Sharpe ratio of 0.7 */
    result->totalTradingDays = 252;        /* 1 year of trading data */
    result->bestDay = 0.05;                /* 5% best day return */
    result->worstDay = -0.05;              /* -5% worst day return */
    strcpy(result->bestDayDate, "2025-02-15");
    strcpy(result->worstDayDate, "2025-03-10");
    
    return 0;
}

/**
 * IMPROVED: Detect EMA crossover signals
 * Detects trading signals based on EMA crossovers with configurable periods
 */
int detectEMACrossoverSignals(const StockData* data, int dataSize, int shortPeriod, int longPeriod, 
                             TradingSignal* signals, int maxSignals) {
    if (!data || dataSize < longPeriod + 10 || !signals || maxSignals <= 0) {
        return 0;
    }
    
    int signalCount = 0;
    
    /* Calculate short and long EMAs */
    double* shortEMA = (double*)malloc(dataSize * sizeof(double));
    double* longEMA = (double*)malloc(dataSize * sizeof(double));
    
    if (!shortEMA || !longEMA) {
        if (shortEMA) free(shortEMA);
        if (longEMA) free(longEMA);
        return 0;
    }
    
    /* Get close prices */
    double* closePrices = (double*)malloc(dataSize * sizeof(double));
    if (!closePrices) {
        free(shortEMA);
        free(longEMA);
        return 0;
    }
    
    for (int i = 0; i < dataSize; i++) {
        closePrices[i] = data[i].close;
    }
    
    /* Calculate EMAs */
    calculateEMA(data, dataSize, shortPeriod, shortEMA);
    calculateEMA(data, dataSize, longPeriod, longEMA);
    
    /* Look for crossovers */
    for (int i = 1; i < dataSize; i++) {
        if (i < longPeriod) continue; /* Skip until we have enough data */
        
        /* Check for bullish crossover (short EMA crosses above long EMA) */
        if (shortEMA[i-1] <= longEMA[i-1] && shortEMA[i] > longEMA[i]) {
            if (signalCount < maxSignals) {
                signals[signalCount].type = SIGNAL_BUY;
                sprintf(signals[signalCount].description, 
                        "Bullish EMA Crossover: %d-day EMA crossed above %d-day EMA", 
                        shortPeriod, longPeriod);
                signals[signalCount].signalIndex = i;
                signals[signalCount].confidence = 0.70; /* Base confidence */
                
                /* Adjust confidence based on volume confirmation */
                if (data[i].volume > data[i-1].volume * 1.2) {
                    signals[signalCount].confidence += 0.15; /* Higher confidence with volume confirmation */
                }
                
                signals[signalCount].entryPrice = data[i].close;
                
                /* Calculate target price (using historical movement after crossovers) */
                double avgMove = 0.03; /* Assume 3% average move after signal */
                signals[signalCount].targetPrice = data[i].close * (1.0 + avgMove);
                
                /* Calculate stop loss (half the expected move in opposite direction) */
                signals[signalCount].stopLossPrice = data[i].close * (1.0 - avgMove/2.0);
                
                /* Calculate risk/reward ratio */
                double reward = signals[signalCount].targetPrice - signals[signalCount].entryPrice;
                double risk = signals[signalCount].entryPrice - signals[signalCount].stopLossPrice;
                signals[signalCount].riskRewardRatio = risk > 0 ? reward / risk : 0.0;
                
                signalCount++;
            }
        } 
        /* Check for bearish crossover (short EMA crosses below long EMA) */
        else if (shortEMA[i-1] >= longEMA[i-1] && shortEMA[i] < longEMA[i]) {
            if (signalCount < maxSignals) {
                signals[signalCount].type = SIGNAL_SELL;
                sprintf(signals[signalCount].description, 
                        "Bearish EMA Crossover: %d-day EMA crossed below %d-day EMA", 
                        shortPeriod, longPeriod);
                signals[signalCount].signalIndex = i;
                signals[signalCount].confidence = 0.70; /* Base confidence */
                
                /* Adjust confidence based on volume confirmation */
                if (data[i].volume > data[i-1].volume * 1.2) {
                    signals[signalCount].confidence += 0.15; /* Higher confidence with volume confirmation */
                }
                
                signals[signalCount].entryPrice = data[i].close;
                
                /* Calculate target price */
                double avgMove = 0.03; /* Assume 3% average move after signal */
                signals[signalCount].targetPrice = data[i].close * (1.0 - avgMove);
                
                /* Calculate stop loss */
                signals[signalCount].stopLossPrice = data[i].close * (1.0 + avgMove/2.0);
                
                /* Calculate risk/reward ratio */
                double reward = signals[signalCount].entryPrice - signals[signalCount].targetPrice;
                double risk = signals[signalCount].stopLossPrice - signals[signalCount].entryPrice;
                signals[signalCount].riskRewardRatio = risk > 0 ? reward / risk : 0.0;
                
                signalCount++;
            }
        }
    }
    
    /* Clean up */
    free(shortEMA);
    free(longEMA);
    free(closePrices);
    
    return signalCount;
}

/**
 * Calculate historical volatility using simple window method
 */
double calculateHistoricalVolatility(const StockData* data, int dataSize, int window) {
    if (!data || dataSize < window + 1) {
        return 0.0;
    }
    
    /* Calculate daily returns */
    double* returns = (double*)malloc((dataSize - 1) * sizeof(double));
    if (!returns) {
        return 0.0;
    }
    
    for (int i = 1; i < dataSize; i++) {
        returns[i-1] = (data[i].close / data[i-1].close) - 1.0;
    }
    
    /* Find starting index for window */
    int startIdx = (dataSize - 1) - window;
    if (startIdx < 0) startIdx = 0;
    
    /* Calculate mean return over window */
    double sum = 0.0;
    for (int i = startIdx; i < dataSize - 1; i++) {
        sum += returns[i];
    }
    double mean = sum / (dataSize - 1 - startIdx);
    
    /* Calculate variance of returns */
    sum = 0.0;
    for (int i = startIdx; i < dataSize - 1; i++) {
        sum += (returns[i] - mean) * (returns[i] - mean);
    }
    double variance = sum / (dataSize - 1 - startIdx);
    
    /* Calculate annualized volatility */
    double stdDev = sqrt(variance);
    double annualizedVol = stdDev * sqrt(252.0); /* Assuming 252 trading days per year */
    
    free(returns);
    return annualizedVol;
}

/**
 * NEW: Perform K-means clustering on stock data
 * Groups similar trading days based on price/volume patterns
 */
int performKMeansClustering(const StockData* data, int dataSize, Cluster* clusters, int k) {
    if (!data || dataSize < k || !clusters || k <= 0 || k > MAX_CLUSTERS) {
        return -1;
    }
    
    /* Normalize the data for clustering */
    StockData* normalizedData = (StockData*)malloc(dataSize * sizeof(StockData));
    if (!normalizedData) {
        return -1;
    }
    
    int result = normalizeStockData(data, dataSize, normalizedData);
    if (result < 0) {
        free(normalizedData);
        return -1;
    }
    
    /* Create points array with 5 dimensions (OHLCV) */
    double** points = (double**)malloc(dataSize * sizeof(double*));
    if (!points) {
        free(normalizedData);
        return -1;
    }
    
    for (int i = 0; i < dataSize; i++) {
        points[i] = (double*)malloc(5 * sizeof(double));
        if (!points[i]) {
            for (int j = 0; j < i; j++) {
                free(points[j]);
            }
            free(points);
            free(normalizedData);
            return -1;
        }
        
        points[i][0] = normalizedData[i].open;
        points[i][1] = normalizedData[i].high;
        points[i][2] = normalizedData[i].low;
        points[i][3] = normalizedData[i].close;
        points[i][4] = normalizedData[i].volume;
    }
    
    /* Initialize cluster centroids randomly */
    for (int i = 0; i < k; i++) {
        int randomIndex = rand() % dataSize;
        for (int d = 0; d < 5; d++) {
            clusters[i].centroid[d] = points[randomIndex][d];
        }
        
        clusters[i].pointIndices = (int*)malloc(dataSize * sizeof(int));
        if (!clusters[i].pointIndices) {
            for (int j = 0; j < i; j++) {
                free(clusters[j].pointIndices);
            }
            for (int j = 0; j < dataSize; j++) {
                free(points[j]);
            }
            free(points);
            free(normalizedData);
            return -1;
        }
        
        clusters[i].pointCount = 0;
        clusters[i].averageDistance = 0.0;
        sprintf(clusters[i].label, "Cluster %d", i+1);
    }
    
    /* Iterative K-means */
    int maxIterations = MAX_ITERATIONS;
    double prevError = DBL_MAX;
    double currError = 0.0;
    
    while (maxIterations-- > 0) {
        /* Reset clusters */
        for (int i = 0; i < k; i++) {
            clusters[i].pointCount = 0;
            clusters[i].averageDistance = 0.0;
        }
        
        /* Assign points to nearest centroid */
        for (int i = 0; i < dataSize; i++) {
            double minDistance = DBL_MAX;
            int closestCluster = 0;
            
            for (int j = 0; j < k; j++) {
                double distance = 0.0;
                for (int d = 0; d < 5; d++) {
                    distance += (points[i][d] - clusters[j].centroid[d]) * 
                                (points[i][d] - clusters[j].centroid[d]);
                }
                distance = sqrt(distance);
                
                if (distance < minDistance) {
                    minDistance = distance;
                    closestCluster = j;
                }
            }
            
            /* Add point to closest cluster */
            clusters[closestCluster].pointIndices[clusters[closestCluster].pointCount++] = i;
            clusters[closestCluster].averageDistance += minDistance;
        }
        
        /* Calculate average distance for each cluster */
        currError = 0.0;
        for (int i = 0; i < k; i++) {
            if (clusters[i].pointCount > 0) {
                clusters[i].averageDistance /= clusters[i].pointCount;
                currError += clusters[i].averageDistance;
            }
        }
        
        /* Check convergence */
        if (fabs(prevError - currError) < CONVERGENCE_THRESHOLD) {
            break;
        }
        prevError = currError;
        
        /* Recalculate centroids */
        for (int i = 0; i < k; i++) {
            if (clusters[i].pointCount > 0) {
                /* Reset centroid */
                for (int d = 0; d < 5; d++) {
                    clusters[i].centroid[d] = 0.0;
                }
                
                /* Sum all points */
                for (int j = 0; j < clusters[i].pointCount; j++) {
                    int pointIdx = clusters[i].pointIndices[j];
                    for (int d = 0; d < 5; d++) {
                        clusters[i].centroid[d] += points[pointIdx][d];
                    }
                }
                
                /* Calculate average */
                for (int d = 0; d < 5; d++) {
                    clusters[i].centroid[d] /= clusters[i].pointCount;
                }
            }
        }
    }
    
    /* Label clusters based on characteristics */
    for (int i = 0; i < k; i++) {
        if (clusters[i].pointCount > 0) {
            /* Check if points in cluster show specific patterns */
            
            /* Check if bullish (close > open) */
            int bullishCount = 0;
            int bearishCount = 0;
            double avgVolume = 0.0;
            
            for (int j = 0; j < clusters[i].pointCount; j++) {
                int pointIdx = clusters[i].pointIndices[j];
                if (normalizedData[pointIdx].close > normalizedData[pointIdx].open) {
                    bullishCount++;
                } else {
                    bearishCount++;
                }
                avgVolume += normalizedData[pointIdx].volume;
            }
            
            avgVolume /= clusters[i].pointCount;
            
            /* Determine cluster characteristics */
            if (bullishCount > bearishCount * 2) {
                /* Strongly bullish cluster */
                if (avgVolume > 0.7) {
                    sprintf(clusters[i].label, "Strong Bull (High Vol)");
                } else {
                    sprintf(clusters[i].label, "Strong Bull");
                }
            } else if (bullishCount > bearishCount) {
                /* Moderately bullish */
                if (avgVolume > 0.7) {
                    sprintf(clusters[i].label, "Moderate Bull (High Vol)");
                } else {
                    sprintf(clusters[i].label, "Moderate Bull");
                }
            } else if (bearishCount > bullishCount * 2) {
                /* Strongly bearish */
                if (avgVolume > 0.7) {
                    sprintf(clusters[i].label, "Strong Bear (High Vol)");
                } else {
                    sprintf(clusters[i].label, "Strong Bear");
                }
            } else if (bearishCount > bullishCount) {
                /* Moderately bearish */
                if (avgVolume > 0.7) {
                    sprintf(clusters[i].label, "Moderate Bear (High Vol)");
                } else {
                    sprintf(clusters[i].label, "Moderate Bear");
                }
            } else {
                /* Neutral */
                if (avgVolume > 0.7) {
                    sprintf(clusters[i].label, "Neutral (High Vol)");
                } else {
                    sprintf(clusters[i].label, "Neutral");
                }
            }
        }
    }
    
    /* Clean up */
    for (int i = 0; i < dataSize; i++) {
        free(points[i]);
    }
    free(points);
    free(normalizedData);
    
    return 0;
}

/**
 * NEW: Detect seasonality patterns in time series data
 * Identifies regular cyclical patterns in price data
 */
int detectSeasonality(const StockData* data, int dataSize, const int* periods, int periodCount, double* results) {
    if (!data || dataSize < 60 || !periods || periodCount <= 0 || !results) {
        return 0;
    }
    
    /* Extract close prices */
    double* prices = (double*)malloc(dataSize * sizeof(double));
    if (!prices) {
        return 0;
    }
    
    for (int i = 0; i < dataSize; i++) {
        prices[i] = data[i].close;
    }
    
    /* Calculate returns */
    double* returns = (double*)malloc((dataSize - 1) * sizeof(double));
    if (!returns) {
        free(prices);
        return 0;
    }
    
    for (int i = 1; i < dataSize; i++) {
        returns[i-1] = (prices[i] / prices[i-1]) - 1.0;
    }
    
    /* Check each period for seasonality */
    double maxCorrelation = 0.0;
    int bestPeriod = 0;
    
    for (int p = 0; p < periodCount; p++) {
        int period = periods[p];
        
        /* Skip if period is too large for data */
        if (period >= dataSize / 2) {
            results[p] = 0.0;
            continue;
        }
        
        /* Calculate autocorrelation at this lag */
        double correlation = 0.0;
        int samples = 0;
        
        for (int i = period; i < dataSize - 1; i++) {
            correlation += returns[i] * returns[i - period];
            samples++;
        }
        
        if (samples > 0) {
            correlation /= samples;
            results[p] = correlation;
            
            /* Track highest correlation */
            if (fabs(correlation) > fabs(maxCorrelation)) {
                maxCorrelation = correlation;
                bestPeriod = period;
            }
        } else {
            results[p] = 0.0;
        }
    }
    
    /* Clean up */
    free(prices);
    free(returns);
    
    /* Return the period with highest correlation if significant */
    if (fabs(maxCorrelation) > 0.2) {
        return bestPeriod;
    } else {
        return 0; /* No significant seasonality detected */
    }
}