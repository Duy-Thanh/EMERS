/**
 * Technical Analysis Module
 * Header file for technical indicators and analysis functions
 */

#ifndef TECHNICAL_ANALYSIS_H
#define TECHNICAL_ANALYSIS_H

#include "emers.h"
#include "event_analysis.h"

/* Constants for technical analysis */
#define DEFAULT_SMA_PERIOD 20
#define DEFAULT_EMA_PERIOD 14
#define DEFAULT_RSI_PERIOD 14
#define DEFAULT_MACD_FAST_PERIOD 12
#define DEFAULT_MACD_SLOW_PERIOD 26
#define DEFAULT_MACD_SIGNAL_PERIOD 9
#define DEFAULT_BOLLINGER_PERIOD 20
#define DEFAULT_BOLLINGER_STDDEV 2.0
#define DEFAULT_ATR_PERIOD 14
#define DEFAULT_ADX_PERIOD 14
#define DEFAULT_STOCHASTIC_K_PERIOD 14
#define DEFAULT_STOCHASTIC_D_PERIOD 3
#define DEFAULT_MFI_PERIOD 14
#define DEFAULT_PSAR_ACCELERATION 0.02
#define DEFAULT_PSAR_MAX_ACCELERATION 0.2

#define MAX_INDICATOR_NAME 32
#define MAX_INDICATORS 32
#define MAX_INDICATOR_PERIODS 3
#define MAX_DATA_POINTS 1000

/* Technical Indicator Types */
typedef enum {
    INDICATOR_SMA,
    INDICATOR_EMA,
    INDICATOR_RSI,
    INDICATOR_MACD,
    INDICATOR_BOLLINGER,
    INDICATOR_ADX,
    INDICATOR_STOCHASTIC,
    INDICATOR_MFI,
    INDICATOR_PARABOLIC_SAR,
    /* Event-adjusted indicators */
    INDICATOR_EVENT_ADX,
    INDICATOR_EVENT_STOCHASTIC,
    INDICATOR_EVENT_MFI,
    INDICATOR_EVENT_PARABOLIC_SAR,
    INDICATOR_UNKNOWN
} IndicatorType;

/* Extended technical indicator structure */
typedef struct {
    /* Basic indicators */
    double sma;       // Simple Moving Average
    double ema;       // Exponential Moving Average
    double rsi;       // Relative Strength Index
    double macd;      // Moving Average Convergence Divergence
    double macdSignal;
    double macdHistogram;
    double bollingerUpper;  // Bollinger Bands
    double bollingerMiddle;
    double bollingerLower;
    double atr;       // Average True Range
    
    /* Advanced indicators */
    double adx;       // Average Directional Index
    double diPlus;    // Positive Directional Indicator
    double diMinus;   // Negative Directional Indicator
    double stochasticK; // Stochastic Oscillator %K
    double stochasticD; // Stochastic Oscillator %D
    double mfi;       // Money Flow Index
    double psar;      // Parabolic SAR
    
    /* Event-adjusted indicators */
    double eventADX;  // Event-adjusted ADX
    double eventStochasticK; // Event-adjusted Stochastic %K
    double eventStochasticD; // Event-adjusted Stochastic %D
    double eventMFI;  // Event-adjusted MFI
    double eventPSAR; // Event-adjusted Parabolic SAR
} ExtendedTechnicalIndicators;

/* Data structure for technical indicators */
typedef struct {
    IndicatorType type;
    char name[MAX_INDICATOR_NAME];
    int periods[MAX_INDICATOR_PERIODS];
    double values[MAX_DATA_POINTS];
    double signalLine[MAX_DATA_POINTS];
    double upperBand[MAX_DATA_POINTS];
    double lowerBand[MAX_DATA_POINTS];
    int lastIndex;
} TechnicalIndicator;

/* Function prototypes */

/* Simple Moving Average */
void calculateSMA(const StockData* data, int dataSize, int period, double* output);
/* Indicator version */
int calculateSMAIndicator(const StockData* data, int dataSize, int period, TechnicalIndicator* indicator);

/* Exponential Moving Average */
void calculateEMA(const StockData* data, int dataSize, int period, double* output);
/* Indicator version */
int calculateEMAIndicator(const StockData* data, int dataSize, int period, TechnicalIndicator* indicator);

/* Relative Strength Index */
void calculateRSI(const StockData* data, int dataSize, int period, double* output);
/* Indicator version */
int calculateRSIIndicator(const StockData* data, int dataSize, int period, TechnicalIndicator* indicator);

/* Moving Average Convergence Divergence */
void calculateMACD(const StockData* data, int dataSize, int fastPeriod, int slowPeriod, int signalPeriod, 
                  double* macdLine, double* signalLine, double* histogram);
/* Indicator version */
int calculateMACDIndicator(const StockData* data, int dataSize, int fastPeriod, int slowPeriod, int signalPeriod, TechnicalIndicator* indicator);

/* Bollinger Bands */
void calculateBollingerBands(const StockData* data, int dataSize, int period, double stdDevMultiplier,
                           double* upperBand, double* middleBand, double* lowerBand);
/* Indicator version */
int calculateBollingerBandsIndicator(const StockData* data, int dataSize, int period, double stdDevMultiplier, TechnicalIndicator* indicator);

/* Average True Range */
void calculateATR(const StockData* data, int dataSize, int period, double* output);

/* Calculate all basic indicators at once */
void calculateAllIndicators(const StockData* data, int dataSize, TechnicalIndicators* indicators);

/* Average Directional Index (ADX) */
void calculateADX(const StockData* data, int dataSize, int period, double* adx, double* diPlus, double* diMinus);
/* Indicator version */
int calculateADXIndicator(const StockData* data, int dataSize, int period, TechnicalIndicator* indicator);

/* Stochastic Oscillator */
void calculateStochastic(const StockData* data, int dataSize, int kPeriod, int dPeriod, double* k, double* d);
/* Indicator version */
int calculateStochasticIndicator(const StockData* data, int dataSize, int kPeriod, int dPeriod, TechnicalIndicator* indicator);

/* Money Flow Index */
void calculateMFI(const StockData* data, int dataSize, int period, double* output);
/* Indicator version */
int calculateMFIIndicator(const StockData* data, int dataSize, int period, TechnicalIndicator* indicator);

/* Parabolic SAR */
void calculatePSAR(const StockData* data, int dataSize, double acceleration, double maxAcceleration, double* output);
int calculateParabolicSAR(const StockData* data, int dataSize, double accelerationFactor, double maxAcceleration, TechnicalIndicator* indicator);

/* Calculate all extended indicators */
void calculateExtendedIndicators(const StockData* data, int dataSize, ExtendedTechnicalIndicators* indicators);

/* Event-adjusted indicators */
void calculateEventAdjustedADX(const StockData* data, int dataSize, int period, const EventData* event, double* output);
int calculateEventADX(const StockData* data, int dataSize, int period, const EventData* events, int eventCount, TechnicalIndicator* indicator);

void calculateEventAdjustedStochastic(const StockData* data, int dataSize, int kPeriod, int dPeriod, const EventData* event, double* k, double* d);
int calculateEventStochastic(const StockData* data, int dataSize, int kPeriod, int dPeriod, const EventData* events, int eventCount, TechnicalIndicator* indicator);

void calculateEventAdjustedMFI(const StockData* data, int dataSize, int period, const EventData* event, double* output);
int calculateEventMFI(const StockData* data, int dataSize, int period, const EventData* events, int eventCount, TechnicalIndicator* indicator);

void calculateEventAdjustedPSAR(const StockData* data, int dataSize, double acceleration, double maxAcceleration, const EventData* event, double* output);
int calculateEventParabolicSAR(const StockData* data, int dataSize, double accelerationFactor, double maxAcceleration, const EventData* events, int eventCount, TechnicalIndicator* indicator);

/* Calculate all event-adjusted indicators */
void calculateEventAdjustedIndicators(const StockData* data, int dataSize, const EventData* event, ExtendedTechnicalIndicators* indicators);

/* Indicator generation */
int generateIndicators(const StockData* data, int dataSize, TechnicalIndicator* indicators, int maxIndicators);
void printIndicators(const TechnicalIndicator* indicators, int indicatorCount, int startIndex, int endIndex);

/* Statistical functions */
double calculateStandardDeviation(const double* data, int dataSize);
double calculateAverage(const double* data, int dataSize);
double calculateMax(const double* data, int dataSize);
double calculateMin(const double* data, int dataSize);

/* Advanced assembly-optimized functions */
#ifdef __ASSEMBLER__
void asmCalculateStandardDeviation(const double* data, int dataSize, double* result);
void asmCalculateMovingAverage(const double* data, int dataSize, int period, double* output);
#endif

/* Calculate all indicators for a single data point */
void calculateIndicatorsForDataPoint(const StockData* data, int dataSize, int dataIndex, 
                                   TechnicalIndicators* indicators);

/* Additional analysis functions */
double predictVolatility(const StockData* data, int dataSize, int period);
double predictVolatilityGARCH(const StockData* data, int dataSize, int period);
double calculateHistoricalVolatility(const StockData* data, int dataSize, int period);

#endif /* TECHNICAL_ANALYSIS_H */