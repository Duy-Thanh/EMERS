/**
 * Data Mining Visualization Module
 * Implementation of visualization functions for data mining results
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "../include/emers.h"
#include "../include/data_mining.h"
#include "../include/technical_analysis.h"

/* Default visualization parameters */
#define DEFAULT_CHART_WIDTH 80
#define DEFAULT_CHART_HEIGHT 20
#define MAX_CHART_WIDTH 200
#define MAX_CHART_HEIGHT 50
#define DEFAULT_OUTPUT_FILE "data_mining_results.txt"
#define DEFAULT_CSV_FILE "data_mining_results.csv"

/**
 * Generate ASCII chart for price data with patterns
 * 
 * @param data Stock data array
 * @param dataSize Number of data points
 * @param patterns Array of detected patterns
 * @param patternCount Number of patterns
 * @param width Chart width
 * @param height Chart height
 * @param output Output file pointer (or NULL for stdout)
 * @return 0 on success, negative on error
 */
int visualizePricePatterns(const StockData* data, int dataSize, 
                          const PatternResult* patterns, int patternCount,
                          int width, int height, FILE* output) {
    if (!data || dataSize <= 0) {
        return -1;
    }
    
    /* Default to stdout if no output file specified */
    if (!output) {
        output = stdout;
    }
    
    /* Validate width and height */
    if (width <= 0) width = DEFAULT_CHART_WIDTH;
    if (height <= 0) height = DEFAULT_CHART_HEIGHT;
    if (width > MAX_CHART_WIDTH) width = MAX_CHART_WIDTH;
    if (height > MAX_CHART_HEIGHT) height = MAX_CHART_HEIGHT;
    
    /* Find min and max prices in the data */
    double minPrice = data[0].low;
    double maxPrice = data[0].high;
    
    for (int i = 1; i < dataSize; i++) {
        if (data[i].low < minPrice) minPrice = data[i].low;
        if (data[i].high > maxPrice) maxPrice = data[i].high;
    }
    
    /* Add some margin */
    double margin = (maxPrice - minPrice) * 0.05;
    minPrice -= margin;
    maxPrice += margin;
    
    /* Create ASCII chart buffer */
    char** chart = (char**)malloc(height * sizeof(char*));
    if (!chart) {
        return -1;
    }
    
    for (int i = 0; i < height; i++) {
        chart[i] = (char*)malloc((width + 1) * sizeof(char));
        if (!chart[i]) {
            for (int j = 0; j < i; j++) {
                free(chart[j]);
            }
            free(chart);
            return -1;
        }
        
        /* Initialize with spaces */
        for (int j = 0; j < width; j++) {
            chart[i][j] = ' ';
        }
        chart[i][width] = '\0';
    }
    
    /* Draw price scale on the left */
    for (int i = 0; i < height; i++) {
        double price = maxPrice - (i * (maxPrice - minPrice) / (height - 1));
        fprintf(output, "%8.2f |", price);
        
        for (int j = 0; j < width; j++) {
            fprintf(output, "%c", chart[i][j]);
        }
        fprintf(output, "\n");
    }
    
    /* Draw time scale at the bottom */
    fprintf(output, "         ");
    for (int j = 0; j < width; j++) {
        fprintf(output, "-");
    }
    fprintf(output, "\n");
    
    fprintf(output, "         ");
    for (int j = 0; j < width; j += width / 10) {
        int dataIndex = j * dataSize / width;
        if (dataIndex < dataSize) {
            fprintf(output, "%-10.10s", data[dataIndex].date);
        }
    }
    fprintf(output, "\n\n");
    
    /* Print patterns */
    if (patterns && patternCount > 0) {
        fprintf(output, "Detected Patterns:\n");
        fprintf(output, "---------------------------------------------------\n");
        
        for (int i = 0; i < patternCount; i++) {
            fprintf(output, "%d. %s (%.0f%% conf.)\n", i+1, patterns[i].name, patterns[i].confidence * 100.0);
            fprintf(output, "   %s\n", patterns[i].description);
            fprintf(output, "   From %s to %s\n", 
                   data[patterns[i].startIndex].date, 
                   data[patterns[i].endIndex].date);
            fprintf(output, "   Expected move: %.2f%%\n", patterns[i].expectedMove * 100.0);
            fprintf(output, "\n");
        }
    }
    
    /* Clean up */
    for (int i = 0; i < height; i++) {
        free(chart[i]);
    }
    free(chart);
    
    return 0;
}

/**
 * Generate visualization for trading signals
 * 
 * @param data Stock data array
 * @param dataSize Number of data points
 * @param signals Array of detected signals
 * @param signalCount Number of signals
 * @param output Output file pointer (or NULL for stdout)
 * @return 0 on success, negative on error
 */
int visualizeTradingSignals(const StockData* data, int dataSize, 
                           const TradingSignal* signals, int signalCount,
                           FILE* output) {
    if (!data || dataSize <= 0 || !signals || signalCount <= 0) {
        return -1;
    }
    
    /* Default to stdout if no output file specified */
    if (!output) {
        output = stdout;
    }
    
    fprintf(output, "Trading Signals Report\n");
    fprintf(output, "=====================================================\n\n");
    
    /* Print each signal with details */
    for (int i = 0; i < signalCount; i++) {
        const TradingSignal* signal = &signals[i];
        
        /* Signal header */
        if (signal->type == SIGNAL_BUY) {
            fprintf(output, "BUY SIGNAL #%d", i+1);
        } else if (signal->type == SIGNAL_SELL) {
            fprintf(output, "SELL SIGNAL #%d", i+1);
        } else {
            fprintf(output, "SIGNAL #%d", i+1);
        }
        
        /* Signal date */
        if (signal->signalIndex >= 0 && signal->signalIndex < dataSize) {
            fprintf(output, " (%s)\n", data[signal->signalIndex].date);
        } else {
            fprintf(output, "\n");
        }
        
        /* Signal details */
        fprintf(output, "Description: %s\n", signal->description);
        fprintf(output, "Confidence: %.1f%%\n", signal->confidence * 100.0);
        fprintf(output, "Entry Price: $%.2f\n", signal->entryPrice);
        fprintf(output, "Target Price: $%.2f\n", signal->targetPrice);
        fprintf(output, "Stop Loss: $%.2f\n", signal->stopLossPrice);
        fprintf(output, "Risk/Reward Ratio: %.2f\n", signal->riskRewardRatio);
        
        /* Signal performance (if we have data after the signal) */
        if (signal->signalIndex < dataSize - 5) {
            int futureIdx = signal->signalIndex + 5; /* 5 days later */
            double futurePct = (data[futureIdx].close - data[signal->signalIndex].close) / 
                              data[signal->signalIndex].close * 100.0;
            
            if ((signal->type == SIGNAL_BUY && futurePct > 0) || 
                (signal->type == SIGNAL_SELL && futurePct < 0)) {
                fprintf(output, "5-Day Performance: %.2f%% (CORRECT)\n", futurePct);
            } else {
                fprintf(output, "5-Day Performance: %.2f%%\n", futurePct);
            }
        }
        
        fprintf(output, "\n");
    }
    
    /* Summary statistics */
    int buySignals = 0;
    int sellSignals = 0;
    
    for (int i = 0; i < signalCount; i++) {
        if (signals[i].type == SIGNAL_BUY) {
            buySignals++;
        } else if (signals[i].type == SIGNAL_SELL) {
            sellSignals++;
        }
    }
    
    fprintf(output, "Summary: %d signals detected (%d buy, %d sell)\n", 
           signalCount, buySignals, sellSignals);
    
    return 0;
}

/**
 * Generate visualization for anomaly detection results
 * 
 * @param data Stock data array
 * @param dataSize Number of data points
 * @param anomalies Array of detected anomalies
 * @param anomalyCount Number of anomalies
 * @param output Output file pointer (or NULL for stdout)
 * @return 0 on success, negative on error
 */
int visualizeAnomalies(const StockData* data, int dataSize, 
                      const AnomalyResult* anomalies, int anomalyCount,
                      FILE* output) {
    if (!data || dataSize <= 0 || !anomalies || anomalyCount <= 0) {
        return -1;
    }
    
    /* Default to stdout if no output file specified */
    if (!output) {
        output = stdout;
    }
    
    fprintf(output, "Anomaly Detection Report\n");
    fprintf(output, "=====================================================\n\n");
    
    /* Print each anomaly with details */
    for (int i = 0; i < anomalyCount; i++) {
        const AnomalyResult* anomaly = &anomalies[i];
        
        /* Anomaly header */
        fprintf(output, "ANOMALY #%d", i+1);
        
        /* Anomaly date */
        if (anomaly->index >= 0 && anomaly->index < dataSize) {
            fprintf(output, " (%s)\n", data[anomaly->index].date);
        } else {
            fprintf(output, "\n");
        }
        
        /* Anomaly details */
        fprintf(output, "Description: %s\n", anomaly->description);
        fprintf(output, "Anomaly Score: %.2f\n", anomaly->score);
        fprintf(output, "Price Deviation: %.2f standard deviations\n", anomaly->priceDeviation);
        fprintf(output, "Volume Deviation: %.2f standard deviations\n", anomaly->volumeDeviation);
        
        /* Price and volume at anomaly */
        if (anomaly->index >= 0 && anomaly->index < dataSize) {
            fprintf(output, "Price: $%.2f\n", data[anomaly->index].close);
            fprintf(output, "Volume: %.0f\n", data[anomaly->index].volume);
            
            /* Calculate price change */
            if (anomaly->index > 0) {
                double priceChange = (data[anomaly->index].close - data[anomaly->index-1].close) / 
                                    data[anomaly->index-1].close * 100.0;
                fprintf(output, "Daily Price Change: %.2f%%\n", priceChange);
            }
            
            /* Calculate volume change */
            if (anomaly->index > 0) {
                double volumeChange = (data[anomaly->index].volume - data[anomaly->index-1].volume) / 
                                     data[anomaly->index-1].volume * 100.0;
                fprintf(output, "Daily Volume Change: %.2f%%\n", volumeChange);
            }
            
            /* Future price movement (if we have data after the anomaly) */
            if (anomaly->index < dataSize - 5) {
                int futureIdx = anomaly->index + 5; /* 5 days later */
                double futurePct = (data[futureIdx].close - data[anomaly->index].close) / 
                                  data[anomaly->index].close * 100.0;
                fprintf(output, "Price Movement 5 Days Later: %.2f%%\n", futurePct);
            }
        }
        
        fprintf(output, "\n");
    }
    
    /* Summary statistics */
    fprintf(output, "Summary: %d anomalies detected\n", anomalyCount);
    
    /* Calculate the average price impact of anomalies */
    if (anomalyCount > 0) {
        double totalImpact = 0.0;
        int countValid = 0;
        
        for (int i = 0; i < anomalyCount; i++) {
            int idx = anomalies[i].index;
            if (idx >= 0 && idx < dataSize - 1) {
                double impact = fabs((data[idx + 1].close - data[idx].close) / data[idx].close * 100.0);
                totalImpact += impact;
                countValid++;
            }
        }
        
        if (countValid > 0) {
            fprintf(output, "Average 1-Day Price Impact: %.2f%%\n", totalImpact / countValid);
        }
    }
    
    return 0;
}

/**
 * Generate visualization for k-means clustering results
 * 
 * @param data Stock data array
 * @param dataSize Number of data points
 * @param clusters Array of clustering results
 * @param clusterCount Number of clusters
 * @param output Output file pointer (or NULL for stdout)
 * @return 0 on success, negative on error
 */
int visualizeClusters(const StockData* data, int dataSize, 
                     const Cluster* clusters, int clusterCount,
                     FILE* output) {
    if (!data || dataSize <= 0 || !clusters || clusterCount <= 0) {
        return -1;
    }
    
    /* Default to stdout if no output file specified */
    if (!output) {
        output = stdout;
    }
    
    fprintf(output, "K-Means Clustering Results (k=%d)\n", clusterCount);
    fprintf(output, "=====================================================\n\n");
    
    /* Print summary for each cluster */
    for (int i = 0; i < clusterCount; i++) {
        const Cluster* cluster = &clusters[i];
        
        fprintf(output, "Cluster %d: %s\n", i+1, cluster->label);
        fprintf(output, "Points: %d (%.1f%% of data)\n", 
               cluster->pointCount, 
               (double)cluster->pointCount / dataSize * 100.0);
        fprintf(output, "Average Distance to Centroid: %.4f\n", cluster->averageDistance);
        
        /* Centroid characteristics */
        fprintf(output, "Centroid: Open=%.2f, High=%.2f, Low=%.2f, Close=%.2f, Volume=%.2f\n",
               cluster->centroid[0], cluster->centroid[1], cluster->centroid[2], 
               cluster->centroid[3], cluster->centroid[4]);
        
        /* Calculate cluster statistics */
        if (cluster->pointCount > 0) {
            double avgReturn = 0.0;
            double avgVolume = 0.0;
            double avgRange = 0.0;
            
            for (int j = 0; j < cluster->pointCount; j++) {
                int pointIdx = cluster->pointIndices[j];
                
                /* Daily return */
                avgReturn += (data[pointIdx].close - data[pointIdx].open) / data[pointIdx].open;
                
                /* Volume */
                avgVolume += data[pointIdx].volume;
                
                /* Range */
                avgRange += (data[pointIdx].high - data[pointIdx].low) / data[pointIdx].low;
            }
            
            avgReturn /= cluster->pointCount;
            avgVolume /= cluster->pointCount;
            avgRange /= cluster->pointCount;
            
            fprintf(output, "Avg Daily Return: %.2f%%\n", avgReturn * 100.0);
            fprintf(output, "Avg Range: %.2f%%\n", avgRange * 100.0);
            fprintf(output, "Avg Volume: %.0f\n", avgVolume);
            
            /* Print a few example dates from this cluster */
            fprintf(output, "Example dates: ");
            int exampleCount = cluster->pointCount < 5 ? cluster->pointCount : 5;
            for (int j = 0; j < exampleCount; j++) {
                int pointIdx = cluster->pointIndices[j];
                fprintf(output, "%s", data[pointIdx].date);
                if (j < exampleCount - 1) {
                    fprintf(output, ", ");
                }
            }
            fprintf(output, "\n");
        }
        
        fprintf(output, "\n");
    }
    
    /* Calculate cluster quality metrics */
    double totalWithinClusterDist = 0.0;
    for (int i = 0; i < clusterCount; i++) {
        totalWithinClusterDist += clusters[i].averageDistance * clusters[i].pointCount;
    }
    
    /* Calculate average distance between clusters */
    double totalInterClusterDist = 0.0;
    int comparisons = 0;
    
    for (int i = 0; i < clusterCount; i++) {
        for (int j = i + 1; j < clusterCount; j++) {
            double distance = 0.0;
            for (int d = 0; d < 5; d++) {
                distance += (clusters[i].centroid[d] - clusters[j].centroid[d]) * 
                           (clusters[i].centroid[d] - clusters[j].centroid[d]);
            }
            distance = sqrt(distance);
            
            totalInterClusterDist += distance;
            comparisons++;
        }
    }
    
    double avgInterClusterDist = comparisons > 0 ? totalInterClusterDist / comparisons : 0.0;
    double silhouetteScore = avgInterClusterDist > 0 ? 
                             (avgInterClusterDist - totalWithinClusterDist / dataSize) / 
                             avgInterClusterDist : 0.0;
    
    fprintf(output, "Clustering Quality Metrics\n");
    fprintf(output, "Average Within-Cluster Distance: %.4f\n", totalWithinClusterDist / dataSize);
    fprintf(output, "Average Between-Cluster Distance: %.4f\n", avgInterClusterDist);
    fprintf(output, "Silhouette Score: %.4f (higher is better, range -1 to 1)\n", silhouetteScore);
    
    return 0;
}

/**
 * Export data mining results to CSV file
 * 
 * @param data Stock data array
 * @param dataSize Number of data points
 * @param patterns Array of detected patterns
 * @param patternCount Number of patterns
 * @param signals Array of detected signals
 * @param signalCount Number of signals
 * @param anomalies Array of detected anomalies
 * @param anomalyCount Number of anomalies
 * @param outputFilename CSV output filename
 * @return 0 on success, negative on error
 */
int exportResultsToCSV(const StockData* data, int dataSize,
                      const PatternResult* patterns, int patternCount,
                      const TradingSignal* signals, int signalCount,
                      const AnomalyResult* anomalies, int anomalyCount,
                      const char* outputFilename) {
    if (!data || dataSize <= 0 || !outputFilename) {
        return -1;
    }
    
    /* Open output file */
    FILE* output = fopen(outputFilename, "w");
    if (!output) {
        return -1;
    }
    
    /* Write header */
    fprintf(output, "Date,Open,High,Low,Close,Volume,PatternDetected,PatternType,PatternConfidence,");
    fprintf(output, "SignalDetected,SignalType,SignalConfidence,AnomalyDetected,AnomalyScore\n");
    
    /* Write data for each day */
    for (int i = 0; i < dataSize; i++) {
        /* Basic price and volume data */
        fprintf(output, "%s,%.2f,%.2f,%.2f,%.2f,%.0f,",
               data[i].date, data[i].open, data[i].high, data[i].low, data[i].close, data[i].volume);
        
        /* Check for patterns */
        int patternFound = 0;
        for (int p = 0; p < patternCount; p++) {
            if (i >= patterns[p].startIndex && i <= patterns[p].endIndex) {
                patternFound = 1;
                fprintf(output, "1,%s,%.2f,", patterns[p].name, patterns[p].confidence);
                break;
            }
        }
        if (!patternFound) {
            fprintf(output, "0,,0,");
        }
        
        /* Check for signals */
        int signalFound = 0;
        for (int s = 0; s < signalCount; s++) {
            if (i == signals[s].signalIndex) {
                signalFound = 1;
                fprintf(output, "1,%s,%.2f,", 
                       (signals[s].type == SIGNAL_BUY) ? "BUY" : 
                       (signals[s].type == SIGNAL_SELL) ? "SELL" : "OTHER",
                       signals[s].confidence);
                break;
            }
        }
        if (!signalFound) {
            fprintf(output, "0,,0,");
        }
        
        /* Check for anomalies */
        int anomalyFound = 0;
        for (int a = 0; a < anomalyCount; a++) {
            if (i == anomalies[a].index) {
                anomalyFound = 1;
                fprintf(output, "1,%.2f\n", anomalies[a].score);
                break;
            }
        }
        if (!anomalyFound) {
            fprintf(output, "0,0\n");
        }
    }
    
    fclose(output);
    return 0;
}

/**
 * Generate comprehensive data mining results report
 * 
 * @param data Stock data array
 * @param dataSize Number of data points
 * @param symbol Stock symbol
 * @param patterns Array of detected patterns
 * @param patternCount Number of patterns
 * @param signals Array of detected signals
 * @param signalCount Number of signals
 * @param anomalies Array of detected anomalies
 * @param anomalyCount Number of anomalies
 * @param clusters Array of clustering results
 * @param clusterCount Number of clusters
 * @param outputFilename Output filename (or NULL for stdout)
 * @return 0 on success, negative on error
 */
int generateDataMiningReport(const StockData* data, int dataSize, const char* symbol,
                           const PatternResult* patterns, int patternCount,
                           const TradingSignal* signals, int signalCount,
                           const AnomalyResult* anomalies, int anomalyCount,
                           const Cluster* clusters, int clusterCount,
                           const char* outputFilename) {
    if (!data || dataSize <= 0) {
        return -1;
    }
    
    /* Open output file if specified, otherwise use stdout */
    FILE* output = outputFilename ? fopen(outputFilename, "w") : stdout;
    if (!output) {
        return -1;
    }
    
    /* Report header */
    time_t now = time(NULL);
    struct tm* timeinfo = localtime(&now);
    char dateStr[64];
    strftime(dateStr, sizeof(dateStr), "%Y-%m-%d %H:%M:%S", timeinfo);
    
    fprintf(output, "=======================================================\n");
    fprintf(output, "          STOCK PREDICT DATA MINING REPORT              \n");
    fprintf(output, "=======================================================\n\n");
    
    fprintf(output, "Generated: %s\n", dateStr);
    fprintf(output, "Symbol: %s\n", symbol ? symbol : "Unknown");
    fprintf(output, "Data Range: %s to %s\n", 
           dataSize > 0 ? data[0].date : "Unknown", 
           dataSize > 0 ? data[dataSize-1].date : "Unknown");
    fprintf(output, "Data Points: %d\n\n", dataSize);
    
    /* Summary statistics */
    double currentPrice = dataSize > 0 ? data[dataSize-1].close : 0.0;
    double startPrice = dataSize > 0 ? data[0].close : 0.0;
    double percentChange = (startPrice > 0) ? (currentPrice - startPrice) / startPrice * 100.0 : 0.0;
    
    fprintf(output, "Current Price: $%.2f\n", currentPrice);
    fprintf(output, "Period Change: %.2f%%\n\n", percentChange);
    
    /* Patterns section */
    if (patterns && patternCount > 0) {
        fprintf(output, "----- PRICE PATTERNS (%d detected) -----\n\n", patternCount);
        visualizePricePatterns(data, dataSize, patterns, patternCount, 80, 20, output);
        fprintf(output, "\n");
    }
    
    /* Signals section */
    if (signals && signalCount > 0) {
        fprintf(output, "----- TRADING SIGNALS (%d detected) -----\n\n", signalCount);
        visualizeTradingSignals(data, dataSize, signals, signalCount, output);
        fprintf(output, "\n");
    }
    
    /* Anomalies section */
    if (anomalies && anomalyCount > 0) {
        fprintf(output, "----- ANOMALIES (%d detected) -----\n\n", anomalyCount);
        visualizeAnomalies(data, dataSize, anomalies, anomalyCount, output);
        fprintf(output, "\n");
    }
    
    /* Clusters section */
    if (clusters && clusterCount > 0) {
        fprintf(output, "----- CLUSTERING ANALYSIS (%d clusters) -----\n\n", clusterCount);
        visualizeClusters(data, dataSize, clusters, clusterCount, output);
        fprintf(output, "\n");
    }
    
    /* Close file if we opened one */
    if (outputFilename) {
        fclose(output);
    }
    
    return 0;
}

/* Create a header for the visualization module */
