/**
 * Emergency Market Event Response System (EMERS)
 * Model Validation Framework
 */

#ifndef MODEL_VALIDATION_H
#define MODEL_VALIDATION_H

#include <stdio.h>
#include <stdlib.h>
#include "emers.h"

/* Signal types for model validation */
#define SIGNAL_NONE         0
#define SIGNAL_BUY          1
#define SIGNAL_SELL         2

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

/* Trading signal structure */
typedef struct {
    int type;                 /* Signal type (buy, sell, none) */
    double strength;          /* Signal strength (0.0-1.0) */
    double price;             /* Suggested price point */
    double stopLoss;          /* Suggested stop loss level */
    double takeProfit;        /* Suggested take profit level */
} TradingSignal;

/* Trading strategy structure */
typedef struct {
    double initialCapital;    /* Initial capital for backtest */
    double positionSize;      /* Position size for trades */
    int allowShort;           /* Whether to allow short positions */
    double entryThreshold;    /* Minimum signal strength to enter */
    void (*signalFunction)(const StockData*, int, TradingSignal*, double*); /* Signal generation function */
} TradingStrategy;

/* Trade record structure */
typedef struct {
    int entryIndex;           /* Index of entry point */
    int exitIndex;            /* Index of exit point */
    double entryPrice;        /* Entry price */
    double exitPrice;         /* Exit price */
    double profit;            /* Profit/loss from the trade */
    int type;                 /* 1 for long, -1 for short */
} Trade;

/* Backtest result structure */
typedef struct {
    double initialCapital;    /* Starting capital */
    double finalCapital;      /* Ending capital */
    int totalTrades;          /* Total number of trades */
    int profitableTrades;     /* Number of profitable trades */
    double maxDrawdown;       /* Maximum drawdown percentage */
    double peakCapital;       /* Highest capital during test */
    double sharpeRatio;       /* Sharpe ratio (risk-adjusted return) */
    double annualizedReturn;  /* Annualized return percentage */
    Trade* tradesArray;       /* Array of all trades */
    int tradeCount;           /* Number of trades in the array */
} BacktestResult;

/* Performance metrics for a backtest */
typedef struct {
    double totalReturn;       /* Total return percentage */
    double annualizedReturn;  /* Annualized return percentage */
    double sharpeRatio;       /* Sharpe ratio */
    double winRate;           /* Win rate (percentage of profitable trades) */
    double averageProfit;     /* Average profit per trade */
    double averageLoss;       /* Average loss per trade */
    double profitFactor;      /* Profit factor (gross profit / gross loss) */
    double maxDrawdown;       /* Maximum drawdown percentage */
    double recoveryFactor;    /* Recovery factor (return / max drawdown) */
    double avgTradeReturn;    /* Average return per trade */
    double calmarRatio;       /* Calmar ratio */
} BacktestPerformance;

/* Model configuration structure */
typedef struct {
    char modelType[32];       /* Model type (e.g., "SMA", "MACD") */
    int shortPeriod;          /* Short period for indicators */
    int longPeriod;           /* Long period for indicators */
    double threshold;         /* Signal threshold */
    int useMachineLearning;   /* Whether to use ML enhancement */
    double trainTestSplit;    /* Train/test split ratio (0.0-1.0) */
    double initialCapital;    /* Initial capital for backtesting */
    double positionSize;      /* Position size for trading */
    int allowShort;           /* Whether to allow short positions */
    double signalThreshold;   /* Signal threshold for entry */
    int startIndex;           /* Start index for backtesting */
    int endIndex;             /* End index for backtesting */
} ModelConfig;

/* Model evaluation structure */
typedef struct {
    ModelConfig config;       /* Model configuration */
    BacktestResult* backtest; /* Backtest results */
    BacktestPerformance performance; /* Performance metrics */
    ValidationMetrics metrics; /* Validation metrics */
    BacktestResult* trainResult; /* Training period results */
    BacktestResult* testResult;  /* Testing period results */
    BacktestPerformance trainPerformance; /* Training performance metrics */
    BacktestPerformance testPerformance;  /* Testing performance metrics */
} ModelEvaluation;

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

/* Backtesting a trading strategy */
BacktestResult* backtestStrategy(const StockData* data, int dataSize, TradingStrategy* strategy,
                                int startIndex, int endIndex);

/* Calculate performance metrics */
void calculatePerformanceMetrics(BacktestResult* result, BacktestPerformance* performance);

/* Free backtest result */
void freeBacktestResult(BacktestResult* result);

/* Build and evaluate a model */
ModelEvaluation* buildAndEvaluateModel(const StockData* data, int dataSize, ModelConfig* config);

/* Free model evaluation */
void freeModelEvaluation(ModelEvaluation* evaluation);

/* Optimize a trading strategy */
TradingStrategy* optimizeStrategy(const StockData* data, int dataSize, int startIndex, int endIndex,
                                 void (*signalFunction)(const StockData*, int, TradingSignal*, double*));

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