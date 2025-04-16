/**
 * Tiingo API Integration Module
 * Implementation file for Tiingo API functions
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <process.h>  /* For _popen() */
#include <direct.h>   /* For _mkdir() on Windows */
#include <sys/stat.h> /* For stat() */

#include "../include/emers.h"
#include "../include/tiingo_api.h"
#include "../include/error_handling.h"  /* Added error_handling.h for logAPIError */

/* Static variables */
static char apiKey[MAX_API_KEY_LENGTH] = "";
static int isInitialized = 0;

/* Initialize the Tiingo API */
int initializeTiingoAPI(const char* key) {
    if (!key || strlen(key) == 0) {
        printf("Error: API key is required for initialization.\n");
        return 0;
    }
    
    /* Store the API key */
    strncpy(apiKey, key, MAX_API_KEY_LENGTH - 1);
    apiKey[MAX_API_KEY_LENGTH - 1] = '\0';
    
    /* Check if curl is available */
    FILE* fp = _popen("curl --version", "r");
    if (!fp) {
        printf("Error: curl command not found. Please install curl.\n");
        return 0;
    }
    
    char buffer[256];
    if (fgets(buffer, sizeof(buffer), fp) == NULL) {
        printf("Error: Failed to read curl version output.\n");
        _pclose(fp);
        return 0;
    }
    _pclose(fp);
    
    /* Check if the output contains "curl" */
    if (strstr(buffer, "curl") == NULL) {
        printf("Error: curl command not found. Please install curl.\n");
        return 0;
    }
    
    /* Create data directory if it doesn't exist */
    struct stat st = {0};
    if (stat(CSV_DATA_DIRECTORY, &st) == -1) {
        /* Directory doesn't exist, create it */
        if (_mkdir(CSV_DATA_DIRECTORY) != 0) {
            printf("Error: Failed to create data directory.\n");
            return 0;
        }
        logMessage(LOG_INFO, "Created data directory for CSV cache: %s", CSV_DATA_DIRECTORY);
    }
    
    isInitialized = 1;
    return 1;
}

/* Set the Tiingo API key */
void setTiingoAPIKey(const char* key) {
    if (key && strlen(key) > 0) {
        strncpy(apiKey, key, MAX_API_KEY_LENGTH - 1);
        apiKey[MAX_API_KEY_LENGTH - 1] = '\0';
    }
}

/* Get the Tiingo API key */
const char* getTiingoAPIKey(void) {
    return apiKey;
}

/* Build a complete API URL with endpoint and parameters */
char* buildAPIUrl(const char* endpoint, const char* params) {
    /* Calculate required buffer size */
    size_t urlLen = strlen(TIINGO_API_BASE_URL) + strlen(endpoint);
    if (params) {
        urlLen += 1 + strlen(params); /* +1 for '?' */
    }
    urlLen += 1; /* +1 for null-terminator */
    
    /* Allocate memory for URL */
    char* url = (char*)malloc(urlLen);
    if (!url) {
        printf("Error: Memory allocation failed for URL\n");
        return NULL;
    }
    
    /* Build URL */
    strcpy(url, TIINGO_API_BASE_URL);
    strcat(url, endpoint);
    
    if (params && strlen(params) > 0) {
        strcat(url, "?");
        strcat(url, params);
    }
    
    return url;
}

/* Perform an API request to Tiingo using curl command */
int performAPIRequest(const char* url, Memory* response) {
    if (!isInitialized) {
        printf("Error: Tiingo API not initialized. Call initializeTiingoAPI() first.\n");
        return 0;
    }
    
    /* Initialize response memory */
    response->data = malloc(1);
    if (!response->data) {
        printf("Error: Memory allocation failed for response\n");
        return 0;
    }
    response->size = 0;
    response->data[0] = '\0';
    
    /* Build curl command */
    char command[MAX_BUFFER_SIZE];
    snprintf(command, sizeof(command),
             "curl -s -H \"Authorization: Token %s\" -H \"Content-Type: application/json\" \"%s\" > curl_output.json",
             apiKey, url);
    
    /* Execute curl command */
    int result = system(command);
    if (result != 0) {
        printf("Error: Failed to execute curl command. Error code: %d\n", result);
        free(response->data);
        response->data = NULL;
        return 0;
    }
    
    /* Read the output file */
    FILE* fp = fopen("curl_output.json", "rb");
    if (!fp) {
        printf("Error: Failed to open curl output file.\n");
        free(response->data);
        response->data = NULL;
        return 0;
    }
    
    /* Get file size */
    fseek(fp, 0, SEEK_END);
    long fileSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    
    if (fileSize <= 0) {
        printf("Error: Empty response from API or failed request.\n");
        fclose(fp);
        free(response->data);
        response->data = NULL;
        return 0;
    }
    
    /* Allocate memory for response data */
    char* newData = (char*)realloc(response->data, fileSize + 1);
    if (!newData) {
        printf("Error: Memory reallocation failed for response\n");
        fclose(fp);
        free(response->data);
        response->data = NULL;
        return 0;
    }
    response->data = newData;
    
    /* Read file content */
    size_t bytesRead = fread(response->data, 1, fileSize, fp);
    fclose(fp);
    
    if (bytesRead != (size_t)fileSize) {
        printf("Error: Failed to read complete curl output. Expected %ld bytes, got %zu bytes.\n", 
               fileSize, bytesRead);
        free(response->data);
        response->data = NULL;
        return 0;
    }
    
    /* Null-terminate the response data */
    response->data[bytesRead] = '\0';
    response->size = bytesRead;
    
    /* Check for HTTP errors in the response */
    if (strstr(response->data, "\"error\"") != NULL || 
        strstr(response->data, "\"detail\"") != NULL) {
        printf("Error: API returned an error response:\n%s\n", response->data);
        
        /* Check for specific permission error */
        if (strstr(response->data, "You do not have permission") != NULL) {
            logAPIError("API permission error: Your API key doesn't have access to this feature", url, 403);
        } else {
            logAPIError("API error response", url, 0);
        }
        
        free(response->data);
        response->data = NULL;
        return 0;
    }
    
    return 1;
}

/* Fetch stock data for a specific symbol and date range */
int fetchStockData(const char* symbol, const char* startDate, const char* endDate, Stock* stock) {
    if (!symbol || !startDate || !endDate || !stock) {
        printf("Error: Invalid parameters for fetchStockData\n");
        return 0;
    }
    
    /* Build API endpoint */
    char endpoint[MAX_URL_LENGTH];
    snprintf(endpoint, MAX_URL_LENGTH, "%s/%s/prices", TIINGO_API_DAILY_URL, symbol);
    
    /* Build parameters */
    char params[MAX_URL_LENGTH];
    snprintf(params, MAX_URL_LENGTH, "startDate=%s&endDate=%s&format=json", startDate, endDate);
    
    /* Build full URL */
    char* url = buildAPIUrl(endpoint, params);
    if (!url) {
        return 0;
    }
    
    /* Perform API request */
    Memory response;
    int success = performAPIRequest(url, &response);
    free(url);
    
    if (!success || !response.data) {
        return 0;
    }
    
    /* Parse JSON response */
    success = parseStockDataJSON(response.data, stock);
    
    /* Clean up */
    free(response.data);
    
    return success;
}

/* Fetch news data for a list of symbols */
int fetchNewsFeed(const char* symbols, EventDatabase* events) {
    if (!symbols || !events) {
        printf("Error: Invalid parameters for fetchNewsFeed\n");
        return 0;
    }
    
    /* Build parameters */
    char params[MAX_URL_LENGTH];
    snprintf(params, MAX_URL_LENGTH, "tickers=%s&limit=50&format=json", symbols);
    
    /* Build full URL */
    char* url = buildAPIUrl(TIINGO_API_NEWS_URL, params);
    if (!url) {
        return 0;
    }
    
    /* Perform API request */
    Memory response;
    int success = performAPIRequest(url, &response);
    free(url);
    
    if (!success || !response.data) {
        /* If we have response data, free it before returning */
        if (response.data) {
            free(response.data);
        }
        return 0;
    }
    
    /* Parse JSON response */
    success = parseNewsDataJSON(response.data, events);
    
    /* Clean up */
    free(response.data);
    
    return success;
}

/* Parse stock data JSON response */
int parseStockDataJSON(const char* jsonData, Stock* stock) {
    if (!jsonData || !stock) {
        logError(ERR_INVALID_PARAMETER, "Invalid parameters for parseStockDataJSON");
        return 0;
    }
    
    /* Initialize stock data array if needed */
    if (!stock->data) {
        stock->dataCapacity = 4000; /* Initial capacity for 10+ years of data */
        stock->data = (StockData*)malloc(stock->dataCapacity * sizeof(StockData));
        if (!stock->data) {
            logError(ERR_OUT_OF_MEMORY, "Failed to allocate memory for stock data");
            return 0;
        }
        stock->dataSize = 0;
    }
    
    /* Simple JSON parser for Tiingo API response format */
    /* Format: [{"date":"2025-04-16", "open":123.45, "high":125.67, ...}, {...}, ...] */
    
    /* Find the beginning of the array */
    const char* pos = strchr(jsonData, '[');
    if (!pos) {
        logError(ERR_DATA_CORRUPTED, "Invalid JSON format: array not found");
        return 0;
    }
    
    /* Iterate through each object in the array */
    while ((pos = strstr(pos, "{"))) {
        /* Check if we need to resize the array */
        if (stock->dataSize >= stock->dataCapacity) {
            stock->dataCapacity *= 2;
            StockData* newData = (StockData*)realloc(stock->data, stock->dataCapacity * sizeof(StockData));
            if (!newData) {
                logError(ERR_OUT_OF_MEMORY, "Memory reallocation failed for stock data");
                return 0;
            }
            stock->data = newData;
        }
        
        /* Parse each field of the object */
        StockData* dataPoint = &stock->data[stock->dataSize];
        
        /* Extract date */
        const char* dateStart = strstr(pos, "\"date\"");
        if (!dateStart) break;
        dateStart = strchr(dateStart, ':');
        if (!dateStart) break;
        dateStart = strchr(dateStart, '"');
        if (!dateStart) break;
        dateStart++;
        const char* dateEnd = strchr(dateStart, '"');
        if (!dateEnd || (dateEnd - dateStart) >= MAX_DATE_LENGTH) break;
        memcpy(dataPoint->date, dateStart, dateEnd - dateStart);
        dataPoint->date[dateEnd - dateStart] = '\0';
        
        /* Extract open price */
        const char* openStart = strstr(pos, "\"open\"");
        if (!openStart) break;
        openStart = strchr(openStart, ':');
        if (!openStart) break;
        dataPoint->open = strtod(openStart + 1, NULL);
        
        /* Extract high price */
        const char* highStart = strstr(pos, "\"high\"");
        if (!highStart) break;
        highStart = strchr(highStart, ':');
        if (!highStart) break;
        dataPoint->high = strtod(highStart + 1, NULL);
        
        /* Extract low price */
        const char* lowStart = strstr(pos, "\"low\"");
        if (!lowStart) break;
        lowStart = strchr(lowStart, ':');
        if (!lowStart) break;
        dataPoint->low = strtod(lowStart + 1, NULL);
        
        /* Extract close price */
        const char* closeStart = strstr(pos, "\"close\"");
        if (!closeStart) break;
        closeStart = strchr(closeStart, ':');
        if (!closeStart) break;
        dataPoint->close = strtod(closeStart + 1, NULL);
        
        /* Extract volume */
        const char* volStart = strstr(pos, "\"volume\"");
        if (!volStart) break;
        volStart = strchr(volStart, ':');
        if (!volStart) break;
        dataPoint->volume = strtod(volStart + 1, NULL);
        
        /* Extract adjusted close (may be named "adjClose" or "adjClose") */
        const char* adjCloseStart = strstr(pos, "\"adjClose\"");
        if (!adjCloseStart) {
            adjCloseStart = strstr(pos, "\"adjClose\"");
        }
        if (adjCloseStart) {
            adjCloseStart = strchr(adjCloseStart, ':');
            if (adjCloseStart) {
                dataPoint->adjClose = strtod(adjCloseStart + 1, NULL);
            } else {
                dataPoint->adjClose = dataPoint->close; /* Use close if adjClose not available */
            }
        } else {
            dataPoint->adjClose = dataPoint->close; /* Use close if adjClose not available */
        }
        
        /* Move to the next record */
        stock->dataSize++;
        pos = strchr(pos, '}');
        if (!pos) break;
    }
    
    if (stock->dataSize == 0) {
        logError(ERR_DATA_CORRUPTED, "Failed to parse any data points from JSON response");
        return 0;
    }
    
    logMessage(LOG_INFO, "Successfully parsed %d data points from JSON response", stock->dataSize);
    return 1;
}

/* Parse news data JSON response */
int parseNewsDataJSON(const char* jsonData, EventDatabase* events) {
    if (!jsonData || !events) {
        logError(ERR_INVALID_PARAMETER, "Invalid parameters for parseNewsDataJSON");
        return 0;
    }
    
    /* Initialize events array if needed */
    if (!events->events) {
        events->eventCapacity = 50; /* Initial capacity */
        events->events = (EventData*)malloc(events->eventCapacity * sizeof(EventData));
        if (!events->events) {
            logError(ERR_OUT_OF_MEMORY, "Memory allocation failed for events");
            return 0;
        }
        events->eventCount = 0;
    }
    
    /* Simple JSON parser for Tiingo News API response format */
    /* Format: [{"publishedDate":"2025-04-16T10:30:00Z", "title":"...", "description":"...", ... }, {...}, ...] */
    
    /* Find the beginning of the array */
    const char* pos = strchr(jsonData, '[');
    if (!pos) {
        logError(ERR_DATA_CORRUPTED, "Invalid JSON format: array not found");
        return 0;
    }
    
    /* Iterate through each object in the array */
    while ((pos = strstr(pos, "{"))) {
        /* Check if we need to resize the array */
        if (events->eventCount >= events->eventCapacity) {
            events->eventCapacity *= 2;
            EventData* newEvents = (EventData*)realloc(events->events, events->eventCapacity * sizeof(EventData));
            if (!newEvents) {
                logError(ERR_OUT_OF_MEMORY, "Memory reallocation failed for events");
                return 0;
            }
            events->events = newEvents;
        }
        
        /* Parse each field of the object */
        EventData* event = &events->events[events->eventCount];
        
        /* Extract date (publishedDate in Tiingo API) */
        const char* dateStart = strstr(pos, "\"publishedDate\"");
        if (!dateStart) break;
        dateStart = strchr(dateStart, ':');
        if (!dateStart) break;
        dateStart = strchr(dateStart, '"');
        if (!dateStart) break;
        dateStart++;
        const char* dateEnd = strchr(dateStart, 'T'); /* Timestamp format: 2025-04-16T10:30:00Z */
        if (!dateEnd) dateEnd = strchr(dateStart, '"'); /* Fallback if no 'T' */
        if (!dateEnd || (dateEnd - dateStart) >= MAX_DATE_LENGTH) break;
        memcpy(event->date, dateStart, dateEnd - dateStart);
        event->date[dateEnd - dateStart] = '\0';
        
        /* Extract title */
        const char* titleStart = strstr(pos, "\"title\"");
        if (!titleStart) break;
        titleStart = strchr(titleStart, ':');
        if (!titleStart) break;
        titleStart = strchr(titleStart, '"');
        if (!titleStart) break;
        titleStart++;
        const char* titleEnd = strchr(titleStart, '"');
        if (!titleEnd || (titleEnd - titleStart) >= MAX_BUFFER_SIZE) break;
        memcpy(event->title, titleStart, titleEnd - titleStart);
        event->title[titleEnd - titleStart] = '\0';
        
        /* Extract description */
        const char* descStart = strstr(pos, "\"description\"");
        if (!descStart) {
            /* Try "content" if "description" is not available */
            descStart = strstr(pos, "\"content\"");
        }
        if (descStart) {
            descStart = strchr(descStart, ':');
            if (descStart) {
                descStart = strchr(descStart, '"');
                if (descStart) {
                    descStart++;
                    const char* descEnd = strchr(descStart, '"');
                    if (descEnd && (descEnd - descStart) < MAX_BUFFER_SIZE) {
                        memcpy(event->description, descStart, descEnd - descStart);
                        event->description[descEnd - descStart] = '\0';
                    } else {
                        /* Description too long or not properly terminated */
                        strncpy(event->description, "No description available", MAX_BUFFER_SIZE - 1);
                        event->description[MAX_BUFFER_SIZE - 1] = '\0';
                    }
                }
            }
        } else {
            /* No description or content found */
            strncpy(event->description, "No description available", MAX_BUFFER_SIZE - 1);
            event->description[MAX_BUFFER_SIZE - 1] = '\0';
        }
        
        /* Extract source if available for sentiment analysis */
        const char* sourceStart = strstr(pos, "\"source\"");
        if (sourceStart) {
            /* Calculate a simple sentiment score based on keywords in title and description */
            /* This is a basic implementation - in real world, we would use NLP or ML models */
            double sentiment = 0.0;
            
            /* Look for positive keywords */
            const char* positiveKeywords[] = {"growth", "increase", "positive", "rise", "record",
                                            "profit", "exceed", "beat", "success", "gain"};
            int posKeywordCount = sizeof(positiveKeywords) / sizeof(positiveKeywords[0]);
            
            /* Look for negative keywords */
            const char* negativeKeywords[] = {"decline", "decrease", "negative", "fall", "loss",
                                            "miss", "below", "fail", "drop", "cut"};
            int negKeywordCount = sizeof(negativeKeywords) / sizeof(negativeKeywords[0]);
            
            /* Count positive keywords */
            int posCount = 0;
            for (int i = 0; i < posKeywordCount; i++) {
                if (strstr(event->title, positiveKeywords[i]) || 
                    strstr(event->description, positiveKeywords[i])) {
                    posCount++;
                }
            }
            
            /* Count negative keywords */
            int negCount = 0;
            for (int i = 0; i < negKeywordCount; i++) {
                if (strstr(event->title, negativeKeywords[i]) || 
                    strstr(event->description, negativeKeywords[i])) {
                    negCount++;
                }
            }
            
            /* Calculate sentiment based on keyword counts */
            if (posCount + negCount > 0) {
                sentiment = (double)(posCount - negCount) / (posCount + negCount);
            }
            event->sentiment = sentiment;
            
            /* Calculate a simple impact score (0-100) based on source credibility and keywords */
            const char* majorSources[] = {"Reuters", "Bloomberg", "CNBC", "WSJ", "Financial Times"};
            int majorSourceCount = sizeof(majorSources) / sizeof(majorSources[0]);
            
            /* Check if from a major source (more impact) */
            int isMajorSource = 0;
            for (int i = 0; i < majorSourceCount; i++) {
                if (strstr(sourceStart, majorSources[i])) {
                    isMajorSource = 1;
                    break;
                }
            }
            
            /* Impact keywords */
            const char* impactKeywords[] = {"significant", "major", "dramatic", "substantial", 
                                          "critical", "breaking", "urgent", "important"};
            int impactKeywordCount = sizeof(impactKeywords) / sizeof(impactKeywords[0]);
            
            /* Count impact keywords */
            int impactCount = 0;
            for (int i = 0; i < impactKeywordCount; i++) {
                if (strstr(event->title, impactKeywords[i]) || 
                    strstr(event->description, impactKeywords[i])) {
                    impactCount++;
                }
            }
            
            /* Calculate impact score */
            int impactScore = impactCount * 15;
            if (isMajorSource) impactScore += 20;
            if (impactScore > 100) impactScore = 100;
            
            event->impactScore = impactScore;
        } else {
            /* Default values if source info is not available */
            event->sentiment = 0.0;
            event->impactScore = 50;
        }
        
        /* Move to the next record */
        events->eventCount++;
        pos = strchr(pos, '}');
        if (!pos) break;
    }
    
    if (events->eventCount == 0) {
        logError(ERR_DATA_CORRUPTED, "Failed to parse any events from JSON response");
        return 0;
    }
    
    logMessage(LOG_INFO, "Successfully parsed %d events from JSON response", events->eventCount);
    return 1;
}

/* Generate a filename for the CSV cache based on symbol and date range */
char* generateCSVFilename(const char* symbol, const char* startDate, const char* endDate) {
    if (!symbol || !startDate || !endDate) {
        return NULL;
    }
    
    char* filename = (char*)malloc(MAX_PATH_LENGTH);
    if (!filename) {
        logError(ERR_OUT_OF_MEMORY, "Failed to allocate memory for CSV filename");
        return NULL;
    }
    
    snprintf(filename, MAX_PATH_LENGTH, "%s%s_%s_to_%s.csv", 
             CSV_DATA_DIRECTORY, symbol, startDate, endDate);
    
    return filename;
}

/* Check if CSV data exists for the given parameters */
int checkCSVDataExists(const char* symbol, const char* startDate, const char* endDate) {
    char* filename = generateCSVFilename(symbol, startDate, endDate);
    if (!filename) {
        return 0;
    }
    
    /* Check if file exists */
    FILE* file = fopen(filename, "r");
    int exists = (file != NULL);
    
    if (file) {
        fclose(file);
    }
    
    free(filename);
    return exists;
}

/* Save stock data to a CSV file */
int saveStockDataToCSV(const Stock* stock, const char* startDate, const char* endDate) {
    if (!stock || !startDate || !endDate) {
        logError(ERR_INVALID_PARAMETER, "Invalid parameters for saving to CSV");
        return 0;
    }
    
    char* filename = generateCSVFilename(stock->symbol, startDate, endDate);
    if (!filename) {
        return 0;
    }
    
    FILE* file = fopen(filename, "w");
    if (!file) {
        logError(ERR_FILE_OPEN_FAILED, "Failed to open CSV file for writing: %s", filename);
        free(filename);
        return 0;
    }
    
    /* Write CSV header */
    fprintf(file, "Date,Open,High,Low,Close,Volume,AdjClose\n");
    
    /* Write data rows */
    for (int i = 0; i < stock->dataSize; i++) {
        fprintf(file, "%s,%.4f,%.4f,%.4f,%.4f,%.0f,%.4f\n",
                stock->data[i].date,
                stock->data[i].open,
                stock->data[i].high,
                stock->data[i].low,
                stock->data[i].close,
                stock->data[i].volume,
                stock->data[i].adjClose);
    }
    
    fclose(file);
    free(filename);
    
    logMessage(LOG_INFO, "Saved %d data points to CSV for %s (%s to %s)", 
               stock->dataSize, stock->symbol, startDate, endDate);
    
    return 1;
}

/* Load stock data from a CSV file */
int loadStockDataFromCSV(const char* symbol, const char* startDate, const char* endDate, Stock* stock) {
    if (!symbol || !startDate || !endDate || !stock) {
        logError(ERR_INVALID_PARAMETER, "Invalid parameters for loading from CSV");
        return 0;
    }
    
    /* Initialize stock struct */
    strncpy(stock->symbol, symbol, MAX_SYMBOL_LENGTH - 1);
    stock->symbol[MAX_SYMBOL_LENGTH - 1] = '\0';
    
    if (!stock->data) {
        stock->dataCapacity = 100; /* Initial capacity */
        stock->data = (StockData*)malloc(stock->dataCapacity * sizeof(StockData));
        if (!stock->data) {
            logError(ERR_OUT_OF_MEMORY, "Failed to allocate memory for stock data");
            return 0;
        }
    }
    stock->dataSize = 0;
    
    char* filename = generateCSVFilename(symbol, startDate, endDate);
    if (!filename) {
        return 0;
    }
    
    FILE* file = fopen(filename, "r");
    if (!file) {
        logError(ERR_FILE_OPEN_FAILED, "Failed to open CSV file for reading: %s", filename);
        free(filename);
        return 0;
    }
    
    char line[256];
    /* Skip header line */
    if (fgets(line, sizeof(line), file) == NULL) {
        logError(ERR_FILE_READ_FAILED, "Failed to read header from CSV file");
        fclose(file);
        free(filename);
        return 0;
    }
    
    /* Read data rows */
    while (fgets(line, sizeof(line), file) != NULL) {
        /* Check if we need to resize the array */
        if (stock->dataSize >= stock->dataCapacity) {
            stock->dataCapacity *= 2;
            StockData* newData = (StockData*)realloc(stock->data, stock->dataCapacity * sizeof(StockData));
            if (!newData) {
                logError(ERR_OUT_OF_MEMORY, "Failed to reallocate memory for stock data");
                fclose(file);
                free(filename);
                return 0;
            }
            stock->data = newData;
        }
        
        /* Parse line */
        StockData* dataPoint = &stock->data[stock->dataSize];
        int result = sscanf(line, "%[^,],%lf,%lf,%lf,%lf,%lf,%lf",
                          dataPoint->date,
                          &dataPoint->open,
                          &dataPoint->high,
                          &dataPoint->low,
                          &dataPoint->close,
                          &dataPoint->volume,
                          &dataPoint->adjClose);
        
        if (result == 7) { /* All fields successfully read */
            stock->dataSize++;
        }
    }
    
    fclose(file);
    free(filename);
    
    logMessage(LOG_INFO, "Loaded %d data points from CSV for %s (%s to %s)", 
               stock->dataSize, symbol, startDate, endDate);
    
    return (stock->dataSize > 0);
}

/* Fetch historical data with cache support (up to 10 years) */
int fetchHistoricalDataWithCache(const char* symbol, const char* startDate, const char* endDate, Stock* stock) {
    if (!symbol || !startDate || !endDate || !stock) {
        logError(ERR_INVALID_PARAMETER, "Invalid parameters for fetchHistoricalDataWithCache");
        return 0;
    }
    
    /* Initialize stock symbol */
    strncpy(stock->symbol, symbol, MAX_SYMBOL_LENGTH - 1);
    stock->symbol[MAX_SYMBOL_LENGTH - 1] = '\0';
    
    /* Check if we already have the data cached */
    if (checkCSVDataExists(symbol, startDate, endDate)) {
        logMessage(LOG_INFO, "Using cached data for %s (%s to %s)", symbol, startDate, endDate);
        return loadStockDataFromCSV(symbol, startDate, endDate, stock);
    }
    
    /* Data not cached, fetch from API */
    logMessage(LOG_INFO, "Fetching data from API for %s (%s to %s)", symbol, startDate, endDate);
    
    /* For large date ranges, we may need to break up the request to avoid API limits */
    /* Most APIs have limits on how much data can be fetched in a single request */
    
    /* Parse start and end dates */
    struct tm start_tm = {0}, end_tm = {0};
    if (sscanf(startDate, "%d-%d-%d", &start_tm.tm_year, &start_tm.tm_mon, &start_tm.tm_mday) != 3 ||
        sscanf(endDate, "%d-%d-%d", &end_tm.tm_year, &end_tm.tm_mon, &end_tm.tm_mday) != 3) {
        logError(ERR_INVALID_PARAMETER, "Invalid date format. Use YYYY-MM-DD");
        return 0;
    }
    
    /* Adjust for tm structure (years since 1900, months 0-11) */
    start_tm.tm_year -= 1900;
    start_tm.tm_mon -= 1;
    end_tm.tm_year -= 1900;
    end_tm.tm_mon -= 1;
    
    /* Convert to time_t for comparison */
    time_t start_time = mktime(&start_tm);
    time_t end_time = mktime(&end_tm);
    
    /* Calculate time difference in seconds */
    double diff = difftime(end_time, start_time);
    
    /* If data range is more than 1 year, break into smaller chunks */
    const double ONE_YEAR_SECONDS = 31536000; /* 365 days */
    if (diff > ONE_YEAR_SECONDS) {
        /* For simplicity, we'll fetch in one-year chunks */
        time_t current_start = start_time;
        time_t current_end;
        char current_start_str[MAX_DATE_LENGTH];
        char current_end_str[MAX_DATE_LENGTH];
        
        Stock tempStock;
        memset(&tempStock, 0, sizeof(Stock));
        strncpy(tempStock.symbol, symbol, MAX_SYMBOL_LENGTH - 1);
        tempStock.symbol[MAX_SYMBOL_LENGTH - 1] = '\0';
        
        while (current_start < end_time) {
            /* Calculate current end (one year later or end_time, whichever is earlier) */
            current_end = current_start + (time_t)ONE_YEAR_SECONDS;
            if (current_end > end_time) {
                current_end = end_time;
            }
            
            /* Convert to string format */
            struct tm* tm_info;
            tm_info = localtime(&current_start);
            strftime(current_start_str, MAX_DATE_LENGTH, "%Y-%m-%d", tm_info);
            
            tm_info = localtime(&current_end);
            strftime(current_end_str, MAX_DATE_LENGTH, "%Y-%m-%d", tm_info);
            
            /* Fetch this chunk */
            Stock chunkStock;
            memset(&chunkStock, 0, sizeof(Stock));
            strncpy(chunkStock.symbol, symbol, MAX_SYMBOL_LENGTH - 1);
            chunkStock.symbol[MAX_SYMBOL_LENGTH - 1] = '\0';
            
            if (!fetchStockData(symbol, current_start_str, current_end_str, &chunkStock)) {
                logError(ERR_API_REQUEST_FAILED, "Failed to fetch data chunk for %s (%s to %s)", 
                        symbol, current_start_str, current_end_str);
                /* Try to continue with next chunk */
            } else {
                /* Add this chunk's data to tempStock */
                int newDataSize = tempStock.dataSize + chunkStock.dataSize;
                if (newDataSize > tempStock.dataCapacity) {
                    /* Need to resize tempStock.data */
                    int newCapacity = newDataSize + 100; /* Add some extra space */
                    StockData* newData = (StockData*)realloc(tempStock.data, newCapacity * sizeof(StockData));
                    if (!newData) {
                        logError(ERR_OUT_OF_MEMORY, "Failed to reallocate memory for combined stock data");
                        if (chunkStock.data) free(chunkStock.data);
                        break;
                    }
                    tempStock.data = newData;
                    tempStock.dataCapacity = newCapacity;
                }
                
                /* Now copy the data from chunkStock to tempStock */
                if (tempStock.data == NULL) {
                    tempStock.data = (StockData*)malloc(chunkStock.dataCapacity * sizeof(StockData));
                    if (!tempStock.data) {
                        logError(ERR_OUT_OF_MEMORY, "Failed to allocate memory for tempStock data");
                        if (chunkStock.data) free(chunkStock.data);
                        break;
                    }
                    tempStock.dataCapacity = chunkStock.dataCapacity;
                }
                
                /* Copy the data */
                memcpy(tempStock.data + tempStock.dataSize, chunkStock.data, chunkStock.dataSize * sizeof(StockData));
                tempStock.dataSize += chunkStock.dataSize;
                
                /* Free the chunk data */
                if (chunkStock.data) free(chunkStock.data);
            }
            
            /* Move to next chunk */
            current_start = current_end + 86400; /* Add one day (in seconds) */
        }
        
        /* Save the complete dataset to CSV */
        if (tempStock.dataSize > 0) {
            saveStockDataToCSV(&tempStock, startDate, endDate);
            
            /* Copy data to output stock */
            if (!stock->data) {
                stock->data = (StockData*)malloc(tempStock.dataSize * sizeof(StockData));
                if (!stock->data) {
                    logError(ERR_OUT_OF_MEMORY, "Failed to allocate memory for combined stock data");
                    free(tempStock.data);
                    return 0;
                }
                stock->dataCapacity = tempStock.dataSize;
            } else if (stock->dataCapacity < tempStock.dataSize) {
                StockData* newData = (StockData*)realloc(stock->data, tempStock.dataSize * sizeof(StockData));
                if (!newData) {
                    logError(ERR_OUT_OF_MEMORY, "Failed to reallocate memory for combined stock data");
                    free(tempStock.data);
                    return 0;
                }
                stock->data = newData;
                stock->dataCapacity = tempStock.dataSize;
            }
            
            memcpy(stock->data, tempStock.data, tempStock.dataSize * sizeof(StockData));
            stock->dataSize = tempStock.dataSize;
            
            free(tempStock.data);
            return 1;
        }
        
        /* If we got here, we failed to fetch any data */
        if (tempStock.data) {
            free(tempStock.data);
        }
        return 0;
    } else {
        /* For small date ranges, fetch directly and cache the result */
        if (fetchStockData(symbol, startDate, endDate, stock)) {
            saveStockDataToCSV(stock, startDate, endDate);
            return 1;
        }
        return 0;
    }
}