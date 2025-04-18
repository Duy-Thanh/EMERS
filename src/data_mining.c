/**
 * Data Mining Module
 * Implementation of pattern recognition and data mining functions
 * FOCUSED VERSION - Data Preprocessing Only
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
    if (!data || dataSize <= 0 || threshold <= 0) {
        return ERR_INVALID_PARAMETER;
    }
    
    int outlierCount = 0;
    
    /* Calculate means */
    double sumOpen = 0, sumHigh = 0, sumLow = 0, sumClose = 0, sumVolume = 0;
    
    for (int i = 0; i < dataSize; i++) {
        sumOpen += data[i].open;
        sumHigh += data[i].high;
        sumLow += data[i].low;
        sumClose += data[i].close;
        sumVolume += data[i].volume;
    }
    
    double meanOpen = sumOpen / dataSize;
    double meanHigh = sumHigh / dataSize;
    double meanLow = sumLow / dataSize;
    double meanClose = sumClose / dataSize;
    double meanVolume = sumVolume / dataSize;
    
    /* Calculate standard deviations */
    double sumSqOpen = 0, sumSqHigh = 0, sumSqLow = 0, sumSqClose = 0, sumSqVolume = 0;
    
    for (int i = 0; i < dataSize; i++) {
        double diffOpen = data[i].open - meanOpen;
        double diffHigh = data[i].high - meanHigh;
        double diffLow = data[i].low - meanLow;
        double diffClose = data[i].close - meanClose;
        double diffVolume = data[i].volume - meanVolume;
        
        sumSqOpen += diffOpen * diffOpen;
        sumSqHigh += diffHigh * diffHigh;
        sumSqLow += diffLow * diffLow;
        sumSqClose += diffClose * diffClose;
        sumSqVolume += diffVolume * diffVolume;
    }
    
    double stdOpen = sqrt(sumSqOpen / dataSize);
    double stdHigh = sqrt(sumSqHigh / dataSize);
    double stdLow = sqrt(sumSqLow / dataSize);
    double stdClose = sqrt(sumSqClose / dataSize);
    double stdVolume = sqrt(sumSqVolume / dataSize);
    
    /* Detect and replace outliers */
    for (int i = 0; i < dataSize; i++) {
        /* Calculate z-scores */
        double zOpen = stdOpen > 0 ? fabs((data[i].open - meanOpen) / stdOpen) : 0;
        double zHigh = stdHigh > 0 ? fabs((data[i].high - meanHigh) / stdHigh) : 0;
        double zLow = stdLow > 0 ? fabs((data[i].low - meanLow) / stdLow) : 0;
        double zClose = stdClose > 0 ? fabs((data[i].close - meanClose) / stdClose) : 0;
        double zVolume = stdVolume > 0 ? fabs((data[i].volume - meanVolume) / stdVolume) : 0;
        
        /* Fix outliers */
        if (zOpen > threshold) {
            data[i].open = meanOpen;
            outlierCount++;
        }
        
        if (zHigh > threshold) {
            data[i].high = meanHigh;
            outlierCount++;
        }
        
        if (zLow > threshold) {
            data[i].low = meanLow;
            outlierCount++;
        }
        
        if (zClose > threshold) {
            data[i].close = meanClose;
            outlierCount++;
        }
        
        if (zVolume > threshold) {
            data[i].volume = meanVolume;
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
    
    /* Check for zero values (assumed missing) and interpolate */
    for (int i = 0; i < dataSize; i++) {
        /* Skip first and last points for simplicity */
        if (i == 0 || i == dataSize - 1) {
            continue;
        }
        
        /* Open price */
        if (data[i].open == 0.0) {
            /* Find previous non-zero value */
            int prev = i - 1;
            while (prev >= 0 && data[prev].open == 0.0) {
                prev--;
            }
            
            /* Find next non-zero value */
            int next = i + 1;
            while (next < dataSize && data[next].open == 0.0) {
                next++;
            }
            
            /* Interpolate if both bounds found */
            if (prev >= 0 && next < dataSize) {
                double weight = (double)(i - prev) / (next - prev);
                data[i].open = data[prev].open + weight * (data[next].open - data[prev].open);
                filledCount++;
            }
        }
        
        /* High price */
        if (data[i].high == 0.0) {
            /* Similar interpolation logic for high */
            int prev = i - 1;
            while (prev >= 0 && data[prev].high == 0.0) {
                prev--;
            }
            
            int next = i + 1;
            while (next < dataSize && data[next].high == 0.0) {
                next++;
            }
            
            if (prev >= 0 && next < dataSize) {
                double weight = (double)(i - prev) / (next - prev);
                data[i].high = data[prev].high + weight * (data[next].high - data[prev].high);
                filledCount++;
            }
        }
        
        /* Low price */
        if (data[i].low == 0.0) {
            /* Similar interpolation logic for low */
            int prev = i - 1;
            while (prev >= 0 && data[prev].low == 0.0) {
                prev--;
            }
            
            int next = i + 1;
            while (next < dataSize && data[next].low == 0.0) {
                next++;
            }
            
            if (prev >= 0 && next < dataSize) {
                double weight = (double)(i - prev) / (next - prev);
                data[i].low = data[prev].low + weight * (data[next].low - data[prev].low);
                filledCount++;
            }
        }
        
        /* Close price */
        if (data[i].close == 0.0) {
            /* Similar interpolation logic for close */
            int prev = i - 1;
            while (prev >= 0 && data[prev].close == 0.0) {
                prev--;
            }
            
            int next = i + 1;
            while (next < dataSize && data[next].close == 0.0) {
                next++;
            }
            
            if (prev >= 0 && next < dataSize) {
                double weight = (double)(i - prev) / (next - prev);
                data[i].close = data[prev].close + weight * (data[next].close - data[prev].close);
                filledCount++;
            }
        }
        
        /* Volume */
        if (data[i].volume == 0.0) {
            /* Similar interpolation logic for volume */
            int prev = i - 1;
            while (prev >= 0 && data[prev].volume == 0.0) {
                prev--;
            }
            
            int next = i + 1;
            while (next < dataSize && data[next].volume == 0.0) {
                next++;
            }
            
            if (prev >= 0 && next < dataSize) {
                double weight = (double)(i - prev) / (next - prev);
                data[i].volume = data[prev].volume + weight * (data[next].volume - data[prev].volume);
                filledCount++;
            }
        }
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
    
    /* Create a temporary copy of the data for preprocessing */
    StockData* tmpData = (StockData*)malloc(inputSize * sizeof(StockData));
    if (!tmpData) {
        return ERR_MEMORY_ALLOCATION;
    }
    
    /* Copy input data to temporary array */
    memcpy(tmpData, inputData, inputSize * sizeof(StockData));
    
    /* Preprocess the data */
    int filledCount = fillMissingData(tmpData, inputSize);
    (void)filledCount;  /* Prevent unused variable warning */
    
    int outlierCount = removeOutliers(tmpData, inputSize, 3.0);
    (void)outlierCount; /* Prevent unused variable warning */
    
    /* Either normalize or just copy the preprocessed data */
    if (shouldNormalize) {
        normalizeStockData(tmpData, inputSize, outputData);
    } else {
        memcpy(outputData, tmpData, inputSize * sizeof(StockData));
    }
    
    /* Clean up */
    free(tmpData);
    
    return 0;
}