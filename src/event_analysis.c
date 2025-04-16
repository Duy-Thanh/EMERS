/**
 * Event Analysis Module
 * Implementation file for event detection and analysis functions
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include "../include/emers.h"
#include "../include/event_analysis.h"
#include "../include/technical_analysis.h"

/* Detect market events based on stock data and news */
int detectMarketEvents(const Stock* stocks, int stockCount, const EventDatabase* newsEvents, EventData* detectedEvents) {
    if (!stocks || stockCount <= 0 || !detectedEvents || !newsEvents) {
        return 0;
    }
    
    int eventCount = 0;
    int i;
    
    /* Check each stock for significant price movements */
    for (i = 0; i < stockCount && eventCount < MAX_EVENTS; i++) {
        const Stock* stock = &stocks[i];
        
        if (stock->dataSize < 5) {
            /* Not enough data points */
            continue;
        }
        
        /* Check for price movements */
        if (detectPriceMovement(stock->data, stock->dataSize, EVENT_DETECTION_THRESHOLD)) {
            /* Create an event for the price movement */
            EventData event;
            memset(&event, 0, sizeof(EventData));
            
            /* Get the date of the event (most recent data point) */
            strncpy(event.date, stock->data[stock->dataSize - 1].date, MAX_DATE_LENGTH - 1);
            
            /* Create a title based on the movement */
            double priceChange = (stock->data[stock->dataSize - 1].close - stock->data[stock->dataSize - 2].close) 
                               / stock->data[stock->dataSize - 2].close;
            
            snprintf(event.title, MAX_BUFFER_SIZE - 1, 
                    "Significant Price Movement in %s: %.2f%%", 
                    stock->symbol, priceChange * 100.0);
            
            /* Create a description */
            snprintf(event.description, MAX_BUFFER_SIZE - 1,
                    "%s moved from %.2f to %.2f, a change of %.2f%% on %s.",
                    stock->symbol, 
                    stock->data[stock->dataSize - 2].close,
                    stock->data[stock->dataSize - 1].close,
                    priceChange * 100.0,
                    event.date);
            
            /* Set sentiment based on direction */
            event.sentiment = (priceChange > 0) ? 0.7 : -0.7;
            
            /* Set impact score based on magnitude */
            event.impactScore = (int)(fabs(priceChange) * 1000);
            if (event.impactScore > 100) event.impactScore = 100;
            
            /* Add to detected events */
            detectedEvents[eventCount] = event;
            eventCount++;
        }
        
        /* Check for volatility spikes */
        if (detectVolatilitySpike(stock->data, stock->dataSize, 2.0) && eventCount < MAX_EVENTS) {
            /* Create an event for the volatility spike */
            EventData event;
            memset(&event, 0, sizeof(EventData));
            
            /* Get the date of the event (most recent data point) */
            strncpy(event.date, stock->data[stock->dataSize - 1].date, MAX_DATE_LENGTH - 1);
            
            /* Create a title */
            snprintf(event.title, MAX_BUFFER_SIZE - 1, 
                    "Volatility Spike Detected in %s", stock->symbol);
            
            /* Create a description */
            snprintf(event.description, MAX_BUFFER_SIZE - 1,
                    "Significant increase in volatility detected for %s on %s. "
                    "High: %.2f, Low: %.2f, Range: %.2f%%",
                    stock->symbol, 
                    event.date,
                    stock->data[stock->dataSize - 1].high,
                    stock->data[stock->dataSize - 1].low,
                    (stock->data[stock->dataSize - 1].high - stock->data[stock->dataSize - 1].low) / 
                    stock->data[stock->dataSize - 1].low * 100.0);
            
            /* Set sentiment (volatility usually perceived as negative) */
            event.sentiment = -0.5;
            
            /* Set impact score */
            event.impactScore = 70; /* High impact for volatility */
            
            /* Add to detected events */
            detectedEvents[eventCount] = event;
            eventCount++;
        }
        
        /* Check for volume spikes */
        if (detectVolumeSpike(stock->data, stock->dataSize, 2.0) && eventCount < MAX_EVENTS) {
            /* Create an event for the volume spike */
            EventData event;
            memset(&event, 0, sizeof(EventData));
            
            /* Get the date of the event (most recent data point) */
            strncpy(event.date, stock->data[stock->dataSize - 1].date, MAX_DATE_LENGTH - 1);
            
            /* Create a title */
            snprintf(event.title, MAX_BUFFER_SIZE - 1, 
                    "Volume Spike Detected in %s", stock->symbol);
            
            /* Create a description */
            snprintf(event.description, MAX_BUFFER_SIZE - 1,
                    "Abnormal trading volume detected for %s on %s. "
                    "Volume: %.0f, significantly above average.",
                    stock->symbol, 
                    event.date,
                    stock->data[stock->dataSize - 1].volume);
            
            /* Set neutral sentiment (volume can be either positive or negative) */
            event.sentiment = 0.0;
            
            /* Set impact score */
            event.impactScore = 60; /* Moderately high impact for volume spike */
            
            /* Add to detected events */
            detectedEvents[eventCount] = event;
            eventCount++;
        }
    }
    
    /* Add high-impact news events */
    for (i = 0; i < newsEvents->eventCount && eventCount < MAX_EVENTS; i++) {
        if (newsEvents->events[i].impactScore >= MIN_EVENT_SCORE) {
            /* Add to detected events */
            detectedEvents[eventCount] = newsEvents->events[i];
            eventCount++;
        }
    }
    
    return eventCount;
}

/* Detect a spike in volatility */
int detectVolatilitySpike(const StockData* data, int dataSize, double threshold) {
    if (!data || dataSize < 10) { /* Need at least 10 data points */
        return 0;
    }
    
    /* Calculate average true range for recent periods */
    double recentATR = 0.0;
    calculateATR(&data[dataSize - 5], 5, 5, &recentATR);
    
    /* Calculate average true range for earlier periods */
    double earlierATR = 0.0;
    calculateATR(&data[dataSize - 10], 5, 5, &earlierATR);
    
    /* Check if recent ATR is significantly higher than earlier ATR */
    if (earlierATR > 0 && (recentATR / earlierATR) >= threshold) {
        return 1; /* Volatility spike detected */
    }
    
    return 0;
}

/* Detect a significant price movement */
int detectPriceMovement(const StockData* data, int dataSize, double threshold) {
    if (!data || dataSize < 2) {
        return 0;
    }
    
    /* Calculate percentage change in closing price */
    double priceChange = fabs((data[dataSize - 1].close - data[dataSize - 2].close) / data[dataSize - 2].close);
    
    /* Check if the change exceeds the threshold */
    if (priceChange >= threshold) {
        return 1; /* Significant price movement detected */
    }
    
    return 0;
}

/* Detect a spike in trading volume */
int detectVolumeSpike(const StockData* data, int dataSize, double threshold) {
    if (!data || dataSize < 6) { /* Need at least 6 data points */
        return 0;
    }
    
    /* Calculate average volume for the previous 5 days */
    double averageVolume = 0.0;
    int i;
    for (i = dataSize - 6; i < dataSize - 1; i++) {
        averageVolume += data[i].volume;
    }
    averageVolume /= 5.0;
    
    /* Check if the most recent volume is significantly higher than the average */
    if (averageVolume > 0 && (data[dataSize - 1].volume / averageVolume) >= threshold) {
        return 1; /* Volume spike detected */
    }
    
    return 0;
}

/* Calculate the impact of an event on the market */
double calculateEventImpact(const EventData* event, const Stock* stocks, int stockCount) {
    if (!event || !stocks || stockCount <= 0) {
        return 0.0;
    }
    
    /* For simplicity, we'll convert the impact score to a double between -1.0 and 1.0 */
    /* Negative sentiment => negative impact, positive sentiment => positive impact */
    double impactDirection = (event->sentiment >= 0) ? 1.0 : -1.0;
    double impactMagnitude = event->impactScore / 100.0; /* Normalize to 0.0-1.0 */
    
    return impactDirection * impactMagnitude;
}

/* Classify an event type based on content */
EventType classifyEvent(const EventData* event) {
    if (!event) {
        return UNKNOWN_EVENT;
    }
    
    // Simple keyword-based classification
    const char* merger_keywords[] = {"merger", "acquisition", "takeover", "acquire"};
    const char* earnings_keywords[] = {"earnings", "profit", "revenue", "quarterly", "results"};
    const char* scandal_keywords[] = {"scandal", "fraud", "lawsuit", "investigation"};
    const char* leadership_keywords[] = {"ceo", "executive", "chairman", "president", "appointed", "resigned", "resignation"};
    const char* split_keywords[] = {"split", "dividend", "buyback"};
    const char* ipo_keywords[] = {"ipo", "initial public offering", "debut", "public trading"};
    const char* layoff_keywords[] = {"layoff", "job cut", "downsizing", "reduction"};
    const char* product_keywords[] = {"launch", "new product", "release", "unveil", "breakthrough"};
    const char* partnership_keywords[] = {"partnership", "alliance", "joint venture", "collaborate"};
    const char* regulatory_keywords[] = {"regulation", "compliance", "regulatory", "law", "legal", "policy"};
    
    // Convert title to lowercase for case-insensitive matching
    char lower_title[MAX_BUFFER_SIZE];
    strncpy(lower_title, event->title, MAX_BUFFER_SIZE - 1);
    lower_title[MAX_BUFFER_SIZE - 1] = '\0';
    
    for (size_t i = 0; i < strlen(lower_title); i++) {
        lower_title[i] = tolower(lower_title[i]);
    }
    
    // Count keyword matches in title
    int scores[10] = {0}; // One score for each event type
    
    // Check title for merger keywords
    for (int i = 0; i < sizeof(merger_keywords)/sizeof(const char*); i++) {
        if (strstr(lower_title, merger_keywords[i]) != NULL) {
            scores[0]++;
        }
    }
    
    // Check title for earnings keywords
    for (int i = 0; i < sizeof(earnings_keywords)/sizeof(const char*); i++) {
        if (strstr(lower_title, earnings_keywords[i]) != NULL) {
            scores[1]++;
        }
    }
    
    // Check title for scandal keywords
    for (int i = 0; i < sizeof(scandal_keywords)/sizeof(const char*); i++) {
        if (strstr(lower_title, scandal_keywords[i]) != NULL) {
            scores[2]++;
        }
    }
    
    // Check title for leadership keywords
    for (int i = 0; i < sizeof(leadership_keywords)/sizeof(const char*); i++) {
        if (strstr(lower_title, leadership_keywords[i]) != NULL) {
            scores[3]++;
        }
    }
    
    // Check title for split keywords
    for (int i = 0; i < sizeof(split_keywords)/sizeof(const char*); i++) {
        if (strstr(lower_title, split_keywords[i]) != NULL) {
            scores[4]++;
        }
    }
    
    // Check title for IPO keywords
    for (int i = 0; i < sizeof(ipo_keywords)/sizeof(const char*); i++) {
        if (strstr(lower_title, ipo_keywords[i]) != NULL) {
            scores[5]++;
        }
    }
    
    // Check title for layoff keywords
    for (int i = 0; i < sizeof(layoff_keywords)/sizeof(const char*); i++) {
        if (strstr(lower_title, layoff_keywords[i]) != NULL) {
            scores[6]++;
        }
    }
    
    // Check title for product keywords
    for (int i = 0; i < sizeof(product_keywords)/sizeof(const char*); i++) {
        if (strstr(lower_title, product_keywords[i]) != NULL) {
            scores[7]++;
        }
    }
    
    // Check title for partnership keywords
    for (int i = 0; i < sizeof(partnership_keywords)/sizeof(const char*); i++) {
        if (strstr(lower_title, partnership_keywords[i]) != NULL) {
            scores[8]++;
        }
    }
    
    // Check title for regulatory keywords
    for (int i = 0; i < sizeof(regulatory_keywords)/sizeof(const char*); i++) {
        if (strstr(lower_title, regulatory_keywords[i]) != NULL) {
            scores[9]++;
        }
    }
    
    // Find highest score and corresponding event type
    int max_score = 0;
    int max_index = -1;
    
    for (int i = 0; i < 10; i++) {
        if (scores[i] > max_score) {
            max_score = scores[i];
            max_index = i;
        }
    }
    
    // Return corresponding event type, or UNKNOWN_EVENT if no match
    if (max_score == 0) {
        return UNKNOWN_EVENT;
    }
    
    switch (max_index) {
        case 0: return MERGER_ACQUISITION;
        case 1: return EARNINGS_REPORT;
        case 2: return CORPORATE_SCANDAL;
        case 3: return LEADERSHIP_CHANGE;
        case 4: return STOCK_SPLIT;
        case 5: return IPO;
        case 6: return LAYOFFS;
        case 7: return PRODUCT_LAUNCH;
        case 8: return PARTNERSHIP;
        case 9: return REGULATORY_CHANGE;
        default: return UNKNOWN_EVENT;
    }
}

/* Assess event severity based on impact */
EventSeverity assessEventSeverity(const EventData* event, const Stock* stocks, int stockCount) {
    if (!event) {
        return EVENT_SEVERITY_LOW;
    }
    
    /* Base severity on impact score */
    if (event->impactScore >= 90) {
        return EVENT_SEVERITY_CRITICAL;
    } else if (event->impactScore >= 70) {
        return EVENT_SEVERITY_HIGH;
    } else if (event->impactScore >= 40) {
        return EVENT_SEVERITY_MEDIUM;
    } else {
        return EVENT_SEVERITY_LOW;
    }
    
    /* Note: stocks and stockCount parameters are kept for future extensions
     * that might analyze stock-specific impact to determine severity */
}

/* Calculate abnormal return for a stock around an event */
double calculateAbnormalReturn(const Stock* stock, const char* eventDate, int window) {
    if (!stock || !eventDate || stock->dataSize <= 0) {
        return 0.0;
    }
    
    /* Find the event date in the stock data */
    int eventIndex = -1;
    int i;
    for (i = 0; i < stock->dataSize; i++) {
        if (strcmp(stock->data[i].date, eventDate) == 0) {
            eventIndex = i;
            break;
        }
    }
    
    if (eventIndex < 0 || eventIndex >= stock->dataSize - window) {
        /* Event date not found or insufficient data */
        return 0.0;
    }
    
    /* Calculate actual return over the window */
    double startPrice = stock->data[eventIndex].close;
    double endPrice = stock->data[eventIndex + window].close;
    double actualReturn = (endPrice - startPrice) / startPrice;
    
    /* For a simple model, assume expected return is 0 */
    /* In a real model, would calculate based on market model */
    double expectedReturn = 0.0;
    
    /* Abnormal return is actual - expected */
    return actualReturn - expectedReturn;
}

/* Calculate change in volatility around an event */
double calculateVolatilityChange(const Stock* stock, const char* eventDate, int preWindow, int postWindow) {
    if (!stock || !eventDate || stock->dataSize <= 0) {
        return 0.0;
    }
    
    /* Find the event date in the stock data */
    int eventIndex = -1;
    int i;
    for (i = 0; i < stock->dataSize; i++) {
        if (strcmp(stock->data[i].date, eventDate) == 0) {
            eventIndex = i;
            break;
        }
    }
    
    if (eventIndex < preWindow || eventIndex >= stock->dataSize - postWindow) {
        /* Insufficient data around event */
        return 0.0;
    }
    
    /* Calculate pre-event volatility (standard deviation of returns) */
    double preReturns[30]; /* Assume max window is 30 */
    for (i = 0; i < preWindow && i < 30; i++) {
        int idx = eventIndex - preWindow + i;
        if (idx > 0) {
            preReturns[i] = (stock->data[idx].close - stock->data[idx-1].close) / stock->data[idx-1].close;
        } else {
            preReturns[i] = 0.0;
        }
    }
    double preVolatility = calculateStandardDeviation(preReturns, preWindow);
    
    /* Calculate post-event volatility */
    double postReturns[30];
    for (i = 0; i < postWindow && i < 30; i++) {
        int idx = eventIndex + i;
        if (idx > 0) {
            postReturns[i] = (stock->data[idx].close - stock->data[idx-1].close) / stock->data[idx-1].close;
        } else {
            postReturns[i] = 0.0;
        }
    }
    double postVolatility = calculateStandardDeviation(postReturns, postWindow);
    
    /* Calculate percentage change in volatility */
    if (preVolatility > 0.0) {
        return (postVolatility - preVolatility) / preVolatility;
    } else {
        return 0.0;
    }
}

/* Recommend defensive strategy based on event type and severity */
void recommendDefensiveStrategy(const DetailedEventData* event, const Stock* stocks, int stockCount, char* strategy, int strategySize) {
    if (!event || !strategy || strategySize <= 0) {
        return;
    }
    
    // Clear the strategy buffer
    strategy[0] = '\0';
    
    // Base strategy on event type
    switch (event->type) {
        case MERGER_ACQUISITION:
            snprintf(strategy, strategySize,
                "Merger/Acquisition event detected. Recommended strategy:\n"
                "1. Evaluate implied acquisition price vs current price\n"
                "2. Consider arbitrage opportunities if applicable\n"
                "3. Assess regulatory risk for deal completion\n"
                "4. Review sector for additional consolidation opportunities");
            break;
        
        case EARNINGS_REPORT:
            snprintf(strategy, strategySize,
                "Earnings report detected. Recommended strategy:\n"
                "1. Compare results to analyst expectations\n"
                "2. Review forward guidance and management commentary\n"
                "3. Assess impact on valuation metrics\n"
                "4. Monitor analyst revisions in the next 1-2 weeks");
            break;
        
        case REGULATORY_CHANGE:
            snprintf(strategy, strategySize,
                "Policy change event detected. Recommended strategy:\n"
                "1. Analyze specific sectors impacted by policy change\n"
                "2. Adjust sector weights accordingly\n"
                "3. Look for opportunities in positively impacted sectors\n"
                "4. Re-evaluate strategy in 10-14 days after full market reaction");
            break;
        
        case LEADERSHIP_CHANGE:
            snprintf(strategy, strategySize,
                "Leadership change detected. Recommended strategy:\n"
                "1. Assess new leadership background and prior performance\n"
                "2. Monitor initial strategic announcements\n"
                "3. Review corporate governance structure\n"
                "4. Evaluate succession planning quality");
            break;
        
        case CORPORATE_SCANDAL:
            snprintf(strategy, strategySize,
                "Corporate event detected for %s. Recommended strategy:\n"
                "1. %s exposure to affected company\n"
                "2. Assess broader sector impact and consider %s sector exposure\n"
                "3. Review competitors for knock-on effects\n"
                "4. Maintain diversification to minimize single-stock risk",
                event->basicData.title,
                (event->marketImpact < 0) ? "Consider reducing" : "Maintain or increase",
                (event->marketImpact < 0) ? "reducing" : "maintaining");
            break;
        
        case PRODUCT_LAUNCH:
            snprintf(strategy, strategySize,
                "Product launch event detected. Recommended strategy:\n"
                "1. Evaluate potential market impact and adoption timeline\n"
                "2. Review competitive landscape implications\n"
                "3. Monitor initial sales/reception data\n"
                "4. Consider supply chain and production capacity risks");
            break;
        
        case UNKNOWN_EVENT:
            snprintf(strategy, strategySize,
                "Event detected with insufficient classification information.\n"
                "Recommended strategy:\n"
                "1. Monitor markets for further clarity\n"
                "2. No immediate action recommended\n"
                "3. Reassess situation as more information becomes available");
            break;
            
        default:
            snprintf(strategy, strategySize, 
                "Unrecognized event type. Maintain diversification and monitor developments.");
            break;
    }
}

/* Generate a detailed event report */
void generateEventReport(const DetailedEventData* event, char* report, int reportSize) {
    if (!event || !report || reportSize <= 0) {
        return;
    }
    
    // Format the timestamp
    char timeStr[64];
    // Use the date string from basicData instead of timestamp
    strncpy(timeStr, event->basicData.date, sizeof(timeStr) - 1);
    timeStr[sizeof(timeStr) - 1] = '\0';
    
    // Get event type as string
    const char* eventTypeStr;
    switch (event->type) {
        case MERGER_ACQUISITION: eventTypeStr = "Merger/Acquisition"; break;
        case EARNINGS_REPORT: eventTypeStr = "Earnings Report"; break;
        case CORPORATE_SCANDAL: eventTypeStr = "Corporate Scandal"; break;
        case LEADERSHIP_CHANGE: eventTypeStr = "Leadership Change"; break;
        case STOCK_SPLIT: eventTypeStr = "Stock Split"; break;
        case IPO: eventTypeStr = "IPO"; break;
        case LAYOFFS: eventTypeStr = "Layoffs"; break;
        case PRODUCT_LAUNCH: eventTypeStr = "Product Launch"; break;
        case PARTNERSHIP: eventTypeStr = "Partnership"; break;
        case REGULATORY_CHANGE: eventTypeStr = "Regulatory Change"; break;
        case UNKNOWN_EVENT: eventTypeStr = "Unknown"; break;
        default: eventTypeStr = "Unknown";
    }
    
    // Get severity as string
    const char* severityStr;
    switch (event->severity) {
        case EVENT_SEVERITY_LOW: severityStr = "Low"; break;
        case EVENT_SEVERITY_MEDIUM: severityStr = "Medium"; break;
        case EVENT_SEVERITY_HIGH: severityStr = "High"; break;
        case EVENT_SEVERITY_CRITICAL: severityStr = "Critical"; break;
        default: severityStr = "Unknown";
    }
    
    // Generate the report
    snprintf(report, reportSize,
            "\nDate: %s\n"
            "Title: %s\n"
            "Description: %s\n"
            "\n"
            "Sentiment: %.2f\n"
            "Impact Score: %d\n"
            "Event Type: %s\n"
            "Severity: %s\n"
            "Market Impact: %.2f%%\n"
            "Abnormal Return: %.2f%%\n"
            "Volatility Change: %.2f%%\n"
            "Affected Sectors: %s\n"
            "Estimated Duration: %d days\n",
            timeStr,
            event->basicData.title,
            event->basicData.description,
            event->basicData.sentiment,
            event->basicData.impactScore,
            eventTypeStr,
            severityStr,
            event->marketImpact * 100.0,
            event->abnormalReturn * 100.0,
            event->volatilityChange * 100.0,
            event->affectedSectors,
            event->durationEstimate);
            
    // Calculate the length of the current report
    int reportLen = strlen(report);
    
    // Add recommended strategy if there's space
    if (reportLen < reportSize - 100) {
        char strategy[1024] = {0};
        recommendDefensiveStrategy(event, NULL, 0, strategy, sizeof(strategy));
        
        // Append strategy to report
        strncat(report, "\nRecommended Strategy:\n", reportSize - reportLen - 1);
        reportLen = strlen(report);
        strncat(report, strategy, reportSize - reportLen - 1);
    }
}

/* Add an event to the database */
int addEvent(EventDatabase* db, const EventData* event) {
    if (!db || !event) {
        return 0;
    }
    
    /* Initialize database if needed */
    if (!db->events) {
        db->eventCapacity = 50;
        db->events = (EventData*)malloc(db->eventCapacity * sizeof(EventData));
        if (!db->events) {
            printf("Error: Failed to allocate memory for event database\n");
            return 0;
        }
        db->eventCount = 0;
    }
    
    /* Check if we need to resize */
    if (db->eventCount >= db->eventCapacity) {
        db->eventCapacity *= 2;
        EventData* newEvents = (EventData*)realloc(db->events, db->eventCapacity * sizeof(EventData));
        if (!newEvents) {
            printf("Error: Failed to resize event database\n");
            return 0;
        }
        db->events = newEvents;
    }
    
    /* Add the event */
    db->events[db->eventCount] = *event;
    db->eventCount++;
    
    return 1;
}

/* Find similar events in the database */
int findSimilarEvents(const EventData* event, const EventDatabase* db, int* similarEventIndices, int maxResults) {
    if (!event || !db || !similarEventIndices || maxResults <= 0) {
        return 0;
    }
    
    int resultCount = 0;
    int i;
    
    /* Calculate similarity score for each event in the database */
    double* similarityScores = (double*)malloc(db->eventCount * sizeof(double));
    if (!similarityScores) {
        printf("Error: Memory allocation failed for similarity scores\n");
        return 0;
    }
    
    for (i = 0; i < db->eventCount; i++) {
        similarityScores[i] = calculateEventSimilarity(event, &db->events[i]);
    }
    
    /* Find the top N most similar events */
    for (i = 0; i < maxResults && i < db->eventCount; i++) {
        /* Find the maximum similarity score */
        double maxScore = -1.0;
        int maxIndex = -1;
        int j;
        
        for (j = 0; j < db->eventCount; j++) {
            if (similarityScores[j] > maxScore) {
                maxScore = similarityScores[j];
                maxIndex = j;
            }
        }
        
        if (maxIndex >= 0 && maxScore > 0.0) {
            similarEventIndices[resultCount] = maxIndex;
            resultCount++;
            
            /* Set this score to -1 so it's not selected again */
            similarityScores[maxIndex] = -1.0;
        } else {
            /* No more similar events */
            break;
        }
    }
    
    free(similarityScores);
    return resultCount;
}

/* Calculate similarity between two events */
double calculateEventSimilarity(const EventData* event1, const EventData* event2) {
    if (!event1 || !event2) {
        return 0.0;
    }
    
    /* Calculate similarity based on multiple factors */
    double sentimentSimilarity = 1.0 - fabs(event1->sentiment - event2->sentiment);
    
    /* For impact score (integers), use an explicit cast to double for the difference */
    double impactDiff = (double)(event1->impactScore - event2->impactScore);
    double impactSimilarity = 1.0 - (fabs(impactDiff) / 100.0);
    
    /* Title and description similarity would use more sophisticated text similarity */
    /* For simplicity, we'll just check for some common words */
    double titleSimilarity = 0.3; /* Default some similarity */
    double descSimilarity = 0.3;
    
    /* Simple keyword matching */
    if (strstr(event1->title, "Market") && strstr(event2->title, "Market")) {
        titleSimilarity += 0.3;
    }
    if (strstr(event1->title, "Economic") && strstr(event2->title, "Economic")) {
        titleSimilarity += 0.3;
    }
    if (strstr(event1->description, "impact") && strstr(event2->description, "impact")) {
        descSimilarity += 0.2;
    }
    
    /* Combine similarities with weights */
    return (sentimentSimilarity * 0.3) + (impactSimilarity * 0.4) + 
           (titleSimilarity * 0.2) + (descSimilarity * 0.1);
}

/* Sector analysis */
void identifyAffectedSectors(const EventData* event, const Stock** sectorIndices, int sectorCount, char* affectedSectors) {
    if (!event || !sectorIndices || sectorCount <= 0 || !affectedSectors) {
        if (affectedSectors) {
            affectedSectors[0] = '\0';
        }
        return;
    }
    
    /* Reset affected sectors string */
    affectedSectors[0] = '\0';
    
    /* Define sector names */
    const char* sectorNames[] = {
        "Technology", "Financial", "Healthcare", "Consumer", "Industrial", 
        "Energy", "Materials", "Real Estate", "Utilities", "Communication"
    };
    
    /* For now, use a simplified approach based on keywords in the event title and description */
    int affectedCount = 0;
    int totalLen = 0;
    
    /* Check each sector for keywords */
    for (int i = 0; i < sectorCount && i < (int)(sizeof(sectorNames) / sizeof(sectorNames[0])); i++) {
        int affected = 0;
        
        /* Check title for sector-specific keywords */
        if (strstr(event->title, sectorNames[i])) {
            affected = 1;
        }
        
        /* Check description for sector-specific keywords */
        if (!affected && strstr(event->description, sectorNames[i])) {
            affected = 1;
        }
        
        /* Additional sector-specific keywords */
        switch (i) {
            case 0: /* Technology */
                if (!affected && (strstr(event->title, "Tech") || 
                                 strstr(event->description, "Tech") ||
                                 strstr(event->title, "Software") || 
                                 strstr(event->description, "Software") ||
                                 strstr(event->title, "Hardware") || 
                                 strstr(event->description, "Hardware"))) {
                    affected = 1;
                }
                break;
            case 1: /* Financial */
                if (!affected && (strstr(event->title, "Bank") || 
                                 strstr(event->description, "Bank") ||
                                 strstr(event->title, "Finance") || 
                                 strstr(event->description, "Finance") ||
                                 strstr(event->title, "Insurance") || 
                                 strstr(event->description, "Insurance"))) {
                    affected = 1;
                }
                break;
            case 2: /* Healthcare */
                if (!affected && (strstr(event->title, "Health") || 
                                 strstr(event->description, "Health") ||
                                 strstr(event->title, "Medical") || 
                                 strstr(event->description, "Medical") ||
                                 strstr(event->title, "Pharma") || 
                                 strstr(event->description, "Pharma"))) {
                    affected = 1;
                }
                break;
            /* Add other sectors with their keywords */
        }
        
        /* If affected, add to the list */
        if (affected) {
            int sectorNameLen = strlen(sectorNames[i]);
            
            /* Check if we have enough space (including comma and space) */
            if (totalLen + sectorNameLen + 2 < 255) {
                /* Add comma if not the first sector */
                if (affectedCount > 0) {
                    strcat(affectedSectors, ", ");
                    totalLen += 2;
                }
                
                /* Add sector name */
                strcat(affectedSectors, sectorNames[i]);
                totalLen += sectorNameLen;
                affectedCount++;
            }
        }
    }
    
    /* If no sectors were identified, set a default message */
    if (affectedCount == 0) {
        strcpy(affectedSectors, "General Market");
    }
}

/* Calculate impact on a specific sector */
double calculateSectorImpact(const EventData* event, const Stock* sectorIndex) {
    if (!event || !sectorIndex) {
        return 0.0;
    }
    
    /* For now, just return a value based on the event's impact score */
    /* In a real implementation, this would analyze the correlation between 
       the event and the sector's historical performance */
    return event->impactScore / 100.0;
} 