/**
 * Event Analysis Module
 * Implementation of event detection and analysis functions
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include "../include/emers.h"
#include "../include/event_analysis.h"
#include "../include/technical_analysis.h"
#include "../include/data_mining.h"

/* Helper function declarations */
static double calculateTimeDifference(const char* date1, const char* date2);
static int parseDate(const char* dateStr, int* year, int* month, int* day);
static int dateToTimestamp(int year, int month, int day);

/* Event detection functions */

/**
 * Detects price movement events based on a threshold
 */
int detectPriceMovement(const StockData* data, int dataSize, double threshold) {
    if (!data || dataSize < 2) return 0;
    
    /* Calculate daily returns */
    for (int i = 1; i < dataSize; i++) {
        double change = (data[i].close - data[i-1].close) / data[i-1].close;
        
        /* Check if the absolute change exceeds the threshold */
        if (fabs(change) >= threshold) {
            return i;  /* Return the index of the event */
        }
    }
    
    return 0;  /* No event detected */
}

/**
 * Detects volume spike events based on a threshold
 */
int detectVolumeSpike(const StockData* data, int dataSize, double threshold) {
    if (!data || dataSize < 10) return 0;
    
    /* Calculate average volume for the past 10 days */
    double avgVolume = 0.0;
    int lookback = (dataSize < 10) ? dataSize : 10;
    
    for (int i = dataSize - lookback; i < dataSize - 1; i++) {
        avgVolume += data[i].volume;
    }
    avgVolume /= (lookback - 1);
    
    /* Check if the latest volume exceeds the average by the threshold factor */
    if (data[dataSize-1].volume >= avgVolume * (1.0 + threshold)) {
        return dataSize - 1;  /* Return the index of the event */
    }
    
    return 0;  /* No event detected */
}

/**
 * Detects volatility spike events based on a threshold
 */
int detectVolatilitySpike(const StockData* data, int dataSize, double threshold) {
    if (!data || dataSize < 20) return 0;
    
    /* Calculate historical volatility for the past 20 days */
    double histVol = calculateHistoricalVolatility(data, dataSize, 20);
    
    /* Calculate recent volatility (past 5 days) */
    double recentVol = calculateHistoricalVolatility(data + (dataSize - 5), 5, 5);
    
    /* Check if recent volatility exceeds historical by the threshold factor */
    if (recentVol >= histVol * (1.0 + threshold)) {
        return dataSize - 1;  /* Return the index of the event */
    }
    
    return 0;  /* No event detected */
}

/**
 * Detects significant market events from stock data and news
 */
int detectMarketEvents(const Stock* stocks, int stockCount, 
                      const EventDatabase* newsEvents, 
                      EventData* detectedEvents) {
    if (!stocks || stockCount <= 0 || !detectedEvents) return 0;
    
    int eventCount = 0;
    int maxEvents = 10;  /* Limit the number of events to detect */
    
    /* Look for price movements in each stock */
    for (int i = 0; i < stockCount && eventCount < maxEvents; i++) {
        const Stock* stock = &stocks[i];
        
        /* Only process stocks with sufficient data */
        if (stock->dataSize < 20) continue;
        
        /* Check for price movements (5% threshold) */
        int priceEventIdx = detectPriceMovement(stock->data, stock->dataSize, 0.05);
        if (priceEventIdx > 0) {
            EventData event;
            memset(&event, 0, sizeof(EventData));
            
            strcpy(event.symbol, stock->symbol);
            strcpy(event.date, stock->data[priceEventIdx].date);
            
            double priceChange = (stock->data[priceEventIdx].close - 
                                 stock->data[priceEventIdx-1].close) / 
                                 stock->data[priceEventIdx-1].close;
            
            sprintf(event.description, "Significant price movement of %.2f%% in %s", 
                   priceChange * 100.0, stock->symbol);
            
            event.type = (priceChange > 0) ? PRICE_JUMP : PRICE_DROP;
            event.magnitude = fabs(priceChange);
            
            detectedEvents[eventCount++] = event;
        }
        
        /* Check for volume spikes (100% increase) */
        int volumeEventIdx = detectVolumeSpike(stock->data, stock->dataSize, 1.0);
        if (volumeEventIdx > 0 && eventCount < maxEvents) {
            EventData event;
            memset(&event, 0, sizeof(EventData));
            
            strcpy(event.symbol, stock->symbol);
            strcpy(event.date, stock->data[volumeEventIdx].date);
            
            /* Calculate volume increase compared to average */
            double avgVolume = 0.0;
            for (int j = volumeEventIdx - 5; j < volumeEventIdx; j++) {
                if (j >= 0) avgVolume += stock->data[j].volume;
            }
            avgVolume /= 5.0;
            double volumeChange = (stock->data[volumeEventIdx].volume / avgVolume) - 1.0;
            
            sprintf(event.description, "Unusual trading volume in %s (%.2fx average)", 
                   stock->symbol, volumeChange + 1.0);
            
            event.type = VOLUME_SPIKE;
            event.magnitude = volumeChange;
            
            detectedEvents[eventCount++] = event;
        }
        
        /* Check for volatility spikes (50% increase) */
        int volEventIdx = detectVolatilitySpike(stock->data, stock->dataSize, 0.5);
        if (volEventIdx > 0 && eventCount < maxEvents) {
            EventData event;
            memset(&event, 0, sizeof(EventData));
            
            strcpy(event.symbol, stock->symbol);
            strcpy(event.date, stock->data[volEventIdx].date);
            
            sprintf(event.description, "Volatility spike detected in %s", stock->symbol);
            
            event.type = VOLATILITY_SPIKE;
            event.magnitude = 0.5;  /* Simplified magnitude */
            
            detectedEvents[eventCount++] = event;
        }
    }
    
    return eventCount;
}

/* Event analysis functions */

/**
 * Classifies an event based on its characteristics
 */
EventType classifyEvent(const EventData* event) {
    if (!event) return UNKNOWN_EVENT;
    
    /* If the event already has a type, return it */
    if (event->type != UNKNOWN_EVENT) {
        return event->type;
    }
    
    /* Simple classification based on description keywords */
    if (strstr(event->description, "price") || 
        strstr(event->description, "jump") || 
        strstr(event->description, "drop")) {
        
        if (strstr(event->description, "jump") || 
            strstr(event->description, "increase") || 
            strstr(event->description, "gain")) {
            return PRICE_JUMP;
        } else {
            return PRICE_DROP;
        }
    }
    
    if (strstr(event->description, "volume") || 
        strstr(event->description, "trading activity")) {
        return VOLUME_SPIKE;
    }
    
    if (strstr(event->description, "volatility") || 
        strstr(event->description, "uncertainty")) {
        return VOLATILITY_SPIKE;
    }
    
    if (strstr(event->description, "earnings") || 
        strstr(event->description, "report")) {
        return EARNINGS_ANNOUNCEMENT;
    }
    
    if (strstr(event->description, "dividend") || 
        strstr(event->description, "payout")) {
        return DIVIDEND_ANNOUNCEMENT;
    }
    
    if (strstr(event->description, "merger") || 
        strstr(event->description, "acquisition") || 
        strstr(event->description, "takeover")) {
        return MERGER_ACQUISITION;
    }
    
    if (strstr(event->description, "fed") || 
        strstr(event->description, "interest rate") || 
        strstr(event->description, "monetary policy")) {
        return FED_ANNOUNCEMENT;
    }
    
    if (strstr(event->description, "economic") || 
        strstr(event->description, "gdp") || 
        strstr(event->description, "employment")) {
        return ECONOMIC_DATA_RELEASE;
    }
    
    return UNKNOWN_EVENT;
}

/**
 * Calculates the impact of an event on the market
 */
double calculateEventImpact(const EventData* event, const Stock* stocks, int stockCount) {
    if (!event || !stocks || stockCount <= 0) {
        return 0.0;
    }
    
    double impact = 0.0;
    int affectedStockCount = 0;
    
    /* Find the stock that matches the event symbol */
    const Stock* eventStock = NULL;
    for (int i = 0; i < stockCount; i++) {
        if (strcmp(stocks[i].symbol, event->symbol) == 0) {
            eventStock = &stocks[i];
            break;
        }
    }
    
    if (!eventStock || eventStock->dataSize < 5) {
        return event->magnitude;  /* Return the event's own magnitude as a fallback */
    }
    
    /* Find the index of the event date in the stock data */
    int eventIndex = -1;
    for (int i = 0; i < eventStock->dataSize; i++) {
        if (strcmp(eventStock->data[i].date, event->date) == 0) {
            eventIndex = i;
            break;
        }
    }
    
    if (eventIndex < 0 || eventIndex >= eventStock->dataSize - 5) {
        return event->magnitude;  /* Return the event's own magnitude as a fallback */
    }
    
    /* Calculate price change from event day to 5 days after */
    double preBefore = eventStock->data[eventIndex].close;
    double postEvent = eventStock->data[eventIndex + 5].close;
    double change = (postEvent - preBefore) / preBefore;
    
    /* For this simplified version, we'll just return the percentage price change */
    return change;
}

/**
 * Assesses the severity of an event based on its impact and characteristics
 */
EventSeverity assessEventSeverity(const EventData* event, const Stock* stocks, int stockCount) {
    if (!event) return EVENT_SEVERITY_LOW;
    
    /* Calculate impact to assess severity */
    double impact = fabs(calculateEventImpact(event, stocks, stockCount));
    
    /* Classify based on impact magnitude */
    if (impact > 0.10) {  /* More than 10% price change */
        return EVENT_SEVERITY_CRITICAL;
    } else if (impact > 0.05) {  /* 5-10% price change */
        return EVENT_SEVERITY_HIGH;
    } else if (impact > 0.02) {  /* 2-5% price change */
        return EVENT_SEVERITY_MEDIUM;
    } else {
        return EVENT_SEVERITY_LOW;
    }
}

/**
 * Calculates the similarity between two events
 */
double calculateEventSimilarity(const EventData* event1, const EventData* event2) {
    if (!event1 || !event2) return 0.0;
    
    double similarity = 0.0;
    double weight_type = 0.3;
    double weight_magnitude = 0.3;
    double weight_symbol = 0.2;
    double weight_time = 0.2;
    
    /* Compare event types */
    double typeSimilarity = (event1->type == event2->type) ? 1.0 : 0.0;
    
    /* Compare event magnitudes */
    double magnitudeDiff = fabs(event1->magnitude - event2->magnitude);
    double magnitudeSimilarity = 1.0 - fmin(magnitudeDiff, 1.0);
    
    /* Compare symbols */
    double symbolSimilarity = (strcmp(event1->symbol, event2->symbol) == 0) ? 1.0 : 0.0;
    
    /* Compare event times (simplified) */
    double timeSimilarity = 1.0;
    if (strcmp(event1->date, event2->date) != 0) {
        double timeDiff = calculateTimeDifference(event1->date, event2->date);
        timeSimilarity = 1.0 - fmin(timeDiff / 365.0, 1.0);  /* Scale by year */
    }
    
    /* Calculate weighted similarity */
    similarity = weight_type * typeSimilarity +
                weight_magnitude * magnitudeSimilarity +
                weight_symbol * symbolSimilarity +
                weight_time * timeSimilarity;
    
    return similarity;
}

/**
 * Finds similar events to the specified event in the database
 */
int findSimilarEvents(const EventData* event, const EventDatabase* db, 
                     int* similarEventIndices, int maxResults) {
    if (!event || !db || !similarEventIndices || maxResults <= 0) {
        return 0;
    }
    
    if (db->eventCount <= 0) {
        return 0;
    }
    
    /* Calculate similarity for all events in the database */
    double* similarityScores = (double*)malloc(db->eventCount * sizeof(double));
    if (!similarityScores) {
        return 0;
    }
    
    for (int i = 0; i < db->eventCount; i++) {
        similarityScores[i] = calculateEventSimilarity(event, &db->events[i]);
    }
    
    /* Find the top 'maxResults' most similar events */
    int resultCount = 0;
    
    /* Initialize with -1 to indicate "not set" */
    for (int i = 0; i < maxResults; i++) {
        similarEventIndices[i] = -1;
    }
    
    for (int k = 0; k < maxResults && k < db->eventCount; k++) {
        double maxSimilarity = -1.0;
        int maxIndex = -1;
        
        for (int i = 0; i < db->eventCount; i++) {
            /* Skip events that are already selected */
            int alreadySelected = 0;
            for (int j = 0; j < resultCount; j++) {
                if (similarEventIndices[j] == i) {
                    alreadySelected = 1;
                    break;
                }
            }
            
            if (!alreadySelected && similarityScores[i] > maxSimilarity) {
                maxSimilarity = similarityScores[i];
                maxIndex = i;
            }
        }
        
        if (maxIndex >= 0 && maxSimilarity > 0.5) {  /* Only include if similarity > 50% */
            similarEventIndices[resultCount++] = maxIndex;
        }
    }
    
    free(similarityScores);
    return resultCount;
}

/**
 * Finds similar historical events and returns detailed information
 */
int findSimilarHistoricalEvents(const EventData* currentEvent, const EventDatabase* historicalEvents, 
                              SimilarHistoricalEvent* similarEvents, int maxResults) {
    if (!currentEvent || !historicalEvents || !similarEvents || maxResults <= 0) {
        return 0;
    }
    
    /* Find indices of similar events */
    int* similarIndices = (int*)malloc(maxResults * sizeof(int));
    if (!similarIndices) {
        return 0;
    }
    
    int count = findSimilarEvents(currentEvent, historicalEvents, similarIndices, maxResults);
    
    /* Copy event data and calculate additional metrics */
    for (int i = 0; i < count; i++) {
        int idx = similarIndices[i];
        
        /* Copy event data */
        similarEvents[i].eventData = historicalEvents->events[idx];
        
        /* Calculate similarity score */
        similarEvents[i].similarityScore = calculateEventSimilarity(currentEvent, &historicalEvents->events[idx]);
        
        /* For this simplified version, we'll use placeholder values for price change and recovery time */
        similarEvents[i].priceChangeAfterEvent = 0.05 * (similarEvents[i].similarityScore - 0.5);  /* Simplified price change estimate */
        similarEvents[i].daysToRecovery = 30 * (2.0 - similarEvents[i].similarityScore);  /* Simplified recovery time estimate */
    }
    
    free(similarIndices);
    return count;
}

/**
 * Predicts the outcome of an event based on similar historical events
 */
double predictEventOutcome(const EventData* event, const SimilarHistoricalEvent* similarEvents, int count) {
    if (!event || !similarEvents || count <= 0) {
        return 0.0;
    }
    
    double totalWeight = 0.0;
    double weightedPriceChange = 0.0;
    
    /* Calculate weighted average of price changes from similar events */
    for (int i = 0; i < count; i++) {
        double weight = similarEvents[i].similarityScore;
        weightedPriceChange += weight * similarEvents[i].priceChangeAfterEvent;
        totalWeight += weight;
    }
    
    if (totalWeight > 0) {
        return weightedPriceChange / totalWeight;
    } else {
        return 0.0;
    }
}

/**
 * Generates a detailed event report
 */
void generateEventReport(const DetailedEventData* event, char* report, int reportSize) {
    if (!event || !report || reportSize <= 0) {
        return;
    }
    
    char severity[20];
    switch (event->severity) {
        case EVENT_SEVERITY_LOW:
            strcpy(severity, "LOW");
            break;
        case EVENT_SEVERITY_MEDIUM:
            strcpy(severity, "MEDIUM");
            break;
        case EVENT_SEVERITY_HIGH:
            strcpy(severity, "HIGH");
            break;
        case EVENT_SEVERITY_CRITICAL:
            strcpy(severity, "CRITICAL");
            break;
        default:
            strcpy(severity, "UNKNOWN");
    }
    
    char eventType[30];
    switch (event->type) {
        case PRICE_JUMP:
            strcpy(eventType, "Price Jump");
            break;
        case PRICE_DROP:
            strcpy(eventType, "Price Drop");
            break;
        case VOLUME_SPIKE:
            strcpy(eventType, "Volume Spike");
            break;
        case VOLATILITY_SPIKE:
            strcpy(eventType, "Volatility Spike");
            break;
        case EARNINGS_ANNOUNCEMENT:
            strcpy(eventType, "Earnings Announcement");
            break;
        case DIVIDEND_ANNOUNCEMENT:
            strcpy(eventType, "Dividend Announcement");
            break;
        case MERGER_ACQUISITION:
            strcpy(eventType, "Merger/Acquisition");
            break;
        case FED_ANNOUNCEMENT:
            strcpy(eventType, "Fed Announcement");
            break;
        case ECONOMIC_DATA_RELEASE:
            strcpy(eventType, "Economic Data Release");
            break;
        default:
            strcpy(eventType, "Unknown Event");
    }
    
    snprintf(report, reportSize,
            "EVENT ANALYSIS REPORT\n"
            "====================\n"
            "Symbol: %s\n"
            "Date: %s\n"
            "Type: %s\n"
            "Severity: %s\n"
            "Description: %s\n"
            "\n"
            "Market Impact: %.2f%%\n"
            "Abnormal Return: %.2f%%\n"
            "Volatility Change: %.2f%%\n"
            "Affected Sectors: %s\n"
            "Estimated Duration: %d days\n",
            event->basicData.symbol,
            event->basicData.date,
            eventType,
            severity,
            event->basicData.description,
            event->marketImpact * 100.0,
            event->abnormalReturn * 100.0,
            event->volatilityChange * 100.0,
            event->affectedSectors,
            event->durationEstimate);
}

/* Helper functions */

/**
 * Calculates the time difference between two dates in days
 */
static double calculateTimeDifference(const char* date1, const char* date2) {
    int year1, month1, day1;
    int year2, month2, day2;
    
    if (!parseDate(date1, &year1, &month1, &day1) || 
        !parseDate(date2, &year2, &month2, &day2)) {
        return 0.0;
    }
    
    int timestamp1 = dateToTimestamp(year1, month1, day1);
    int timestamp2 = dateToTimestamp(year2, month2, day2);
    
    return fabs(timestamp1 - timestamp2);
}

/**
 * Parses a date string in format "YYYY-MM-DD"
 */
static int parseDate(const char* dateStr, int* year, int* month, int* day) {
    if (!dateStr || !year || !month || !day) return 0;
    
    if (sscanf(dateStr, "%d-%d-%d", year, month, day) != 3) {
        return 0;
    }
    
    return 1;
}

/**
 * Converts a date to a timestamp (days since epoch)
 * Simple approximation for date comparison
 */
static int dateToTimestamp(int year, int month, int day) {
    /* Simple approximation using 365 days per year */
    int timestamp = year * 365;
    
    /* Add days for months (approximation) */
    static const int daysInMonth[] = {
        0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
    };
    
    for (int i = 1; i < month; i++) {
        timestamp += daysInMonth[i];
    }
    
    /* Add leap days for past years (simplified) */
    timestamp += (year / 4);
    
    /* Add days of current month */
    timestamp += day;
    
    return timestamp;
}