/**
 * Emergency Market Event Response System (EMERS)
 * Main application file
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../include/emers.h"
#include "../include/tiingo_api.h"
#include "../include/technical_analysis.h"
#include "../include/event_analysis.h"
#include "../include/error_handling.h"
#include "../include/event_database.h"
#include "../include/data_mining.h"

#define MAX_DETECTED_EVENTS 20
#define DEFAULT_LOOKBACK_DAYS 3650  // 10 years of historical data (approximately)

/* Function prototypes */
void printUsage(const char* programName);
void printStock(const Stock* stock);
void printEvent(const EventData* event);
void printDetailedEvent(const DetailedEventData* event);
void printTechnicalIndicators(const TechnicalIndicators* indicators);
void printExtendedTechnicalIndicators(const ExtendedTechnicalIndicators* indicators);
void analyzeStock(const Stock* stock, const EventDatabase* newsEvents);

int main(int argc, char* argv[]) {
    char apiKey[MAX_API_KEY_LENGTH] = "";
    char symbols[MAX_STOCKS][MAX_SYMBOL_LENGTH];
    int symbolCount = 0;
    char startDate[MAX_DATE_LENGTH] = "";
    char endDate[MAX_DATE_LENGTH] = "";
    int i;

    /* Initialize error handling */
    initErrorHandling("emers_log.txt", LOG_DEBUG, LOG_INFO);
    
    /* Initialize the event database */
    initEventDatabase();
    
    printf("Emergency Market Event Response System (EMERS)\n");
    
    /* Parse command line arguments */
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-k") == 0 || strcmp(argv[i], "--api-key") == 0) {
            if (i + 1 < argc) {
                strncpy(apiKey, argv[i + 1], MAX_API_KEY_LENGTH - 1);
                apiKey[MAX_API_KEY_LENGTH - 1] = '\0';
                i++;
            }
        } else if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--symbols") == 0) {
            if (i + 1 < argc) {
                char* token = strtok(argv[i + 1], ",");
                while (token != NULL && symbolCount < MAX_STOCKS) {
                    strncpy(symbols[symbolCount], token, MAX_SYMBOL_LENGTH - 1);
                    symbols[symbolCount][MAX_SYMBOL_LENGTH - 1] = '\0';
                    symbolCount++;
                    token = strtok(NULL, ",");
                }
                i++;
            }
        } else if (strcmp(argv[i], "--start-date") == 0) {
            if (i + 1 < argc) {
                strncpy(startDate, argv[i + 1], MAX_DATE_LENGTH - 1);
                startDate[MAX_DATE_LENGTH - 1] = '\0';
                i++;
            }
        } else if (strcmp(argv[i], "--end-date") == 0) {
            if (i + 1 < argc) {
                strncpy(endDate, argv[i + 1], MAX_DATE_LENGTH - 1);
                endDate[MAX_DATE_LENGTH - 1] = '\0';
                i++;
            }
        } else {
            printf("Unknown option: %s\n", argv[i]);
            printUsage(argv[0]);
            return 1;
        }
    }
    
    /* Validate required parameters */
    if (strlen(apiKey) == 0 || symbolCount == 0) {
        printf("Error: API key and at least one symbol are required.\n");
        printUsage(argv[0]);
        return 1;
    }
    
    /* Use default dates if not provided */
    if (strlen(endDate) == 0) {
        char buffer[MAX_DATE_LENGTH];
        getCurrentDate(buffer);
        strncpy(endDate, buffer, MAX_DATE_LENGTH - 1);
        endDate[MAX_DATE_LENGTH - 1] = '\0';
    }
    
    if (strlen(startDate) == 0) {
        char buffer[MAX_DATE_LENGTH];
        getPastDate(buffer, DEFAULT_LOOKBACK_DAYS);  /* Default: last 10 years */
        strncpy(startDate, buffer, MAX_DATE_LENGTH - 1);
        startDate[MAX_DATE_LENGTH - 1] = '\0';
    }
    
    /* Initialize API */
    if (!initializeTiingoAPI(apiKey)) {
        return 1;
    }
    
    /* Initialize stocks and events */
    Stock stocks[MAX_STOCKS];
    EventDatabase newsEvents;
    initializeEventDatabase(&newsEvents);
    
    /* Create a comma-separated list of symbols for API request */
    char symbolList[MAX_STOCKS * (MAX_SYMBOL_LENGTH + 1)];
    symbolList[0] = '\0';
    
    for (i = 0; i < symbolCount; i++) {
        if (i > 0) {
            strcat(symbolList, ",");
        }
        strcat(symbolList, symbols[i]);
    }
    
    printf("Fetching news data for symbols: %s\n", symbolList);
    
    /* Fetch news feed */
    if (!fetchNewsFeed(symbolList, &newsEvents)) {
        printf("Error: Failed to fetch news data.\n");
        return 1;
    }
    
    printf("Retrieved %d news events.\n", newsEvents.eventCount);
    
    /* Process each stock */
    for (i = 0; i < symbolCount; i++) {
        printf("\nAnalyzing stock: %s\n", symbols[i]);
        
        /* Initialize stock structure */
        initializeStock(&stocks[i], symbols[i]);
        
        /* Fetch historical data */
        if (!fetchHistoricalDataWithCache(symbols[i], startDate, endDate, &stocks[i])) {
            printf("Error: Failed to fetch data for %s.\n", symbols[i]);
            continue;
        }
        
        printf("Retrieved %d data points for %s.\n", stocks[i].dataSize, symbols[i]);
        
        /* Analyze the stock */
        analyzeStock(&stocks[i], &newsEvents);
    }
    
    /* Clean up */
    for (i = 0; i < symbolCount; i++) {
        freeStock(&stocks[i]);
    }
    
    if (newsEvents.events) {
        free(newsEvents.events);
    }
    
    /* Save events to database before exit */
    saveEventsToDatabase(&newsEvents);
    
    /* Cleanup the event database */
    cleanupEventDatabase();
    
    /* Cleanup error handling */
    cleanupErrorHandling();
    
    return 0;
}

void analyzeStock(const Stock* stock, const EventDatabase* newsEvents) {
    if (!stock || !newsEvents) {
        return;
    }
    
    printf("\nAnalyzing stock: %s\n", stock->symbol);
    printf("Retrieved %d data points for %s.\n", stock->dataSize, stock->symbol);
    
    /* Calculate technical indicators */
    ExtendedTechnicalIndicators* indicators = (ExtendedTechnicalIndicators*)malloc(sizeof(ExtendedTechnicalIndicators));
    if (!indicators) {
        printf("Error: Memory allocation failed for technical indicators\n");
        return;
    }
    
    /* Most recent date in the stock data */
    const char* latestDate = stock->data[stock->dataSize - 1].date;
    
    /* Calculate all extended technical indicators */
    calculateExtendedIndicators(stock->data, stock->dataSize, indicators);
    
    printf("\nTechnical indicators for %s (%s):\n", 
           stock->symbol, latestDate);
    printExtendedTechnicalIndicators(indicators);
    
    /* Detect price patterns */
    printf("\n=== PATTERN RECOGNITION ANALYSIS ===\n");
    PatternResult patterns[5];
    int patternCount = detectPricePatterns(stock->data, stock->dataSize, patterns, 5);
    if (patternCount > 0) {
        printf("Detected %d price patterns:\n\n", patternCount);
        for (int i = 0; i < patternCount; i++) {
            printf("Pattern #%d: %s\n", i + 1, patterns[i].description);
            printf("  Confidence: %.1f%%\n", patterns[i].confidence * 100.0);
            printf("  Expected price move: %.2f%%\n", patterns[i].expectedMove * 100.0);
            printf("  Pattern spans from index %d to %d\n\n", 
                   patterns[i].startIndex, patterns[i].endIndex);
        }
    } else {
        printf("No significant price patterns detected.\n\n");
    }
    
    /* Volatility prediction */
    printf("=== VOLATILITY PREDICTION ===\n");
    int horizons[] = {5, 10, 30};
    for (int i = 0; i < 3; i++) {
        double predictedVol = predictVolatility(stock->data, stock->dataSize, horizons[i]);
        printf("%d-day predicted volatility: %.2f%%\n", horizons[i], predictedVol * 100.0);
    }
    
    /* Alternative GARCH model for longer horizon */
    double garchVol = predictVolatilityGARCH(stock->data, stock->dataSize, 30);
    printf("30-day GARCH volatility: %.2f%%\n\n", garchVol * 100.0);
    
    /* Anomaly detection */
    printf("=== ANOMALY DETECTION ===\n");
    double anomalyScore = calculateAnomalyScore(stock->data, stock->dataSize);
    printf("Current anomaly score: %.2f", anomalyScore);
    if (anomalyScore > 3.0) {
        printf(" (SIGNIFICANT ANOMALY DETECTED)\n");
    } else if (anomalyScore > 2.0) {
        printf(" (Moderate anomaly)\n");
    } else {
        printf(" (Normal range)\n");
    }
    
    int anomalyIndices[10];
    int anomalyCount = detectAnomalies(stock->data, stock->dataSize, anomalyIndices, 10);
    if (anomalyCount > 0) {
        printf("Detected %d historical anomalies on dates:\n", anomalyCount);
        for (int i = 0; i < anomalyCount; i++) {
            int idx = anomalyIndices[i];
            if (idx >= 0 && idx < stock->dataSize) {
                printf("  %s: %.2f%% price change\n", 
                       stock->data[idx].date, 
                       (stock->data[idx].close - stock->data[idx-1].close) / stock->data[idx-1].close * 100.0);
            }
        }
        printf("\n");
    } else {
        printf("No significant historical anomalies detected.\n\n");
    }
    
    /* Detect market events */
    EventData detectedEvents[MAX_EVENTS];
    int numEvents = detectMarketEvents(stock, 1, newsEvents, detectedEvents);
    
    printf("\n=== EVENT ANALYSIS ===\n");
    printf("Detected %d significant market events:\n\n", numEvents);
    
    /* Create dummy sector indices for sector analysis */
    Stock dummySectors[10]; /* Technology, Financial, Healthcare, etc. */
    Stock* sectorPointers[10];
    
    /* Initialize dummy sectors */
    for (int i = 0; i < 10; i++) {
        char sectorSymbol[16];
        sprintf(sectorSymbol, "SECTOR%d", i + 1);
        initializeStock(&dummySectors[i], sectorSymbol);
        sectorPointers[i] = &dummySectors[i];
    }
    
    /* Process each event */
    for (int i = 0; i < numEvents; i++) {
        printf("Event #%d:\n", i + 1);
        
        /* Print basic event info */
        printf("Date: %s\n", detectedEvents[i].date);
        printf("Title: %s\n", detectedEvents[i].title);
        printf("Description: %s\n", detectedEvents[i].description);
        printf("\n");
        
        /* Create a detailed event analysis */
        DetailedEventData detailedEvent;
        memset(&detailedEvent, 0, sizeof(DetailedEventData));
        
        /* Copy basic data */
        detailedEvent.basicData = detectedEvents[i];
        
        /* Analyze the event */
        detailedEvent.type = classifyEvent(&detectedEvents[i]);
        detailedEvent.severity = assessEventSeverity(&detectedEvents[i], stock, 1);
        detailedEvent.marketImpact = detectedEvents[i].impactScore / 100.0;
        if (detectedEvents[i].sentiment < 0) {
            detailedEvent.marketImpact = -detailedEvent.marketImpact;
        }
        
        /* Calculate abnormal return and volatility change */
        detailedEvent.abnormalReturn = calculateAbnormalReturn(stock, detectedEvents[i].date, 5);
        detailedEvent.volatilityChange = calculateVolatilityChange(stock, detectedEvents[i].date, 5, 5);
        
        /* Identify affected sectors */
        identifyAffectedSectors(&detectedEvents[i], (const Stock**)sectorPointers, 10, 
                              detailedEvent.affectedSectors);
        
        /* Estimate event duration based on severity and type */
        switch (detailedEvent.severity) {
            case EVENT_SEVERITY_LOW: 
                detailedEvent.durationEstimate = 1; 
                break;
            case EVENT_SEVERITY_MEDIUM: 
                detailedEvent.durationEstimate = 3; 
                break;
            case EVENT_SEVERITY_HIGH: 
                detailedEvent.durationEstimate = 7; 
                break;
            case EVENT_SEVERITY_CRITICAL: 
                detailedEvent.durationEstimate = 14; 
                break;
            default: 
                detailedEvent.durationEstimate = 1;
        }
        
        /* Print detailed analysis */
        printDetailedEvent(&detailedEvent);
        
        /* Find similar historical events */
        SimilarHistoricalEvent similarEvents[MAX_SIMILAR_EVENTS];
        int similarCount = findSimilarHistoricalEvents(&detectedEvents[i], newsEvents, similarEvents, MAX_SIMILAR_EVENTS);
        
        if (similarCount > 0) {
            printf("\nSimilar Historical Events:\n");
            for (int j = 0; j < similarCount; j++) {
                printf("- %s (%.0f%% similar): %.2f%% price impact, %d days to recovery\n",
                       similarEvents[j].eventData.title,
                       similarEvents[j].similarityScore * 100.0,
                       similarEvents[j].priceChangeAfterEvent * 100.0,
                       similarEvents[j].daysToRecovery);
            }
            
            /* Predict outcome based on similar events */
            double predictedOutcome = predictEventOutcome(&detectedEvents[i], similarEvents, similarCount);
            printf("\nPredicted price impact based on similar events: %.2f%%\n", predictedOutcome * 100.0);
        }
        
        /* Calculate event-adjusted technical indicators */
        printf("\nCalculating event-adjusted technical indicators based on this event...\n");
        calculateEventAdjustedIndicators(stock->data, stock->dataSize, &detectedEvents[i], indicators);
        printf("\nEvent-adjusted technical indicators for %s based on event impact:\n", stock->symbol);
        printExtendedTechnicalIndicators(indicators);
        
        /* Generate and display defensive strategy */
        char strategy[MAX_BUFFER_SIZE];
        recommendDefensiveStrategy(&detailedEvent, stock, 1, strategy, MAX_BUFFER_SIZE);
        printf("\nRecommended Strategy:\n%s\n", strategy);
    }
    
    /* Clean up dummy sectors */
    for (int i = 0; i < 10; i++) {
        freeStock(&dummySectors[i]);
    }
    
    free(indicators);
}

void printUsage(const char* programName) {
    printf("Usage: %s -k API_KEY -s SYMBOLS [options]\n\n", programName);
    printf("Options:\n");
    printf("  -k, --api-key KEY       Tiingo API key\n");
    printf("  -s, --symbols SYM1,SYM2 Comma-separated list of stock symbols\n");
    printf("  --start-date DATE       Start date (YYYY-MM-DD), default is 10 years ago\n");
    printf("  --end-date DATE         End date (YYYY-MM-DD), default is today\n");
}

void printStock(const Stock* stock) {
    printf("Symbol: %s\n", stock->symbol);
    printf("Data points: %d\n", stock->dataSize);
    
    if (stock->dataSize > 0) {
        printf("Latest price data (%s):\n", stock->data[stock->dataSize - 1].date);
        printf("  Open:  %.2f\n", stock->data[stock->dataSize - 1].open);
        printf("  High:  %.2f\n", stock->data[stock->dataSize - 1].high);
        printf("  Low:   %.2f\n", stock->data[stock->dataSize - 1].low);
        printf("  Close: %.2f\n", stock->data[stock->dataSize - 1].close);
        printf("  Volume: %.0f\n", stock->data[stock->dataSize - 1].volume);
    }
}

void printEvent(const EventData* event) {
    printf("Date: %s\n", event->date);
    printf("Title: %s\n", event->title);
    printf("Description: %s\n", event->description);
    printf("Sentiment: %.2f\n", event->sentiment);
    printf("Impact Score: %d\n", event->impactScore);
}

void printDetailedEvent(const DetailedEventData* event) {
    /* Print basic event data */
    printEvent(&event->basicData);
    
    /* Print additional detailed information */
    printf("Event Type: ");
    switch (event->type) {
        case MERGER_ACQUISITION: printf("Merger/Acquisition\n"); break;
        case EARNINGS_REPORT: printf("Earnings Report\n"); break;
        case CORPORATE_SCANDAL: printf("Corporate Scandal\n"); break;
        case LEADERSHIP_CHANGE: printf("Leadership Change\n"); break;
        case STOCK_SPLIT: printf("Stock Split\n"); break;
        case IPO: printf("IPO\n"); break;
        case LAYOFFS: printf("Layoffs\n"); break;
        case PRODUCT_LAUNCH: printf("Product Launch\n"); break;
        case PARTNERSHIP: printf("Partnership\n"); break;
        case REGULATORY_CHANGE: printf("Regulatory Change\n"); break;
        case UNKNOWN_EVENT: printf("Unknown\n"); break;
        default: printf("Unknown\n");
    }
    
    printf("Severity: ");
    switch (event->severity) {
        case EVENT_SEVERITY_LOW: printf("Low\n"); break;
        case EVENT_SEVERITY_MEDIUM: printf("Medium\n"); break;
        case EVENT_SEVERITY_HIGH: printf("High\n"); break;
        case EVENT_SEVERITY_CRITICAL: printf("Critical\n"); break;
        default: printf("Unknown\n");
    }
    
    printf("Market Impact: %.2f%%\n", event->marketImpact * 100.0);
    printf("Abnormal Return: %.2f%%\n", event->abnormalReturn * 100.0);
    printf("Volatility Change: %.2f%%\n", event->volatilityChange * 100.0);
    printf("Affected Sectors: %s\n", event->affectedSectors);
    printf("Estimated Duration: %d days\n", event->durationEstimate);
}

void printTechnicalIndicators(const TechnicalIndicators* indicators) {
    if (!indicators) {
        return;
    }
    
    printf("SMA(20): %.2f\n", indicators->sma);
    printf("EMA(14): %.2f\n", indicators->ema);
    printf("RSI(14): %.2f\n", indicators->rsi);
    printf("MACD: %.2f Signal: %.2f Histogram: %.2f\n", 
           indicators->macd, indicators->macdSignal, indicators->macdHistogram);
    printf("Bollinger Bands: Upper %.2f Middle %.2f Lower %.2f\n", 
           indicators->bollingerUpper, indicators->bollingerMiddle, indicators->bollingerLower);
    printf("ATR: %.2f\n", indicators->atr);
}

/* Print extended technical indicators */
void printExtendedTechnicalIndicators(const ExtendedTechnicalIndicators* indicators) {
    if (!indicators) {
        return;
    }
    
    /* Print basic indicators */
    printf("=== BASIC INDICATORS ===\n");
    printf("SMA(20): %.2f\n", indicators->sma);
    printf("EMA(14): %.2f\n", indicators->ema);
    printf("RSI(14): %.2f\n", indicators->rsi);
    printf("MACD: %.2f Signal: %.2f Histogram: %.2f\n", 
           indicators->macd, indicators->macdSignal, indicators->macdHistogram);
    printf("Bollinger Bands: Upper %.2f Middle %.2f Lower %.2f\n", 
           indicators->bollingerUpper, indicators->bollingerMiddle, indicators->bollingerLower);
    printf("ATR: %.2f\n\n", indicators->atr);
    
    /* Print advanced indicators */
    printf("=== ADVANCED INDICATORS ===\n");
    printf("ADX: %.2f +DI: %.2f -DI: %.2f\n", 
           indicators->adx, indicators->diPlus, indicators->diMinus);
    printf("Stochastic %%K: %.2f %%D: %.2f\n", 
           indicators->stochasticK, indicators->stochasticD);
    printf("Money Flow Index: %.2f\n", indicators->mfi);
    printf("Parabolic SAR: %.2f\n\n", indicators->psar);
    
    /* Print event-adjusted indicators, if available */
    if (indicators->eventADX > 0 || indicators->eventMFI > 0) {
        printf("=== EVENT-ADJUSTED INDICATORS ===\n");
        printf("Event-adjusted ADX: %.2f\n", indicators->eventADX);
        printf("Event-adjusted Stochastic %%K: %.2f %%D: %.2f\n", 
               indicators->eventStochasticK, indicators->eventStochasticD);
        printf("Event-adjusted MFI: %.2f\n", indicators->eventMFI);
        printf("Event-adjusted PSAR: %.2f\n", indicators->eventPSAR);
    }
}