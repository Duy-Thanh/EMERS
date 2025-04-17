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
    
    /* Allocate memory for stock data if it hasn't been allocated yet
       or increase capacity if needed */
    int initialCapacity = 100; // Reasonable initial size
    if (stock->data == NULL) {
        stock->data = (StockData*)malloc(initialCapacity * sizeof(StockData));
        if (!stock->data) {
            logError(ERR_OUT_OF_MEMORY, "Failed to allocate memory for stock data");
            free(response.data);
            return 0;
        }
        stock->dataCapacity = initialCapacity;
        stock->dataSize = 0;
    } else if (stock->dataCapacity <= stock->dataSize) {
        /* Need to expand capacity */
        int newCapacity = stock->dataCapacity * 2;
        StockData* newData = (StockData*)realloc(stock->data, newCapacity * sizeof(StockData));
        if (!newData) {
            logError(ERR_OUT_OF_MEMORY, "Failed to reallocate memory for stock data");
            free(response.data);
            return 0;
        }
        stock->data = newData;
        stock->dataCapacity = newCapacity;
    }
    
    /* Parse JSON response */
    success = parseStockDataJSON(response.data, stock->data + stock->dataSize, stock->dataCapacity - stock->dataSize);
    
    if (success > 0) {
        /* Update the data size with the number of items parsed */
        stock->dataSize += success;
    }
    
    /* Clean up */
    free(response.data);
    
    return success;
}

/* Parse JSON response from Tiingo API for stock data */
int parseStockDataJSON(const char* jsonData, StockData* data, int maxItems) {
    if (!jsonData || !data || maxItems <= 0) {
        logError(ERR_INVALID_PARAMETER, "Invalid parameters for parseStockDataJSON");
        return 0;
    }

    /* Use cJSON to parse the API response */
    cJSON *json = cJSON_Parse(jsonData);
    if (!json) {
        const char* errorPtr = cJSON_GetErrorPtr();
        if (errorPtr) {
            logError(ERR_DATA_CORRUPTED, "JSON parsing error: %s", errorPtr);
        } else {
            logError(ERR_DATA_CORRUPTED, "Unknown JSON parsing error");
        }
        return 0;
    }

    /* Check if the response is an error message or empty */
    if (cJSON_IsObject(json)) {
        cJSON *errorObj = cJSON_GetObjectItemCaseSensitive(json, "error");
        cJSON *messageObj = cJSON_GetObjectItemCaseSensitive(json, "message");
        
        if (cJSON_IsString(errorObj) || cJSON_IsString(messageObj)) {
            char errorMsg[256] = "Unknown API error";
            
            if (cJSON_IsString(errorObj) && errorObj->valuestring) {
                snprintf(errorMsg, sizeof(errorMsg), "API error: %s", errorObj->valuestring);
            } else if (cJSON_IsString(messageObj) && messageObj->valuestring) {
                snprintf(errorMsg, sizeof(errorMsg), "API message: %s", messageObj->valuestring);
            }
            
            /* Check for specific types of errors */
            if (strstr(jsonData, "api_key") || strstr(jsonData, "token")) {
                logError(ERR_API_RESPONSE_INVALID, "API key error: %s", errorMsg);
            } 
            else if (strstr(jsonData, "not found") || strstr(jsonData, "notFound")) {
                logError(ERR_API_RESPONSE_INVALID, "Ticker not found: %s", errorMsg);
            }
            else {
                logError(ERR_API_REQUEST_FAILED, "API error: %s", errorMsg);
            }
            
            cJSON_Delete(json);
            return 0;
        }
    }

    /* Determine if we're dealing with a single object response or an array */
    cJSON *priceArray = NULL;
    
    if (cJSON_IsArray(json)) {
        priceArray = json;
    } else if (cJSON_IsObject(json)) {
        /* For single ticker responses, the data might be directly in the object */
        /* First, check if this is a single price object with fields like 'close', 'open', etc. */
        cJSON *closeObj = cJSON_GetObjectItemCaseSensitive(json, "close");
        if (cJSON_IsNumber(closeObj)) {
            /* This appears to be a single price entry - treat as an array of one */
            priceArray = cJSON_CreateArray();
            cJSON_AddItemToArray(priceArray, cJSON_Duplicate(json, 1));
        } else {
            /* Check for common fields that might contain the price array */
            cJSON *dataObj = cJSON_GetObjectItemCaseSensitive(json, "data");
            if (dataObj && cJSON_IsArray(dataObj)) {
                priceArray = dataObj;
            } else {
                cJSON *pricesObj = cJSON_GetObjectItemCaseSensitive(json, "prices");
                if (pricesObj && cJSON_IsArray(pricesObj)) {
                    priceArray = pricesObj;
                } else {
                    cJSON *resultObj = cJSON_GetObjectItemCaseSensitive(json, "result");
                    if (resultObj && cJSON_IsArray(resultObj)) {
                        priceArray = resultObj;
                    }
                }
            }
        }
    }

    if (!priceArray) {
        logError(ERR_DATA_CORRUPTED, "Invalid stock data JSON format: can't find price data");
        if (priceArray != json) {
            cJSON_Delete(priceArray);  /* Only delete if we created it */
        }
        cJSON_Delete(json);
        return 0;
    }

    /* Extract price data */
    int count = 0;
    int arraySize = cJSON_GetArraySize(priceArray);
    
    for (int i = 0; i < arraySize && count < maxItems; i++) {
        cJSON *item = cJSON_GetArrayItem(priceArray, i);
        if (!cJSON_IsObject(item)) {
            continue;  /* Skip non-object items */
        }
        
        /* Extract date */
        cJSON *dateObj = cJSON_GetObjectItemCaseSensitive(item, "date");
        if (!cJSON_IsString(dateObj)) {
            /* Try alternate date fields */
            dateObj = cJSON_GetObjectItemCaseSensitive(item, "datetime");
            if (!cJSON_IsString(dateObj)) {
                dateObj = cJSON_GetObjectItemCaseSensitive(item, "timestamp");
            }
        }

        if (cJSON_IsString(dateObj) && dateObj->valuestring) {
            strncpy(data[count].date, dateObj->valuestring, sizeof(data[count].date) - 1);
            data[count].date[sizeof(data[count].date) - 1] = '\0';
            
            /* Truncate time part if present */
            char *timePos = strchr(data[count].date, 'T');
            if (timePos != NULL) {
                *timePos = '\0';
            }
        } else {
            /* Use current date if not available */
            time_t now = time(NULL);
            struct tm *timeinfo = localtime(&now);
            strftime(data[count].date, sizeof(data[count].date), "%Y-%m-%d", timeinfo);
        }

        /* Extract open price */
        cJSON *openObj = cJSON_GetObjectItemCaseSensitive(item, "open");
        if (cJSON_IsNumber(openObj)) {
            data[count].open = (float)openObj->valuedouble;
        } else {
            data[count].open = 0.0f;
        }

        /* Extract high price */
        cJSON *highObj = cJSON_GetObjectItemCaseSensitive(item, "high");
        if (cJSON_IsNumber(highObj)) {
            data[count].high = (float)highObj->valuedouble;
        } else {
            data[count].high = 0.0f;
        }

        /* Extract low price */
        cJSON *lowObj = cJSON_GetObjectItemCaseSensitive(item, "low");
        if (cJSON_IsNumber(lowObj)) {
            data[count].low = (float)lowObj->valuedouble;
        } else {
            data[count].low = 0.0f;
        }

        /* Extract close price */
        cJSON *closeObj = cJSON_GetObjectItemCaseSensitive(item, "close");
        if (cJSON_IsNumber(closeObj)) {
            data[count].close = (float)closeObj->valuedouble;
        } else {
            data[count].close = 0.0f;
        }

        /* Extract volume */
        cJSON *volumeObj = cJSON_GetObjectItemCaseSensitive(item, "volume");
        if (cJSON_IsNumber(volumeObj)) {
            data[count].volume = (long)volumeObj->valuedouble;
        } else {
            data[count].volume = 0;
        }

        /* Extract adjusted close if available */
        cJSON *adjCloseObj = cJSON_GetObjectItemCaseSensitive(item, "adjClose");
        if (!cJSON_IsNumber(adjCloseObj)) {
            adjCloseObj = cJSON_GetObjectItemCaseSensitive(item, "adjclose");
            if (!cJSON_IsNumber(adjCloseObj)) {
                adjCloseObj = cJSON_GetObjectItemCaseSensitive(item, "adjusted_close");
                if (!cJSON_IsNumber(adjCloseObj)) {
                    adjCloseObj = cJSON_GetObjectItemCaseSensitive(item, "adjustedClose");
                }
            }
        }

        if (cJSON_IsNumber(adjCloseObj)) {
            data[count].adjClose = (float)adjCloseObj->valuedouble;
        } else {
            /* If adjusted close is not available, use regular close */
            data[count].adjClose = data[count].close;
        }

        count++;
    }

    /* Clean up */
    if (priceArray != json && priceArray != NULL) {
        cJSON_Delete(priceArray);  /* Only delete if we created it */
    }
    cJSON_Delete(json);

    return count;
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
    // First try with marketaux API which is more reliable for news
    char curlCmd[1024];
    char tempFilePath[256];
    getTempFilePath(tempFilePath, sizeof(tempFilePath), "news_response.json");
    
    // Removed the problematic --dns-servers option
    snprintf(curlCmd, sizeof(curlCmd),
        "curl -s -o \"%s\" -w \"%%{http_code}\" "
        "\"https://api.marketaux.com/v1/news/all?symbols=%s&filter_entities=true&language=en&api_token=%s&limit=50\"",
        tempFilePath, symbols, marketauxKey);
    
    char statusCodeStr[8] = {0};
    FILE* fp = popen(curlCmd, "r");
    if (!fp) {
        logError(ERR_API_REQUEST_FAILED, "Failed to execute curl command");
        
        // Try an even simpler fallback command if the first one fails
        snprintf(curlCmd, sizeof(curlCmd),
            "curl -s \"https://api.marketaux.com/v1/news/all?symbols=%s&api_token=%s&limit=10\" -o \"%s\"",
            symbols, marketauxKey, tempFilePath);
        
        system(curlCmd);
        goto read_response;
    }
    
    if (fgets(statusCodeStr, sizeof(statusCodeStr), fp) == NULL) {
        pclose(fp);
        logError(ERR_API_REQUEST_FAILED, "Failed to read curl output");
        
        // Try an even simpler fallback command
        snprintf(curlCmd, sizeof(curlCmd),
            "curl -s \"https://api.marketaux.com/v1/news/all?symbols=%s&api_token=%s&limit=10\" -o \"%s\"",
            symbols, marketauxKey, tempFilePath);
        
        system(curlCmd);
        goto read_response;
    }
    
    int statusCode = atoi(statusCodeStr);
    pclose(fp);
    
    if (statusCode != 200) {
        // Log specific error based on status code
        if (statusCode == 403) {
            logError(ERR_API_REQUEST_FAILED, "API permission error: Your API key doesn't have access to this feature - URL: https://api.marketaux.com/v1/news/all, Status: %d", statusCode);
        } else if (statusCode == 429) {
            logError(ERR_API_REQUEST_FAILED, "API rate limit exceeded - URL: https://api.marketaux.com/v1/news/all, Status: %d", statusCode);
        } else {
            logError(ERR_API_REQUEST_FAILED, "API request failed with status code %d - URL: https://api.marketaux.com/v1/news/all", statusCode);
        }
        
        // Try fallback to Tiingo if MarketAux fails
        if (apiKey && strlen(apiKey) > 0) {
            snprintf(curlCmd, sizeof(curlCmd),
                "curl -s \"https://api.tiingo.com/tiingo/news?tickers=%s&limit=50&format=json\" "
                "-H \"Authorization: Token %s\" -o \"%s\"",
                symbols, apiKey, tempFilePath);
            
            system(curlCmd);
        } else {
            // Even if the request failed, let's check if we have a response file with content
            goto read_response;
        }
    }

read_response:
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
    
    // Verify we got valid JSON by checking for opening brackets
    if (responseBuffer[0] != '{' && responseBuffer[0] != '[') {
        logError(ERR_DATA_CORRUPTED, "Invalid JSON response: %s", 
            strlen(responseBuffer) > 100 ? 
            "Response too large to display" : responseBuffer);
        return ERR_DATA_CORRUPTED;
    }
    
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
                
                // Add to database
                if (addEventToDatabase(events, &event)) {
                    count++;
                }
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
                    
                    // Add to database
                    if (addEventToDatabase(events, &event)) {
                        count++;
                    }
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