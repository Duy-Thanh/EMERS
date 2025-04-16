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
#include <cJSON.h>    /* For JSON parsing */

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
    success = parseStockDataJSON(response.data, stock->data, stock->dataCapacity - stock->dataSize);
    
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
    
    /* Improved curl command with better options - removed problematic --dns-servers option */
    char command[MAX_BUFFER_SIZE];
    snprintf(command, sizeof(command),
             "curl -s --connect-timeout 30 --max-time 60 --retry 5 --retry-delay 2 --retry-connrefused "
             "--retry-max-time 120 \"%s\" > curl_output.json 2>curl_error.txt",
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
    int success = parseNewsDataJSON(response.data, events->events, events->eventCapacity - events->eventCount);
    
    /* Update event count */
    events->eventCount += success;
    
    /* Clean up */
    free(response.data);
    
    return success;
}

/* Parse JSON response from MarketAux API for news data */
int parseNewsDataJSON(const char* jsonData, NewsItem* news, int maxItems) {
    if (!jsonData || !news || maxItems <= 0) {
        logError(ERR_INVALID_PARAMETER, "Invalid parameters for parseNewsDataJSON");
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

    /* Check if the response is an error message */
    cJSON *errorObj = cJSON_GetObjectItemCaseSensitive(json, "error");
    if (cJSON_IsString(errorObj) || cJSON_IsObject(errorObj)) {
        char errorMsg[256] = "Unknown API error";
        
        if (cJSON_IsString(errorObj) && errorObj->valuestring) {
            snprintf(errorMsg, sizeof(errorMsg), "API error: %s", errorObj->valuestring);
        }
        
        /* Check for specific types of errors */
        if (strstr(jsonData, "api_token") || strstr(jsonData, "token")) {
            logError(ERR_API_RESPONSE_INVALID, "API key error in news response: %s", errorMsg);
        } 
        else if (strstr(jsonData, "permission") || strstr(jsonData, "unauthorized")) {
            logError(ERR_API_REQUEST_FAILED, "API permission error: %s", errorMsg);
        }
        else {
            logError(ERR_API_REQUEST_FAILED, "API error: %s", errorMsg);
        }
        
        cJSON_Delete(json);
        return 0;
    }

    /* Determine the response structure */
    cJSON *newsArray = NULL;
    
    /* Try to find a news array - could be at root or under a data field */
    if (cJSON_IsArray(json)) {
        newsArray = json;
    } else {
        /* Check for common fields that might contain the news array */
        cJSON *dataObj = cJSON_GetObjectItemCaseSensitive(json, "data");
        if (dataObj && cJSON_IsArray(dataObj)) {
            newsArray = dataObj;
        } else {
            cJSON *articlesObj = cJSON_GetObjectItemCaseSensitive(json, "articles");
            if (articlesObj && cJSON_IsArray(articlesObj)) {
                newsArray = articlesObj;
            } else {
                cJSON *resultsObj = cJSON_GetObjectItemCaseSensitive(json, "results");
                if (resultsObj && cJSON_IsArray(resultsObj)) {
                    newsArray = resultsObj;
                }
            }
        }
    }

    if (!newsArray) {
        logError(ERR_DATA_CORRUPTED, "Invalid news JSON format: can't find news array");
        cJSON_Delete(json);
        return 0;
    }

    /* Extract news items */
    int count = 0;
    int arraySize = cJSON_GetArraySize(newsArray);
    
    for (int i = 0; i < arraySize && count < maxItems; i++) {
        cJSON *item = cJSON_GetArrayItem(newsArray, i);
        if (!cJSON_IsObject(item)) {
            continue;  /* Skip non-object items */
        }

        /* Extract title */
        cJSON *titleObj = cJSON_GetObjectItemCaseSensitive(item, "title");
        if (cJSON_IsString(titleObj) && titleObj->valuestring) {
            strncpy(news[count].title, titleObj->valuestring, sizeof(news[count].title) - 1);
            news[count].title[sizeof(news[count].title) - 1] = '\0';
        } else {
            /* Try headline if title is not available */
            cJSON *headlineObj = cJSON_GetObjectItemCaseSensitive(item, "headline");
            if (cJSON_IsString(headlineObj) && headlineObj->valuestring) {
                strncpy(news[count].title, headlineObj->valuestring, sizeof(news[count].title) - 1);
                news[count].title[sizeof(news[count].title) - 1] = '\0';
            } else {
                strcpy(news[count].title, "[No Title]");
            }
        }

        /* Extract description */
        cJSON *descObj = cJSON_GetObjectItemCaseSensitive(item, "description");
        if (cJSON_IsString(descObj) && descObj->valuestring) {
            strncpy(news[count].description, descObj->valuestring, sizeof(news[count].description) - 1);
            news[count].description[sizeof(news[count].description) - 1] = '\0';
        } else {
            /* Try summary if description is not available */
            cJSON *summaryObj = cJSON_GetObjectItemCaseSensitive(item, "summary");
            if (cJSON_IsString(summaryObj) && summaryObj->valuestring) {
                strncpy(news[count].description, summaryObj->valuestring, sizeof(news[count].description) - 1);
                news[count].description[sizeof(news[count].description) - 1] = '\0';
            } else {
                /* Try snippet if summary is not available */
                cJSON *snippetObj = cJSON_GetObjectItemCaseSensitive(item, "snippet");
                if (cJSON_IsString(snippetObj) && snippetObj->valuestring) {
                    strncpy(news[count].description, snippetObj->valuestring, sizeof(news[count].description) - 1);
                    news[count].description[sizeof(news[count].description) - 1] = '\0';
                } else {
                    strcpy(news[count].description, "[No Description]");
                }
            }
        }

        /* Extract published date */
        cJSON *dateObj = cJSON_GetObjectItemCaseSensitive(item, "published_date");
        if (!cJSON_IsString(dateObj) || !dateObj->valuestring) {
            /* Try alternate date fields */
            dateObj = cJSON_GetObjectItemCaseSensitive(item, "publishedDate");
            if (!cJSON_IsString(dateObj) || !dateObj->valuestring) {
                dateObj = cJSON_GetObjectItemCaseSensitive(item, "date");
                if (!cJSON_IsString(dateObj) || !dateObj->valuestring) {
                    dateObj = cJSON_GetObjectItemCaseSensitive(item, "datetime");
                    if (!cJSON_IsString(dateObj) || !dateObj->valuestring) {
                        dateObj = cJSON_GetObjectItemCaseSensitive(item, "created_at");
                    }
                }
            }
        }

        if (cJSON_IsString(dateObj) && dateObj->valuestring) {
            strncpy(news[count].date, dateObj->valuestring, sizeof(news[count].date) - 1);
            news[count].date[sizeof(news[count].date) - 1] = '\0';
        } else {
            /* Use current date if not available */
            time_t now = time(NULL);
            struct tm *timeinfo = localtime(&now);
            strftime(news[count].date, sizeof(news[count].date), "%Y-%m-%d", timeinfo);
        }

        /* Extract source */
        cJSON *sourceObj = cJSON_GetObjectItemCaseSensitive(item, "source");
        if (cJSON_IsString(sourceObj) && sourceObj->valuestring) {
            strncpy(news[count].source, sourceObj->valuestring, sizeof(news[count].source) - 1);
            news[count].source[sizeof(news[count].source) - 1] = '\0';
        } else {
            /* Check if source is an object with a name field */
            if (cJSON_IsObject(sourceObj)) {
                cJSON *sourceNameObj = cJSON_GetObjectItemCaseSensitive(sourceObj, "name");
                if (cJSON_IsString(sourceNameObj) && sourceNameObj->valuestring) {
                    strncpy(news[count].source, sourceNameObj->valuestring, sizeof(news[count].source) - 1);
                    news[count].source[sizeof(news[count].source) - 1] = '\0';
                } else {
                    strcpy(news[count].source, "Unknown");
                }
            } else {
                strcpy(news[count].source, "Unknown");
            }
        }

        /* Extract URL */
        cJSON *urlObj = cJSON_GetObjectItemCaseSensitive(item, "url");
        if (!cJSON_IsString(urlObj) || !urlObj->valuestring) {
            /* Try alternate URL fields */
            urlObj = cJSON_GetObjectItemCaseSensitive(item, "link");
            if (!cJSON_IsString(urlObj) || !urlObj->valuestring) {
                urlObj = cJSON_GetObjectItemCaseSensitive(item, "web_url");
            }
        }

        if (cJSON_IsString(urlObj) && urlObj->valuestring) {
            strncpy(news[count].url, urlObj->valuestring, sizeof(news[count].url) - 1);
            news[count].url[sizeof(news[count].url) - 1] = '\0';
        } else {
            strcpy(news[count].url, "");
        }

        count++;
    }

    /* Clean up */
    cJSON_Delete(json);

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