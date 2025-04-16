/**
 * Utility Functions
 * Implementation of utility functions for the EMERS system
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../include/emers.h"

/* Initialize a stock structure */
void initializeStock(Stock* stock, const char* symbol) {
    if (!stock || !symbol) {
        return;
    }
    
    /* Copy the symbol */
    strncpy(stock->symbol, symbol, MAX_SYMBOL_LENGTH - 1);
    stock->symbol[MAX_SYMBOL_LENGTH - 1] = '\0';
    
    /* Initialize data array */
    stock->data = NULL;
    stock->dataSize = 0;
    stock->dataCapacity = 0;
}

/* Free memory used by a stock structure */
void freeStock(Stock* stock) {
    if (!stock) {
        return;
    }
    
    /* Free the data array */
    if (stock->data) {
        free(stock->data);
        stock->data = NULL;
    }
    
    stock->dataSize = 0;
    stock->dataCapacity = 0;
}

/* Initialize an event database structure */
void initializeEventDatabase(EventDatabase* db) {
    if (!db) {
        return;
    }
    
    /* Initialize the events array */
    db->events = NULL;
    db->eventCount = 0;
    db->eventCapacity = 0;
}

/* Free memory used by an event database structure */
void freeEventDatabase(EventDatabase* db) {
    if (!db) {
        return;
    }
    
    /* Free the events array */
    if (db->events) {
        free(db->events);
        db->events = NULL;
    }
    
    db->eventCount = 0;
    db->eventCapacity = 0;
}

/* Get the current date as a string (YYYY-MM-DD) */
char* getCurrentDate(char* buffer) {
    if (!buffer) {
        return NULL;
    }
    
    /* Get current time */
    time_t now = time(NULL);
    struct tm* timeinfo = localtime(&now);
    
    /* Format date as YYYY-MM-DD */
    strftime(buffer, MAX_DATE_LENGTH, "%Y-%m-%d", timeinfo);
    
    return buffer;
}

/* Get a date N days in the past as a string (YYYY-MM-DD) */
char* getPastDate(char* buffer, int daysBack) {
    if (!buffer) {
        return NULL;
    }
    
    /* Get current time */
    time_t now = time(NULL);
    struct tm* timeinfo = localtime(&now);
    
    /* Subtract daysBack days */
    timeinfo->tm_mday -= daysBack;
    mktime(timeinfo); /* Normalize date (handle month/year boundaries) */
    
    /* Format date as YYYY-MM-DD */
    strftime(buffer, MAX_DATE_LENGTH, "%Y-%m-%d", timeinfo);
    
    return buffer;
}

/* Compare two dates in YYYY-MM-DD format */
int compareDates(const char* date1, const char* date2) {
    /* Parse first date */
    int year1, month1, day1;
    sscanf(date1, "%d-%d-%d", &year1, &month1, &day1);
    
    /* Parse second date */
    int year2, month2, day2;
    sscanf(date2, "%d-%d-%d", &year2, &month2, &day2);
    
    /* Compare years */
    if (year1 < year2) return -1;
    if (year1 > year2) return 1;
    
    /* Same year, compare months */
    if (month1 < month2) return -1;
    if (month1 > month2) return 1;
    
    /* Same year and month, compare days */
    if (day1 < day2) return -1;
    if (day1 > day2) return 1;
    
    /* Dates are equal */
    return 0;
}

/* Calculate the number of days between two dates */
int daysBetween(const char* startDate, const char* endDate) {
    /* Parse start date */
    struct tm start = {0};
    sscanf(startDate, "%d-%d-%d", &start.tm_year, &start.tm_mon, &start.tm_mday);
    start.tm_year -= 1900; /* Adjust for tm_year base (years since 1900) */
    start.tm_mon -= 1;     /* Adjust for tm_mon range (0-11) */
    
    /* Parse end date */
    struct tm end = {0};
    sscanf(endDate, "%d-%d-%d", &end.tm_year, &end.tm_mon, &end.tm_mday);
    end.tm_year -= 1900;
    end.tm_mon -= 1;
    
    /* Convert to time_t (seconds since epoch) */
    time_t startTime = mktime(&start);
    time_t endTime = mktime(&end);
    
    /* Calculate difference in seconds and convert to days */
    double diffSeconds = difftime(endTime, startTime);
    return (int)(diffSeconds / (60 * 60 * 24));
}

/* Find a stock by symbol in an array of stocks */
Stock* findStockBySymbol(Stock* stocks, int stockCount, const char* symbol) {
    if (!stocks || stockCount <= 0 || !symbol) {
        return NULL;
    }
    
    int i;
    for (i = 0; i < stockCount; i++) {
        if (strcmp(stocks[i].symbol, symbol) == 0) {
            return &stocks[i];
        }
    }
    
    return NULL;
}

/* Format a number with commas for thousands */
void formatNumberWithCommas(double value, char* buffer, int bufferSize) {
    if (!buffer || bufferSize <= 0) {
        return;
    }
    
    /* Convert to fixed-point notation */
    char tempBuf[128];
    snprintf(tempBuf, sizeof(tempBuf), "%.2f", value);
    
    /* Find decimal point */
    char* decimalPoint = strchr(tempBuf, '.');
    int integerLen = (decimalPoint) ? (decimalPoint - tempBuf) : strlen(tempBuf);
    
    /* Insert commas */
    int j = 0;
    int k;
    for (k = 0; k < integerLen; k++) {
        /* Add comma every three digits (except at the beginning) */
        if (k > 0 && (integerLen - k) % 3 == 0) {
            if (j < bufferSize - 1) {
                buffer[j++] = ',';
            }
        }
        
        /* Copy digit */
        if (j < bufferSize - 1) {
            buffer[j++] = tempBuf[k];
        }
    }
    
    /* Copy decimal part if exists */
    if (decimalPoint) {
        while (*decimalPoint && j < bufferSize - 1) {
            buffer[j++] = *decimalPoint++;
        }
    }
    
    buffer[j] = '\0';
}

/* Convert a date string to a readable format */
void formatDateString(const char* dateStr, char* buffer, int bufferSize) {
    if (!dateStr || !buffer || bufferSize <= 0) {
        if (buffer && bufferSize > 0) {
            buffer[0] = '\0';
        }
        return;
    }
    
    /* Parse date string (YYYY-MM-DD) */
    int year, month, day;
    if (sscanf(dateStr, "%d-%d-%d", &year, &month, &day) != 3) {
        /* Invalid format, just copy the original */
        strncpy(buffer, dateStr, bufferSize - 1);
        buffer[bufferSize - 1] = '\0';
        return;
    }
    
    /* Create a time structure */
    struct tm timeinfo = {0};
    timeinfo.tm_year = year - 1900;
    timeinfo.tm_mon = month - 1;
    timeinfo.tm_mday = day;
    
    /* Format date string */
    strftime(buffer, bufferSize, "%B %d, %Y", &timeinfo);
}

/* Create a simple ASCII chart of stock prices */
void createAsciiChart(const StockData* data, int dataSize, int width, int height, char* buffer, int bufferSize) {
    if (!data || dataSize <= 0 || !buffer || bufferSize <= 0) {
        if (buffer && bufferSize > 0) {
            buffer[0] = '\0';
        }
        return;
    }
    
    /* Find min and max closing prices */
    double min = data[0].close;
    double max = data[0].close;
    int i, j;
    
    for (i = 1; i < dataSize; i++) {
        if (data[i].close < min) min = data[i].close;
        if (data[i].close > max) max = data[i].close;
    }
    
    /* Add padding to min and max */
    double range = max - min;
    min -= range * 0.1;
    max += range * 0.1;
    range = max - min;
    
    /* Create chart */
    char chart[100][100]; /* Maximum size */
    
    /* Initialize chart space */
    for (i = 0; i < height; i++) {
        for (j = 0; j < width; j++) {
            chart[i][j] = ' ';
        }
        chart[i][width] = '\0';
    }
    
    /* Draw price levels */
    for (i = 0; i < height; i++) {
        chart[i][0] = '|';
    }
    
    /* Draw time axis */
    for (j = 0; j < width; j++) {
        chart[height-1][j] = '-';
    }
    
    /* Plot data points */
    for (i = 0; i < dataSize && i < width - 1; i++) {
        double normalizedPrice = (data[i].close - min) / range;
        int y = height - 1 - (int)(normalizedPrice * (height - 2));
        
        if (y >= 0 && y < height) {
            chart[y][i + 1] = '*';
        }
    }
    
    /* Copy chart to output buffer */
    int pos = 0;
    for (i = 0; i < height && pos < bufferSize - 2; i++) {
        int len = strlen(chart[i]);
        if (pos + len >= bufferSize - 2) {
            len = bufferSize - pos - 2;
        }
        
        if (len > 0) {
            memcpy(buffer + pos, chart[i], len);
            pos += len;
        }
        
        if (pos < bufferSize - 2) {
            buffer[pos++] = '\n';
        }
    }
    
    buffer[pos] = '\0';
} 