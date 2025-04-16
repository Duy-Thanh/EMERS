/**
 * Emergency Market Event Response System (EMERS)
 * Event Database Module - Persistence of market events
 */

#ifndef EVENT_DATABASE_H
#define EVENT_DATABASE_H

#include "emers.h"
#include <stdio.h>

/* Define EVENT_TYPE_COUNT based on the UNKNOWN_EVENT enum value + 1 */
#define EVENT_TYPE_COUNT (UNKNOWN_EVENT + 1)

/* Database file paths */
#define EVENT_DB_FILENAME "events.db"
#define EVENT_DB_BACKUP "events.db.bak"
#define EVENT_DB_TEMP "events.db.tmp"

/* Initialize the event database */
int initEventDatabase(void);

/* Cleanup the event database */
void cleanupEventDatabase(void);

/* Save events to the database file */
int saveEventsToDatabase(const EventDatabase* eventDB);

/* Load events from the database file */
int loadEventsFromDatabase(EventDatabase* eventDB);

/* Add a new event to the database */
int addEventToDatabase(EventDatabase* eventDB, const EventData* event);

/* Search for events by date range */
int findEventsByDateRange(const EventDatabase* eventDB, const char* startDate, 
                         const char* endDate, EventData** results, int* resultCount);

/* Search for events by type */
int findEventsByType(const EventDatabase* eventDB, EventType type, 
                    EventData** results, int* resultCount);

/* Get statistics about the event database */
typedef struct {
    int totalEvents;
    int eventsByType[EVENT_TYPE_COUNT];
    int eventsInLastMonth;
    int eventsInLastYear;
    char oldestEventDate[MAX_DATE_LENGTH];
    char newestEventDate[MAX_DATE_LENGTH];
} EventDBStats;

int getEventDatabaseStats(const EventDatabase* eventDB, EventDBStats* stats);

/* Backup the event database */
int backupEventDatabase(void);

/* Restore from backup */
int restoreEventDatabaseFromBackup(void);

#endif /* EVENT_DATABASE_H */ 