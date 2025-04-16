/**
 * Event Analysis Module
 * Header file for event detection and analysis functions
 */

#ifndef EVENT_ANALYSIS_H
#define EVENT_ANALYSIS_H

#include "emers.h"

/* Constants for event analysis */
#define MAX_EVENTS 100
#define EVENT_DETECTION_THRESHOLD 0.05  // 5% price movement
#define EVENT_IMPACT_WINDOW 5           // Days to analyze impact
#define MIN_EVENT_SCORE 30              // Minimum score to consider an event significant

/* Event severity */
typedef enum {
    EVENT_SEVERITY_LOW = 1,
    EVENT_SEVERITY_MEDIUM,
    EVENT_SEVERITY_HIGH,
    EVENT_SEVERITY_CRITICAL
} EventSeverity;

/* Extended event data structure */
typedef struct {
    EventData basicData;
    EventType type;
    EventSeverity severity;
    double marketImpact;          // Quantified market impact (-1.0 to 1.0)
    double abnormalReturn;        // Abnormal return calculation
    double volatilityChange;      // Change in volatility
    char affectedSectors[256];    // Comma-separated list of affected sectors
    int durationEstimate;         // Estimated duration in days
} DetailedEventData;

/* Function prototypes */

/* Event detection */
int detectMarketEvents(const Stock* stocks, int stockCount, const EventDatabase* newsEvents, EventData* detectedEvents);
int detectVolatilitySpike(const StockData* data, int dataSize, double threshold);
int detectPriceMovement(const StockData* data, int dataSize, double threshold);
int detectVolumeSpike(const StockData* data, int dataSize, double threshold);

/* Event analysis */
double calculateEventImpact(const EventData* event, const Stock* stocks, int stockCount);
EventType classifyEvent(const EventData* event);
EventSeverity assessEventSeverity(const EventData* event, const Stock* stocks, int stockCount);
double calculateAbnormalReturn(const Stock* stock, const char* eventDate, int window);
double calculateVolatilityChange(const Stock* stock, const char* eventDate, int preWindow, int postWindow);

/* Sector analysis */
void identifyAffectedSectors(const EventData* event, const Stock** sectorIndices, int sectorCount, char* affectedSectors);
double calculateSectorImpact(const EventData* event, const Stock* sectorIndex);

/* Event response */
void generateEventReport(const DetailedEventData* event, char* report, int reportSize);
void recommendDefensiveStrategy(const DetailedEventData* event, const Stock* stocks, int stockCount, char* strategy, int strategySize);

/* Event database management */
int addEvent(EventDatabase* db, const EventData* event);
int findSimilarEvents(const EventData* event, const EventDatabase* db, int* similarEventIndices, int maxResults);
double calculateEventSimilarity(const EventData* event1, const EventData* event2);

#endif /* EVENT_ANALYSIS_H */ 