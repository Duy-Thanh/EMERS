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

/**
 * CORE ALGORITHM 1: Detect price patterns using pattern recognition
 * Focuses on support/resistance levels, trend changes, and double tops/bottoms
 */
int detectPricePatterns(const StockData* data, int dataSize, PatternResult* patterns, int maxPatterns) {
    if (!data || dataSize < 20 || !patterns || maxPatterns <= 0) {
        return 0;
    }
    
    int patternCount = 0;
    
    /* Detect support and resistance levels */
    if (patternCount < maxPatterns) {
        /* Find recent highs and lows */
        double recentHigh = 0.0;
        double recentLow = 999999.0;
        int highIndex = 0;
        int lowIndex = 0;
        
        for (int i = dataSize - 20; i < dataSize; i++) {
            if (data[i].high > recentHigh) {
                recentHigh = data[i].high;
                highIndex = i;
            }
            if (data[i].low < recentLow) {
                recentLow = data[i].low;
                lowIndex = i;
            }
        }
        
        /* Create pattern for resistance level */
        strcpy(patterns[patternCount].name, "Resistance Level");
        patterns[patternCount].type = PATTERN_RESISTANCE;
        snprintf(patterns[patternCount].description, 
                 sizeof(patterns[patternCount].description),
                 "Price resistance at %.2f", recentHigh);
        patterns[patternCount].startIndex = dataSize - 20;
        patterns[patternCount].endIndex = dataSize - 1;
        patterns[patternCount].confidence = 0.6;
        patterns[patternCount].expectedMove = -0.02; /* Expected 2% drop when price approaches resistance */
        patternCount++;
        
        /* Create pattern for support level if we have room */
        if (patternCount < maxPatterns) {
            strcpy(patterns[patternCount].name, "Support Level");
            patterns[patternCount].type = PATTERN_SUPPORT;
            snprintf(patterns[patternCount].description, 
                     sizeof(patterns[patternCount].description),
                     "Price support at %.2f", recentLow);
            patterns[patternCount].startIndex = dataSize - 20;
            patterns[patternCount].endIndex = dataSize - 1;
            patterns[patternCount].confidence = 0.6;
            patterns[patternCount].expectedMove = 0.02; /* Expected 2% rise when price approaches support */
            patternCount++;
        }
    }
    
    /* Detect trend change if we have room */
    if (patternCount < maxPatterns) {
        /* Calculate short and long moving averages */
        double shortSMA = 0.0;
        double longSMA = 0.0;
        
        /* Use last 10 days for short MA and last 30 days for long MA */
        double shortSum = 0.0;
        double longSum = 0.0;
        
        for (int i = dataSize - 30; i < dataSize; i++) {
            if (i >= dataSize - 10) {
                shortSum += data[i].close;
            }
            longSum += data[i].close;
        }
        
        shortSMA = shortSum / 10;
        longSMA = longSum / 30;
        
        /* Check for crossover */
        double prevShortSMA = 0.0;
        double prevLongSMA = 0.0;
        
        shortSum = 0.0;
        longSum = 0.0;
        
        for (int i = dataSize - 31; i < dataSize - 1; i++) {
            if (i >= dataSize - 11) {
                shortSum += data[i].close;
            }
            longSum += data[i].close;
        }
        
        prevShortSMA = shortSum / 10;
        prevLongSMA = longSum / 30;
        
        if (prevShortSMA <= prevLongSMA && shortSMA > longSMA) {
            /* Bullish crossover */
            strcpy(patterns[patternCount].name, "Bullish Trend Change");
            patterns[patternCount].type = PATTERN_TREND_CHANGE;
            strcpy(patterns[patternCount].description, "Short-term MA crossed above long-term MA");
            patterns[patternCount].startIndex = dataSize - 30;
            patterns[patternCount].endIndex = dataSize - 1;
            patterns[patternCount].confidence = 0.7;
            patterns[patternCount].expectedMove = 0.03; /* Expected 3% rise */
            patternCount++;
        } else if (prevShortSMA >= prevLongSMA && shortSMA < longSMA && patternCount < maxPatterns) {
            /* Bearish crossover */
            strcpy(patterns[patternCount].name, "Bearish Trend Change");
            patterns[patternCount].type = PATTERN_TREND_CHANGE;
            strcpy(patterns[patternCount].description, "Short-term MA crossed below long-term MA");
            patterns[patternCount].startIndex = dataSize - 30;
            patterns[patternCount].endIndex = dataSize - 1;
            patterns[patternCount].confidence = 0.7;
            patterns[patternCount].expectedMove = -0.03; /* Expected 3% drop */
            patternCount++;
        }
    }
    
    /* Detect double top/bottom patterns */
    if (patternCount < maxPatterns && dataSize > 60) {
        /* Look for double top pattern */
        int peak1Index = -1;
        int peak2Index = -1;
        int troughIndex = -1;
        double peak1Value = 0.0;
        double peak2Value = 0.0;
        double troughValue = 0.0;
        
        /* Simplified algorithm to find peaks and troughs */
        for (int i = dataSize - 60; i < dataSize - 40; i++) {
            if (data[i].high > data[i-1].high && data[i].high > data[i+1].high) {
                peak1Index = i;
                peak1Value = data[i].high;
                break;
            }
        }
        
        if (peak1Index != -1) {
            /* Look for trough after first peak */
            for (int i = peak1Index + 1; i < dataSize - 20; i++) {
                if (data[i].low < data[i-1].low && data[i].low < data[i+1].low) {
                    troughIndex = i;
                    troughValue = data[i].low;
                    break;
                }
            }
            
            if (troughIndex != -1) {
                /* Look for second peak after trough */
                for (int i = troughIndex + 1; i < dataSize; i++) {
                    if (data[i].high > data[i-1].high && data[i].high > data[i+1].high) {
                        peak2Index = i;
                        peak2Value = data[i].high;
                        break;
                    }
                }
            }
        }
        
        /* Check if we found a double top pattern */
        if (peak1Index != -1 && troughIndex != -1 && peak2Index != -1) {
            /* Check if peaks are at similar levels (within 2%) */
            double peakDiff = fabs(peak1Value - peak2Value) / peak1Value;
            
            if (peakDiff < 0.02 && (peak1Value - troughValue) / peak1Value > 0.03) {
                strcpy(patterns[patternCount].name, "Double Top");
                patterns[patternCount].type = PATTERN_DOUBLE_TOP;
                strcpy(patterns[patternCount].description, "Double top pattern detected");
                patterns[patternCount].startIndex = peak1Index;
                patterns[patternCount].endIndex = peak2Index;
                patterns[patternCount].confidence = 0.6;
                patterns[patternCount].expectedMove = -0.04; /* Expected 4% drop */
                patternCount++;
            }
        }
        
        /* Check for double bottom pattern if we have room */
        if (patternCount < maxPatterns) {
            /* Reset variables */
            int bottom1Index = -1;
            int bottom2Index = -1;
            int peakIndex = -1;
            double bottom1Value = 9999.0;
            double bottom2Value = 9999.0;
            double peakValue = 0.0;
            
            /* Simplified algorithm to find bottoms and peak */
            for (int i = dataSize - 60; i < dataSize - 40; i++) {
                if (data[i].low < data[i-1].low && data[i].low < data[i+1].low) {
                    bottom1Index = i;
                    bottom1Value = data[i].low;
                    break;
                }
            }
            
            if (bottom1Index != -1) {
                /* Look for peak after first bottom */
                for (int i = bottom1Index + 1; i < dataSize - 20; i++) {
                    if (data[i].high > data[i-1].high && data[i].high > data[i+1].high) {
                        peakIndex = i;
                        peakValue = data[i].high;
                        break;
                    }
                }
                
                if (peakIndex != -1) {
                    /* Look for second bottom after peak */
                    for (int i = peakIndex + 1; i < dataSize; i++) {
                        if (data[i].low < data[i-1].low && data[i].low < data[i+1].low) {
                            bottom2Index = i;
                            bottom2Value = data[i].low;
                            break;
                        }
                    }
                }
            }
            
            /* Check if we found a double bottom pattern */
            if (bottom1Index != -1 && peakIndex != -1 && bottom2Index != -1) {
                /* Check if bottoms are at similar levels (within 2%) */
                double bottomDiff = fabs(bottom1Value - bottom2Value) / bottom1Value;
                
                if (bottomDiff < 0.02 && (peakValue - bottom1Value) / bottom1Value > 0.03) {
                    strcpy(patterns[patternCount].name, "Double Bottom");
                    patterns[patternCount].type = PATTERN_DOUBLE_BOTTOM;
                    strcpy(patterns[patternCount].description, "Double bottom pattern detected");
                    patterns[patternCount].startIndex = bottom1Index;
                    patterns[patternCount].endIndex = bottom2Index;
                    patterns[patternCount].confidence = 0.6;
                    patterns[patternCount].expectedMove = 0.04; /* Expected 4% rise */
                    patternCount++;
                }
            }
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
 * Detect anomalies in price and volume data
 * Enhanced to provide more detailed anomaly information
 */
int detectAnomalies(const StockData* data, int dataSize, AnomalyResult* anomalies, int maxAnomalies) {
    if (!data || dataSize < 30 || !anomalies || maxAnomalies <= 0) {
        return 0;
    }
    
    int anomalyCount = 0;
    
    /* Calculate average price change and volatility */
    double sumChange = 0.0;
    double sumVolChange = 0.0;
    double sumSqChange = 0.0;
    double sumSqVolChange = 0.0;
    
    for (int i = dataSize - 30; i < dataSize; i++) {
        double priceChange = fabs((data[i].close - data[i-1].close) / data[i-1].close);
        sumChange += priceChange;
        sumSqChange += priceChange * priceChange;
        
        if (i > dataSize - 30) {
            double volChange = fabs((data[i].volume - data[i-1].volume) / data[i-1].volume);
            sumVolChange += volChange;
            sumSqVolChange += volChange * volChange;
        }
    }
    
    double avgChange = sumChange / 30.0;
    double stdDevChange = sqrt(sumSqChange / 30.0 - avgChange * avgChange);
    
    double avgVolChange = sumVolChange / 29.0;
    double stdDevVolChange = sqrt(sumSqVolChange / 29.0 - avgVolChange * avgVolChange);
    
    /* Threshold for anomaly detection */
    double priceThreshold = avgChange + (2.5 * stdDevChange);
    double volumeThreshold = avgVolChange + (2.5 * stdDevVolChange);
    
    /* Scan for anomalies */
    for (int i = dataSize - 30; i < dataSize && anomalyCount < maxAnomalies; i++) {
        double priceChange = fabs((data[i].close - data[i-1].close) / data[i-1].close);
        double volChange = 0.0;
        
        if (i > dataSize - 30) {
            volChange = fabs((data[i].volume - data[i-1].volume) / data[i-1].volume);
            
            double priceDeviation = (priceChange - avgChange) / stdDevChange;
            double volumeDeviation = (volChange - avgVolChange) / stdDevVolChange;
            
            /* Calculate anomaly score */
            double score = sqrt(priceDeviation*priceDeviation + volumeDeviation*volumeDeviation);
            
            if (score > 2.5) {
                anomalies[anomalyCount].index = i;
                anomalies[anomalyCount].score = score;
                anomalies[anomalyCount].priceDeviation = priceDeviation;
                anomalies[anomalyCount].volumeDeviation = volumeDeviation;
                
                /* Create description based on the nature of the anomaly */
                if (priceDeviation > 2.0 && volumeDeviation > 2.0) {
                    snprintf(anomalies[anomalyCount].description, 128, 
                        "Major price (%.1f sigma) and volume (%.1f sigma) anomaly on %s", 
                        priceDeviation, volumeDeviation, data[i].date);
                } else if (priceDeviation > 2.0) {
                    snprintf(anomalies[anomalyCount].description, 128, 
                        "Price anomaly (%.1f sigma) on %s", priceDeviation, data[i].date);
                } else if (volumeDeviation > 2.0) {
                    snprintf(anomalies[anomalyCount].description, 128, 
                        "Volume anomaly (%.1f sigma) on %s", volumeDeviation, data[i].date);
                } else {
                    snprintf(anomalies[anomalyCount].description, 128, 
                        "Combined anomaly (score: %.1f) on %s", score, data[i].date);
                }
                
                anomalyCount++;
            }
        }
    }
    
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