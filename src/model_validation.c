/**
 * Model Validation Module
 * Implementation of backtesting and model validation functions
 * SIMPLIFIED VERSION
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include "../include/emers.h"
#include "../include/model_validation.h"
#include "../include/technical_analysis.h"
/* Include data_mining.h without using its TradingSignal type */
#define TradingSignal TradingSignal_DataMining
#include "../include/data_mining.h"
#undef TradingSignal
#include "../include/error_handling.h"

/* Use a local signal struct to avoid type conflicts */
typedef struct {
    int type;
    double strength;
    double price;
    double stopLoss;
    double takeProfit;
} SignalType;

/**
 * Simplified backtest implementation
 * Focuses on essential performance metrics with reduced complexity
 */
BacktestResult* backtestStrategy(const StockData* data, int dataSize, TradingStrategy* strategy, 
                                int startIndex, int endIndex) {
    if (!data || !strategy || dataSize <= 0 || startIndex < 0 || 
        endIndex >= dataSize || startIndex >= endIndex) {
        return NULL;
    }
    
    BacktestResult* result = (BacktestResult*)malloc(sizeof(BacktestResult));
    if (!result) {
        return NULL;
    }
    
    /* Initialize result structure */
    result->initialCapital = strategy->initialCapital;
    result->finalCapital = strategy->initialCapital;
    result->totalTrades = 0;
    result->profitableTrades = 0;
    result->maxDrawdown = 0.0;
    result->peakCapital = strategy->initialCapital;
    result->sharpeRatio = 0.0;
    result->tradesArray = NULL;
    result->tradeCount = 0;
    result->annualizedReturn = 0.0;
    
    double currentCapital = strategy->initialCapital;
    int position = 0;  /* 0 = no position, 1 = long, -1 = short */
    double entryPrice = 0.0;
    int entryIndex = 0;
    double highestCapital = currentCapital;
    
    /* Arrays for calculating Sharpe ratio */
    double* dailyReturns = (double*)malloc((endIndex - startIndex + 1) * sizeof(double));
    if (!dailyReturns) {
        free(result);
        return NULL;
    }
    
    /* Initialize daily returns to zero (no change when not in market) */
    for (int i = 0; i <= endIndex - startIndex; i++) {
        dailyReturns[i] = 0.0;
    }
    
    /* Allocate memory for trades */
    int maxTrades = (endIndex - startIndex) / 2;  /* Estimate, but could be less */
    result->tradesArray = (Trade*)malloc(maxTrades * sizeof(Trade));
    
    if (!result->tradesArray) {
        free(dailyReturns);
        free(result);
        return NULL;
    }
    
    /* Run the backtest */
    for (int i = startIndex; i <= endIndex; i++) {
        SignalType signal;
        double signalStrength = 0.0;
        
        /* Get signal from strategy function */
        if (strategy->signalFunction) {
            /* The strategy function uses TradingSignal from model_validation.h */
            /* We need to be careful with the type conversion - use void* to bypass strict type checking */
            TradingSignal modelSignal;
            memset(&modelSignal, 0, sizeof(modelSignal));
            
            void (*signalFunc)(const StockData*, int, void*, double*) = 
                (void (*)(const StockData*, int, void*, double*))strategy->signalFunction;
            signalFunc(data, i, &modelSignal, &signalStrength);
            
            /* Copy values from model's signal type to our local type */
            signal.type = modelSignal.type;
            signal.strength = modelSignal.strength;
            signal.price = modelSignal.price;
            signal.stopLoss = modelSignal.stopLoss;
            signal.takeProfit = modelSignal.takeProfit;
        } else {
            /* Default to simple moving average crossover */
            if (i > startIndex + 20) {
                double shortSMA = 0.0, longSMA = 0.0;
                double prevShortSMA = 0.0, prevLongSMA = 0.0;
                
                /* Calculate short SMA (10-day) */
                for (int j = i - 9; j <= i; j++) {
                    shortSMA += data[j].close;
                }
                shortSMA /= 10.0;
                
                /* Calculate previous short SMA */
                for (int j = i - 10; j <= i - 1; j++) {
                    prevShortSMA += data[j].close;
                }
                prevShortSMA /= 10.0;
                
                /* Calculate long SMA (30-day) */
                for (int j = i - 29; j <= i; j++) {
                    longSMA += data[j].close;
                }
                longSMA /= 30.0;
                
                /* Calculate previous long SMA */
                for (int j = i - 30; j <= i - 1; j++) {
                    prevLongSMA += data[j].close;
                }
                prevLongSMA /= 30.0;
                
                /* Detect crossovers */
                if (prevShortSMA <= prevLongSMA && shortSMA > longSMA) {
                    signal.type = SIGNAL_BUY;
                    signalStrength = 1.0;
                } else if (prevShortSMA >= prevLongSMA && shortSMA < longSMA) {
                    signal.type = SIGNAL_SELL;
                    signalStrength = 1.0;
                } else {
                    signal.type = SIGNAL_NONE;
                    signalStrength = 0.0;
                }
            } else {
                signal.type = SIGNAL_NONE;
                signalStrength = 0.0;
            }
        }
        
        /* Process the signal */
        if (signal.type == SIGNAL_BUY && position <= 0 && signalStrength >= strategy->entryThreshold) {
            /* Enter long position */
            if (position == -1) {
                /* Close short position first */
                double profit = entryPrice - data[i].close;
                currentCapital += profit * position * -1.0 * strategy->positionSize;
                
                /* Record the trade */
                if (result->tradeCount < maxTrades) {
                    Trade* trade = &result->tradesArray[result->tradeCount];
                    trade->entryIndex = entryIndex;
                    trade->exitIndex = i;
                    trade->entryPrice = entryPrice;
                    trade->exitPrice = data[i].close;
                    trade->profit = profit * position * -1.0 * strategy->positionSize;
                    trade->type = -1;  /* Short */
                    result->tradeCount++;
                }
                
                result->totalTrades++;
                if (profit > 0) {
                    result->profitableTrades++;
                }
            }
            
            /* Enter new long position */
            position = 1;
            entryPrice = data[i].close;
            entryIndex = i;
        } 
        else if (signal.type == SIGNAL_SELL && position >= 0 && signalStrength >= strategy->entryThreshold) {
            /* Enter short position */
            if (position == 1) {
                /* Close long position first */
                double profit = data[i].close - entryPrice;
                currentCapital += profit * position * strategy->positionSize;
                
                /* Record the trade */
                if (result->tradeCount < maxTrades) {
                    Trade* trade = &result->tradesArray[result->tradeCount];
                    trade->entryIndex = entryIndex;
                    trade->exitIndex = i;
                    trade->entryPrice = entryPrice;
                    trade->exitPrice = data[i].close;
                    trade->profit = profit * position * strategy->positionSize;
                    trade->type = 1;  /* Long */
                    result->tradeCount++;
                }
                
                result->totalTrades++;
                if (profit > 0) {
                    result->profitableTrades++;
                }
            }
            
            /* Enter new short position if allowed */
            if (strategy->allowShort) {
                position = -1;
                entryPrice = data[i].close;
                entryIndex = i;
            } else {
                position = 0;
            }
        }
        
        /* Update daily returns based on position */
        if (i > startIndex) {
            double dailyChange = (data[i].close - data[i-1].close) / data[i-1].close;
            if (position != 0) {
                dailyReturns[i - startIndex] = dailyChange * position;
            }
        }
        
        /* Track peak capital and drawdown */
        if (currentCapital > highestCapital) {
            highestCapital = currentCapital;
        }
        
        double currentDrawdown = (highestCapital - currentCapital) / highestCapital;
        if (currentDrawdown > result->maxDrawdown) {
            result->maxDrawdown = currentDrawdown;
        }
    }
    
    /* Close any open position */
    if (position != 0) {
        double profit = 0.0;
        if (position > 0) {
            profit = data[endIndex].close - entryPrice;
        } else {
            profit = entryPrice - data[endIndex].close;
        }
        
        currentCapital += profit * abs(position) * strategy->positionSize;
        
        /* Record the final trade */
        if (result->tradeCount < maxTrades) {
            Trade* trade = &result->tradesArray[result->tradeCount];
            trade->entryIndex = entryIndex;
            trade->exitIndex = endIndex;
            trade->entryPrice = entryPrice;
            trade->exitPrice = data[endIndex].close;
            trade->profit = profit * abs(position) * strategy->positionSize;
            trade->type = (position > 0) ? 1 : -1;
            result->tradeCount++;
        }
        
        result->totalTrades++;
        if (profit > 0) {
            result->profitableTrades++;
        }
    }
    
    /* Update final capital */
    result->finalCapital = currentCapital;
    
    /* Calculate Sharpe ratio (simplified) */
    double totalReturn = 0.0;
    double sumSquaredReturns = 0.0;
    int daysInMarket = 0;
    
    for (int i = 0; i <= endIndex - startIndex; i++) {
        if (dailyReturns[i] != 0.0) {
            totalReturn += dailyReturns[i];
            sumSquaredReturns += dailyReturns[i] * dailyReturns[i];
            daysInMarket++;
        }
    }
    
    if (daysInMarket > 0) {
        double avgReturn = totalReturn / daysInMarket;
        double stdDev = sqrt((sumSquaredReturns / daysInMarket) - (avgReturn * avgReturn));
        
        if (stdDev > 0) {
            /* Simplified Sharpe calculation */
            result->sharpeRatio = (avgReturn / stdDev) * sqrt(252.0);  /* Annualize */
        }
    }
    
    /* Calculate annualized return */
    double totalDays = endIndex - startIndex + 1;
    double years = totalDays / 252.0;  /* Approximate trading days in a year */
    
    if (years > 0) {
        double totalReturnPct = (result->finalCapital / result->initialCapital) - 1.0;
        result->annualizedReturn = pow(1.0 + totalReturnPct, 1.0 / years) - 1.0;
    }
    
    free(dailyReturns);
    
    return result;
}

/**
 * Calculate key performance metrics for a backtest
 * Simplified version focusing only on essential metrics
 */
void calculatePerformanceMetrics(BacktestResult* result, BacktestPerformance* performance) {
    if (!result || !performance) {
        return;
    }
    
    /* Calculate win rate */
    performance->winRate = (result->totalTrades > 0) ? 
                          ((double)result->profitableTrades / result->totalTrades) : 0.0;
    
    /* Total return */
    performance->totalReturn = (result->finalCapital - result->initialCapital) / result->initialCapital;
    
    /* Copy other metrics directly from the result */
    performance->maxDrawdown = result->maxDrawdown;
    performance->sharpeRatio = result->sharpeRatio;
    performance->annualizedReturn = result->annualizedReturn;
    
    /* Calculate profit factor if we have trades */
    double grossProfit = 0.0;
    double grossLoss = 0.0;
    
    for (int i = 0; i < result->tradeCount; i++) {
        if (result->tradesArray[i].profit > 0) {
            grossProfit += result->tradesArray[i].profit;
        } else {
            grossLoss += fabs(result->tradesArray[i].profit);
        }
    }
    
    performance->profitFactor = (grossLoss > 0) ? (grossProfit / grossLoss) : 
                               (grossProfit > 0 ? 999.0 : 0.0);
    
    /* Calculate average profit per trade */
    performance->avgTradeReturn = (result->totalTrades > 0) ? 
                                 (performance->totalReturn / result->totalTrades) : 0.0;
    
    /* Simplified Calmar ratio (annualized return / max drawdown) */
    performance->calmarRatio = (performance->maxDrawdown > 0) ? 
                              (performance->annualizedReturn / performance->maxDrawdown) : 0.0;
}

/**
 * Free memory allocated for backtest result
 */
void freeBacktestResult(BacktestResult* result) {
    if (result) {
        if (result->tradesArray) {
            free(result->tradesArray);
        }
        free(result);
    }
}

/**
 * Build and evaluate a simple trading model
 * This is a simplified version that focuses on essential functionality
 */
ModelEvaluation* buildAndEvaluateModel(const StockData* data, int dataSize, ModelConfig* config) {
    if (!data || dataSize < 100 || !config) {
        return NULL;
    }
    
    ModelEvaluation* evaluation = (ModelEvaluation*)malloc(sizeof(ModelEvaluation));
    if (!evaluation) {
        return NULL;
    }
    
    /* Initialize evaluation structure */
    evaluation->trainResult = NULL;
    evaluation->testResult = NULL;
    evaluation->trainPerformance.winRate = 0.0;
    evaluation->trainPerformance.totalReturn = 0.0;
    evaluation->trainPerformance.maxDrawdown = 0.0;
    evaluation->trainPerformance.sharpeRatio = 0.0;
    evaluation->trainPerformance.profitFactor = 0.0;
    evaluation->trainPerformance.avgTradeReturn = 0.0;
    evaluation->trainPerformance.calmarRatio = 0.0;
    evaluation->trainPerformance.annualizedReturn = 0.0;
    evaluation->testPerformance.winRate = 0.0;
    evaluation->testPerformance.totalReturn = 0.0;
    evaluation->testPerformance.maxDrawdown = 0.0;
    evaluation->testPerformance.sharpeRatio = 0.0;
    evaluation->testPerformance.profitFactor = 0.0;
    evaluation->testPerformance.avgTradeReturn = 0.0;
    evaluation->testPerformance.calmarRatio = 0.0;
    evaluation->testPerformance.annualizedReturn = 0.0;
    
    /* Calculate split indices */
    int trainEnd = dataSize * config->trainTestSplit;
    int testStart = trainEnd + 1;
    
    if (testStart >= dataSize) {
        testStart = dataSize - 20;  /* Ensure at least some test data */
        trainEnd = testStart - 1;
    }
    
    /* Create trading strategy */
    TradingStrategy strategy;
    strategy.initialCapital = config->initialCapital;
    strategy.positionSize = config->positionSize;
    strategy.allowShort = config->allowShort;
    strategy.entryThreshold = config->signalThreshold;
    strategy.signalFunction = NULL;  /* Use default SMA crossover */
    
    /* Run backtest on training data */
    evaluation->trainResult = backtestStrategy(data, dataSize, &strategy, config->startIndex, trainEnd);
    
    if (evaluation->trainResult) {
        calculatePerformanceMetrics(evaluation->trainResult, &evaluation->trainPerformance);
    }
    
    /* Run backtest on test data */
    evaluation->testResult = backtestStrategy(data, dataSize, &strategy, testStart, config->endIndex);
    
    if (evaluation->testResult) {
        calculatePerformanceMetrics(evaluation->testResult, &evaluation->testPerformance);
    }
    
    return evaluation;
}

/**
 * Free memory allocated for model evaluation
 */
void freeModelEvaluation(ModelEvaluation* evaluation) {
    if (evaluation) {
        if (evaluation->trainResult) {
            freeBacktestResult(evaluation->trainResult);
        }
        if (evaluation->testResult) {
            freeBacktestResult(evaluation->testResult);
        }
        free(evaluation);
    }
}

/**
 * Optimize strategy parameters using grid search
 * Simplified version that just returns a default strategy
 */
TradingStrategy* optimizeStrategy(const StockData* data, int dataSize, int startIndex, int endIndex, 
                                 void (*signalFunction)(const StockData*, int, TradingSignal*, double*)) {
    if (!data || dataSize <= 0 || startIndex < 0 || endIndex >= dataSize || startIndex >= endIndex) {
        return NULL;
    }
    
    TradingStrategy* bestStrategy = (TradingStrategy*)malloc(sizeof(TradingStrategy));
    if (!bestStrategy) {
        return NULL;
    }
    
    /* Default strategy parameters */
    bestStrategy->initialCapital = 100000.0;
    bestStrategy->positionSize = 10000.0;
    bestStrategy->allowShort = 0;
    bestStrategy->entryThreshold = 0.5;
    /* Bypass type checking by using a void pointer cast */
    bestStrategy->signalFunction = NULL;
    
    /* For simplicity, we're not implementing optimization in this version */
    
    return bestStrategy;
}