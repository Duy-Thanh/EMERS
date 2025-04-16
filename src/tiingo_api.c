/**
 * Tiingo API Integration Module
 * Implementation file for Tiingo API functions
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <ctype.h>
#include <math.h>
#include <process.h>  /* For _popen() */
#include <direct.h>   /* For _mkdir() on Windows */
#include <sys/stat.h> /* For stat() */

#include "../include/emers.h"
#include "../include/tiingo_api.h"
#include "../include/error_handling.h"  /* Added error_handling.h for logAPIError */

/* Define NewsAPI.ai URL and API parameters */
#define NEWSAPI_API_URL "https://api.newsapi.ai/api/v1/article/getArticles"
#define NEWSAPI_API_KEY "YOUR_NEWSAPI_AI_KEY"  /* Replace with your actual NewsAPI.ai key */

/* Forward declarations of helper functions */
double calculateSentiment(const char* title, const char* description);
double calculateImpactScore(const EventData* event);

/* Static variables */
static char apiKey[MAX_API_KEY_LENGTH] = "";
static char marketauxApiKey[MARKETAUX_API_KEY_LENGTH] = "";
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

/* Set the MarketAux API key */
void setMarketAuxAPIKey(const char* key) {
    if (key && strlen(key) > 0) {
        strncpy(marketauxApiKey, key, MARKETAUX_API_KEY_LENGTH - 1);
        marketauxApiKey[MARKETAUX_API_KEY_LENGTH - 1] = '\0';
    }
}

/* Get the MarketAux API key */
const char* getMarketAuxAPIKey(void) {
    return marketauxApiKey;
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

/* Fetch news data using MarketAux API instead of Tiingo News API */
int fetchNewsFeed(const char* symbols, EventDatabase* events) {
    if (!symbols || !events) {
        logError(ERR_INVALID_PARAMETER, "Invalid parameters for fetchNewsFeed");
        return 0;
    }
    
    /* Get API key for MarketAux */
    const char* apiKey = getMarketAuxAPIKey();
    if (!apiKey || strlen(apiKey) == 0) {
        logError(ERR_INVALID_PARAMETER, "MarketAux API key not set");
        return 0;
    }
    
    /* Build the MarketAux API URL */
    char url[MAX_URL_LENGTH];
    snprintf(url, MAX_URL_LENGTH, 
             "%s?symbols=%s&limit=50&language=en&api_token=%s", 
             MARKETAUX_API_URL, symbols, apiKey);
    
    /* Perform API request using curl */
    Memory response;
    response.data = malloc(1);
    if (!response.data) {
        logError(ERR_OUT_OF_MEMORY, "Memory allocation failed for response");
        return 0;
    }
    response.size = 0;
    response.data[0] = '\0';
    
    /* Build curl command with proper parameters for MarketAux */
    char command[MAX_BUFFER_SIZE];
    
    /* Improve curl command with more options to handle network issues */
    snprintf(command, sizeof(command),
             "curl -s --connect-timeout 30 --max-time 60 --retry 5 --retry-delay 2 --retry-connrefused "
             "--retry-max-time 120 --dns-servers 8.8.8.8,1.1.1.1 \"%s\" > curl_output.json 2>curl_error.txt",
             url);
    
    /* Execute curl command */
    int result = system(command);
    if (result != 0) {
        /* Try to read error details from curl_error.txt */
        FILE* errFp = fopen("curl_error.txt", "r");
        char errorDetails[256] = "Unknown error";
        
        if (errFp) {
            if (fgets(errorDetails, sizeof(errorDetails), errFp)) {
                /* Remove newline if present */
                char* newline = strchr(errorDetails, '\n');
                if (newline) *newline = '\0';
            }
            fclose(errFp);
        }
        
        /* Fall back to a simpler curl command if the first one fails */
        snprintf(command, sizeof(command),
                 "curl -s -k --ipv4 --connect-timeout 30 --max-time 60 \"%s\" > curl_output.json",
                 url);
        
        result = system(command);
        if (result != 0) {
            logError(ERR_SYSTEM, "Failed to execute curl command. Error code: %d. Details: %s", 
                    result, errorDetails);
            free(response.data);
            return 0;
        }
    }
    
    /* Read the output file */
    FILE* fp = fopen("curl_output.json", "rb");
    if (!fp) {
        logError(ERR_FILE_OPEN_FAILED, "Failed to open curl output file");
        free(response.data);
        return 0;
    }
    
    /* Get file size */
    fseek(fp, 0, SEEK_END);
    long fileSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    
    if (fileSize <= 0) {
        logError(ERR_DATA_CORRUPTED, "Empty response from API or failed request");
        fclose(fp);
        free(response.data);
        return 0;
    }
    
    /* Allocate memory for response data */
    char* newData = (char*)realloc(response.data, fileSize + 1);
    if (!newData) {
        logError(ERR_OUT_OF_MEMORY, "Memory reallocation failed for response");
        fclose(fp);
        free(response.data);
        return 0;
    }
    response.data = newData;
    
    /* Read file content */
    size_t bytesRead = fread(response.data, 1, fileSize, fp);
    fclose(fp);
    
    if (bytesRead != (size_t)fileSize) {
        logError(ERR_FILE_READ_FAILED, "Failed to read complete curl output");
        free(response.data);
        return 0;
    }
    
    /* Null-terminate the response data */
    response.data[bytesRead] = '\0';
    response.size = bytesRead;
    
    /* Check for API errors in the response */
    if (strstr(response.data, "\"error\"") != NULL) {
        logError(ERR_API_REQUEST_FAILED, "API returned an error response: %s", response.data);
        free(response.data);
        return 0;
    }
    
    /* Parse JSON response using MarketAux format */
    int success = parseNewsDataJSON(response.data, events);
    
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

/* Parse JSON response from MarketAux API for news data */
int parseNewsDataJSON(const char* jsonData, EventDatabase* events) {
    if (!jsonData || !events) {
        logError(ERR_INVALID_PARAMETER, "Invalid parameters for parseNewsDataJSON");
        return 0;
    }

    /* Check for API errors in MarketAux response */
    if (strstr(jsonData, "\"error\"") || strstr(jsonData, "\"errors\"")) {
        /* Check for common API errors */
        if (strstr(jsonData, "api_token") || strstr(jsonData, "token")) {
            logError(ERR_API_RESPONSE_INVALID, "API key error in MarketAux response: %s", jsonData);
        } 
        else if (strstr(jsonData, "permission") || strstr(jsonData, "unauthorized")) {
            logError(ERR_API_REQUEST_FAILED, "API permission error: %s", jsonData);
        }
        else if (strstr(jsonData, "limit") || strstr(jsonData, "quota")) {
            logError(ERR_API_REQUEST_FAILED, "API rate limit reached: %s", jsonData);
        }
        else {
            logError(ERR_API_REQUEST_FAILED, "Unknown API error: %s", jsonData);
        }
        return 0;
    }

    /* Look for the data array in MarketAux response format */
    const char* dataStart = strstr(jsonData, "\"data\"");
    if (!dataStart) {
        logError(ERR_DATA_CORRUPTED, "Invalid MarketAux JSON format: 'data' field not found");
        return 0;
    }

    /* Find the array start */
    const char* arrayStart = strchr(dataStart, '[');
    if (!arrayStart) {
        logError(ERR_DATA_CORRUPTED, "Invalid MarketAux JSON format: array not found");
        return 0;
    }

    /* Count how many objects we have */
    int objectCount = 0;
    const char* pos = arrayStart;
    while ((pos = strstr(pos + 1, "{"))) {
        objectCount++;
    }

    if (objectCount == 0) {
        /* No news articles found, but not an error */
        return 1;
    }

    /* Ensure enough capacity in the events database */
    if (events->eventCount + objectCount > events->eventCapacity) {
        int newCapacity = events->eventCapacity + objectCount + 10;  /* Add some extra */
        EventData* newEvents = realloc(events->events, newCapacity * sizeof(EventData));
        if (!newEvents) {
            logError(ERR_OUT_OF_MEMORY, "Failed to allocate memory for news events");
            return 0;
        }
        events->events = newEvents;
        events->eventCapacity = newCapacity;
    }

    /* Process each object */
    int count = 0;
    pos = arrayStart;
    
    while (count < objectCount) {
        /* Find start of object */
        const char* objStart = strstr(pos, "{");
        if (!objStart) break;
        
        /* Find end of object - handle nested objects properly */
        const char* objEnd = NULL;
        int braceCount = 1;
        for (const char* c = objStart + 1; *c; c++) {
            if (*c == '{') braceCount++;
            else if (*c == '}') {
                braceCount--;
                if (braceCount == 0) {
                    objEnd = c;
                    break;
                }
            }
        }
        
        if (!objEnd) break;
        
        /* Process this object/article */
        EventData* event = &events->events[events->eventCount];
        memset(event, 0, sizeof(EventData));
        
        /* Extract date from "published_at" field in MarketAux format */
        const char* publishedField = strstr(objStart, "\"published_at\"");
        if (publishedField) {
            const char* datePos = strchr(publishedField, ':');
            if (datePos) {
                datePos = strchr(datePos, '"');
                if (datePos) {
                    datePos++; /* Skip first quote */
                    int i = 0;
                    while (datePos[i] && datePos[i] != '"' && i < (int)(sizeof(event->date) - 1)) {
                        /* Only copy the date part (up to 10 chars) */
                        if (i < 10) {
                            event->date[i] = datePos[i];
                        }
                        i++;
                    }
                    event->date[10] = '\0'; /* Ensure we only store the date part */
                }
            }
        }
        
        /* Extract title from MarketAux format */
        const char* titleField = strstr(objStart, "\"title\"");
        if (titleField) {
            const char* titlePos = strchr(titleField, ':');
            if (titlePos) {
                titlePos = strchr(titlePos, '"');
                if (titlePos) {
                    titlePos++; /* Skip first quote */
                    int i = 0;
                    while (titlePos[i] && titlePos[i] != '"' && i < (int)(sizeof(event->title) - 1)) {
                        event->title[i] = titlePos[i];
                        i++;
                    }
                    event->title[i] = '\0';
                }
            }
        }
        
        /* Extract description from "description" field in MarketAux format */
        const char* descField = strstr(objStart, "\"description\"");
        if (descField) {
            const char* descPos = strchr(descField, ':');
            if (descPos) {
                descPos = strchr(descPos, '"');
                if (descPos) {
                    descPos++; /* Skip first quote */
                    int i = 0;
                    while (descPos[i] && descPos[i] != '"' && i < (int)(sizeof(event->description) - 1)) {
                        event->description[i] = descPos[i];
                        i++;
                    }
                    event->description[i] = '\0';
                }
            }
        }
        
        /* Extract source information from MarketAux format */
        const char* sourceField = strstr(objStart, "\"source\"");
        if (sourceField) {
            const char* sourceNameField = strstr(sourceField, "\"name\"");
            if (sourceNameField) {
                const char* namePos = strchr(sourceNameField, ':');
                if (namePos) {
                    namePos = strchr(namePos, '"');
                    if (namePos) {
                        namePos++; /* Skip first quote */
                        
                        /* Create a source prefix to add to title */
                        char sourcePrefix[MAX_BUFFER_SIZE/2] = {0};
                        int i = 0;
                        while (namePos[i] && namePos[i] != '"' && i < (int)(sizeof(sourcePrefix) - 10)) {
                            sourcePrefix[i] = namePos[i];
                            i++;
                        }
                        sourcePrefix[i] = '\0';
                        
                        /* Prepend source to title if we have both */
                        if (sourcePrefix[0] && event->title[0]) {
                            char tempTitle[MAX_BUFFER_SIZE] = {0};
                            snprintf(tempTitle, sizeof(tempTitle), "[%s] %s", sourcePrefix, event->title);
                            strncpy(event->title, tempTitle, sizeof(event->title) - 1);
                            event->title[sizeof(event->title) - 1] = '\0';
                        }
                    }
                }
            }
        }
        
        /* Extract URL from MarketAux format */
        const char* urlField = strstr(objStart, "\"url\"");
        if (urlField) {
            const char* urlPos = strchr(urlField, ':');
            if (urlPos) {
                urlPos = strchr(urlPos, '"');
                if (urlPos) {
                    urlPos++; /* Skip first quote */
                    
                    /* Extract the URL into a temp buffer */
                    char urlBuffer[MAX_BUFFER_SIZE/2] = {0};
                    int i = 0;
                    while (urlPos[i] && urlPos[i] != '"' && i < (int)(sizeof(urlBuffer) - 1)) {
                        urlBuffer[i] = urlPos[i];
                        i++;
                    }
                    urlBuffer[i] = '\0';
                    
                    /* If we have room, append URL to description */
                    if (strlen(event->description) + strlen(urlBuffer) + 10 < sizeof(event->description)) {
                        strcat(event->description, "\nSource URL: ");
                        strcat(event->description, urlBuffer);
                    }
                }
            }
        }
        
        /* Extract symbols/tickers mentioned in the news from MarketAux format */
        const char* entitiesField = strstr(objStart, "\"entities\"");
        if (entitiesField) {
            const char* symbolsStart = strstr(entitiesField, "\"symbols\"");
            if (symbolsStart) {
                /* Find symbols array start */
                const char* symbolsArray = strchr(symbolsStart, '[');
                if (symbolsArray) {
                    char symbolsBuffer[256] = {0};
                    const char* symbolName;
                    const char* symbolEnd;
                    const char* symbolPos = symbolsArray;
                    int symbolsAdded = 0;
                    
                    /* Iterate through symbols array */
                    while ((symbolName = strstr(symbolPos, "\"name\"")) && symbolsAdded < 5) {
                        symbolName = strchr(symbolName, ':');
                        if (!symbolName) break;
                        symbolName = strchr(symbolName, '"');
                        if (!symbolName) break;
                        symbolName++; /* Skip first quote */
                        
                        symbolEnd = strchr(symbolName, '"');
                        if (!symbolEnd) break;
                        
                        /* Add symbol to our buffer with separator */
                        if (symbolsAdded > 0) {
                            strcat(symbolsBuffer, ", ");
                        }
                        
                        /* Copy symbol name */
                        int nameLen = symbolEnd - symbolName;
                        strncat(symbolsBuffer, symbolName, nameLen > 10 ? 10 : nameLen);
                        
                        symbolsAdded++;
                        symbolPos = symbolEnd + 1;
                    }
                    
                    /* If we found symbols, add them to the event description */
                    if (symbolsBuffer[0]) {
                        char symbolsInfo[300];
                        snprintf(symbolsInfo, sizeof(symbolsInfo), "\nMentioned symbols: %s", symbolsBuffer);
                        
                        if (strlen(event->description) + strlen(symbolsInfo) < sizeof(event->description)) {
                            strcat(event->description, symbolsInfo);
                        }
                    }
                }
            }
        }
        
        /* Extract sentiment information from MarketAux if available */
        const char* sentimentField = strstr(objStart, "\"sentiment\"");
        if (sentimentField) {
            /* Try to get the sentiment score directly from MarketAux */
            const char* scoreField = strstr(sentimentField, "\"score\"");
            if (scoreField) {
                const char* scorePos = strchr(scoreField, ':');
                if (scorePos) {
                    double marketauxSentiment = strtod(scorePos + 1, NULL);
                    /* MarketAux sentiment is typically 0 to 1, scale to our -1 to 1 range */
                    event->sentiment = (marketauxSentiment - 0.5) * 2.0;
                }
            }
        } else {
            /* Fall back to our own sentiment analysis if MarketAux doesn't provide it */
            event->sentiment = calculateSentiment(event->title, event->description);
        }
        
        /* Calculate impact score */
        event->impactScore = calculateImpactScore(event);
        
        /* Check if it's a major news source to adjust the score */
        const char* majorSources[] = {"Bloomberg", "Reuters", "CNBC", "WSJ", "Barron", "Forbes", "MarketWatch"};
        for (int i = 0; i < (int)(sizeof(majorSources) / sizeof(majorSources[0])); i++) {
            if (strstr(event->title, majorSources[i])) {
                /* Major sources get a boost in impact score */
                event->impactScore += 2;
                break;
            }
        }
        
        /* Clamp impact score to reasonable range */
        if (event->impactScore > 10) event->impactScore = 10;
        if (event->impactScore < 0) event->impactScore = 0;
        
        /* Add to event database */
        events->eventCount++;
        count++;
        
        /* Move to next position */
        pos = objEnd + 1;
    }
    
    /* Return success if we processed at least one event */
    return count > 0;
}

/* Calculate sentiment score for a news event based on title and description */
double calculateSentiment(const char* title, const char* description) {
    /* Simple sentiment analysis based on keyword matching */
    const char* positiveWords[] = {
        "up", "rise", "gain", "surge", "jump", "positive", "growth",
        "profit", "success", "beat", "exceed", "strong", "bullish",
        "rally", "record", "high", "opportunity", "upgrade"
    };
    
    const char* negativeWords[] = {
        "down", "fall", "drop", "decline", "slip", "negative", "loss",
        "miss", "fail", "weak", "bearish", "crash", "plunge", "concern",
        "risk", "fear", "warn", "downgrade", "trouble", "crisis"
    };
    
    int posCount = 0;
    int negCount = 0;
    
    /* Check title for sentiment keywords */
    for (size_t i = 0; i < sizeof(positiveWords) / sizeof(positiveWords[0]); i++) {
        if (strstr(title, positiveWords[i])) {
            posCount += 2;  /* Title matches have higher weight */
        }
    }
    
    for (size_t i = 0; i < sizeof(negativeWords) / sizeof(negativeWords[0]); i++) {
        if (strstr(title, negativeWords[i])) {
            negCount += 2;  /* Title matches have higher weight */
        }
    }
    
    /* Check description for sentiment keywords */
    for (size_t i = 0; i < sizeof(positiveWords) / sizeof(positiveWords[0]); i++) {
        if (strstr(description, positiveWords[i])) {
            posCount++;
        }
    }
    
    for (size_t i = 0; i < sizeof(negativeWords) / sizeof(negativeWords[0]); i++) {
        if (strstr(description, negativeWords[i])) {
            negCount++;
        }
    }
    
    /* Calculate sentiment score from -1.0 (very negative) to 1.0 (very positive) */
    if (posCount + negCount == 0) {
        return 0.0;  /* Neutral if no sentiment words found */
    }
    
    return (double)(posCount - negCount) / (posCount + negCount);
}

/* Calculate impact score for a news event (0-10 scale) */
double calculateImpactScore(const EventData* event) {
    double score = 5.0;  /* Start with neutral score */
    
    /* Adjust based on sentiment - stronger sentiment = higher impact */
    double sentimentImpact = fabs(event->sentiment) * 2.0;
    score += sentimentImpact;
    
    /* Important keywords increase impact score */
    const char* highImpactWords[] = {
        "earnings", "merger", "acquisition", "bankruptcy", "CEO", 
        "executive", "lawsuit", "settlement", "FDA", "approval",
        "patent", "investigation", "dividend", "guidance", "forecast",
        "outlook", "revenue", "profit", "scandal", "breach", "hack",
        "recall", "crisis", "significant", "substantial", "breakthrough"
    };
    
    for (size_t i = 0; i < sizeof(highImpactWords) / sizeof(highImpactWords[0]); i++) {
        if (strstr(event->title, highImpactWords[i]) || 
            strstr(event->description, highImpactWords[i])) {
            score += 1.0;
            /* Cap the boost at 3.0 */
            if (score > 8.0) {
                score = 8.0;
                break;
            }
        }
    }
    
    /* Cap final score between 0 and 10 */
    if (score < 0.0) score = 0.0;
    if (score > 10.0) score = 10.0;
    
    return score;
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