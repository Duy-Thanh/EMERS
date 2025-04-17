/**
 * Technical Analysis Module
 * Implementation of technical indicators and analysis functions
 * SIMPLIFIED VERSION
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "../include/emers.h"
#include "../include/technical_analysis.h"
#include "../include/error_handling.h"

/* Define missing constants */
#define SUCCESS 0

/* Simple Moving Average (SMA) */
void calculateSMA(const StockData* data, int dataSize, int period, double* output) {
    if (dataSize < period || !data || !output) {
        if (output) *output = 0.0;
        return;
    }
    
    double sum = 0.0;
    int i;
    
    /* Sum the most recent 'period' closing prices */
    for (i = dataSize - period; i < dataSize; i++) {
        sum += data[i].close;
    }
    
    /* Calculate the average */
    *output = sum / period;
}

/* Exponential Moving Average (EMA) */
void calculateEMA(const StockData* data, int dataSize, int period, double* output) {
    if (dataSize < period || !data || !output) {
        if (output) *output = 0.0;
        return;
    }
    
    /* Start with SMA as the first EMA value */
    double sum = 0.0;
    int i;
    
    for (i = 0; i < period; i++) {
        sum += data[i].close;
    }
    
    double ema = sum / period;
    
    /* Calculate the multiplier */
    double multiplier = 2.0 / (period + 1.0);
    
    /* Calculate EMA for remaining data points */
    for (i = period; i < dataSize; i++) {
        ema = (data[i].close - ema) * multiplier + ema;
    }
    
    *output = ema;
}

/* Relative Strength Index (RSI) */
void calculateRSI(const StockData* data, int dataSize, int period, double* output) {
    if (dataSize < period + 1 || !data || !output) {
        if (output) *output = 0.0;
        return;
    }
    
    double gain = 0.0;
    double loss = 0.0;
    double change;
    int i;
    
    /* Calculate first average gain and loss */
    for (i = 1; i <= period; i++) {
        change = data[i].close - data[i-1].close;
        if (change >= 0) {
            gain += change;
        } else {
            loss -= change;
        }
    }
    
    /* Initial averages */
    double avgGain = gain / period;
    double avgLoss = loss / period;
    
    /* Calculate RSI for remaining data */
    for (i = period + 1; i < dataSize; i++) {
        change = data[i].close - data[i-1].close;
        
        if (change >= 0) {
            avgGain = (avgGain * (period - 1) + change) / period;
            avgLoss = (avgLoss * (period - 1)) / period;
        } else {
            avgGain = (avgGain * (period - 1)) / period;
            avgLoss = (avgLoss * (period - 1) - change) / period;
        }
    }
    
    /* Calculate RSI */
    if (avgLoss > 0) {
        *output = 100.0 - (100.0 / (1.0 + (avgGain / avgLoss)));
    } else {
        *output = 100.0;
    }
}

/* Bollinger Bands */
void calculateBollingerBands(const StockData* data, int dataSize, int period, double stdDevMultiplier,
                           double* upperBand, double* middleBand, double* lowerBand) {
    if (dataSize < period || !data || !upperBand || !middleBand || !lowerBand) {
        if (upperBand) *upperBand = 0.0;
        if (middleBand) *middleBand = 0.0;
        if (lowerBand) *lowerBand = 0.0;
        return;
    }
    
    /* Calculate the SMA (middle band) */
    double sum = 0.0;
    int i;
    
    for (i = dataSize - period; i < dataSize; i++) {
        sum += data[i].close;
    }
    
    double sma = sum / period;
    *middleBand = sma;
    
    /* Calculate standard deviation */
    double sumSquaredDiff = 0.0;
    
    for (i = dataSize - period; i < dataSize; i++) {
        double diff = data[i].close - sma;
        sumSquaredDiff += diff * diff;
    }
    
    double stdDev = sqrt(sumSquaredDiff / period);
    
    /* Calculate upper and lower bands */
    *upperBand = sma + (stdDevMultiplier * stdDev);
    *lowerBand = sma - (stdDevMultiplier * stdDev);
}

/* Calculate all essential indicators */
void calculateAllIndicators(const StockData* data, int dataSize, TechnicalIndicators* indicators) {
    if (!data || dataSize < 20 || !indicators) {
        return;
    }
    
    /* Initialize indicators structure */
    memset(indicators, 0, sizeof(TechnicalIndicators));
    
    /* Calculate essential indicators */
    calculateSMA(data, dataSize, DEFAULT_SMA_PERIOD, &indicators->sma);
    calculateEMA(data, dataSize, DEFAULT_EMA_PERIOD, &indicators->ema);
    calculateRSI(data, dataSize, DEFAULT_RSI_PERIOD, &indicators->rsi);
    
    /* Calculate MACD - simplified version */
    double fastEMA, slowEMA;
    calculateEMA(data, dataSize, DEFAULT_MACD_FAST_PERIOD, &fastEMA);
    calculateEMA(data, dataSize, DEFAULT_MACD_SLOW_PERIOD, &slowEMA);
    
    indicators->macd = fastEMA - slowEMA;
    indicators->macdSignal = indicators->macd * 0.8; // Simplified approximation
    indicators->macdHistogram = indicators->macd - indicators->macdSignal;
    
    calculateBollingerBands(data, dataSize, DEFAULT_BOLLINGER_PERIOD, DEFAULT_BOLLINGER_STDDEV,
                          &indicators->bollingerUpper, &indicators->bollingerMiddle, &indicators->bollingerLower);
}