/**
 * Data Mining Module Header
 * Core pattern recognition and data mining functions
 * FOCUSED VERSION - Data Preprocessing Only
 */

#ifndef DATA_MINING_H
#define DATA_MINING_H

#include "emers.h"

/* Data Preprocessing Functions */

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

#endif /* DATA_MINING_H */