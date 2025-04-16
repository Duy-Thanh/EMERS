/**
 * Emergency Market Event Response System (EMERS)
 * Model Validation Framework
 */

#ifndef MODEL_VALIDATION_H
#define MODEL_VALIDATION_H

#include <stdio.h>
#include <stdlib.h>
#include "emers.h"

/* Validation metrics structure */
typedef struct {
    double accuracy;          /* Overall accuracy (0.0-1.0) */
    double precision;         /* Precision (true positives / predicted positives) */
    double recall;            /* Recall/sensitivity (true positives / actual positives) */
    double f1Score;           /* F1 score (harmonic mean of precision and recall) */
    double meanAbsoluteError; /* MAE for regression models */
    double rootMeanSquareError; /* RMSE for regression models */
    double r2Score;           /* R-squared for regression models */
} ValidationMetrics;

/* Backtesting results structure */
typedef struct {
    ValidationMetrics eventDetectionMetrics;   /* Metrics for event detection */
    ValidationMetrics eventImpactMetrics;      /* Metrics for event impact prediction */
    ValidationMetrics priceDirectionMetrics;   /* Metrics for price direction prediction */
    
    int totalPredictions;       /* Total number of predictions made */
    int correctPredictions;     /* Number of correct predictions */
    
    double profitLoss;          /* Simulated profit/loss from strategy */
    double maxDrawdown;         /* Maximum drawdown during backtesting period */
    double sharpeRatio;         /* Sharpe ratio (risk-adjusted return) */
    
    char* detailedReport;       /* Detailed report text */
} BacktestResults;

/* Cross-validation fold structure */
typedef struct {
    StockData* trainingData;    /* Training data for this fold */
    int trainingSize;           /* Size of training data */
    
    StockData* validationData;  /* Validation data for this fold */
    int validationSize;         /* Size of validation data */
    
    ValidationMetrics metrics;  /* Validation metrics for this fold */
} CrossValidationFold;

/* Cross-validation results structure */
typedef struct {
    CrossValidationFold* folds; /* Array of cross-validation folds */
    int foldCount;              /* Number of folds */
    
    ValidationMetrics averageMetrics;  /* Average metrics across all folds */
    ValidationMetrics bestMetrics;     /* Best metrics across all folds */
    ValidationMetrics worstMetrics;    /* Worst metrics across all folds */
    
    double standardDeviation;   /* Standard deviation of metrics across folds */
} CrossValidationResults;

/* Initialize the model validation framework */
int initModelValidation(void);

/* Clean up the model validation framework */
void cleanupModelValidation(void);

/* Perform backtesting on historical data */
BacktestResults* performBacktest(const Stock* stocks, int stockCount, 
                               const char* startDate, const char* endDate,
                               const char* strategy);

/* Free backtesting results */
void freeBacktestResults(BacktestResults* results);

/* Perform k-fold cross-validation */
CrossValidationResults* performCrossValidation(const Stock* stocks, int stockCount,
                                            int folds, const char* modelType);

/* Free cross-validation results */
void freeCrossValidationResults(CrossValidationResults* results);

/* Calculate validation metrics for event detection */
ValidationMetrics calculateEventDetectionMetrics(const EventData* predictedEvents, int predictedCount,
                                              const EventData* actualEvents, int actualCount);

/* Calculate validation metrics for price prediction */
ValidationMetrics calculatePricePredictionMetrics(const double* predictedPrices, 
                                               const double* actualPrices, int dataSize);

/* Generate a validation report */
char* generateValidationReport(const ValidationMetrics* metrics, const char* modelName);

/* Print validation metrics */
void printValidationMetrics(const ValidationMetrics* metrics, const char* title);

/* Regression testing to ensure model stability */
int performRegressionTest(const char* baselineResultsFile, const char* modelType);

/* Save validation results to file */
int saveValidationResults(const ValidationMetrics* metrics, const char* filename);

/* Load validation results from file */
ValidationMetrics* loadValidationResults(const char* filename);

/* Generate validation results visualization data */
int generateVisualizationData(const ValidationMetrics** metricsArray, 
                             const char** labels,
                             int count, 
                             const char* outputFile);

#endif /* MODEL_VALIDATION_H */