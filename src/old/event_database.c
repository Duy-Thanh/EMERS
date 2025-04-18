/**
 * Emergency Market Event Response System (EMERS)
 * Event Database Module Implementation
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../include/emers.h"
#include "../include/event_database.h"
#include "../include/error_handling.h"

/* File format version */
#define DB_VERSION 1

/* Static variables */
static int isInitialized = 0;

/* Initialize the event database */
int initEventDatabase(void) {
    isInitialized = 1;
    logMessage(LOG_INFO, "Event database initialized");
    return 1;
}

/* Cleanup the event database */
void cleanupEventDatabase(void) {
    if (isInitialized) {
        logMessage(LOG_INFO, "Event database cleaned up");
        isInitialized = 0;
    }
}

/* Save events to the database file */
int saveEventsToDatabase(const EventDatabase* eventDB) {
    if (!eventDB) {
        logError(ERR_INVALID_PARAMETER, "NULL event database pointer in saveEventsToDatabase");
        return 0;
    }
    
    FILE* file = fopen(EVENT_DB_FILENAME, "wb");
    if (!file) {
        logError(ERR_FILE_OPEN_FAILED, "Failed to open file '%s' for writing", EVENT_DB_FILENAME);
        return 0;
    }
    
    /* Write header information */
    int version = DB_VERSION;
    fwrite(&version, sizeof(int), 1, file);
    
    /* Write the number of events */
    fwrite(&eventDB->eventCount, sizeof(int), 1, file);
    
    /* Write all events */
    for (int i = 0; i < eventDB->eventCount; i++) {
        fwrite(&eventDB->events[i], sizeof(EventData), 1, file);
    }
    
    fclose(file);
    logMessage(LOG_INFO, "Saved %d events to database", eventDB->eventCount);
    
    return 1;
}

/* Load events from the database file */
int loadEventsFromDatabase(EventDatabase* eventDB) {
    if (!eventDB) {
        logError(ERR_INVALID_PARAMETER, "NULL event database pointer in loadEventsFromDatabase");
        return 0;
    }
    
    /* Reset the event database */
    if (eventDB->events) {
        free(eventDB->events);
        eventDB->events = NULL;
    }
    eventDB->eventCount = 0;
    eventDB->eventCapacity = 0;
    
    FILE* file = fopen(EVENT_DB_FILENAME, "rb");
    if (!file) {
        /* Not necessarily an error, could be a new database */
        logMessage(LOG_INFO, "No existing database file found");
        return 1;
    }
    
    /* Read and verify header information */
    int version;
    if (fread(&version, sizeof(int), 1, file) != 1 || version != DB_VERSION) {
        logError(ERR_DATA_CORRUPTED, "Invalid database version: %d", version);
        fclose(file);
        return 0;
    }
    
    /* Read the number of events */
    int count;
    if (fread(&count, sizeof(int), 1, file) != 1) {
        logError(ERR_FILE_READ_FAILED, "Failed to read event count from database");
        fclose(file);
        return 0;
    }
    
    /* Allocate memory for events */
    if (count > 0) {
        eventDB->events = (EventData*)malloc(count * sizeof(EventData));
        if (!eventDB->events) {
            logError(ERR_OUT_OF_MEMORY, "Failed to allocate memory for %d events", count);
            fclose(file);
            return 0;
        }
        
        /* Read all events */
        if (fread(eventDB->events, sizeof(EventData), count, file) != (size_t)count) {
            logError(ERR_FILE_READ_FAILED, "Failed to read all events from database");
            free(eventDB->events);
            eventDB->events = NULL;
            fclose(file);
            return 0;
        }
        
        eventDB->eventCount = count;
        eventDB->eventCapacity = count;
    }
    
    fclose(file);
    logMessage(LOG_INFO, "Loaded %d events from database", count);
    
    return 1;
}

/* Add a new event to the database */
int addEventToDatabase(EventDatabase* eventDB, const EventData* event) {
    if (!eventDB || !event) {
        logError(ERR_INVALID_PARAMETER, "NULL pointer in addEventToDatabase");
        return 0;
    }
    
    /* Check if the database needs to be expanded */
    if (eventDB->eventCount >= eventDB->eventCapacity) {
        int newCapacity = (eventDB->eventCapacity == 0) ? 10 : eventDB->eventCapacity * 2;
        EventData* newEvents = (EventData*)realloc(eventDB->events, 
                                                 newCapacity * sizeof(EventData));
        if (!newEvents) {
            logError(ERR_OUT_OF_MEMORY, "Failed to expand event database");
            return 0;
        }
        
        eventDB->events = newEvents;
        eventDB->eventCapacity = newCapacity;
    }
    
    /* Add the event to the database */
    eventDB->events[eventDB->eventCount] = *event;
    eventDB->eventCount++;
    
    logMessage(LOG_INFO, "Added new event to database: %s", event->description);
    
    return 1;
}

/* Helper function to compare dates (YYYY-MM-DD format) */
static int compareDates(const char* date1, const char* date2) {
    return strcmp(date1, date2);
}

/* Search for events by date range */
int findEventsByDateRange(const EventDatabase* eventDB, const char* startDate, 
                        const char* endDate, EventData** results, int* resultCount) {
    if (!eventDB || !startDate || !endDate || !results || !resultCount) {
        logError(ERR_INVALID_PARAMETER, "NULL pointer in findEventsByDateRange");
        return 0;
    }
    
    /* Initialize result count */
    *resultCount = 0;
    
    /* Allocate temporary buffer for results (maximum is all events) */
    EventData* tempResults = (EventData*)malloc(eventDB->eventCount * sizeof(EventData));
    if (!tempResults) {
        logError(ERR_OUT_OF_MEMORY, "Failed to allocate memory for event search results");
        return 0;
    }
    
    /* Filter events by date range */
    for (int i = 0; i < eventDB->eventCount; i++) {
        if (compareDates(eventDB->events[i].date, startDate) >= 0 &&
            compareDates(eventDB->events[i].date, endDate) <= 0) {
            tempResults[*resultCount] = eventDB->events[i];
            (*resultCount)++;
        }
    }
    
    /* Allocate final result array of exact size */
    if (*resultCount > 0) {
        *results = (EventData*)malloc(*resultCount * sizeof(EventData));
        if (!*results) {
            logError(ERR_OUT_OF_MEMORY, "Failed to allocate memory for final event results");
            free(tempResults);
            *resultCount = 0;
            return 0;
        }
        
        /* Copy results */
        memcpy(*results, tempResults, *resultCount * sizeof(EventData));
    } else {
        *results = NULL;
    }
    
    free(tempResults);
    
    logMessage(LOG_INFO, "Found %d events in date range %s to %s", 
              *resultCount, startDate, endDate);
    
    return 1;
}

/* Search for events by type */
int findEventsByType(const EventDatabase* eventDB, EventType type, 
                   EventData** results, int* resultCount) {
    if (!eventDB || !results || !resultCount) {
        logError(ERR_INVALID_PARAMETER, "NULL pointer in findEventsByType");
        return 0;
    }
    
    /* Initialize result count */
    *resultCount = 0;
    
    /* Allocate temporary buffer for results (maximum is all events) */
    EventData* tempResults = (EventData*)malloc(eventDB->eventCount * sizeof(EventData));
    if (!tempResults) {
        logError(ERR_OUT_OF_MEMORY, "Failed to allocate memory for event search results");
        return 0;
    }
    
    /* Filter events by type */
    for (int i = 0; i < eventDB->eventCount; i++) {
        /* In this implementation, we need to classify events on-the-fly */
        /* since EventData doesn't directly store event type */
        EventType eventType = classifyEvent(&eventDB->events[i]);
        if (eventType == type) {
            tempResults[*resultCount] = eventDB->events[i];
            (*resultCount)++;
        }
    }
    
    /* Allocate final result array of exact size */
    if (*resultCount > 0) {
        *results = (EventData*)malloc(*resultCount * sizeof(EventData));
        if (!*results) {
            logError(ERR_OUT_OF_MEMORY, "Failed to allocate memory for final event results");
            free(tempResults);
            *resultCount = 0;
            return 0;
        }
        
        /* Copy results */
        memcpy(*results, tempResults, *resultCount * sizeof(EventData));
    } else {
        *results = NULL;
    }
    
    free(tempResults);
    
    logMessage(LOG_INFO, "Found %d events of type %d", *resultCount, type);
    
    return 1;
}

/* Get statistics about the event database */
int getEventDatabaseStats(const EventDatabase* eventDB, EventDBStats* stats) {
    if (!eventDB || !stats) {
        logError(ERR_INVALID_PARAMETER, "NULL pointer in getEventDatabaseStats");
        return 0;
    }
    
    /* Initialize stats */
    memset(stats, 0, sizeof(EventDBStats));
    
    stats->totalEvents = eventDB->eventCount;
    
    if (eventDB->eventCount == 0) {
        return 1;  /* No events to analyze */
    }
    
    /* Get current time for date comparisons */
    time_t now;
    struct tm* timeinfo;
    char currentDate[MAX_DATE_LENGTH];
    
    time(&now);
    timeinfo = localtime(&now);
    strftime(currentDate, MAX_DATE_LENGTH, "%Y-%m-%d", timeinfo);
    
    /* Calculate one month and one year ago */
    struct tm oneMonthAgo = *timeinfo;
    struct tm oneYearAgo = *timeinfo;
    
    oneMonthAgo.tm_mon -= 1;
    if (oneMonthAgo.tm_mon < 0) {
        oneMonthAgo.tm_mon += 12;
        oneMonthAgo.tm_year -= 1;
    }
    
    oneYearAgo.tm_year -= 1;
    
    char oneMonthAgoStr[MAX_DATE_LENGTH];
    char oneYearAgoStr[MAX_DATE_LENGTH];
    
    strftime(oneMonthAgoStr, MAX_DATE_LENGTH, "%Y-%m-%d", &oneMonthAgo);
    strftime(oneYearAgoStr, MAX_DATE_LENGTH, "%Y-%m-%d", &oneYearAgo);
    
    /* Initialize oldest/newest dates to the first event */
    strcpy(stats->oldestEventDate, eventDB->events[0].date);
    strcpy(stats->newestEventDate, eventDB->events[0].date);
    
    /* Calculate statistics */
    for (int i = 0; i < eventDB->eventCount; i++) {
        /* Count by event type */
        EventType eventType = classifyEvent(&eventDB->events[i]);
        if (eventType < EVENT_TYPE_COUNT) {
            stats->eventsByType[eventType]++;
        }
        
        /* Count recent events */
        if (compareDates(eventDB->events[i].date, oneMonthAgoStr) >= 0) {
            stats->eventsInLastMonth++;
        }
        
        if (compareDates(eventDB->events[i].date, oneYearAgoStr) >= 0) {
            stats->eventsInLastYear++;
        }
        
        /* Track oldest and newest events */
        if (compareDates(eventDB->events[i].date, stats->oldestEventDate) < 0) {
            strcpy(stats->oldestEventDate, eventDB->events[i].date);
        }
        
        if (compareDates(eventDB->events[i].date, stats->newestEventDate) > 0) {
            strcpy(stats->newestEventDate, eventDB->events[i].date);
        }
    }
    
    return 1;
}

/* Backup the event database */
int backupEventDatabase(void) {
    FILE* srcFile = fopen(EVENT_DB_FILENAME, "rb");
    if (!srcFile) {
        logError(ERR_FILE_OPEN_FAILED, "Failed to open source database for backup");
        return 0;
    }
    
    FILE* destFile = fopen(EVENT_DB_BACKUP, "wb");
    if (!destFile) {
        logError(ERR_FILE_OPEN_FAILED, "Failed to open backup file for writing");
        fclose(srcFile);
        return 0;
    }
    
    /* Copy file contents */
    char buffer[4096];
    size_t bytesRead;
    
    while ((bytesRead = fread(buffer, 1, sizeof(buffer), srcFile)) > 0) {
        if (fwrite(buffer, 1, bytesRead, destFile) != bytesRead) {
            logError(ERR_FILE_WRITE_FAILED, "Failed to write to backup file");
            fclose(srcFile);
            fclose(destFile);
            return 0;
        }
    }
    
    fclose(srcFile);
    fclose(destFile);
    
    logMessage(LOG_INFO, "Event database backed up to %s", EVENT_DB_BACKUP);
    
    return 1;
}

/* Restore from backup */
int restoreEventDatabaseFromBackup(void) {
    FILE* srcFile = fopen(EVENT_DB_BACKUP, "rb");
    if (!srcFile) {
        logError(ERR_FILE_OPEN_FAILED, "Failed to open backup file for restore");
        return 0;
    }
    
    FILE* destFile = fopen(EVENT_DB_FILENAME, "wb");
    if (!destFile) {
        logError(ERR_FILE_OPEN_FAILED, "Failed to open database file for writing");
        fclose(srcFile);
        return 0;
    }
    
    /* Copy file contents */
    char buffer[4096];
    size_t bytesRead;
    
    while ((bytesRead = fread(buffer, 1, sizeof(buffer), srcFile)) > 0) {
        if (fwrite(buffer, 1, bytesRead, destFile) != bytesRead) {
            logError(ERR_FILE_WRITE_FAILED, "Failed to write to database file");
            fclose(srcFile);
            fclose(destFile);
            return 0;
        }
    }
    
    fclose(srcFile);
    fclose(destFile);
    
    logMessage(LOG_INFO, "Event database restored from backup");
    
    return 1;
} 