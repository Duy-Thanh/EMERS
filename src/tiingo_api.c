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
#include <process.h>  /* For _popen() on Windows */
#include <direct.h>   /* For _mkdir() on Windows */
#include <sys/stat.h> /* For stat() */
#include <cJSON.h>    /* For JSON parsing */

#include "../include/emers.h"
#include "../include/tiingo_api.h"
#include "../include/error_handling.h"  /* Added error_handling.h for logAPIError */

/* Define SUCCESS constant if not already defined */    
#ifndef SUCCESS
#define SUCCESS 0
#endif

/* Define NewsAPI.ai URL and API parameters */
#define NEWSAPI_API_URL "https://api.newsapi.ai/api/v1/article/getArticles"
#define NEWSAPI_API_KEY "YOUR_NEWSAPI_AI_KEY"  /* Replace with your actual NewsAPI.ai key */
#define MAX_NEWS_ITEMS 50

/* Forward declarations of helper functions */
double calculateSentiment(const char* title, const char* description);
double calculateImpactScore(const EventData* event);
void getTempFilePath(char* buffer, size_t bufferSize, const char* filename);
time_t parseISOTimeString(const char* timeString);

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
    
    /* Build curl command - simplify and make more robust */
    char command[MAX_BUFFER_SIZE];
    snprintf(command, sizeof(command),
             "curl -s -S -f -H \"Authorization: Token %s\" -H \"Content-Type: application/json\" \"%s\" > curl_output.json 2> curl_error.txt",
             apiKey, url);
    
    /* Execute curl command */
    int result = system(command);
    if (result != 0) {
        FILE* error_file = fopen("curl_error.txt", "r");
        char error_msg[256] = "Unknown curl error";
        
        if (error_file) {
            if (fgets(error_msg, sizeof(error_msg), error_file)) {
                /* Remove newline character if present */
                char* newline = strchr(error_msg, '\n');
                if (newline) *newline = '\0';
            }
            fclose(error_file);
        }
        
        printf("Error: Failed to execute curl command. Error code: %d\n", result);
        logError(ERR_CURL_FAILED, "Failed to execute curl command. Error: %s", error_msg);
        free(response->data);
        response->data = NULL;
        return 0;
    }
    
    /* Read the output file */
    FILE* fp = fopen("curl_output.json", "rb");
    if (!fp) {
        printf("Error: Failed to open curl output file.\n");
        logError(ERR_FILE_ACCESS, "Failed to open curl output file");
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
        logError(ERR_API_RESPONSE_EMPTY, "Empty response from API");
        fclose(fp);
        free(response->data);
        response->data = NULL;
        return 0;
    }
    
    /* Allocate memory for response data */
    char* newData = (char*)realloc(response->data, fileSize + 1);
    if (!newData) {
        printf("Error: Memory reallocation failed for response\n");
        logError(ERR_OUT_OF_MEMORY, "Memory reallocation failed for API response");
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
        logError(ERR_FILE_READ, "Failed to read complete curl output file");
        free(response->data);
        response->data = NULL;
        return 0;
    }
    
    /* Null-terminate the response data */
    response->data[bytesRead] = '\0';
    response->size = bytesRead;
    
    /* Check if the response contains valid JSON */
    cJSON *testJson = cJSON_Parse(response->data);
    if (!testJson) {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr) {
            printf("Error: Invalid JSON response: %s\n", error_ptr);
            logError(ERR_DATA_CORRUPTED, "Invalid JSON response: %s", error_ptr);
        } else {
            printf("Error: Invalid JSON response\n");
            logError(ERR_DATA_CORRUPTED, "Invalid JSON response");
        }
        free(response->data);
        response->data = NULL;
        return 0;
    }
    cJSON_Delete(testJson);
    
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
    StockData* dataArray = NULL;
    int dataCount = 0;
    success = parseStockDataJSON(response.data, &dataArray, &dataCount);
    
    if (success && dataArray) {
        /* Allocate memory for stock data if it hasn't been allocated yet
           or increase capacity if needed */
        if (stock->data == NULL) {
            stock->data = (StockData*)malloc(dataCount * sizeof(StockData));
            if (!stock->data) {
                logError(ERR_OUT_OF_MEMORY, "Failed to allocate memory for stock data");
                free(dataArray);
                free(response.data);
                return 0;
            }
            stock->dataCapacity = dataCount;
            stock->dataSize = 0;
        } else if (stock->dataCapacity < dataCount) {
            /* Need to expand capacity */
            StockData* newData = (StockData*)realloc(stock->data, dataCount * sizeof(StockData));
            if (!newData) {
                logError(ERR_OUT_OF_MEMORY, "Failed to reallocate memory for stock data");
                free(dataArray);
                free(response.data);
                return 0;
            }
            stock->data = newData;
            stock->dataCapacity = dataCount;
        }
        
        /* Copy parsed data to stock structure */
        memcpy(stock->data, dataArray, dataCount * sizeof(StockData));
        stock->dataSize = dataCount;
        free(dataArray);
    }
    
    /* Clean up */
    free(response.data);
    
    return success;
}

/* Parse the JSON response from Tiingo API and extract stock data */
int parseStockDataJSON(const char* jsonData, StockData** dataArray, int* dataCount) {
    if (!jsonData || !dataArray || !dataCount) {
        logError(ERR_INVALID_PARAMETER, "Invalid parameters for parseStockDataJSON");
        return 0;
    }
    
    *dataArray = NULL;
    *dataCount = 0;
    
    cJSON* json = cJSON_Parse(jsonData);
    if (!json) {
        const char* error_ptr = cJSON_GetErrorPtr();
        if (error_ptr) {
            logError(ERR_DATA_CORRUPTED, "JSON parsing failed at: %s", error_ptr);
        } else {
            logError(ERR_DATA_CORRUPTED, "JSON parsing failed with unknown error");
        }
        return 0;
    }
    
    /* Tiingo API can return either a single object or an array of objects */
    cJSON* dataItems = NULL;
    
    /* Check if the root is an array */
    if (cJSON_IsArray(json)) {
        dataItems = json;
    } 
    /* Check if the root is an object that contains a data array */
    else if (cJSON_IsObject(json)) {
        /* Check common response formats */
        cJSON* data = cJSON_GetObjectItem(json, "data");
        if (data && cJSON_IsArray(data)) {
            dataItems = data;
        }
        /* Some Tiingo endpoints return an object with date fields directly */
        else if (cJSON_GetObjectItem(json, "date") != NULL) {
            /* Treat the single object as a one-item array */
            dataItems = json;
            /* Create a temporary array to hold this single item */
            cJSON* tempArray = cJSON_CreateArray();
            if (tempArray) {
                cJSON* cloned = cJSON_Duplicate(json, 1);
                if (cloned) {
                    cJSON_AddItemToArray(tempArray, cloned);
                    dataItems = tempArray;
                } else {
                    cJSON_Delete(tempArray);
                    logError(ERR_OUT_OF_MEMORY, "Failed to clone JSON object");
                    cJSON_Delete(json);
                    return 0;
                }
            } else {
                logError(ERR_OUT_OF_MEMORY, "Failed to create temporary JSON array");
                cJSON_Delete(json);
                return 0;
            }
        }
        else {
            /* If we didn't find any known array structure, log and fail */
            logError(ERR_DATA_CORRUPTED, "Invalid JSON format: array not found");
            cJSON_Delete(json);
            return 0;
        }
    } else {
        logError(ERR_DATA_CORRUPTED, "JSON response is neither an object nor an array");
        cJSON_Delete(json);
        return 0;
    }
    
    /* Count items in the array */
    int itemCount = cJSON_GetArraySize(dataItems);
    if (itemCount <= 0) {
        logError(ERR_DATA_CORRUPTED, "Failed to parse any data points from JSON response");
        cJSON_Delete(json);
        return 0;
    }
    
    /* Allocate memory for stock data array */
    StockData* stockData = (StockData*)malloc(itemCount * sizeof(StockData));
    if (!stockData) {
        logError(ERR_OUT_OF_MEMORY, "Memory allocation failed for stock data array");
        cJSON_Delete(json);
        return 0;
    }
    
    /* Process each item in the array */
    int validItems = 0;
    for (int i = 0; i < itemCount; i++) {
        cJSON* item = cJSON_GetArrayItem(dataItems, i);
        if (!item) continue;
        
        /* Initialize with default values */
        memset(&stockData[validItems], 0, sizeof(StockData));
        stockData[validItems].date[0] = '\0';
        stockData[validItems].open = 0.0;
        stockData[validItems].high = 0.0;
        stockData[validItems].low = 0.0;
        stockData[validItems].close = 0.0;
        stockData[validItems].volume = 0;
        
        /* Extract date */
        cJSON* dateField = cJSON_GetObjectItem(item, "date");
        if (dateField && cJSON_IsString(dateField) && dateField->valuestring) {
            strncpy(stockData[validItems].date, dateField->valuestring, sizeof(stockData[validItems].date) - 1);
            stockData[validItems].date[sizeof(stockData[validItems].date) - 1] = '\0';
        }
        
        /* Extract price data - handle different possible field names */
        cJSON* open = cJSON_GetObjectItem(item, "open");
        if (!open) open = cJSON_GetObjectItem(item, "openPrice");
        if (open && cJSON_IsNumber(open)) {
            stockData[validItems].open = open->valuedouble;
        }
        
        cJSON* high = cJSON_GetObjectItem(item, "high");
        if (!high) high = cJSON_GetObjectItem(item, "highPrice");
        if (high && cJSON_IsNumber(high)) {
            stockData[validItems].high = high->valuedouble;
        }
        
        cJSON* low = cJSON_GetObjectItem(item, "low");
        if (!low) low = cJSON_GetObjectItem(item, "lowPrice");
        if (low && cJSON_IsNumber(low)) {
            stockData[validItems].low = low->valuedouble;
        }
        
        cJSON* close = cJSON_GetObjectItem(item, "close");
        if (!close) close = cJSON_GetObjectItem(item, "closePrice");
        if (!close) close = cJSON_GetObjectItem(item, "adjClose");
        if (close && cJSON_IsNumber(close)) {
            stockData[validItems].close = close->valuedouble;
        }
        
        cJSON* volume = cJSON_GetObjectItem(item, "volume");
        if (!volume) volume = cJSON_GetObjectItem(item, "adjVolume");
        if (volume && cJSON_IsNumber(volume)) {
            stockData[validItems].volume = (long)volume->valuedouble;
        }
        
        /* Only count the item if we have at least a date and close price */
        if (stockData[validItems].date[0] != '\0' && stockData[validItems].close > 0) {
            validItems++;
        }
    }
    
    /* Handle the case where we didn't parse any valid items */
    if (validItems == 0) {
        free(stockData);
        logError(ERR_DATA_CORRUPTED, "No valid data points found in JSON response");
        cJSON_Delete(json);
        return 0;
    }
    
    /* If we have fewer valid items than the total, resize the array */
    if (validItems < itemCount) {
        StockData* resized = (StockData*)realloc(stockData, validItems * sizeof(StockData));
        if (resized) {
            stockData = resized;
        } else {
            /* If realloc fails, we can still proceed with the original array */
            logWarning("Memory reallocation failed for stock data array resizing");
        }
    }
    
    *dataArray = stockData;
    *dataCount = validItems;
    
    cJSON_Delete(json);
    return 1;
}

/**
 * Fetch news data using MarketAux API instead of Tiingo News API
 */
int fetchNewsFeed(const char* symbols, EventDatabase* events) {
    if (!symbols || !events) {
        logError(ERR_INVALID_PARAMETER, "Invalid parameters passed to fetchNewsFeed");
        return ERR_INVALID_PARAMETER;
    }

    const char* marketauxKey = getMarketAuxAPIKey();
    const char* apiKey = getTiingoAPIKey();
    
    if (!marketauxKey || strlen(marketauxKey) < 1) {
        logError(ERR_INVALID_PARAMETER, "MarketAux API key not set");
        return ERR_INVALID_PARAMETER;
    }

    char responseBuffer[MAX_BUFFER_SIZE];
    char tempFilePath[256];
    getTempFilePath(tempFilePath, sizeof(tempFilePath), "news_response.json");
    
    // Use a simpler curl command with no problematic options
    char curlCmd[1024];
    snprintf(curlCmd, sizeof(curlCmd),
        "curl -s -o \"%s\" \"https://api.marketaux.com/v1/news/all?symbols=%s&filter_entities=true&language=en&api_token=%s&limit=50\" 2> curl_error.txt",
        tempFilePath, symbols, marketauxKey);
    
    int result = system(curlCmd);
    if (result != 0) {
        // Read error message if any
        FILE* error_file = fopen("curl_error.txt", "r");
        char error_msg[256] = "Unknown curl error";
        
        if (error_file) {
            if (fgets(error_msg, sizeof(error_msg), error_file)) {
                /* Remove newline character if present */
                char* newline = strchr(error_msg, '\n');
                if (newline) *newline = '\0';
            }
            fclose(error_file);
        }
        
        logError(ERR_API_REQUEST_FAILED, "Failed to execute curl command: %s", error_msg);
        
        // Try fallback to Tiingo API
        if (apiKey && strlen(apiKey) > 0) {
            logMessage(LOG_INFO, "Trying fallback to Tiingo API for news data");
            snprintf(curlCmd, sizeof(curlCmd),
                "curl -s -o \"%s\" \"https://api.tiingo.com/tiingo/news?tickers=%s&limit=50&format=json\" "
                "-H \"Authorization: Token %s\" 2> curl_error.txt",
                tempFilePath, symbols, apiKey);
            
            system(curlCmd);
        }
    }

    // Read the response from the temp file
    FILE* responseFile = fopen(tempFilePath, "r");
    if (!responseFile) {
        logError(ERR_FILE_OPEN_FAILED, "Failed to open response file");
        return ERR_FILE_OPEN_FAILED;
    }
    
    size_t bytesRead = fread(responseBuffer, 1, sizeof(responseBuffer) - 1, responseFile);
    fclose(responseFile);
    
    if (bytesRead == 0) {
        logError(ERR_DATA_CORRUPTED, "Empty response from API");
        return ERR_DATA_CORRUPTED;
    }
    
    responseBuffer[bytesRead] = '\0';
    
    // Test the JSON response
    cJSON* testJson = cJSON_Parse(responseBuffer);
    if (!testJson) {
        logError(ERR_DATA_CORRUPTED, "Invalid JSON in response");
        return ERR_DATA_CORRUPTED;
    }
    cJSON_Delete(testJson);
    
    // Parse the news data and add it to the events database
    int count = parseNewsDataJSON(responseBuffer, events);
    
    // Return success even with partial data as long as we got at least one item
    return (count > 0) ? SUCCESS : ERR_DATA_CORRUPTED;
}

/* Parse news data from JSON response (handles both Tiingo and MarketAux formats) */
int parseNewsDataJSON(const char* jsonData, EventDatabase* events) {
    if (!jsonData || !events) {
        logError(ERR_INVALID_PARAMETER, "Invalid parameters for parseNewsDataJSON");
        return 0;
    }

    cJSON* root = cJSON_Parse(jsonData);
    if (!root) {
        const char* errorPtr = cJSON_GetErrorPtr();
        if (errorPtr) {
            logError(ERR_DATA_CORRUPTED, "JSON parse error before: %s", errorPtr);
        } else {
            logError(ERR_DATA_CORRUPTED, "JSON parse error at unknown location");
        }
        return 0;
    }

    int count = 0;
    
    // First, check if this is a MarketAux format (has "data" array at root)
    cJSON* dataArray = cJSON_GetObjectItem(root, "data");
    
    if (dataArray && cJSON_IsArray(dataArray)) {
        // MarketAux format
        int dataSize = cJSON_GetArraySize(dataArray);
        if (dataSize == 0) {
            logError(ERR_DATA_CORRUPTED, "Empty data array in JSON response");
            cJSON_Delete(root);
            return 0;
        }
        
        for (int i = 0; i < dataSize && i < MAX_NEWS_ITEMS; i++) {
            cJSON* newsItem = cJSON_GetArrayItem(dataArray, i);
            
            // Extract fields from MarketAux format
            cJSON* title = cJSON_GetObjectItem(newsItem, "title");
            cJSON* description = cJSON_GetObjectItem(newsItem, "description");
            cJSON* url = cJSON_GetObjectItem(newsItem, "url");
            cJSON* publishedAt = cJSON_GetObjectItem(newsItem, "published_at");
            cJSON* sentiment = cJSON_GetObjectItem(newsItem, "sentiment");
            
            if (title && cJSON_IsString(title) && 
                publishedAt && cJSON_IsString(publishedAt)) {
                
                EventData event;
                memset(&event, 0, sizeof(EventData));
                
                strncpy(event.title, cJSON_GetStringValue(title), MAX_BUFFER_SIZE - 1);
                
                if (description && cJSON_IsString(description)) {
                    strncpy(event.description, cJSON_GetStringValue(description), MAX_BUFFER_SIZE - 1);
                } else {
                    event.description[0] = '\0';
                }
                
                if (url && cJSON_IsString(url)) {
                    strncpy(event.url, cJSON_GetStringValue(url), MAX_URL_LENGTH - 1);
                } else {
                    event.url[0] = '\0';
                }
                
                // Parse published_at time string (ISO 8601 format)
                event.timestamp = parseISOTimeString(cJSON_GetStringValue(publishedAt));
                strftime(event.date, MAX_DATE_LENGTH, "%Y-%m-%d", localtime(&event.timestamp));
                
                // Parse sentiment if available
                if (sentiment && cJSON_IsObject(sentiment)) {
                    cJSON* sentimentScore = cJSON_GetObjectItem(sentiment, "score");
                    if (sentimentScore && cJSON_IsNumber(sentimentScore)) {
                        event.sentiment = (float)cJSON_GetNumberValue(sentimentScore);
                    } else {
                        event.sentiment = 0.0f;
                    }
                } else {
                    event.sentiment = 0.0f;
                }
                
                // Calculate impact score
                event.impactScore = (int)calculateImpactScore(&event);
                
                // // Add to database
                // if (addEventToDatabase(events, &event)) {
                //     count++;
                // }
            }
        }
    } else {
        // Try Tiingo format (root is array)
        if (cJSON_IsArray(root)) {
            int arraySize = cJSON_GetArraySize(root);
            if (arraySize == 0) {
                logError(ERR_DATA_CORRUPTED, "Empty array in JSON response");
                cJSON_Delete(root);
                return 0;
            }
            
            for (int i = 0; i < arraySize && i < MAX_NEWS_ITEMS; i++) {
                cJSON* newsItem = cJSON_GetArrayItem(root, i);
                
                // Extract fields from Tiingo format
                cJSON* title = cJSON_GetObjectItem(newsItem, "title");
                cJSON* description = cJSON_GetObjectItem(newsItem, "description");
                cJSON* url = cJSON_GetObjectItem(newsItem, "url");
                cJSON* publishedDate = cJSON_GetObjectItem(newsItem, "publishedDate");
                
                if (title && cJSON_IsString(title) && 
                    publishedDate && cJSON_IsString(publishedDate)) {
                    
                    EventData event;
                    memset(&event, 0, sizeof(EventData));
                    
                    strncpy(event.title, cJSON_GetStringValue(title), MAX_BUFFER_SIZE - 1);
                    
                    if (description && cJSON_IsString(description)) {
                        strncpy(event.description, cJSON_GetStringValue(description), MAX_BUFFER_SIZE - 1);
                    } else {
                        event.description[0] = '\0';
                    }
                    
                    if (url && cJSON_IsString(url)) {
                        strncpy(event.url, cJSON_GetStringValue(url), MAX_URL_LENGTH - 1);
                    } else {
                        event.url[0] = '\0';
                    }
                    
                    // Parse published_at time string
                    event.timestamp = parseISOTimeString(cJSON_GetStringValue(publishedDate));
                    strftime(event.date, MAX_DATE_LENGTH, "%Y-%m-%d", localtime(&event.timestamp));
                    
                    // No sentiment in Tiingo API by default
                    event.sentiment = 0.0f;
                    
                    // Calculate impact score
                    event.impactScore = (int)calculateImpactScore(&event);
                    
                    // // Add to database
                    // if (addEventToDatabase(events, &event)) {
                    //     count++;
                    // }
                }
            }
        } else {
            logError(ERR_DATA_CORRUPTED, "Invalid JSON format: array not found");
            cJSON_Delete(root);
            return 0;
        }
    }
    
    cJSON_Delete(root);
    
    if (count == 0) {
        logError(ERR_DATA_CORRUPTED, "Failed to parse any data points from JSON response");
    }
    
    return count;
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

/* Fetch historical data with CSV cache */
int fetchHistoricalDataWithCache(const char* symbol, const char* startDate, const char* endDate, Stock* stock) {
    if (!symbol || !startDate || !endDate || !stock) {
        logError(ERR_INVALID_PARAMETER, "Invalid parameters for fetchHistoricalDataWithCache");
        return 0;
    }
    
    /* Initialize the stock structure */
    strncpy(stock->symbol, symbol, MAX_SYMBOL_LENGTH - 1);
    stock->symbol[MAX_SYMBOL_LENGTH - 1] = '\0';
    stock->dataSize = 0;
    
    /* Check if data exists in cache */
    if (checkCSVDataExists(symbol, startDate, endDate)) {
        /* Load data from cache */
        logMessage(LOG_INFO, "Loading cached data for %s (%s to %s)", symbol, startDate, endDate);
        return loadStockDataFromCSV(symbol, startDate, endDate, stock);
    }
    
    /* Data not in cache, fetch from API */
    logMessage(LOG_INFO, "Fetching data from API for %s (%s to %s)", symbol, startDate, endDate);
    int success = fetchStockData(symbol, startDate, endDate, stock);
    
    /* If fetch was successful, save to cache */
    if (success) {
        saveStockDataToCSV(stock, startDate, endDate);
    }
    
    return success;
}

/* Get a temporary file path */
void getTempFilePath(char* buffer, size_t bufferSize, const char* filename) {
    if (!buffer || bufferSize == 0) return;
    
    const char* tempDir = getenv("TEMP");
    if (!tempDir) tempDir = getenv("TMP");
    if (!tempDir) tempDir = "."; // Use current directory as fallback
    
    snprintf(buffer, bufferSize, "%s\\%s", tempDir, filename);
}

/* Parse ISO time string (YYYY-MM-DDTHH:MM:SSZ) to time_t */
time_t parseISOTimeString(const char* timeString) {
    if (!timeString || strlen(timeString) < 10) return 0;
    
    struct tm tm = {0};
    char datePart[11] = {0};
    
    // Copy just the date part (YYYY-MM-DD)
    strncpy(datePart, timeString, 10);
    datePart[10] = '\0';
    
    // Parse date
    sscanf(datePart, "%d-%d-%d", &tm.tm_year, &tm.tm_mon, &tm.tm_mday);
    tm.tm_year -= 1900; // Adjust year (tm_year is years since 1900)
    tm.tm_mon -= 1;     // Adjust month (tm_mon is 0-11)
    
    // Parse time if available
    if (strlen(timeString) >= 19) {
        sscanf(timeString + 11, "%d:%d:%d", &tm.tm_hour, &tm.tm_min, &tm.tm_sec);
    }
    
    // Convert to time_t
    return mktime(&tm);
}