/**
 * @file asm_optimize.h
 * @brief Assembly-optimized functions for performance-critical operations
 */

#ifndef ASM_OPTIMIZE_H
#define ASM_OPTIMIZE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

/**
 * @brief Assembly-optimized implementation of Simple Moving Average calculation
 * 
 * @param data Array of price data
 * @param n Size of the data array
 * @param period The period for the moving average
 * @param output Array to store the result (must be pre-allocated with size n)
 */
void asmCalculateSMA(const double* data, int n, int period, double* output);

/**
 * @brief Assembly-optimized implementation of Exponential Moving Average calculation
 * 
 * @param data Array of price data
 * @param n Size of the data array
 * @param period The period for the moving average
 * @param output Array to store the result (must be pre-allocated with size n)
 */
void asmCalculateEMA(const double* data, int n, int period, double* output);

/**
 * @brief Assembly-optimized implementation of Relative Strength Index calculation
 * 
 * @param data Array of price data
 * @param n Size of the data array
 * @param period The period for the RSI
 * @param output Array to store the result (must be pre-allocated with size n)
 */
void asmCalculateRSI(const double* data, int n, int period, double* output);

/**
 * @brief Assembly-optimized SIMD implementation of vector-to-vector arithmetic operation
 * 
 * @param a First vector
 * @param b Second vector
 * @param n Size of the vectors
 * @param op Operation to perform (0: add, 1: subtract, 2: multiply, 3: divide)
 * @param output Result vector (must be pre-allocated with size n)
 */
void asmVectorOp(const double* a, const double* b, int n, int op, double* output);

/**
 * @brief SIMD-optimized string search for keyword matching
 * 
 * @param text The text to search in
 * @param keywords Array of keywords to search for
 * @param keywordCount Number of keywords
 * @param matches Array to store the indices of matched keywords
 * @param maxMatches Maximum number of matches to store
 * @return Number of matches found
 */
int asmFindKeywordsInText(const char* text, const char** keywords, int keywordCount, 
                         int* matches, int maxMatches);

/**
 * @brief SIMD-optimized implementation of Porter stemming algorithm for English
 * 
 * @param word The word to stem (modified in-place)
 * @return Length of stemmed word
 */
int asmPorterStemmer(char* word);

/**
 * @brief Assembly-optimized sentiment scoring for bag-of-words model
 * 
 * @param text The text to analyze
 * @param positiveWords Array of positive sentiment words
 * @param positiveCount Number of positive words
 * @param negativeWords Array of negative sentiment words
 * @param negativeCount Number of negative words
 * @param score Output parameter for sentiment score (-1.0 to 1.0)
 * @param confidence Output parameter for confidence score (0.0 to 1.0)
 */
void asmCalculateSentimentScore(const char* text, const char** positiveWords, int positiveCount,
                              const char** negativeWords, int negativeCount,
                              double* score, double* confidence);

#endif /* ASM_OPTIMIZE_H */ 