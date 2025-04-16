/**
 * Data Mining Module
 * Implementation of pattern recognition and data mining algorithms
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "../include/emers.h"
#include "../include/data_mining.h"
#include "../include/technical_analysis.h"

/* Helper function declarations */
static int detectDoubleTop(const StockData* data, int dataSize, PatternResult* pattern);
static int detectDoubleBottom(const StockData* data, int dataSize, PatternResult* pattern);
static int detectHeadAndShoulders(const StockData* data, int dataSize, PatternResult* pattern);
static double calculateZScore(const double* series, int length, double value);
static double min3(double a, double b, double c);
static double normCDF(double x);

/* Pattern recognition */
int detectPricePatterns(const StockData* data, int dataSize, PatternResult* patterns, int maxPatterns) {
    if (!data || dataSize < 20 || !patterns || maxPatterns <= 0) {
        return 0;
    }
    
    int patternCount = 0;
    
    /* Detect Double Top pattern */
    if (patternCount < maxPatterns && 
        detectDoubleTop(data, dataSize, &patterns[patternCount])) {
        patternCount++;
    }
    
    /* Detect Double Bottom pattern */
    if (patternCount < maxPatterns && 
        detectDoubleBottom(data, dataSize, &patterns[patternCount])) {
        patternCount++;
    }
    
    /* Detect Head and Shoulders pattern */
    if (patternCount < maxPatterns && 
        detectHeadAndShoulders(data, dataSize, &patterns[patternCount])) {
        patternCount++;
    }
    
    /* Other pattern detections would be implemented here */
    
    return patternCount;
}

/* Detect Double Top pattern */
static int detectDoubleTop(const StockData* data, int dataSize, PatternResult* pattern) {
    if (dataSize < 40) return 0;  /* Need enough data */
    
    /* Extract closing prices */
    double* closes = (double*)malloc(dataSize * sizeof(double));
    if (!closes) return 0;
    
    int i;
    for (i = 0; i < dataSize; i++) {
        closes[i] = data[i].close;
    }
    
    /* Look for two peaks with similar heights */
    int foundPattern = 0;
    int peakIdx1 = -1, peakIdx2 = -1, troughIdx = -1;
    
    /* Scan the last 50 days (or less if not enough data) */
    int scanStart = dataSize - 50;
    if (scanStart < 0) scanStart = 0;
    
    /* Simple peak detection */
    for (i = scanStart + 5; i < dataSize - 5; i++) {
        /* Potential peak: higher than 2 points on either side */
        if (closes[i] > closes[i-1] && closes[i] > closes[i-2] && 
            closes[i] > closes[i+1] && closes[i] > closes[i+2]) {
            
            if (peakIdx1 == -1) {
                peakIdx1 = i;
            } else if (peakIdx2 == -1 && i - peakIdx1 >= 10) {  /* Peaks should be separated */
                /* Only consider if both peaks are at similar heights */
                double peakRatio = fabs(closes[i] - closes[peakIdx1]) / closes[peakIdx1];
                
                if (peakRatio < 0.05) {  /* Peaks within 5% of each other */
                    peakIdx2 = i;
                    
                    /* Look for trough between peaks */
                    double minVal = INFINITY;
                    int minIdx = -1;
                    
                    for (int j = peakIdx1 + 1; j < peakIdx2; j++) {
                        if (closes[j] < minVal) {
                            minVal = closes[j];
                            minIdx = j;
                        }
                    }
                    
                    if (minIdx != -1) {
                        troughIdx = minIdx;
                        
                        /* Confirm pattern: trough should be significantly lower than peaks */
                        double troughDepth = (closes[peakIdx1] - closes[troughIdx]) / closes[peakIdx1];
                        
                        if (troughDepth > 0.03) {  /* Trough at least 3% lower than peaks */
                            foundPattern = 1;
                            break;
                        }
                    }
                }
                
                /* Reset if second peak not valid */
                peakIdx1 = i;
                peakIdx2 = -1;
            }
        }
    }
    
    /* Fill in pattern details if found */
    if (foundPattern) {
        pattern->type = PATTERN_DOUBLE_TOP;
        strcpy(pattern->description, "Double Top pattern detected - bearish reversal pattern");
        pattern->startIndex = peakIdx1 - 5;  /* Include some context */
        pattern->endIndex = peakIdx2 + 5;    /* Include some context */
        pattern->confidence = 0.7;  /* Basic confidence level */
        
        /* Expected downside move based on pattern height */
        double patternHeight = closes[peakIdx1] - closes[troughIdx];
        pattern->expectedMove = -patternHeight / closes[peakIdx2];  /* Negative for downside move */
    }
    
    free(closes);
    return foundPattern;
}

/* Detect Double Bottom pattern */
static int detectDoubleBottom(const StockData* data, int dataSize, PatternResult* pattern) {
    if (dataSize < 40) return 0;  /* Need enough data */
    
    /* Extract closing prices */
    double* closes = (double*)malloc(dataSize * sizeof(double));
    if (!closes) return 0;
    
    int i;
    for (i = 0; i < dataSize; i++) {
        closes[i] = data[i].close;
    }
    
    /* Look for two troughs with similar heights */
    int foundPattern = 0;
    int troughIdx1 = -1, troughIdx2 = -1, peakIdx = -1;
    
    /* Scan the last 50 days (or less if not enough data) */
    int scanStart = dataSize - 50;
    if (scanStart < 0) scanStart = 0;
    
    /* Simple trough detection */
    for (i = scanStart + 5; i < dataSize - 5; i++) {
        /* Potential trough: lower than 2 points on either side */
        if (closes[i] < closes[i-1] && closes[i] < closes[i-2] && 
            closes[i] < closes[i+1] && closes[i] < closes[i+2]) {
            
            if (troughIdx1 == -1) {
                troughIdx1 = i;
            } else if (troughIdx2 == -1 && i - troughIdx1 >= 10) {  /* Troughs should be separated */
                /* Only consider if both troughs are at similar heights */
                double troughRatio = fabs(closes[i] - closes[troughIdx1]) / closes[troughIdx1];
                
                if (troughRatio < 0.05) {  /* Troughs within 5% of each other */
                    troughIdx2 = i;
                    
                    /* Look for peak between troughs */
                    double maxVal = -INFINITY;
                    int maxIdx = -1;
                    
                    for (int j = troughIdx1 + 1; j < troughIdx2; j++) {
                        if (closes[j] > maxVal) {
                            maxVal = closes[j];
                            maxIdx = j;
                        }
                    }
                    
                    if (maxIdx != -1) {
                        peakIdx = maxIdx;
                        
                        /* Confirm pattern: peak should be significantly higher than troughs */
                        double peakHeight = (closes[peakIdx] - closes[troughIdx1]) / closes[troughIdx1];
                        
                        if (peakHeight > 0.03) {  /* Peak at least 3% higher than troughs */
                            foundPattern = 1;
                            break;
                        }
                    }
                }
                
                /* Reset if second trough not valid */
                troughIdx1 = i;
                troughIdx2 = -1;
            }
        }
    }
    
    /* Fill in pattern details if found */
    if (foundPattern) {
        pattern->type = PATTERN_DOUBLE_BOTTOM;
        strcpy(pattern->description, "Double Bottom pattern detected - bullish reversal pattern");
        pattern->startIndex = troughIdx1 - 5;  /* Include some context */
        pattern->endIndex = troughIdx2 + 5;    /* Include some context */
        pattern->confidence = 0.7;  /* Basic confidence level */
        
        /* Expected upside move based on pattern height */
        double patternHeight = closes[peakIdx] - closes[troughIdx1];
        pattern->expectedMove = patternHeight / closes[troughIdx2];  /* Positive for upside move */
    }
    
    free(closes);
    return foundPattern;
}

/* Detect Head and Shoulders pattern */
static int detectHeadAndShoulders(const StockData* data, int dataSize, PatternResult* pattern) {
    if (dataSize < 50) return 0;  /* Need enough data */
    
    /* Extract closing prices */
    double* closes = (double*)malloc(dataSize * sizeof(double));
    if (!closes) return 0;
    
    int i;
    for (i = 0; i < dataSize; i++) {
        closes[i] = data[i].close;
    }
    
    /* Look for 3 peaks with the middle one higher (head) */
    int foundPattern = 0;
    int leftShoulder = -1, head = -1, rightShoulder = -1;
    int leftTrough = -1, rightTrough = -1;
    
    /* Scan the last 60 days (or less if not enough data) */
    int scanStart = dataSize - 60;
    if (scanStart < 0) scanStart = 0;
    
    /* Find peaks (simple method) */
    for (i = scanStart + 5; i < dataSize - 5; i++) {
        /* Potential peak: higher than 2 points on either side */
        if (closes[i] > closes[i-1] && closes[i] > closes[i-2] && 
            closes[i] > closes[i+1] && closes[i] > closes[i+2]) {
            
            if (leftShoulder == -1) {
                leftShoulder = i;
            } else if (head == -1 && i - leftShoulder >= 5) {
                /* Head should be higher than left shoulder */
                if (closes[i] > closes[leftShoulder]) {
                    head = i;
                    
                    /* Find trough between left shoulder and head */
                    double minVal = INFINITY;
                    int minIdx = -1;
                    
                    for (int j = leftShoulder + 1; j < head; j++) {
                        if (closes[j] < minVal) {
                            minVal = closes[j];
                            minIdx = j;
                        }
                    }
                    
                    if (minIdx != -1) {
                        leftTrough = minIdx;
                    } else {
                        /* Reset if trough not found */
                        leftShoulder = i;
                        head = -1;
                    }
                } else {
                    /* If this peak is not higher, make it the new left shoulder */
                    leftShoulder = i;
                }
            } else if (rightShoulder == -1 && head != -1 && i - head >= 5) {
                /* Right shoulder should be lower than head and similar to left shoulder */
                if (closes[i] < closes[head]) {
                    double shoulderRatio = fabs(closes[i] - closes[leftShoulder]) / closes[leftShoulder];
                    
                    if (shoulderRatio < 0.1) {  /* Shoulders within 10% of each other */
                        rightShoulder = i;
                        
                        /* Find trough between head and right shoulder */
                        double minVal = INFINITY;
                        int minIdx = -1;
                        
                        for (int j = head + 1; j < rightShoulder; j++) {
                            if (closes[j] < minVal) {
                                minVal = closes[j];
                                minIdx = j;
                            }
                        }
                        
                        if (minIdx != -1) {
                            rightTrough = minIdx;
                            
                            /* Check if troughs form a neckline (similar levels) */
                            double troughRatio = fabs(closes[rightTrough] - closes[leftTrough]) / closes[leftTrough];
                            
                            if (troughRatio < 0.05) {  /* Troughs within 5% of each other */
                                foundPattern = 1;
                                break;
                            }
                        }
                    }
                }
                
                /* Reset head and left shoulder if right shoulder not valid */
                leftShoulder = head;
                head = i;
                leftTrough = rightTrough;
                rightTrough = -1;
            }
        }
    }
    
    /* Fill in pattern details if found */
    if (foundPattern) {
        pattern->type = PATTERN_HEAD_AND_SHOULDERS;
        strcpy(pattern->description, "Head and Shoulders pattern detected - bearish reversal pattern");
        pattern->startIndex = leftShoulder - 5;  /* Include some context */
        pattern->endIndex = rightShoulder + 5;   /* Include some context */
        pattern->confidence = 0.8;  /* Higher confidence for this complex pattern */
        
        /* Expected downside move based on pattern height */
        double necklineLevel = (closes[leftTrough] + closes[rightTrough]) / 2;
        double patternHeight = closes[head] - necklineLevel;
        pattern->expectedMove = -patternHeight / closes[rightShoulder];  /* Negative for downside move */
    }
    
    free(closes);
    return foundPattern;
}

/* Time series similarity measures */

/* Euclidean Distance */
double calculateEuclideanDistance(const double* series1, const double* series2, int length) {
    if (!series1 || !series2 || length <= 0) {
        return INFINITY;
    }
    
    double sumSquared = 0.0;
    int i;
    
    for (i = 0; i < length; i++) {
        double diff = series1[i] - series2[i];
        sumSquared += diff * diff;
    }
    
    return sqrt(sumSquared);
}

/* Pearson Correlation Coefficient */
double calculatePearsonCorrelation(const double* series1, const double* series2, int length) {
    if (!series1 || !series2 || length <= 1) {
        return 0.0;
    }
    
    double sum1 = 0.0, sum2 = 0.0, sumSq1 = 0.0, sumSq2 = 0.0, sumProd = 0.0;
    int i;
    
    for (i = 0; i < length; i++) {
        sum1 += series1[i];
        sum2 += series2[i];
        sumSq1 += series1[i] * series1[i];
        sumSq2 += series2[i] * series2[i];
        sumProd += series1[i] * series2[i];
    }
    
    double numPoints = (double)length;
    double numerator = sumProd - (sum1 * sum2 / numPoints);
    double denominator1 = sumSq1 - (sum1 * sum1 / numPoints);
    double denominator2 = sumSq2 - (sum2 * sum2 / numPoints);
    
    if (denominator1 <= 0.0 || denominator2 <= 0.0) {
        return 0.0;  /* No variation in at least one series */
    }
    
    return numerator / sqrt(denominator1 * denominator2);
}

/* Helper function for DTW */
static double min3(double a, double b, double c) {
    return (a < b) ? ((a < c) ? a : c) : ((b < c) ? b : c);
}

/* Dynamic Time Warping (DTW) distance */
double calculateDTW(const double* series1, int length1, const double* series2, int length2) {
    if (!series1 || !series2 || length1 <= 0 || length2 <= 0) {
        return INFINITY;
    }
    
    /* Allocate DTW matrix */
    double** dtw = (double**)malloc((length1 + 1) * sizeof(double*));
    if (!dtw) return INFINITY;
    
    int i, j;
    for (i = 0; i <= length1; i++) {
        dtw[i] = (double*)malloc((length2 + 1) * sizeof(double));
        if (!dtw[i]) {
            /* Clean up already allocated memory */
            for (j = 0; j < i; j++) {
                free(dtw[j]);
            }
            free(dtw);
            return INFINITY;
        }
    }
    
    /* Initialize matrix */
    for (i = 0; i <= length1; i++) {
        for (j = 0; j <= length2; j++) {
            dtw[i][j] = INFINITY;
        }
    }
    dtw[0][0] = 0.0;
    
    /* Fill the matrix */
    for (i = 1; i <= length1; i++) {
        for (j = 1; j <= length2; j++) {
            double cost = fabs(series1[i-1] - series2[j-1]);
            dtw[i][j] = cost + min3(dtw[i-1][j], dtw[i][j-1], dtw[i-1][j-1]);
        }
    }
    
    /* Get the result */
    double result = dtw[length1][length2];
    
    /* Clean up */
    for (i = 0; i <= length1; i++) {
        free(dtw[i]);
    }
    free(dtw);
    
    return result;
}

/* Volatility prediction */

/* Predict volatility using a simple historical method (standard deviation of returns) */
double predictVolatility(const StockData* data, int dataSize, int horizon) {
    if (!data || dataSize < 20) {
        return 0.0;
    }
    
    /* Calculate log returns */
    int returnCount = dataSize - 1;
    double* returns = (double*)malloc(returnCount * sizeof(double));
    if (!returns) return 0.0;
    
    int i;
    for (i = 0; i < returnCount; i++) {
        returns[i] = log(data[i+1].close / data[i].close);
    }
    
    /* Calculate sample standard deviation of returns */
    double sum = 0.0, sumSq = 0.0;
    for (i = 0; i < returnCount; i++) {
        sum += returns[i];
        sumSq += returns[i] * returns[i];
    }
    
    double mean = sum / returnCount;
    double variance = (sumSq / returnCount) - (mean * mean);
    double dailyVol = sqrt(variance);
    
    /* Scale to horizon (assuming returns are IID) */
    double horizonVol = dailyVol * sqrt(horizon);
    
    free(returns);
    return horizonVol;
}

/* Predict volatility using Exponentially Weighted Moving Average (EWMA) */
double predictVolatilityEWMA(const StockData* data, int dataSize, int lookback) {
    if (!data || dataSize < lookback + 1) {
        return 0.0;
    }
    
    /* Calculate log returns */
    int returnCount = dataSize - 1;
    double* returns = (double*)malloc(returnCount * sizeof(double));
    if (!returns) return 0.0;
    
    int i;
    for (i = 0; i < returnCount; i++) {
        returns[i] = log(data[i+1].close / data[i].close);
    }
    
    /* EWMA parameters */
    double lambda = 0.94;  /* Standard decay factor for EWMA (RiskMetrics) */
    double ewmaVar = returns[returnCount-1] * returns[returnCount-1];  /* Initialize with last return squared */
    
    /* Apply EWMA starting from the second most recent return and going backward */
    for (i = returnCount - 2; i >= returnCount - lookback && i >= 0; i--) {
        ewmaVar = lambda * ewmaVar + (1 - lambda) * returns[i] * returns[i];
    }
    
    free(returns);
    return sqrt(ewmaVar);
}

/* Predict volatility using a simplified GARCH(1,1) model */
double predictVolatilityGARCH(const StockData* data, int dataSize, int horizon) {
    if (!data || dataSize < 30) {
        return 0.0;
    }
    
    /* Calculate log returns */
    int returnCount = dataSize - 1;
    double* returns = (double*)malloc(returnCount * sizeof(double));
    if (!returns) return 0.0;
    
    int i;
    for (i = 0; i < returnCount; i++) {
        returns[i] = log(data[i+1].close / data[i].close);
    }
    
    /* GARCH(1,1) parameters (typical values) */
    double omega = 0.000001;  /* Long-run average variance (small positive value) */
    double alpha = 0.1;       /* Weight of most recent squared return */
    double beta = 0.85;       /* Weight of previous variance estimate */
    
    /* Calculate long-run sample variance for initialization */
    double sum = 0.0, sumSq = 0.0;
    for (i = 0; i < returnCount; i++) {
        sum += returns[i];
        sumSq += returns[i] * returns[i];
    }
    double mean = sum / returnCount;
    double sampleVar = (sumSq / returnCount) - (mean * mean);
    
    /* Initialize variance with sample variance */
    double variance = sampleVar;
    
    /* Update variance with GARCH(1,1) formula for the last 30 returns */
    int start = returnCount - 30;
    if (start < 0) start = 0;
    
    for (i = start; i < returnCount; i++) {
        double returnSq = returns[i] * returns[i];
        variance = omega + alpha * returnSq + beta * variance;
    }
    
    /* Forecast variance for the specified horizon */
    double longRunVar = omega / (1 - alpha - beta);
    double forecastVar = variance;
    
    /* Iteratively apply GARCH formula for horizon steps ahead */
    for (i = 0; i < horizon; i++) {
        forecastVar = omega + alpha * forecastVar + beta * forecastVar;
    }
    
    /* If horizon is large, variance converges to long-run value */
    if (horizon > 20) {
        forecastVar = 0.7 * forecastVar + 0.3 * longRunVar;
    }
    
    /* Convert to volatility (standard deviation) */
    double volatility = sqrt(forecastVar);
    
    free(returns);
    return volatility * sqrt(horizon);  /* Scale to horizon */
}

/* Anomaly detection */

/* Calculate anomaly score for current market state */
double calculateAnomalyScore(const StockData* data, int dataSize) {
    if (!data || dataSize < 20) {
        return 0.0;
    }
    
    int lookback = 20;  /* Compare with last 20 days */
    if (lookback > dataSize - 1) lookback = dataSize - 1;
    
    /* Calculate metrics for anomaly detection */
    double currentPrice = data[dataSize-1].close;
    double previousPrice = data[dataSize-2].close;
    double priceChange = (currentPrice - previousPrice) / previousPrice;
    
    /* Compare current volume with average */
    double volumeSum = 0.0;
    int i;
    for (i = dataSize - lookback - 1; i < dataSize - 1; i++) {
        volumeSum += data[i].volume;
    }
    double avgVolume = volumeSum / lookback;
    double volumeRatio = data[dataSize-1].volume / avgVolume;
    
    /* Calculate historical price volatility */
    double* returns = (double*)malloc(lookback * sizeof(double));
    if (!returns) return 0.0;
    
    for (i = 0; i < lookback; i++) {
        returns[i] = (data[dataSize-i-2].close - data[dataSize-i-3].close) / data[dataSize-i-3].close;
    }
    
    double sumReturns = 0.0, sumSqReturns = 0.0;
    for (i = 0; i < lookback; i++) {
        sumReturns += returns[i];
        sumSqReturns += returns[i] * returns[i];
    }
    double meanReturn = sumReturns / lookback;
    double stdDevReturns = sqrt((sumSqReturns / lookback) - (meanReturn * meanReturn));
    
    /* Calculate Z-score of current return */
    double returnZScore = fabs(priceChange - meanReturn) / stdDevReturns;
    
    /* Calculate true range ratio */
    double currentTR = fmax(data[dataSize-1].high - data[dataSize-1].low,
                         fmax(fabs(data[dataSize-1].high - data[dataSize-2].close),
                              fabs(data[dataSize-1].low - data[dataSize-2].close)));
    
    double trSum = 0.0;
    for (i = 0; i < lookback; i++) {
        double tr = fmax(data[dataSize-i-2].high - data[dataSize-i-2].low,
                      fmax(fabs(data[dataSize-i-2].high - data[dataSize-i-3].close),
                           fabs(data[dataSize-i-2].low - data[dataSize-i-3].close)));
        trSum += tr;
    }
    double avgTR = trSum / lookback;
    double trRatio = currentTR / avgTR;
    
    /* Combine factors into anomaly score */
    double anomalyScore = (0.4 * returnZScore) + (0.3 * (volumeRatio - 1.0)) + (0.3 * (trRatio - 1.0));
    
    free(returns);
    return anomalyScore;
}

/* Detect historical anomalies in price data */
int detectAnomalies(const StockData* data, int dataSize, int* anomalyIndices, int maxAnomalies) {
    if (!data || dataSize < 30 || !anomalyIndices || maxAnomalies <= 0) {
        return 0;
    }
    
    int anomalyCount = 0;
    int lookback = 20;  /* Use 20 days of history for anomaly detection */
    
    /* Calculate anomaly scores for each day after lookback period */
    double* scores = (double*)malloc((dataSize - lookback) * sizeof(double));
    if (!scores) return 0;
    
    int i, j;
    for (i = lookback; i < dataSize; i++) {
        /* Use a subset of data ending at current index */
        const StockData* subsetData = &data[i - lookback];
        scores[i - lookback] = calculateAnomalyScore(subsetData, lookback + 1);
    }
    
    /* Find threshold for significant anomalies (e.g., 2 standard deviations) */
    double sumScores = 0.0, sumSqScores = 0.0;
    for (i = 0; i < dataSize - lookback; i++) {
        sumScores += scores[i];
        sumSqScores += scores[i] * scores[i];
    }
    double meanScore = sumScores / (dataSize - lookback);
    double stdDevScore = sqrt((sumSqScores / (dataSize - lookback)) - (meanScore * meanScore));
    double threshold = meanScore + 2.0 * stdDevScore;
    
    /* Identify days with anomaly scores above threshold */
    for (i = 0; i < dataSize - lookback && anomalyCount < maxAnomalies; i++) {
        if (scores[i] > threshold) {
            /* Skip if too close to previous anomaly */
            int tooClose = 0;
            for (j = 0; j < anomalyCount; j++) {
                if (abs((i + lookback) - anomalyIndices[j]) < 5) {
                    tooClose = 1;
                    break;
                }
            }
            
            if (!tooClose) {
                anomalyIndices[anomalyCount++] = i + lookback;
            }
        }
    }
    
    free(scores);
    return anomalyCount;
}

/* Event similarity analysis */

/* Calculate similarity between two events (Internal version for data mining module) */
static double calculateEventSimilarityInternal(const EventData* event1, const EventData* event2) {
    if (!event1 || !event2) {
        return 0.0;
    }
    
    /* Initialize weights for different factors */
    double sentimentWeight = 0.4;
    double impactWeight = 0.3;
    double titleWeight = 0.15;
    double descriptionWeight = 0.15;
    
    /* Calculate sentiment similarity */
    double sentimentSim = 1.0 - fabs(event1->sentiment - event2->sentiment);
    
    /* Calculate impact score similarity */
    double impactRatio = (double)event1->impactScore / (double)event2->impactScore;
    if (impactRatio > 1.0) impactRatio = 1.0 / impactRatio;  /* Normalize to 0-1 */
    
    /* Use a simplified approach rather than tokenizing */
    /* Just check if titles and descriptions have some common terms */
    double titleOverlap = 0.5;  /* Assume moderate similarity by default */
    double descOverlap = 0.5;   /* Assume moderate similarity by default */
    
    /* Combine factors into overall similarity score */
    double similarity = (sentimentWeight * sentimentSim) +
                       (impactWeight * impactRatio) +
                       (titleWeight * titleOverlap) +
                       (descriptionWeight * descOverlap);
    
    return similarity;
}

/* Find similar historical events */
int findSimilarHistoricalEvents(const EventData* currentEvent, const EventDatabase* historicalEvents, 
                     SimilarHistoricalEvent* similarEvents, int maxResults) {
    if (!currentEvent || !historicalEvents || !similarEvents || maxResults <= 0) {
        return 0;
    }
    
    int similarCount = 0;
    int i;
    
    /* Calculate similarity for each historical event */
    for (i = 0; i < historicalEvents->eventCount; i++) {
        double similarity = calculateEventSimilarityInternal(currentEvent, &historicalEvents->events[i]);
        
        /* If similarity is above threshold, add to results */
        if (similarity > 0.6) {  /* 60% similarity threshold */
            if (similarCount < maxResults) {
                similarEvents[similarCount].eventData = historicalEvents->events[i];
                similarEvents[similarCount].similarityScore = similarity;
                
                /* Dummy values for demonstration - in a real system, these would be calculated */
                similarEvents[similarCount].priceChangeAfterEvent = -0.05 + (0.1 * (rand() / (double)RAND_MAX));
                similarEvents[similarCount].daysToRecovery = 5 + (rand() % 20);
                
                similarCount++;
            } else {
                /* Find the least similar event in our results */
                int leastSimilarIdx = 0;
                double minSimilarity = similarEvents[0].similarityScore;
                
                for (int j = 1; j < maxResults; j++) {
                    if (similarEvents[j].similarityScore < minSimilarity) {
                        minSimilarity = similarEvents[j].similarityScore;
                        leastSimilarIdx = j;
                    }
                }
                
                /* Replace if current event is more similar */
                if (similarity > minSimilarity) {
                    similarEvents[leastSimilarIdx].eventData = historicalEvents->events[i];
                    similarEvents[leastSimilarIdx].similarityScore = similarity;
                    similarEvents[leastSimilarIdx].priceChangeAfterEvent = -0.05 + (0.1 * (rand() / (double)RAND_MAX));
                    similarEvents[leastSimilarIdx].daysToRecovery = 5 + (rand() % 20);
                }
            }
        }
    }
    
    /* Sort by similarity score (descending) */
    for (i = 0; i < similarCount - 1; i++) {
        for (int j = i + 1; j < similarCount; j++) {
            if (similarEvents[j].similarityScore > similarEvents[i].similarityScore) {
                /* Swap */
                SimilarHistoricalEvent temp = similarEvents[i];
                similarEvents[i] = similarEvents[j];
                similarEvents[j] = temp;
            }
        }
    }
    
    return similarCount;
}

/* Predict price outcome based on similar historical events */
double predictEventOutcome(const EventData* event, const SimilarHistoricalEvent* similarEvents, int count) {
    if (!event || !similarEvents || count <= 0) {
        return 0.0;
    }
    
    double weightedSum = 0.0;
    double sumWeights = 0.0;
    int i;
    
    /* Calculate weighted average of price changes */
    for (i = 0; i < count; i++) {
        double weight = similarEvents[i].similarityScore;
        weightedSum += weight * similarEvents[i].priceChangeAfterEvent;
        sumWeights += weight;
    }
    
    /* Adjust prediction based on current event's sentiment and impact */
    double predictedChange = sumWeights > 0 ? weightedSum / sumWeights : 0.0;
    double sentimentFactor = event->sentiment * (event->impactScore / 100.0);
    
    /* Combine historical outcomes with current event's sentiment */
    double finalPrediction = (0.7 * predictedChange) + (0.3 * sentimentFactor * 0.1);
    
    return finalPrediction;
}

/* Helper function to calculate Z-score */
static double calculateZScore(const double* series, int length, double value) {
    if (!series || length <= 1) {
        return 0.0;
    }
    
    double sum = 0.0, sumSq = 0.0;
    int i;
    
    for (i = 0; i < length; i++) {
        sum += series[i];
        sumSq += series[i] * series[i];
    }
    
    double mean = sum / length;
    double variance = (sumSq / length) - (mean * mean);
    
    if (variance <= 0.0) {
        return 0.0;
    }
    
    double stdDev = sqrt(variance);
    return (value - mean) / stdDev;
}

/* Historical data mining functions using CSV cache */

/* Fetch and analyze historical data for a single symbol */
int fetchAndAnalyzeHistoricalData(const char* symbol, const char* startDate, const char* endDate, HistoricalAnalysis* result) {
    if (!symbol || !startDate || !endDate || !result) {
        logError(ERR_INVALID_PARAMETER, "Invalid parameters for historical data analysis");
        return 0;
    }
    
    /* Initialize result structure */
    memset(result, 0, sizeof(HistoricalAnalysis));
    strncpy(result->symbol, symbol, MAX_SYMBOL_LENGTH - 1);
    result->symbol[MAX_SYMBOL_LENGTH - 1] = '\0';
    
    /* Fetch historical data using the CSV cache system */
    Stock stock;
    memset(&stock, 0, sizeof(Stock));
    
    if (!fetchHistoricalDataWithCache(symbol, startDate, endDate, &stock)) {
        logError(ERR_DATA_FETCH_FAILED, "Failed to fetch historical data for %s", symbol);
        return 0;
    }
    
    /* Check if we have enough data for analysis */
    if (stock.dataSize < 20) {
        logError(ERR_INSUFFICIENT_DATA, "Insufficient data points for analysis");
        if (stock.data) free(stock.data);
        return 0;
    }
    
    logMessage(LOG_INFO, "Analyzing %d data points for %s (%s to %s)", 
               stock.dataSize, symbol, startDate, endDate);
    
    /* Calculate returns */
    int returnCount = stock.dataSize - 1;
    double* returns = (double*)malloc(returnCount * sizeof(double));
    if (!returns) {
        logError(ERR_OUT_OF_MEMORY, "Failed to allocate memory for returns calculation");
        if (stock.data) free(stock.data);
        return 0;
    }
    
    int i;
    for (i = 0; i < returnCount; i++) {
        returns[i] = (stock.data[i+1].close - stock.data[i].close) / stock.data[i].close;
    }
    
    /* Calculate mean return */
    double sumReturns = 0.0;
    for (i = 0; i < returnCount; i++) {
        sumReturns += returns[i];
    }
    result->meanReturn = sumReturns / returnCount;
    
    /* Calculate annualized return */
    double cumulativeReturn = 1.0;
    for (i = 0; i < returnCount; i++) {
        cumulativeReturn *= (1.0 + returns[i]);
    }
    /* Annualize by assuming 252 trading days per year */
    result->annualizedReturn = pow(cumulativeReturn, 252.0 / returnCount) - 1.0;
    
    /* Calculate volatility (standard deviation of returns) */
    double sumSquaredDeviation = 0.0;
    for (i = 0; i < returnCount; i++) {
        double deviation = returns[i] - result->meanReturn;
        sumSquaredDeviation += deviation * deviation;
    }
    result->volatility = sqrt(sumSquaredDeviation / returnCount);
    result->volatility *= sqrt(252.0); /* Annualize volatility */
    
    /* Calculate maximum drawdown */
    double maxPrice = stock.data[0].close;
    double maxDrawdown = 0.0;
    
    for (i = 1; i < stock.dataSize; i++) {
        double currentPrice = stock.data[i].close;
        if (currentPrice > maxPrice) {
            maxPrice = currentPrice;
        } else {
            double currentDrawdown = (maxPrice - currentPrice) / maxPrice;
            if (currentDrawdown > maxDrawdown) {
                maxDrawdown = currentDrawdown;
            }
        }
    }
    result->maxDrawdown = maxDrawdown;
    
    /* Calculate Sharpe ratio (using 0% as risk-free rate for simplicity) */
    result->sharpeRatio = (result->annualizedReturn) / result->volatility;
    
    /* Find best and worst trading days */
    double bestReturn = returns[0];
    double worstReturn = returns[0];
    int bestDay = 0;
    int worstDay = 0;
    
    for (i = 1; i < returnCount; i++) {
        if (returns[i] > bestReturn) {
            bestReturn = returns[i];
            bestDay = i;
        }
        if (returns[i] < worstReturn) {
            worstReturn = returns[i];
            worstDay = i;
        }
    }
    
    result->bestDay = bestReturn;
    result->worstDay = worstReturn;
    strcpy(result->bestDayDate, stock.data[bestDay + 1].date);
    strcpy(result->worstDayDate, stock.data[worstDay + 1].date);
    
    /* Detect price patterns */
    PatternResult patterns[MAX_PATTERNS];
    int patternCount = detectPricePatterns(stock.data, stock.dataSize, patterns, MAX_PATTERNS);
    
    /* Count patterns by type */
    for (i = 0; i < patternCount; i++) {
        result->patterns[patterns[i].type]++;
    }
    
    /* Calculate average return following patterns */
    if (patternCount > 0) {
        double totalPatternReturn = 0.0;
        int validPatternCount = 0;
        
        for (i = 0; i < patternCount; i++) {
            int endIndex = patterns[i].endIndex;
            if (endIndex + 20 < stock.dataSize) {
                /* Calculate 20-day return after pattern */
                double patternReturn = (stock.data[endIndex + 20].close - stock.data[endIndex].close) / stock.data[endIndex].close;
                totalPatternReturn += patternReturn;
                validPatternCount++;
            }
        }
        
        if (validPatternCount > 0) {
            result->avgPatternReturn = totalPatternReturn / validPatternCount;
        }
    }
    
    /* Set total trading days */
    result->totalTradingDays = stock.dataSize;
    
    /* Correlation with market is set to 0 by default */
    /* To calculate this properly, we would need market index data for the same period */
    result->correlationWithMarket = 0.0;
    
    /* Clean up */
    free(returns);
    if (stock.data) free(stock.data);
    
    return 1;
}

/* Batch analyze historical data for multiple symbols */
int batchAnalyzeHistoricalData(const char** symbols, int symbolCount, const char* startDate, const char* endDate, HistoricalAnalysis* results) {
    if (!symbols || symbolCount <= 0 || symbolCount > MAX_STOCKS || !startDate || !endDate || !results) {
        logError(ERR_INVALID_PARAMETER, "Invalid parameters for batch historical analysis");
        return 0;
    }
    
    int successCount = 0;
    int i;
    
    for (i = 0; i < symbolCount; i++) {
        if (fetchAndAnalyzeHistoricalData(symbols[i], startDate, endDate, &results[i])) {
            successCount++;
        }
    }
    
    return successCount;
}

/* Calculate correlation matrix for multiple symbols */
int calculateCorrelationMatrix(const char** symbols, int symbolCount, const char* startDate, const char* endDate, double** correlationMatrix) {
    if (!symbols || symbolCount <= 0 || symbolCount > MAX_STOCKS || !startDate || !endDate || !correlationMatrix) {
        logError(ERR_INVALID_PARAMETER, "Invalid parameters for correlation matrix");
        return 0;
    }
    
    /* Fetch data for all symbols */
    Stock* stocks = (Stock*)malloc(symbolCount * sizeof(Stock));
    if (!stocks) {
        logError(ERR_OUT_OF_MEMORY, "Failed to allocate memory for stocks");
        return 0;
    }
    
    int i, j;
    int success = 1;
    
    /* Fetch historical data for each symbol */
    for (i = 0; i < symbolCount; i++) {
        memset(&stocks[i], 0, sizeof(Stock));
        if (!fetchHistoricalDataWithCache(symbols[i], startDate, endDate, &stocks[i])) {
            logError(ERR_DATA_FETCH_FAILED, "Failed to fetch historical data for %s", symbols[i]);
            success = 0;
            break;
        }
    }
    
    if (!success) {
        /* Clean up on failure */
        for (i = 0; i < symbolCount; i++) {
            if (stocks[i].data) free(stocks[i].data);
        }
        free(stocks);
        return 0;
    }
    
    /* Calculate returns for each stock */
    double** returns = (double**)malloc(symbolCount * sizeof(double*));
    int* returnCounts = (int*)malloc(symbolCount * sizeof(int));
    
    if (!returns || !returnCounts) {
        logError(ERR_OUT_OF_MEMORY, "Failed to allocate memory for returns");
        for (i = 0; i < symbolCount; i++) {
            if (stocks[i].data) free(stocks[i].data);
        }
        free(stocks);
        if (returns) free(returns);
        if (returnCounts) free(returnCounts);
        return 0;
    }
    
    /* Calculate returns for each stock */
    for (i = 0; i < symbolCount; i++) {
        returnCounts[i] = stocks[i].dataSize - 1;
        returns[i] = (double*)malloc(returnCounts[i] * sizeof(double));
        
        if (!returns[i]) {
            logError(ERR_OUT_OF_MEMORY, "Failed to allocate memory for returns[%d]", i);
            success = 0;
            break;
        }
        
        for (j = 0; j < returnCounts[i]; j++) {
            returns[i][j] = (stocks[i].data[j+1].close - stocks[i].data[j].close) / stocks[i].data[j].close;
        }
    }
    
    if (!success) {
        /* Clean up on failure */
        for (i = 0; i < symbolCount; i++) {
            if (stocks[i].data) free(stocks[i].data);
            if (i < symbolCount && returns[i]) free(returns[i]);
        }
        free(stocks);
        free(returns);
        free(returnCounts);
        return 0;
    }
    
    /* Calculate correlation matrix */
    for (i = 0; i < symbolCount; i++) {
        /* Diagonal is always 1.0 */
        correlationMatrix[i][i] = 1.0;
        
        for (j = i + 1; j < symbolCount; j++) {
            /* Find the minimum length of both return series */
            int minLength = (returnCounts[i] < returnCounts[j]) ? returnCounts[i] : returnCounts[j];
            
            /* Calculate correlation for the common time period */
            correlationMatrix[i][j] = calculatePearsonCorrelation(returns[i], returns[j], minLength);
            
            /* Matrix is symmetric */
            correlationMatrix[j][i] = correlationMatrix[i][j];
        }
    }
    
    /* Clean up */
    for (i = 0; i < symbolCount; i++) {
        if (stocks[i].data) free(stocks[i].data);
        if (returns[i]) free(returns[i]);
    }
    free(stocks);
    free(returns);
    free(returnCounts);
    
    return 1;
}

/* Calculate statistical significance of comparison between two datasets */
int calculateStatisticalSignificance(const StockData* data1, int dataSize1, const StockData* data2, int dataSize2, StatisticalResult* result) {
    if (!data1 || dataSize1 < 20 || !data2 || dataSize2 < 20 || !result) {
        logError(ERR_INVALID_PARAMETER, "Invalid parameters for statistical significance test");
        return 0;
    }
    
    /* Calculate returns for both datasets */
    double* returns1 = (double*)malloc((dataSize1 - 1) * sizeof(double));
    double* returns2 = (double*)malloc((dataSize2 - 1) * sizeof(double));
    
    if (!returns1 || !returns2) {
        logError(ERR_OUT_OF_MEMORY, "Failed to allocate memory for returns");
        if (returns1) free(returns1);
        if (returns2) free(returns2);
        return 0;
    }
    
    int i;
    for (i = 0; i < dataSize1 - 1; i++) {
        returns1[i] = (data1[i+1].close - data1[i].close) / data1[i].close;
    }
    
    for (i = 0; i < dataSize2 - 1; i++) {
        returns2[i] = (data2[i+1].close - data2[i].close) / data2[i].close;
    }
    
    /* Calculate means and standard deviations */
    double sum1 = 0.0, sumSq1 = 0.0;
    double sum2 = 0.0, sumSq2 = 0.0;
    
    for (i = 0; i < dataSize1 - 1; i++) {
        sum1 += returns1[i];
        sumSq1 += returns1[i] * returns1[i];
    }
    
    for (i = 0; i < dataSize2 - 1; i++) {
        sum2 += returns2[i];
        sumSq2 += returns2[i] * returns2[i];
    }
    
    double mean1 = sum1 / (dataSize1 - 1);
    double var1 = (sumSq1 / (dataSize1 - 1)) - (mean1 * mean1);
    
    double mean2 = sum2 / (dataSize2 - 1);
    double var2 = (sumSq2 / (dataSize2 - 1)) - (mean2 * mean2);
    
    /* Calculate t-statistic for difference in means */
    double meanDiff = mean1 - mean2;
    double pooledStdErr = sqrt((var1 / (dataSize1 - 1)) + (var2 / (dataSize2 - 1)));
    double tStat = meanDiff / pooledStdErr;
    
    /* Calculate degrees of freedom (conservative approach) */
    int df = (dataSize1 - 1) + (dataSize2 - 1) - 2;
    
    /* Calculate p-value (simplified approach) */
    double pValue = 2.0 * (1.0 - normCDF(fabs(tStat)));
    
    /* Calculate effect size (Cohen's d) */
    double pooledStdDev = sqrt(((dataSize1 - 1) * var1 + (dataSize2 - 1) * var2) / (dataSize1 + dataSize2 - 2));
    double effectSize = meanDiff / pooledStdDev;
    
    /* Calculate 95% confidence interval */
    double criticalValue = 1.96;  /* Approximate for large df at 95% confidence */
    double margin = criticalValue * pooledStdErr;
    
    /* Fill in results */
    result->pValue = pValue;
    result->significantAt95pct = (pValue < 0.05) ? 1 : 0;
    result->significantAt99pct = (pValue < 0.01) ? 1 : 0;
    result->effectSize = effectSize;
    result->confidenceInterval[0] = meanDiff - margin;
    result->confidenceInterval[1] = meanDiff + margin;
    
    /* Clean up */
    free(returns1);
    free(returns2);
    
    return 1;
}

/* Helper function: Standard normal CDF approximation */
static double normCDF(double x) {
    return 0.5 * (1 + erf(x / sqrt(2.0)));
}

/* Find seasonal patterns in historical data */
int findSeasonalPatterns(const StockData* data, int dataSize, PatternResult* patterns, int maxPatterns) {
    if (!data || dataSize < 252 || !patterns || maxPatterns <= 0) {  /* Need at least 1 year of data */
        logError(ERR_INVALID_PARAMETER, "Invalid parameters for seasonal patterns");
        return 0;
    }
    
    int patternCount = 0;
    
    /* Initialize month and day of week arrays */
    double monthlyReturns[12][100] = {{0}};  /* Up to 100 occurrences per month */
    int monthCounts[12] = {0};
    
    double dowReturns[7][500] = {{0}};  /* Up to 500 occurrences per day of week */
    int dowCounts[7] = {0};
    
    /* Parse data into monthly and day-of-week buckets */
    int i;
    for (i = 1; i < dataSize; i++) {
        /* Calculate daily return */
        double dailyReturn = (data[i].close - data[i-1].close) / data[i-1].close;
        
        /* Parse date for month and day of week */
        int year, month, day;
        if (sscanf(data[i].date, "%d-%d-%d", &year, &month, &day) == 3) {
            /* Month (1-12) */
            month--; /* Convert to 0-based index */
            if (month >= 0 && month < 12 && monthCounts[month] < 100) {
                monthlyReturns[month][monthCounts[month]++] = dailyReturn;
            }
            
            /* Calculate day of week (0=Sunday, 6=Saturday) */
            struct tm timeinfo = {0};
            timeinfo.tm_year = year - 1900;
            timeinfo.tm_mon = month;
            timeinfo.tm_mday = day;
            mktime(&timeinfo);
            
            int dow = timeinfo.tm_wday;
            if (dow >= 0 && dow < 7 && dowCounts[dow] < 500) {
                dowReturns[dow][dowCounts[dow]++] = dailyReturn;
            }
        }
    }
    
    /* Calculate average returns by month and day of week */
    double avgMonthlyReturn[12] = {0};
    double avgDOWReturn[7] = {0};
    
    for (i = 0; i < 12; i++) {
        if (monthCounts[i] > 0) {
            double sum = 0.0;
            int j;
            for (j = 0; j < monthCounts[i]; j++) {
                sum += monthlyReturns[i][j];
            }
            avgMonthlyReturn[i] = sum / monthCounts[i];
        }
    }
    
    for (i = 0; i < 7; i++) {
        if (dowCounts[i] > 0) {
            double sum = 0.0;
            int j;
            for (j = 0; j < dowCounts[i]; j++) {
                sum += dowReturns[i][j];
            }
            avgDOWReturn[i] = sum / dowCounts[i];
        }
    }
    
    /* Identify best and worst months */
    int bestMonth = 0, worstMonth = 0;
    for (i = 1; i < 12; i++) {
        if (avgMonthlyReturn[i] > avgMonthlyReturn[bestMonth]) {
            bestMonth = i;
        }
        if (avgMonthlyReturn[i] < avgMonthlyReturn[worstMonth]) {
            worstMonth = i;
        }
    }
    
    /* Identify best and worst days of week */
    int bestDOW = 0, worstDOW = 0;
    for (i = 1; i < 7; i++) {
        if (avgDOWReturn[i] > avgDOWReturn[bestDOW]) {
            bestDOW = i;
        }
        if (avgDOWReturn[i] < avgDOWReturn[worstDOW]) {
            worstDOW = i;
        }
    }
    
    /* Create patterns for significant seasonal effects */
    const char* monthNames[] = {"January", "February", "March", "April", "May", "June", 
                              "July", "August", "September", "October", "November", "December"};
    
    const char* dowNames[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
    
    /* Add best month pattern */
    if (patternCount < maxPatterns && avgMonthlyReturn[bestMonth] > 0) {
        patterns[patternCount].type = PATTERN_UNKNOWN;  /* Using unknown for seasonal */
        snprintf(patterns[patternCount].description, 128, 
                "Seasonal: %s historically strong (%.2f%% avg return, %d occurrences)",
                monthNames[bestMonth], avgMonthlyReturn[bestMonth] * 100.0, monthCounts[bestMonth]);
        patterns[patternCount].startIndex = 0;
        patterns[patternCount].endIndex = dataSize - 1;
        patterns[patternCount].confidence = 0.7;  /* Arbitrary confidence */
        patterns[patternCount].expectedMove = avgMonthlyReturn[bestMonth];
        patternCount++;
    }
    
    /* Add worst month pattern */
    if (patternCount < maxPatterns && avgMonthlyReturn[worstMonth] < 0) {
        patterns[patternCount].type = PATTERN_UNKNOWN;  /* Using unknown for seasonal */
        snprintf(patterns[patternCount].description, 128, 
                "Seasonal: %s historically weak (%.2f%% avg return, %d occurrences)",
                monthNames[worstMonth], avgMonthlyReturn[worstMonth] * 100.0, monthCounts[worstMonth]);
        patterns[patternCount].startIndex = 0;
        patterns[patternCount].endIndex = dataSize - 1;
        patterns[patternCount].confidence = 0.7;  /* Arbitrary confidence */
        patterns[patternCount].expectedMove = avgMonthlyReturn[worstMonth];
        patternCount++;
    }
    
    /* Add best day of week pattern */
    if (patternCount < maxPatterns && avgDOWReturn[bestDOW] > 0) {
        patterns[patternCount].type = PATTERN_UNKNOWN;  /* Using unknown for seasonal */
        snprintf(patterns[patternCount].description, 128, 
                "Seasonal: %s historically strong (%.2f%% avg return, %d occurrences)",
                dowNames[bestDOW], avgDOWReturn[bestDOW] * 100.0, dowCounts[bestDOW]);
        patterns[patternCount].startIndex = 0;
        patterns[patternCount].endIndex = dataSize - 1;
        patterns[patternCount].confidence = 0.6;  /* Slightly lower confidence */
        patterns[patternCount].expectedMove = avgDOWReturn[bestDOW];
        patternCount++;
    }
    
    /* Add worst day of week pattern */
    if (patternCount < maxPatterns && avgDOWReturn[worstDOW] < 0) {
        patterns[patternCount].type = PATTERN_UNKNOWN;  /* Using unknown for seasonal */
        snprintf(patterns[patternCount].description, 128, 
                "Seasonal: %s historically weak (%.2f%% avg return, %d occurrences)",
                dowNames[worstDOW], avgDOWReturn[worstDOW] * 100.0, dowCounts[worstDOW]);
        patterns[patternCount].startIndex = 0;
        patterns[patternCount].endIndex = dataSize - 1;
        patterns[patternCount].confidence = 0.6;  /* Slightly lower confidence */
        patterns[patternCount].expectedMove = avgDOWReturn[worstDOW];
        patternCount++;
    }
    
    /* Check for January effect */
    if (patternCount < maxPatterns) {
        double janReturn = avgMonthlyReturn[0];  /* January is month 0 */
        double otherMonthsAvg = 0.0;
        int otherMonthsCount = 0;
        
        for (i = 1; i < 12; i++) {
            if (monthCounts[i] > 0) {
                otherMonthsAvg += avgMonthlyReturn[i] * monthCounts[i];
                otherMonthsCount += monthCounts[i];
            }
        }
        
        if (otherMonthsCount > 0) {
            otherMonthsAvg /= otherMonthsCount;
            
            if (janReturn > 1.5 * otherMonthsAvg && janReturn > 0) {
                patterns[patternCount].type = PATTERN_UNKNOWN;  /* Using unknown for seasonal */
                snprintf(patterns[patternCount].description, 128, 
                        "January Effect detected: January returns (%.2f%%) exceed other months (%.2f%%)",
                        janReturn * 100.0, otherMonthsAvg * 100.0);
                patterns[patternCount].startIndex = 0;
                patterns[patternCount].endIndex = dataSize - 1;
                patterns[patternCount].confidence = 0.7;
                patterns[patternCount].expectedMove = janReturn - otherMonthsAvg;
                patternCount++;
            }
        }
    }
    
    return patternCount;
}

/* Test trading strategy on historical data */
int testTradingStrategy(const StockData* data, int dataSize, const char* strategyParams, double* returnRate, double* sharpeRatio) {
    if (!data || dataSize < 100 || !strategyParams || !returnRate || !sharpeRatio) {
        logError(ERR_INVALID_PARAMETER, "Invalid parameters for testing trading strategy");
        return 0;
    }
    
    /* Parse strategy parameters */
    int shortSMA = 10, longSMA = 50;  /* Default values */
    sscanf(strategyParams, "sma_crossover:%d,%d", &shortSMA, &longSMA);
    
    /* Ensure valid parameter ranges */
    if (shortSMA < 2) shortSMA = 2;
    if (longSMA <= shortSMA) longSMA = shortSMA * 2;
    
    /* Calculate SMAs */
    double* shortSMAValues = (double*)malloc((dataSize - shortSMA + 1) * sizeof(double));
    double* longSMAValues = (double*)malloc((dataSize - longSMA + 1) * sizeof(double));
    
    if (!shortSMAValues || !longSMAValues) {
        logError(ERR_OUT_OF_MEMORY, "Failed to allocate memory for SMA values");
        if (shortSMAValues) free(shortSMAValues);
        if (longSMAValues) free(longSMAValues);
        return 0;
    }
    
    int i, j;
    
    /* Calculate short SMA */
    for (i = shortSMA - 1; i < dataSize; i++) {
        double sum = 0.0;
        for (j = i - shortSMA + 1; j <= i; j++) {
            sum += data[j].close;
        }
        shortSMAValues[i - shortSMA + 1] = sum / shortSMA;
    }
    
    /* Calculate long SMA */
    for (i = longSMA - 1; i < dataSize; i++) {
        double sum = 0.0;
        for (j = i - longSMA + 1; j <= i; j++) {
            sum += data[j].close;
        }
        longSMAValues[i - longSMA + 1] = sum / longSMA;
    }
    
    /* Simulate trading based on SMA crossover */
    double startingCapital = 10000.0;
    double capital = startingCapital;
    int position = 0;  /* 0 = no position, 1 = long */
    int tradesCount = 0;
    double* tradeReturns = (double*)malloc(dataSize * sizeof(double));
    if (!tradeReturns) {
        logError(ERR_OUT_OF_MEMORY, "Failed to allocate memory for trade returns");
        free(shortSMAValues);
        free(longSMAValues);
        return 0;
    }
    
    /* Start trading from the point where both SMAs are available */
    int startIndex = longSMA;
    double sharePrice = 0.0, sharesOwned = 0.0;
    double entryPrice = 0.0;
    
    for (i = startIndex; i < dataSize; i++) {
        /* Check for crossover signal */
        int prevIndex = i - startIndex;
        int shortSMAIndex = i - shortSMA + 1;
        int longSMAIndex = i - longSMA + 1;
        
        int crossover = 0;
        if (prevIndex > 0) {
            /* Short SMA crosses above long SMA (buy signal) */
            if (shortSMAValues[shortSMAIndex - 1] <= longSMAValues[longSMAIndex - 1] &&
                shortSMAValues[shortSMAIndex] > longSMAValues[longSMAIndex]) {
                crossover = 1;
            }
            /* Short SMA crosses below long SMA (sell signal) */
            else if (shortSMAValues[shortSMAIndex - 1] >= longSMAValues[longSMAIndex - 1] &&
                    shortSMAValues[shortSMAIndex] < longSMAValues[longSMAIndex]) {
                crossover = -1;
            }
        }
        
        /* Execute trades */
        if (crossover == 1 && position == 0) {
            /* Buy signal */
            sharePrice = data[i].close;
            sharesOwned = capital / sharePrice;
            position = 1;
            entryPrice = sharePrice;
            tradeReturns[tradesCount++] = 0.0;  /* Placeholder, will update on exit */
        }
        else if ((crossover == -1 || i == dataSize - 1) && position == 1) {
            /* Sell signal or end of data */
            sharePrice = data[i].close;
            capital = sharesOwned * sharePrice;
            double tradeReturn = (sharePrice - entryPrice) / entryPrice;
            tradeReturns[tradesCount - 1] = tradeReturn;  /* Update last trade return */
            position = 0;
            sharesOwned = 0.0;
        }
    }
    
    /* Calculate overall return */
    double overallReturn = (capital - startingCapital) / startingCapital;
    *returnRate = overallReturn;
    
    /* Calculate Sharpe ratio on trade returns (if any trades were made) */
    if (tradesCount > 0) {
        double sumReturns = 0.0, sumSqReturns = 0.0;
        for (i = 0; i < tradesCount; i++) {
            sumReturns += tradeReturns[i];
            sumSqReturns += tradeReturns[i] * tradeReturns[i];
        }
        
        double meanReturn = sumReturns / tradesCount;
        double stdDev = sqrt((sumSqReturns / tradesCount) - (meanReturn * meanReturn));
        
        /* Annualized Sharpe ratio (assuming 252 trading days and 0% risk-free rate) */
        *sharpeRatio = (meanReturn / stdDev) * sqrt(252.0 / tradesCount);
    } else {
        *sharpeRatio = 0.0;
    }
    
    /* Clean up */
    free(shortSMAValues);
    free(longSMAValues);
    free(tradeReturns);
    
    return 1;
}