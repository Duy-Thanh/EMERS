/**
 * Technical Analysis Module
 * Implementation of technical indicators and analysis functions
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
#define INDICATOR_PLUS_DI 100
#define INDICATOR_MINUS_DI 101

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
    
    /* First EMA value is SMA */
    double ema = 0.0;
    calculateSMA(&data[dataSize - period], period, period, &ema);
    
    /* Calculate the multiplier */
    double multiplier = 2.0 / (period + 1.0);
    
    /* Calculate EMA for the remaining points */
    int i;
    for (i = dataSize - period + 1; i < dataSize; i++) {
        ema = (data[i].close - ema) * multiplier + ema;
    }
    
    *output = ema;
}

/* Relative Strength Index (RSI) */
void calculateRSI(const StockData* data, int dataSize, int period, double* output) {
    if (dataSize <= period || !data || !output) {
        if (output) *output = 0.0;
        return;
    }
    
    double gains = 0.0;
    double losses = 0.0;
    double change;
    int i;
    
    /* Calculate initial average gain and loss */
    for (i = 1; i <= period; i++) {
        change = data[dataSize - period - 1 + i].close - data[dataSize - period - 1 + i - 1].close;
        if (change > 0) {
            gains += change;
        } else {
            losses -= change; /* Make losses positive */
        }
    }
    
    double avgGain = gains / period;
    double avgLoss = losses / period;
    
    /* Calculate RSI for the remaining points using smoothed averages */
    for (i = period + 1; i < dataSize; i++) {
        change = data[dataSize - period - 1 + i].close - data[dataSize - period - 1 + i - 1].close;
        
        if (change > 0) {
            avgGain = (avgGain * (period - 1) + change) / period;
            avgLoss = (avgLoss * (period - 1)) / period;
        } else {
            avgGain = (avgGain * (period - 1)) / period;
            avgLoss = (avgLoss * (period - 1) - change) / period;
        }
    }
    
    /* Calculate RSI */
    if (avgLoss < 0.0001) { /* Avoid division by zero */
        *output = 100.0;
    } else {
        double rs = avgGain / avgLoss;
        *output = 100.0 - (100.0 / (1.0 + rs));
    }
}

/* Moving Average Convergence Divergence (MACD) */
void calculateMACD(const StockData* data, int dataSize, int fastPeriod, int slowPeriod, int signalPeriod, 
                  double* macdLine, double* signalLine, double* histogram) {
    if (dataSize < slowPeriod + signalPeriod || !data || !macdLine || !signalLine || !histogram) {
        if (macdLine) *macdLine = 0.0;
        if (signalLine) *signalLine = 0.0;
        if (histogram) *histogram = 0.0;
        return;
    }
    
    double fastEMA = 0.0;
    double slowEMA = 0.0;
    
    /* Calculate fast and slow EMAs */
    calculateEMA(data, dataSize, fastPeriod, &fastEMA);
    calculateEMA(data, dataSize, slowPeriod, &slowEMA);
    
    /* MACD Line = Fast EMA - Slow EMA */
    *macdLine = fastEMA - slowEMA;
    
    /* Calculate Signal Line (EMA of MACD Line) */
    /* For this simplified implementation, we'll use an approximation */
    *signalLine = *macdLine * 0.8; /* Simplified approximation */
    
    /* MACD Histogram = MACD Line - Signal Line */
    *histogram = *macdLine - *signalLine;
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
    
    /* Middle band is just the SMA */
    calculateSMA(data, dataSize, period, middleBand);
    
    /* Calculate standard deviation */
    double sum = 0.0;
    double deviation;
    int i;
    
    for (i = dataSize - period; i < dataSize; i++) {
        deviation = data[i].close - *middleBand;
        sum += deviation * deviation;
    }
    
    double stdDev = sqrt(sum / period);
    
    /* Calculate upper and lower bands */
    *upperBand = *middleBand + (stdDevMultiplier * stdDev);
    *lowerBand = *middleBand - (stdDevMultiplier * stdDev);
}

/* Average True Range (ATR) */
void calculateATR(const StockData* data, int dataSize, int period, double* output) {
    if (dataSize <= period || !data || !output) {
        if (output) *output = 0.0;
        return;
    }
    
    double* trueRanges = (double*)malloc(dataSize * sizeof(double));
    if (!trueRanges) {
        *output = 0.0;
        return;
    }
    
    /* Calculate True Range for each period */
    int i;
    for (i = 1; i < dataSize; i++) {
        /* True Range is the greatest of:
           1. Current High - Current Low
           2. |Current High - Previous Close|
           3. |Current Low - Previous Close| */
        double highLow = data[i].high - data[i].low;
        double highPrevClose = fabs(data[i].high - data[i-1].close);
        double lowPrevClose = fabs(data[i].low - data[i-1].close);
        
        trueRanges[i] = highLow;
        if (highPrevClose > trueRanges[i]) trueRanges[i] = highPrevClose;
        if (lowPrevClose > trueRanges[i]) trueRanges[i] = lowPrevClose;
    }
    
    /* Calculate first ATR as simple average of the first 'period' true ranges */
    double sum = 0.0;
    for (i = dataSize - period; i < dataSize; i++) {
        sum += trueRanges[i];
    }
    
    double atr = sum / period;
    
    free(trueRanges);
    *output = atr;
}

/* Money Flow Index (MFI) */
void calculateMFI(const StockData* data, int dataSize, int period, double* output) {
    if (!data || dataSize < period + 1 || !output) {
        if (output) *output = 0.0;
        return;
    }
    
    double* moneyFlow = (double*)malloc(dataSize * sizeof(double));
    double* positiveFlow = (double*)malloc(dataSize * sizeof(double));
    double* negativeFlow = (double*)malloc(dataSize * sizeof(double));
    
    if (!moneyFlow || !positiveFlow || !negativeFlow) {
        if (output) *output = 0.0;
        free(moneyFlow);
        free(positiveFlow);
        free(negativeFlow);
        return;
    }
    
    int i;
    /* Calculate typical price and money flow for each data point */
    for (i = 0; i < dataSize; i++) {
        double typicalPrice = (data[i].high + data[i].low + data[i].close) / 3.0;
        moneyFlow[i] = typicalPrice * data[i].volume;
    }
    
    /* Determine positive and negative money flows */
    for (i = 1; i < dataSize; i++) {
        double typicalPriceCurrent = (data[i].high + data[i].low + data[i].close) / 3.0;
        double typicalPricePrev = (data[i-1].high + data[i-1].low + data[i-1].close) / 3.0;
        
        if (typicalPriceCurrent > typicalPricePrev) {
            positiveFlow[i] = moneyFlow[i];
            negativeFlow[i] = 0.0;
        } else if (typicalPriceCurrent < typicalPricePrev) {
            positiveFlow[i] = 0.0;
            negativeFlow[i] = moneyFlow[i];
        } else {
            positiveFlow[i] = 0.0;
            negativeFlow[i] = 0.0;
        }
    }
    
    /* Calculate sums for the specified period */
    double posSum = 0.0;
    double negSum = 0.0;
    
    for (i = dataSize - period; i < dataSize; i++) {
        posSum += positiveFlow[i];
        negSum += negativeFlow[i];
    }
    
    /* Calculate MFI */
    if (negSum < 0.0001) { /* Avoid division by zero */
        *output = 100.0;
    } else {
        double moneyRatio = posSum / negSum;
        *output = 100.0 - (100.0 / (1.0 + moneyRatio));
    }
    
    /* Clean up */
    free(moneyFlow);
    free(positiveFlow);
    free(negativeFlow);
}

/* Calculate all indicators at once */
void calculateAllIndicators(const StockData* data, int dataSize, TechnicalIndicators* indicators) {
    if (!data || !indicators || dataSize <= 0) {
        return;
    }
    
    /* Initialize indicators */
    memset(indicators, 0, sizeof(TechnicalIndicators));
    
    /* Skip calculation if not enough data */
    if (dataSize < DEFAULT_SMA_PERIOD) {
        return;
    }
    
    /* Calculate each indicator */
    calculateSMA(data, dataSize, DEFAULT_SMA_PERIOD, &indicators->sma);
    calculateEMA(data, dataSize, DEFAULT_EMA_PERIOD, &indicators->ema);
    calculateRSI(data, dataSize, DEFAULT_RSI_PERIOD, &indicators->rsi);
    
    calculateMACD(data, dataSize, DEFAULT_MACD_FAST_PERIOD, DEFAULT_MACD_SLOW_PERIOD, DEFAULT_MACD_SIGNAL_PERIOD,
                 &indicators->macd, &indicators->macdSignal, &indicators->macdHistogram);
    
    calculateBollingerBands(data, dataSize, DEFAULT_BOLLINGER_PERIOD, DEFAULT_BOLLINGER_STDDEV,
                          &indicators->bollingerUpper, &indicators->bollingerMiddle, &indicators->bollingerLower);
    
    calculateATR(data, dataSize, DEFAULT_ATR_PERIOD, &indicators->atr);
}

/* Calculate standard deviation of an array of doubles */
double calculateStandardDeviation(const double* data, int dataSize) {
    if (!data || dataSize <= 1) {
        return 0.0;
    }
    
    /* Calculate mean */
    double sum = 0.0;
    int i;
    for (i = 0; i < dataSize; i++) {
        sum += data[i];
    }
    double mean = sum / dataSize;
    
    /* Calculate sum of squared differences */
    sum = 0.0;
    for (i = 0; i < dataSize; i++) {
        double diff = data[i] - mean;
        sum += diff * diff;
    }
    
    /* Return standard deviation */
    return sqrt(sum / dataSize);
}

/* Calculate average of an array of doubles */
double calculateAverage(const double* data, int dataSize) {
    if (!data || dataSize <= 0) {
        return 0.0;
    }
    
    double sum = 0.0;
    int i;
    for (i = 0; i < dataSize; i++) {
        sum += data[i];
    }
    
    return sum / dataSize;
}

/* Find the maximum value in an array of doubles */
double calculateMax(const double* data, int dataSize) {
    if (!data || dataSize <= 0) {
        return 0.0;
    }
    
    double max = data[0];
    int i;
    for (i = 1; i < dataSize; i++) {
        if (data[i] > max) {
            max = data[i];
        }
    }
    
    return max;
}

/* Find the minimum value in an array of doubles */
double calculateMin(const double* data, int dataSize) {
    if (!data || dataSize <= 0) {
        return 0.0;
    }
    
    double min = data[0];
    int i;
    for (i = 1; i < dataSize; i++) {
        if (data[i] < min) {
            min = data[i];
        }
    }
    
    return min;
}

/* Assembly-optimized functions would be implemented here */
#ifdef __ASSEMBLER__

/* Example of an assembly-optimized standard deviation calculation */
void asmCalculateStandardDeviation(const double* data, int dataSize, double* result) {
    /* This would be implemented with inline assembly for performance */
    /* For now, we'll just call the C implementation */
    *result = calculateStandardDeviation(data, dataSize);
}

/* Example of an assembly-optimized moving average calculation */
void asmCalculateMovingAverage(const double* data, int dataSize, int period, double* output) {
    /* This would be implemented with inline assembly for performance */
    /* For now, we'll generate simple output using the C implementation */
    int i;
    for (i = 0; i <= dataSize - period; i++) {
        output[i] = calculateAverage(&data[i], period);
    }
}

#endif

/* Parabolic SAR */
void calculatePSAR(const StockData* data, int dataSize, double acceleration, double maxAcceleration, double* output) {
    if (dataSize < 2 || !data || !output) {
        if (output) *output = 0.0;
        return;
    }

    // Initialize variables
    int trend = 0; // 1 for uptrend, -1 for downtrend
    double extremePoint = 0.0;
    double sar = 0.0;
    double af = acceleration; // Acceleration factor

    // Determine initial trend
    if (data[dataSize - 1].close > data[dataSize - 2].close) {
        trend = 1; // Uptrend
        sar = data[dataSize - 1].low;
        extremePoint = data[dataSize - 1].high;
    } else {
        trend = -1; // Downtrend
        sar = data[dataSize - 1].high;
        extremePoint = data[dataSize - 1].low;
    }

    // Calculate SAR value (we're only calculating the most recent value)
    if (trend == 1) { // Uptrend
        // SAR = Prior SAR + AF * (EP - Prior SAR)
        sar = sar + af * (extremePoint - sar);
        
        // Ensure SAR is not higher than the lowest low of the previous two periods
        double lowestLow = fmin(data[dataSize - 1].low, data[dataSize - 2].low);
        if (sar > lowestLow) {
            sar = lowestLow;
        }
        
        // Check if trend is changing
        if (data[dataSize - 1].low < sar) {
            // Trend changes to downtrend
            trend = -1;
            sar = extremePoint;
            extremePoint = data[dataSize - 1].low;
            af = acceleration;
        } else {
            // Check if we have a new extreme point
            if (data[dataSize - 1].high > extremePoint) {
                extremePoint = data[dataSize - 1].high;
                af = fmin(af + acceleration, maxAcceleration);
            }
        }
    } else { // Downtrend
        // SAR = Prior SAR - AF * (Prior SAR - EP)
        sar = sar - af * (sar - extremePoint);
        
        // Ensure SAR is not lower than the highest high of the previous two periods
        double highestHigh = fmax(data[dataSize - 1].high, data[dataSize - 2].high);
        if (sar < highestHigh) {
            sar = highestHigh;
        }
        
        // Check if trend is changing
        if (data[dataSize - 1].high > sar) {
            // Trend changes to uptrend
            trend = 1;
            sar = extremePoint;
            extremePoint = data[dataSize - 1].high;
            af = acceleration;
        } else {
            // Check if we have a new extreme point
            if (data[dataSize - 1].low < extremePoint) {
                extremePoint = data[dataSize - 1].low;
                af = fmin(af + acceleration, maxAcceleration);
            }
        }
    }
    
    *output = sar;
}

int calculateParabolicSAR(const StockData* data, int dataSize, double accelerationFactor, double maxAcceleration, TechnicalIndicator* indicator) {
    if (dataSize < 2 || !data || !indicator) {
        return ERR_INVALID_PARAMETER;
    }
    
    // Initialize the indicator
    strncpy(indicator->name, "Parabolic SAR", MAX_INDICATOR_NAME - 1);
    indicator->type = INDICATOR_PARABOLIC_SAR;
    indicator->periods[0] = 0; // PSAR doesn't use periods in the traditional sense
    indicator->lastIndex = 0;
    
    int i;
    int maxPoints = dataSize > MAX_DATA_POINTS ? MAX_DATA_POINTS : dataSize;
    
    // Calculate PSAR for each point
    double sar = 0.0;
    int trend = 0; // 1 for uptrend, -1 for downtrend
    double ep = 0.0; // Extreme point
    double af = accelerationFactor;
    
    // Determine initial trend
    if (data[0].close > data[1].close) {
        trend = 1; // Uptrend
        sar = data[0].low;
        ep = data[0].high;
    } else {
        trend = -1; // Downtrend
        sar = data[0].high;
        ep = data[0].low;
    }
    
    // Store the initial value
    indicator->values[0] = sar;
    
    // Calculate for the remaining points
    for (i = 1; i < maxPoints; i++) {
        if (trend == 1) { // Uptrend
            // SAR = Prior SAR + AF * (EP - Prior SAR)
            sar = indicator->values[i-1] + af * (ep - indicator->values[i-1]);
            
            // Ensure SAR is not higher than the lowest low of the previous two periods
            if (i > 1) {
                double lowLimit = fmin(data[i-1].low, data[i-2].low);
                if (sar > lowLimit) {
                    sar = lowLimit;
                }
            }
            
            // Check if trend is changing
            if (data[i].low < sar) {
                // Trend changes to downtrend
                trend = -1;
                sar = ep;
                ep = data[i].low;
                af = accelerationFactor;
            } else {
                // Check if we have a new extreme point
                if (data[i].high > ep) {
                    ep = data[i].high;
                    af = fmin(af + accelerationFactor, maxAcceleration);
                }
            }
        } else { // Downtrend
            // SAR = Prior SAR - AF * (Prior SAR - EP)
            sar = indicator->values[i-1] - af * (indicator->values[i-1] - ep);
            
            // Ensure SAR is not lower than the highest high of the previous two periods
            if (i > 1) {
                double highLimit = fmax(data[i-1].high, data[i-2].high);
                if (sar < highLimit) {
                    sar = highLimit;
                }
            }
            
            // Check if trend is changing
            if (data[i].high > sar) {
                // Trend changes to uptrend
                trend = 1;
                sar = ep;
                ep = data[i].high;
                af = accelerationFactor;
            } else {
                // Check if we have a new extreme point
                if (data[i].low < ep) {
                    ep = data[i].low;
                    af = fmin(af + accelerationFactor, maxAcceleration);
                }
            }
        }
        
        indicator->values[i] = sar;
    }
    
    indicator->lastIndex = maxPoints - 1;
    return SUCCESS;
}

/* Stochastic Oscillator */
void calculateStochastic(const StockData* data, int dataSize, int kPeriod, int dPeriod, double* k, double* d) {
    if (!data || dataSize < kPeriod || !k || !d) {
        if (k) *k = 0.0;
        if (d) *d = 0.0;
        return;
    }
    
    int i;
    double highestHigh = data[dataSize - kPeriod].high;
    double lowestLow = data[dataSize - kPeriod].low;
    
    /* Find highest high and lowest low over the kPeriod */
    for (i = dataSize - kPeriod + 1; i < dataSize; i++) {
        if (data[i].high > highestHigh) {
            highestHigh = data[i].high;
        }
        if (data[i].low < lowestLow) {
            lowestLow = data[i].low;
        }
    }
    
    /* Calculate %K */
    double currentClose = data[dataSize - 1].close;
    if (highestHigh - lowestLow < 0.0001) { /* Avoid division by zero */
        *k = 50.0; /* Default to middle value */
    } else {
        *k = 100.0 * ((currentClose - lowestLow) / (highestHigh - lowestLow));
    }
    
    /* For simplicity, we'll just use a simple average for %D */
    /* In a complete implementation, this would be a moving average over dPeriod */
    *d = *k;  /* Simplified implementation */
}

/* Average Directional Index (ADX) */
void calculateADX(const StockData* data, int dataSize, int period, double* adx, double* diPlus, double* diMinus) {
    if (!data || dataSize < period + 1 || !adx || !diPlus || !diMinus) {
        if (adx) *adx = 0.0;
        if (diPlus) *diPlus = 0.0;
        if (diMinus) *diMinus = 0.0;
        return;
    }

    /* Allocate memory for intermediate calculations */
    double* trueRanges = (double*)malloc(dataSize * sizeof(double));
    double* plusDM = (double*)malloc(dataSize * sizeof(double));
    double* minusDM = (double*)malloc(dataSize * sizeof(double));

    if (!trueRanges || !plusDM || !minusDM) {
        if (adx) *adx = 0.0;
        if (diPlus) *diPlus = 0.0;
        if (diMinus) *diMinus = 0.0;
        free(trueRanges);
        free(plusDM);
        free(minusDM);
        return;
    }

    /* Calculate TR, +DM, -DM for each period */
    int i;
    for (i = 1; i < dataSize; i++) {
        /* True Range */
        double highLow = data[i].high - data[i].low;
        double highPrevClose = fabs(data[i].high - data[i-1].close);
        double lowPrevClose = fabs(data[i].low - data[i-1].close);
        
        trueRanges[i] = fmax(highLow, fmax(highPrevClose, lowPrevClose));

        /* Directional Movement */
        double upMove = data[i].high - data[i-1].high;
        double downMove = data[i-1].low - data[i].low;

        if (upMove > downMove && upMove > 0) {
            plusDM[i] = upMove;
        } else {
            plusDM[i] = 0;
        }

        if (downMove > upMove && downMove > 0) {
            minusDM[i] = downMove;
        } else {
            minusDM[i] = 0;
        }
    }

    /* Calculate smoothed TR and DMs */
    double smoothedTR = 0;
    double smoothedPlusDM = 0;
    double smoothedMinusDM = 0;

    /* First period values are sums */
    for (i = 1; i <= period; i++) {
        smoothedTR += trueRanges[i];
        smoothedPlusDM += plusDM[i];
        smoothedMinusDM += minusDM[i];
    }

    /* Calculate first +DI and -DI values */
    double plusDI_val = 100.0 * smoothedPlusDM / smoothedTR;
    double minusDI_val = 100.0 * smoothedMinusDM / smoothedTR;

    /* Calculate first DX value */
    double dx = 100.0 * fabs(plusDI_val - minusDI_val) / (plusDI_val + minusDI_val);

    /* Initialize ADX with first DX value */
    double adxValue = dx;

    /* For the remaining periods, use smoothing */
    for (i = period + 1; i < dataSize; i++) {
        /* Update smoothed values */
        smoothedTR = smoothedTR - (smoothedTR / period) + trueRanges[i];
        smoothedPlusDM = smoothedPlusDM - (smoothedPlusDM / period) + plusDM[i];
        smoothedMinusDM = smoothedMinusDM - (smoothedMinusDM / period) + minusDM[i];

        /* Update +DI and -DI */
        plusDI_val = 100.0 * smoothedPlusDM / smoothedTR;
        minusDI_val = 100.0 * smoothedMinusDM / smoothedTR;

        /* Update DX */
        dx = 100.0 * fabs(plusDI_val - minusDI_val) / (plusDI_val + minusDI_val);

        /* Smooth ADX with previous value */
        adxValue = ((period - 1) * adxValue + dx) / period;
    }

    /* Set return values */
    *adx = adxValue;
    *diPlus = plusDI_val;
    *diMinus = minusDI_val;

    /* Clean up */
    free(trueRanges);
    free(plusDM);
    free(minusDM);
}

/* Calculate all extended indicators at once */
void calculateExtendedIndicators(const StockData* data, int dataSize, ExtendedTechnicalIndicators* indicators) {
    if (!data || !indicators || dataSize <= 0) {
        return;
    }

    /* Initialize indicators */
    memset(indicators, 0, sizeof(ExtendedTechnicalIndicators));
    
    /* Skip calculation if not enough data */
    if (dataSize < DEFAULT_SMA_PERIOD) {
        return;
    }
    
    /* Calculate basic indicators */
    calculateSMA(data, dataSize, DEFAULT_SMA_PERIOD, &indicators->sma);
    calculateEMA(data, dataSize, DEFAULT_EMA_PERIOD, &indicators->ema);
    calculateRSI(data, dataSize, DEFAULT_RSI_PERIOD, &indicators->rsi);
    
    calculateMACD(data, dataSize, DEFAULT_MACD_FAST_PERIOD, DEFAULT_MACD_SLOW_PERIOD, DEFAULT_MACD_SIGNAL_PERIOD,
                 &indicators->macd, &indicators->macdSignal, &indicators->macdHistogram);
    
    calculateBollingerBands(data, dataSize, DEFAULT_BOLLINGER_PERIOD, DEFAULT_BOLLINGER_STDDEV,
                          &indicators->bollingerUpper, &indicators->bollingerMiddle, &indicators->bollingerLower);
    
    calculateATR(data, dataSize, DEFAULT_ATR_PERIOD, &indicators->atr);
    
    /* Calculate advanced indicators */
    double adx, diPlus, diMinus;
    calculateADX(data, dataSize, DEFAULT_ADX_PERIOD, &adx, &diPlus, &diMinus);
    indicators->adx = adx;
    indicators->diPlus = diPlus;
    indicators->diMinus = diMinus;
    
    double stochK, stochD;
    calculateStochastic(data, dataSize, DEFAULT_STOCHASTIC_K_PERIOD, DEFAULT_STOCHASTIC_D_PERIOD, &stochK, &stochD);
    indicators->stochasticK = stochK;
    indicators->stochasticD = stochD;
    
    calculateMFI(data, dataSize, DEFAULT_MFI_PERIOD, &indicators->mfi);
    calculatePSAR(data, dataSize, DEFAULT_PSAR_ACCELERATION, DEFAULT_PSAR_MAX_ACCELERATION, &indicators->psar);
    
    /* Event-adjusted indicators would be calculated here in a real implementation */
    /* For now, just set them to regular values */
    indicators->eventADX = indicators->adx;
    indicators->eventStochasticK = indicators->stochasticK;
    indicators->eventStochasticD = indicators->stochasticD;
    indicators->eventMFI = indicators->mfi;
    indicators->eventPSAR = indicators->psar;
}

/* Calculate all event-adjusted indicators */
void calculateEventAdjustedIndicators(const StockData* data, int dataSize, const EventData* event, ExtendedTechnicalIndicators* indicators) {
    if (!data || !indicators || dataSize <= 0) {
        return;
    }
    
    /* First calculate regular technical indicators */
    calculateExtendedIndicators(data, dataSize, indicators);
    
    /* If no event is provided, just use the regular indicators */
    if (!event) {
        indicators->eventADX = indicators->adx;
        indicators->eventStochasticK = indicators->stochasticK;
        indicators->eventStochasticD = indicators->stochasticD; 
        indicators->eventMFI = indicators->mfi;
        indicators->eventPSAR = indicators->psar;
        return;
    }
    
    /* Adjust indicators based on event sentiment and impact */
    /* This is a simplified implementation for demonstration purposes */
    /* In a real implementation, this would analyze the event in detail */
    
    /* Event adjustment factor based on sentiment and impact */
    double eventFactor = event->sentiment * (event->impactScore / 100.0);
    
    /* Adjust indicators with event factor */
    /* Positive sentiment increases bullish indicators, negative decreases them */
    
    /* ADX adjustment - increase for high impact events */
    indicators->eventADX = indicators->adx * (1.0 + fabs(eventFactor) * 0.2);
    
    /* Stochastic adjustment - shift up for positive events, down for negative */
    indicators->eventStochasticK = fmin(100.0, fmax(0.0, indicators->stochasticK + (eventFactor * 10.0)));
    indicators->eventStochasticD = fmin(100.0, fmax(0.0, indicators->stochasticD + (eventFactor * 10.0)));
    
    /* MFI adjustment - similar to stochastic */
    indicators->eventMFI = fmin(100.0, fmax(0.0, indicators->mfi + (eventFactor * 10.0)));
    
    /* PSAR adjustment - move SAR down for positive events (making uptrend more likely)
       and move SAR up for negative events */
    if (eventFactor > 0) {
        indicators->eventPSAR = indicators->psar * (1.0 - eventFactor * 0.1);
    } else {
        indicators->eventPSAR = indicators->psar * (1.0 - eventFactor * 0.1);
    }
}