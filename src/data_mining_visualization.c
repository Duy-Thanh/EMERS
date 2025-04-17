#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../include/emers.h"
#include "../include/data_mining.h"
#include "../include/technical_analysis.h"

/**
 * Simple GUI for Data Mining Visualization
 * This module implements a text-based visualization for the three core data mining algorithms:
 * 1. Price Pattern Recognition
 * 2. SMA Crossover Signal Detection
 * 3. Anomaly Detection
 */

/**
 * Visualizes price patterns in ASCII format
 */
void visualizePricePatterns(const StockData* data, int dataSize, const PatternResult* patterns, int patternCount) {
    printf("\n=== PRICE PATTERN VISUALIZATION ===\n");
    
    if (patternCount == 0) {
        printf("No patterns detected in the current data.\n");
        return;
    }
    
    // Get max and min price for scaling
    double maxPrice = 0;
    double minPrice = 999999;
    int startIdx = dataSize - 60 < 0 ? 0 : dataSize - 60;
    
    for (int i = startIdx; i < dataSize; i++) {
        if (data[i].high > maxPrice) maxPrice = data[i].high;
        if (data[i].low < minPrice) minPrice = data[i].low;
    }
    
    // Buffer for the chart (50 rows x 60 columns)
    char chart[50][61];
    memset(chart, ' ', sizeof(chart));
    for (int i = 0; i < 50; i++) chart[i][60] = '\0';
    
    // Plot the price data
    for (int col = 0; col < 60; col++) {
        int dataIdx = startIdx + col;
        if (dataIdx >= dataSize) break;
        
        // Scale price to fit within 0-49 rows (inverted)
        double priceRange = maxPrice - minPrice;
        int closeRow = (int)(49 - ((data[dataIdx].close - minPrice) / priceRange * 49));
        if (closeRow < 0) closeRow = 0;
        if (closeRow > 49) closeRow = 49;
        
        chart[closeRow][col] = '.';
    }
    
    // Highlight patterns on the chart
    for (int p = 0; p < patternCount; p++) {
        // Skip patterns outside our visualization range
        if (patterns[p].endIndex < startIdx || patterns[p].startIndex >= dataSize) continue;
        
        // Convert pattern start/end to chart columns
        int patternStartCol = patterns[p].startIndex - startIdx;
        int patternEndCol = patterns[p].endIndex - startIdx;
        
        if (patternStartCol < 0) patternStartCol = 0;
        if (patternEndCol >= 60) patternEndCol = 59;
        
        // Mark pattern on chart based on type
        char marker;
        switch (patterns[p].type) {
            case PATTERN_SUPPORT:
                marker = 'S';
                break;
            case PATTERN_RESISTANCE:
                marker = 'R';
                break;
            case PATTERN_TREND_CHANGE:
                marker = 'T';
                break;
            case PATTERN_DOUBLE_TOP:
                marker = 'D';
                break;
            case PATTERN_DOUBLE_BOTTOM:
                marker = 'B';
                break;
            default:
                marker = 'X';
        }
        
        for (int col = patternStartCol; col <= patternEndCol; col++) {
            // Find the price point for this column
            int dataIdx = startIdx + col;
            if (dataIdx >= dataSize) break;
            
            double price = data[dataIdx].close;
            int row = (int)(49 - ((price - minPrice) / (maxPrice - minPrice) * 49));
            if (row < 0) row = 0;
            if (row > 49) row = 49;
            
            chart[row][col] = marker;
        }
    }
    
    // Draw the chart
    printf("\nPrice range: %.2f - %.2f\n", minPrice, maxPrice);
    printf("Time period: last 60 days\n\n");
    printf("Legend: . = Price, S = Support, R = Resistance, T = Trend Change, D = Double Top, B = Double Bottom\n\n");
    
    for (int row = 0; row < 50; row++) {
        printf("%s\n", chart[row]);
    }
    
    // Print horizontal timeline
    printf("\n");
    printf("     |         |         |         |         |         |         |\n");
    printf("-60d     -50d     -40d     -30d     -20d     -10d      today\n");
    
    // Print patterns with details
    printf("\nDetected Patterns:\n");
    for (int p = 0; p < patternCount; p++) {
        printf("%d. %s (%s)\n", p+1, patterns[p].name, patterns[p].description);
        printf("   - Confidence: %.1f%%\n", patterns[p].confidence * 100);
        printf("   - Expected Move: %.1f%%\n", patterns[p].expectedMove * 100);
        printf("\n");
    }
}

/**
 * Visualizes SMA crossover signals in ASCII format
 */
void visualizeSMACrossoverSignals(const StockData* data, int dataSize, 
                                  const TradingSignal* signals, int signalCount,
                                  int shortPeriod, int longPeriod) {
    printf("\n=== SMA CROSSOVER SIGNAL VISUALIZATION ===\n");
    
    if (signalCount == 0) {
        printf("No SMA crossover signals detected in the current data.\n");
        return;
    }
    
    // Calculate SMAs for visualization
    double* shortSMA = malloc(dataSize * sizeof(double));
    double* longSMA = malloc(dataSize * sizeof(double));
    
    if (!shortSMA || !longSMA) {
        printf("Error: Could not allocate memory for SMA calculation.\n");
        free(shortSMA);
        free(longSMA);
        return;
    }
    
    // Calculate short SMA
    for (int i = 0; i < dataSize; i++) {
        if (i < shortPeriod - 1) {
            shortSMA[i] = 0; // Not enough data points
        } else {
            double sum = 0;
            for (int j = i - shortPeriod + 1; j <= i; j++) {
                sum += data[j].close;
            }
            shortSMA[i] = sum / shortPeriod;
        }
    }
    
    // Calculate long SMA
    for (int i = 0; i < dataSize; i++) {
        if (i < longPeriod - 1) {
            longSMA[i] = 0; // Not enough data points
        } else {
            double sum = 0;
            for (int j = i - longPeriod + 1; j <= i; j++) {
                sum += data[j].close;
            }
            longSMA[i] = sum / longPeriod;
        }
    }
    
    // Get max and min values for scaling
    double maxVal = 0;
    double minVal = 999999;
    int startIdx = dataSize - 60 < 0 ? 0 : dataSize - 60;
    
    for (int i = startIdx; i < dataSize; i++) {
        if (shortSMA[i] > maxVal) maxVal = shortSMA[i];
        if (longSMA[i] > maxVal) maxVal = longSMA[i];
        if (shortSMA[i] < minVal && shortSMA[i] > 0) minVal = shortSMA[i];
        if (longSMA[i] < minVal && longSMA[i] > 0) minVal = longSMA[i];
    }
    
    // Buffer for the chart (30 rows x 60 columns)
    char chart[30][61];
    memset(chart, ' ', sizeof(chart));
    for (int i = 0; i < 30; i++) chart[i][60] = '\0';
    
    // Plot the SMAs
    for (int col = 0; col < 60; col++) {
        int dataIdx = startIdx + col;
        if (dataIdx >= dataSize || dataIdx < longPeriod - 1) continue;
        
        // Scale values to fit within 0-29 rows (inverted)
        double valueRange = maxVal - minVal;
        
        int shortRow = (int)(29 - ((shortSMA[dataIdx] - minVal) / valueRange * 29));
        if (shortRow < 0) shortRow = 0;
        if (shortRow > 29) shortRow = 29;
        
        int longRow = (int)(29 - ((longSMA[dataIdx] - minVal) / valueRange * 29));
        if (longRow < 0) longRow = 0;
        if (longRow > 29) longRow = 29;
        
        chart[shortRow][col] = 's';
        chart[longRow][col] = 'l';
        
        // If they are at the same position, mark as crossover
        if (shortRow == longRow) {
            chart[shortRow][col] = 'X';
        }
    }
    
    // Mark trading signals on the chart
    for (int s = 0; s < signalCount; s++) {
        int signalCol = signals[s].day - startIdx;
        if (signalCol < 0 || signalCol >= 60) continue;
        
        int dataIdx = signals[s].day;
        if (dataIdx >= dataSize) continue;
        
        // Calculate position for signal
        double valueRange = maxVal - minVal;
        int row = (int)(29 - ((data[dataIdx].close - minVal) / valueRange * 29));
        if (row < 0) row = 0;
        if (row > 29) row = 29;
        
        // Mark buy/sell signals
        if (signals[s].action == ACTION_BUY) {
            chart[row][signalCol] = 'B';
        } else if (signals[s].action == ACTION_SELL) {
            chart[row][signalCol] = 'S';
        }
    }
    
    // Draw the chart
    printf("\nVisualization of %d-day SMA (s) and %d-day SMA (l)\n", shortPeriod, longPeriod);
    printf("Value range: %.2f - %.2f\n", minVal, maxVal);
    printf("Time period: last 60 days\n\n");
    printf("Legend: s = Short SMA, l = Long SMA, X = Crossover, B = Buy Signal, S = Sell Signal\n\n");
    
    for (int row = 0; row < 30; row++) {
        printf("%s\n", chart[row]);
    }
    
    // Print horizontal timeline
    printf("\n");
    printf("     |         |         |         |         |         |         |\n");
    printf("-60d     -50d     -40d     -30d     -20d     -10d      today\n");
    
    // Print signals with details
    printf("\nDetected Signals:\n");
    for (int s = 0; s < signalCount; s++) {
        printf("%d. %s Signal on day %d (", s+1, 
               signals[s].action == ACTION_BUY ? "Buy" : "Sell", 
               signals[s].day);
        
        // Calculate days from current day
        int daysAgo = dataSize - 1 - signals[s].day;
        if (daysAgo == 0) {
            printf("today");
        } else if (daysAgo == 1) {
            printf("yesterday");
        } else {
            printf("%d days ago", daysAgo);
        }
        printf(")\n");
        
        printf("   - Strength: %.1f%%\n", signals[s].strength * 100);
        printf("   - Price: $%.2f\n", data[signals[s].day].close);
        printf("\n");
    }
    
    free(shortSMA);
    free(longSMA);
}

/**
 * Visualizes anomaly detection results in ASCII format
 */
void visualizeAnomalies(const StockData* data, int dataSize, 
                        const AnomalyResult* anomalies, int anomalyCount, 
                        double threshold) {
    printf("\n=== ANOMALY DETECTION VISUALIZATION ===\n");
    
    if (anomalyCount == 0) {
        printf("No anomalies detected above threshold %.2f in the current data.\n", threshold);
        return;
    }
    
    // Get max and min price for scaling
    double maxPrice = 0;
    double minPrice = 999999;
    int startIdx = dataSize - 60 < 0 ? 0 : dataSize - 60;
    
    for (int i = startIdx; i < dataSize; i++) {
        if (data[i].high > maxPrice) maxPrice = data[i].high;
        if (data[i].low < minPrice) minPrice = data[i].low;
    }
    
    // Buffer for the chart (40 rows x 60 columns)
    char chart[40][61];
    memset(chart, ' ', sizeof(chart));
    for (int i = 0; i < 40; i++) chart[i][60] = '\0';
    
    // Plot the price data
    for (int col = 0; col < 60; col++) {
        int dataIdx = startIdx + col;
        if (dataIdx >= dataSize) break;
        
        // Scale price to fit within 0-39 rows (inverted)
        double priceRange = maxPrice - minPrice;
        int closeRow = (int)(39 - ((data[dataIdx].close - minPrice) / priceRange * 39));
        if (closeRow < 0) closeRow = 0;
        if (closeRow > 39) closeRow = 39;
        
        chart[closeRow][col] = '.';
    }
    
    // Mark anomalies on the chart
    for (int a = 0; a < anomalyCount; a++) {
        int anomalyCol = anomalies[a].day - startIdx;
        if (anomalyCol < 0 || anomalyCol >= 60) continue;
        
        int dataIdx = anomalies[a].day;
        if (dataIdx >= dataSize) continue;
        
        // Calculate position for anomaly
        double priceRange = maxPrice - minPrice;
        int row = (int)(39 - ((data[dataIdx].close - minPrice) / priceRange * 39));
        if (row < 0) row = 0;
        if (row > 39) row = 39;
        
        // Mark anomaly with its severity
        char marker = 'A';
        if (anomalies[a].score > threshold * 2) {
            marker = '!';  // Very strong anomaly
        } else if (anomalies[a].score > threshold * 1.5) {
            marker = '*';  // Strong anomaly
        }
        
        chart[row][anomalyCol] = marker;
    }
    
    // Draw the chart
    printf("\nPrice range: %.2f - %.2f\n", minPrice, maxPrice);
    printf("Anomaly threshold: %.2f\n", threshold);
    printf("Time period: last 60 days\n\n");
    printf("Legend: . = Price, A = Anomaly, * = Strong Anomaly, ! = Very Strong Anomaly\n\n");
    
    for (int row = 0; row < 40; row++) {
        printf("%s\n", chart[row]);
    }
    
    // Print horizontal timeline
    printf("\n");
    printf("     |         |         |         |         |         |         |\n");
    printf("-60d     -50d     -40d     -30d     -20d     -10d      today\n");
    
    // Print anomalies with details
    printf("\nDetected Anomalies:\n");
    for (int a = 0; a < anomalyCount; a++) {
        printf("%d. Anomaly on day %d (", a+1, anomalies[a].day);
        
        // Calculate days from current day
        int daysAgo = dataSize - 1 - anomalies[a].day;
        if (daysAgo == 0) {
            printf("today");
        } else if (daysAgo == 1) {
            printf("yesterday");
        } else {
            printf("%d days ago", daysAgo);
        }
        printf(")\n");
        
        printf("   - Score: %.2f (%.1fx threshold)\n", 
               anomalies[a].score, anomalies[a].score / threshold);
        printf("   - Type: %s\n", 
               anomalies[a].type == ANOMALY_PRICE ? "Price Anomaly" : 
               anomalies[a].type == ANOMALY_VOLUME ? "Volume Anomaly" : "Combined Anomaly");
        printf("   - Price: $%.2f\n", data[anomalies[a].day].close);
        printf("   - Volume: %.0f\n", data[anomalies[a].day].volume);
        printf("\n");
    }
}

/**
 * Main visualization function that displays results from all three core algorithms
 */
void visualizeDataMiningResults(const StockData* data, int dataSize, 
                               const PatternResult* patterns, int patternCount,
                               const TradingSignal* signals, int signalCount,
                               const AnomalyResult* anomalies, int anomalyCount,
                               int shortPeriod, int longPeriod, double anomalyThreshold) {
    printf("\n============================================================\n");
    printf("              STOCKPREDICT DATA MINING RESULTS               \n");
    printf("============================================================\n");
    printf("Analyzing %d days of market data\n", dataSize);
    printf("Last date: %s\n", data[dataSize-1].date);
    printf("Last price: $%.2f\n", data[dataSize-1].close);
    
    // Visualize each core algorithm's results
    visualizePricePatterns(data, dataSize, patterns, patternCount);
    visualizeSMACrossoverSignals(data, dataSize, signals, signalCount, shortPeriod, longPeriod);
    visualizeAnomalies(data, dataSize, anomalies, anomalyCount, anomalyThreshold);
    
    // Show summary of all results
    printf("\n============================================================\n");
    printf("                     SUMMARY OF RESULTS                      \n");
    printf("============================================================\n");
    printf("Total patterns detected: %d\n", patternCount);
    printf("Total trading signals: %d\n", signalCount);
    printf("Total anomalies detected: %d\n", anomalyCount);
    
    // Provide an overall market analysis
    printf("\nOVERALL MARKET ANALYSIS:\n");
    
    // Calculate recent trend
    double recentChange = 0;
    if (dataSize >= 10) {
        recentChange = (data[dataSize-1].close - data[dataSize-10].close) / data[dataSize-10].close * 100;
    }
    
    printf("Recent 10-day trend: %.2f%%\n", recentChange);
    
    // Count recent signals and anomalies (last 10 days)
    int recentSignals = 0;
    int recentBuySignals = 0;
    int recentSellSignals = 0;
    for (int i = 0; i < signalCount; i++) {
        if (signals[i].day >= dataSize - 10) {
            recentSignals++;
            if (signals[i].action == ACTION_BUY) recentBuySignals++;
            if (signals[i].action == ACTION_SELL) recentSellSignals++;
        }
    }
    
    int recentAnomalies = 0;
    for (int i = 0; i < anomalyCount; i++) {
        if (anomalies[i].day >= dataSize - 10) {
            recentAnomalies++;
        }
    }
    
    printf("Recent signals: %d (%d buy, %d sell)\n", recentSignals, recentBuySignals, recentSellSignals);
    printf("Recent anomalies: %d\n", recentAnomalies);
    
    // Generate a simple market sentiment analysis
    printf("\nMARKET SENTIMENT: ");
    if (recentBuySignals > recentSellSignals && recentChange > 0 && recentAnomalies == 0) {
        printf("BULLISH");
    } else if (recentSellSignals > recentBuySignals && recentChange < 0) {
        printf("BEARISH");
    } else if (recentAnomalies > 3) {
        printf("TURBULENT");
    } else {
        printf("NEUTRAL");
    }
    printf("\n\n");
    
    // Calculate volatility
    double volatility = 0;
    if (dataSize >= 20) {
        double sum = 0;
        double mean = 0;
        
        // Calculate mean of daily returns
        for (int i = dataSize - 20; i < dataSize; i++) {
            double dailyReturn = (data[i].close - data[i-1].close) / data[i-1].close;
            sum += dailyReturn;
        }
        mean = sum / 20;
        
        // Calculate standard deviation
        sum = 0;
        for (int i = dataSize - 20; i < dataSize; i++) {
            double dailyReturn = (data[i].close - data[i-1].close) / data[i-1].close;
            sum += (dailyReturn - mean) * (dailyReturn - mean);
        }
        volatility = sqrt(sum / 20) * 100; // Convert to percentage
    }
    
    printf("20-day volatility: %.2f%%\n", volatility);
    
    // Recent pattern analysis
    printf("\nRECENT PATTERN ANALYSIS:\n");
    int recentPatterns = 0;
    for (int i = 0; i < patternCount; i++) {
        if (patterns[i].endIndex >= dataSize - 15) {
            recentPatterns++;
            printf("- %s (Confidence: %.1f%%, Expected Move: %.1f%%)\n", 
                   patterns[i].name, 
                   patterns[i].confidence * 100,
                   patterns[i].expectedMove * 100);
        }
    }
    
    if (recentPatterns == 0) {
        printf("No significant patterns detected in the last 15 days.\n");
    }
    
    // Trading recommendation
    printf("\nTRADING RECOMMENDATION: ");
    if (recentBuySignals > recentSellSignals && recentChange > 0 && volatility < 2.0) {
        printf("CONSIDER BUYING\n");
    } else if (recentSellSignals > recentBuySignals && recentChange < 0) {
        printf("CONSIDER SELLING\n");
    } else if (volatility > 3.0) {
        printf("CAUTION - HIGH VOLATILITY\n");
    } else if (recentAnomalies > 2) {
        printf("WAIT - MARKET ANOMALIES DETECTED\n");
    } else {
        printf("HOLD CURRENT POSITION\n");
    }
    
    // Price target calculation
    if (dataSize >= 30) {
        double avgDailyMove = 0;
        for (int i = dataSize - 30; i < dataSize; i++) {
            avgDailyMove += fabs((data[i].close - data[i-1].close) / data[i-1].close);
        }
        avgDailyMove = (avgDailyMove / 30) * 100; // Convert to percentage
        
        double currentPrice = data[dataSize-1].close;
        printf("\nPrice targets (7 days):\n");
        printf("- Bearish: $%.2f\n", currentPrice * (1 - (avgDailyMove/100 * 7 * 1.5)));
        printf("- Neutral: $%.2f\n", currentPrice);
        printf("- Bullish: $%.2f\n", currentPrice * (1 + (avgDailyMove/100 * 7 * 1.5)));
    }
    
    // Calculate support and resistance levels
    printf("\nKEY SUPPORT AND RESISTANCE LEVELS:\n");
    
    double supportLevel1 = 0, supportLevel2 = 0;
    double resistanceLevel1 = 0, resistanceLevel2 = 0;
    
    // Use a simple method to identify support and resistance levels
    // based on recent lows and highs with a price clustering approach
    if (dataSize >= 30) {
        // Find potential support levels (recent lows)
        double lowPrices[30];
        for (int i = 0; i < 30; i++) {
            lowPrices[i] = data[dataSize - 30 + i].low;
        }
        
        // Sort the array to find price clusters
        for (int i = 0; i < 29; i++) {
            for (int j = 0; j < 29 - i; j++) {
                if (lowPrices[j] > lowPrices[j + 1]) {
                    double temp = lowPrices[j];
                    lowPrices[j] = lowPrices[j + 1];
                    lowPrices[j + 1] = temp;
                }
            }
        }
        
        // Find clusters (prices within 1% range)
        int maxClusterSize = 0;
        int maxClusterIndex = 0;
        int currentClusterSize = 1;
        int currentClusterIndex = 0;
        
        for (int i = 1; i < 30; i++) {
            if ((lowPrices[i] - lowPrices[currentClusterIndex]) / lowPrices[currentClusterIndex] <= 0.01) {
                currentClusterSize++;
                if (currentClusterSize > maxClusterSize) {
                    maxClusterSize = currentClusterSize;
                    maxClusterIndex = currentClusterIndex;
                }
            } else {
                currentClusterIndex = i;
                currentClusterSize = 1;
            }
        }
        
        // First support level based on the largest cluster
        supportLevel1 = lowPrices[maxClusterIndex];
        
        // Find the second largest cluster for support level 2
        int secondMaxClusterSize = 0;
        int secondMaxClusterIndex = 0;
        currentClusterSize = 1;
        currentClusterIndex = 0;
        
        for (int i = 1; i < 30; i++) {
            if (i >= maxClusterIndex && i < maxClusterIndex + maxClusterSize) {
                continue; // Skip the largest cluster we already found
            }
            
            if ((lowPrices[i] - lowPrices[currentClusterIndex]) / lowPrices[currentClusterIndex] <= 0.01) {
                currentClusterSize++;
                if (currentClusterSize > secondMaxClusterSize) {
                    secondMaxClusterSize = currentClusterSize;
                    secondMaxClusterIndex = currentClusterIndex;
                }
            } else {
                currentClusterIndex = i;
                currentClusterSize = 1;
            }
        }
        
        supportLevel2 = lowPrices[secondMaxClusterIndex];
        
        // Do the same for resistance levels with high prices
        double highPrices[30];
        for (int i = 0; i < 30; i++) {
            highPrices[i] = data[dataSize - 30 + i].high;
        }
        
        // Sort the array
        for (int i = 0; i < 29; i++) {
            for (int j = 0; j < 29 - i; j++) {
                if (highPrices[j] > highPrices[j + 1]) {
                    double temp = highPrices[j];
                    highPrices[j] = highPrices[j + 1];
                    highPrices[j + 1] = temp;
                }
            }
        }
        
        // Find clusters for resistance
        maxClusterSize = 0;
        maxClusterIndex = 0;
        currentClusterSize = 1;
        currentClusterIndex = 0;
        
        for (int i = 1; i < 30; i++) {
            if ((highPrices[i] - highPrices[currentClusterIndex]) / highPrices[currentClusterIndex] <= 0.01) {
                currentClusterSize++;
                if (currentClusterSize > maxClusterSize) {
                    maxClusterSize = currentClusterSize;
                    maxClusterIndex = currentClusterIndex;
                }
            } else {
                currentClusterIndex = i;
                currentClusterSize = 1;
            }
        }
        
        resistanceLevel1 = highPrices[maxClusterIndex];
        
        // Find the second largest cluster for resistance level 2
        secondMaxClusterSize = 0;
        secondMaxClusterIndex = 0;
        currentClusterSize = 1;
        currentClusterIndex = 0;
        
        for (int i = 1; i < 30; i++) {
            if (i >= maxClusterIndex && i < maxClusterIndex + maxClusterSize) {
                continue; // Skip the largest cluster we already found
            }
            
            if ((highPrices[i] - highPrices[currentClusterIndex]) / highPrices[currentClusterIndex] <= 0.01) {
                currentClusterSize++;
                if (currentClusterSize > secondMaxClusterSize) {
                    secondMaxClusterSize = currentClusterSize;
                    secondMaxClusterIndex = currentClusterIndex;
                }
            } else {
                currentClusterIndex = i;
                currentClusterSize = 1;
            }
        }
        
        resistanceLevel2 = highPrices[secondMaxClusterIndex];
        
        // Make sure support levels are below current price and resistance levels are above
        double currentPrice = data[dataSize-1].close;
        
        // Print the levels
        printf("Support levels:\n");
        if (supportLevel1 < currentPrice) {
            printf("- S1: $%.2f (%.1f%% below current)\n", 
                supportLevel1, (currentPrice - supportLevel1) / currentPrice * 100);
        }
        if (supportLevel2 < currentPrice && fabs(supportLevel2 - supportLevel1) > 0.01 * currentPrice) {
            printf("- S2: $%.2f (%.1f%% below current)\n", 
                supportLevel2, (currentPrice - supportLevel2) / currentPrice * 100);
        }
        
        printf("Resistance levels:\n");
        if (resistanceLevel1 > currentPrice) {
            printf("- R1: $%.2f (%.1f%% above current)\n", 
                resistanceLevel1, (resistanceLevel1 - currentPrice) / currentPrice * 100);
        }
        if (resistanceLevel2 > currentPrice && fabs(resistanceLevel2 - resistanceLevel1) > 0.01 * currentPrice) {
            printf("- R2: $%.2f (%.1f%% above current)\n", 
                resistanceLevel2, (resistanceLevel2 - currentPrice) / currentPrice * 100);
        }
    } else {
        printf("Insufficient data to calculate support and resistance levels.\n");
    }
    
    // Event correlation analysis
    printf("\nEVENT CORRELATION ANALYSIS:\n");
    if (anomalyCount > 0 && patternCount > 0) {
        // Look for anomalies that occur near pattern formations
        int correlatedEvents = 0;
        printf("Pattern-Anomaly Correlations:\n");
        
        for (int a = 0; a < anomalyCount; a++) {
            for (int p = 0; p < patternCount; p++) {
                // If anomaly occurs within 3 days of pattern end
                if (abs(anomalies[a].day - patterns[p].endIndex) <= 3) {
                    correlatedEvents++;
                    printf("- %s pattern correlated with %s anomaly (score: %.2f)\n",
                        patterns[p].name,
                        anomalies[a].type == ANOMALY_PRICE ? "price" :
                        anomalies[a].type == ANOMALY_VOLUME ? "volume" : "combined",
                        anomalies[a].score);
                    
                    // Only report first correlation for this anomaly
                    break;
                }
            }
        }
        
        if (correlatedEvents == 0) {
            printf("No significant correlations found between patterns and anomalies.\n");
        }
    } else {
        printf("Insufficient data to perform correlation analysis.\n");
    }
    
    // Volume analysis
    if (dataSize >= 20) {
        printf("\nVOLUME ANALYSIS:\n");
        
        // Calculate average volume over last 20 days
        double avgVolume = 0;
        for (int i = dataSize - 20; i < dataSize; i++) {
            avgVolume += data[i].volume;
        }
        avgVolume /= 20;
        
        // Calculate recent volume trend (last 5 days vs previous 15)
        double recentAvgVolume = 0;
        for (int i = dataSize - 5; i < dataSize; i++) {
            recentAvgVolume += data[i].volume;
        }
        recentAvgVolume /= 5;
        
        double prevAvgVolume = 0;
        for (int i = dataSize - 20; i < dataSize - 5; i++) {
            prevAvgVolume += data[i].volume;
        }
        prevAvgVolume /= 15;
        
        double volumeChange = (recentAvgVolume - prevAvgVolume) / prevAvgVolume * 100;
        
        printf("20-day average volume: %.0f\n", avgVolume);
        printf("Recent volume trend: %.1f%%\n", volumeChange);
        
        if (volumeChange > 50) {
            printf("ALERT: Significant volume increase detected - Potential strong price move ahead\n");
        } else if (volumeChange > 20) {
            printf("NOTE: Volume increasing - Watch for trend continuation or reversal\n");
        } else if (volumeChange < -30) {
            printf("NOTE: Volume decreasing - Possible loss of momentum\n");
        }
        
        // Check for volume spikes in last 5 days
        int volumeSpikes = 0;
        for (int i = dataSize - 5; i < dataSize; i++) {
            if (data[i].volume > 2 * avgVolume) {
                volumeSpikes++;
            }
        }
        
        if (volumeSpikes > 0) {
            printf("Found %d volume spike(s) in the last 5 days\n", volumeSpikes);
            if (recentChange > 0) {
                printf("High volume with price increase suggests bullish conviction\n");
            } else if (recentChange < 0) {
                printf("High volume with price decrease suggests bearish conviction\n");
            }
        }
    }
    
    // Risk assessment table
    printf("\nRISK ASSESSMENT:\n");
    printf("+----------------+---------+--------------------------------------------+\n");
    printf("| FACTOR         | RATING  | DESCRIPTION                                |\n");
    printf("+----------------+---------+--------------------------------------------+\n");
    
    // Assess volatility risk
    printf("| Volatility     | ");
    if (volatility < 1.0) {
        printf("LOW     | Very stable price action                     |\n");
    } else if (volatility < 2.5) {
        printf("MEDIUM  | Normal market volatility                     |\n");
    } else {
        printf("HIGH    | Elevated volatility, consider smaller size   |\n");
    }
    
    // Assess trend risk
    printf("| Trend          | ");
    if (recentChange > 3.0) {
        printf("LOW     | Strong uptrend, good for long positions      |\n");
    } else if (recentChange < -3.0) {
        printf("LOW     | Strong downtrend, good for short positions   |\n");
    } else if (recentChange > 0.5) {
        printf("MEDIUM  | Mild uptrend, use caution                    |\n");
    } else if (recentChange < -0.5) {
        printf("MEDIUM  | Mild downtrend, use caution                  |\n");
    } else {
        printf("HIGH    | No clear trend direction                     |\n");
    }
    
    // Assess anomaly risk
    printf("| Anomalies      | ");
    if (recentAnomalies == 0) {
        printf("LOW     | No recent anomalies detected                 |\n");
    } else if (recentAnomalies < 3) {
        printf("MEDIUM  | Some anomalies detected, proceed with caution|\n");
    } else {
        printf("HIGH    | Multiple anomalies, high market uncertainty  |\n");
    }
    
    // Assess signal consistency
    printf("| Signal Clarity | ");
    if (recentBuySignals > 0 && recentSellSignals == 0) {
        printf("HIGH    | Clear buy signals with no conflicting signals|\n");
    } else if (recentSellSignals > 0 && recentBuySignals == 0) {
        printf("HIGH    | Clear sell signals with no conflicting signals|\n");
    } else if (recentBuySignals > 0 && recentSellSignals > 0) {
        printf("LOW     | Mixed signals indicating market indecision   |\n");
    } else {
        printf("MEDIUM  | No strong signals in either direction        |\n");
    }
    
    // Overall risk assessment
    int riskScore = 0;
    
    // Add to risk score based on various factors
    if (volatility > 2.5) riskScore += 2;
    else if (volatility > 1.0) riskScore += 1;
    
    if (fabs(recentChange) < 0.5) riskScore += 2;
    else if (fabs(recentChange) < 3.0) riskScore += 1;
    
    if (recentAnomalies >= 3) riskScore += 2;
    else if (recentAnomalies > 0) riskScore += 1;
    
    if (recentBuySignals > 0 && recentSellSignals > 0) riskScore += 2;
    else if (recentBuySignals == 0 && recentSellSignals == 0) riskScore += 1;
    
    printf("| OVERALL RISK   | ");
    if (riskScore <= 2) {
        printf("LOW     | Good conditions for implementing strategies  |\n");
    } else if (riskScore <= 5) {
        printf("MEDIUM  | Exercise caution with position sizing        |\n");
    } else {
        printf("HIGH    | Consider waiting for clearer market condition|\n");
    }
    printf("+----------------+---------+--------------------------------------------+\n");
    
    // Print timestamp of analysis
    time_t rawtime;
    struct tm * timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    
    printf("\nAnalysis generated: %s", asctime(timeinfo));
    printf("============================================================\n");
}