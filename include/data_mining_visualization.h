/**
 * Data Mining Visualization Module
 * Header file for data mining results visualization
 */

#ifndef DATA_MINING_VISUALIZATION_H
#define DATA_MINING_VISUALIZATION_H

#include <stdio.h>
#include "emers.h"
#include "data_mining.h"

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
                          int width, int height, FILE* output);

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
                          FILE* output);

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
                     FILE* output);

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
                    FILE* output);

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
                     const char* outputFilename);

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
                          const char* outputFilename);

#endif /* DATA_MINING_VISUALIZATION_H */ 