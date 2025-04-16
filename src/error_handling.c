/**
 * Emergency Market Event Response System (EMERS)
 * Error Handling & Logging Module Implementation
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>

#include "../include/error_handling.h"

/* Static variables */
static FILE* logFile = NULL;
static LogLevel minFileLogLevel = LOG_INFO;
static LogLevel minConsoleLogLevel = LOG_WARNING;
static int lastErrorCode = ERR_SUCCESS;
static char lastErrorMessage[1024] = {0};
static ErrorCallback errorCallback = NULL;

/* Error code mapping */
static const ErrorCode errorCodes[] = {
    {ERR_SUCCESS,               ERR_NONE,       LOG_DEBUG,   "Success"},
    {ERR_NULL_POINTER,          ERR_MEMORY,     LOG_ERROR,   "Null pointer error"},
    {ERR_OUT_OF_MEMORY,         ERR_MEMORY,     LOG_CRITICAL,"Out of memory"},
    {ERR_INVALID_PARAMETER,     ERR_PARAM,      LOG_ERROR,   "Invalid parameter"},
    {ERR_API_INITIALIZATION,    ERR_INIT,       LOG_CRITICAL,"API initialization failed"},
    {ERR_API_REQUEST_FAILED,    ERR_API,        LOG_ERROR,   "API request failed"},
    {ERR_API_RESPONSE_INVALID,  ERR_API,        LOG_ERROR,   "Invalid API response"},
    {ERR_FILE_NOT_FOUND,        ERR_IO,         LOG_ERROR,   "File not found"},
    {ERR_FILE_OPEN_FAILED,      ERR_IO,         LOG_ERROR,   "Failed to open file"},
    {ERR_FILE_READ_FAILED,      ERR_IO,         LOG_ERROR,   "Failed to read file"},
    {ERR_FILE_WRITE_FAILED,     ERR_IO,         LOG_ERROR,   "Failed to write file"},
    {ERR_CALCULATION_OVERFLOW,  ERR_CALCULATION,LOG_ERROR,   "Calculation overflow"},
    {ERR_CALCULATION_UNDERFLOW, ERR_CALCULATION,LOG_ERROR,   "Calculation underflow"},
    {ERR_CALCULATION_DIVIDE_ZERO,ERR_CALCULATION,LOG_ERROR,  "Division by zero"},
    {ERR_DATA_VALIDATION,       ERR_DATA,       LOG_ERROR,   "Data validation failed"},
    {ERR_DATA_INSUFFICIENT,     ERR_DATA,       LOG_WARNING, "Insufficient data"},
    {ERR_DATA_CORRUPTED,        ERR_DATA,       LOG_ERROR,   "Data corrupted"}
};

/* Number of error codes */
static const int numErrorCodes = sizeof(errorCodes) / sizeof(ErrorCode);

/* Get error code details */
static const ErrorCode* getErrorDetails(int code) {
    int i;
    for (i = 0; i < numErrorCodes; i++) {
        if (errorCodes[i].code == code) {
            return &errorCodes[i];
        }
    }
    return NULL;
}

/* Convert log level to string */
static const char* logLevelToString(LogLevel level) {
    switch (level) {
        case LOG_DEBUG:   return "DEBUG";
        case LOG_INFO:    return "INFO";
        case LOG_WARNING: return "WARNING";
        case LOG_ERROR:   return "ERROR";
        case LOG_CRITICAL:return "CRITICAL";
        default:          return "UNKNOWN";
    }
}

/* Initialize the error handling and logging system */
int initErrorHandling(const char* logFilePath, LogLevel minFileLevel, LogLevel minConsoleLevel) {
    /* Close existing log file if open */
    if (logFile != NULL) {
        fclose(logFile);
        logFile = NULL;
    }
    
    /* Open new log file if path is provided */
    if (logFilePath != NULL && strlen(logFilePath) > 0) {
        logFile = fopen(logFilePath, "a");
        if (logFile == NULL) {
            /* Failed to open log file, but we can continue with console logging */
            fprintf(stderr, "Warning: Failed to open log file '%s'\n", logFilePath);
        }
    }
    
    /* Set log levels */
    minFileLogLevel = minFileLevel;
    minConsoleLogLevel = minConsoleLevel;
    
    /* Initialize error state */
    lastErrorCode = ERR_SUCCESS;
    lastErrorMessage[0] = '\0';
    
    /* Log initialization */
    logMessage(LOG_INFO, "Error handling system initialized");
    
    return 1;
}

/* Clean up the error handling and logging system */
void cleanupErrorHandling(void) {
    /* Log shutdown */
    logMessage(LOG_INFO, "Error handling system shutting down");
    
    /* Close log file if open */
    if (logFile != NULL) {
        fclose(logFile);
        logFile = NULL;
    }
}

/* Set the minimum log level for console output */
void setConsoleLogLevel(LogLevel level) {
    minConsoleLogLevel = level;
}

/* Set the minimum log level for file output */
void setFileLogLevel(LogLevel level) {
    minFileLogLevel = level;
}

/* Log a message */
void logMessage(LogLevel level, const char* format, ...) {
    if ((logFile == NULL || level < minFileLogLevel) && level < minConsoleLogLevel) {
        return;  /* Nothing to log */
    }
    
    /* Get current time */
    time_t now;
    struct tm* timeinfo;
    char timeStr[20];
    
    time(&now);
    timeinfo = localtime(&now);
    strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", timeinfo);
    
    /* Format the message */
    char message[1024];
    va_list args;
    va_start(args, format);
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);
    
    /* Log to file if needed */
    if (logFile != NULL && level >= minFileLogLevel) {
        fprintf(logFile, "[%s] [%s] %s\n", timeStr, logLevelToString(level), message);
        fflush(logFile);  /* Ensure message is written immediately */
    }
    
    /* Log to console if needed */
    if (level >= minConsoleLogLevel) {
        FILE* output = (level >= LOG_ERROR) ? stderr : stdout;
        fprintf(output, "[%s] [%s] %s\n", timeStr, logLevelToString(level), message);
    }
}

/* Log an error with an error code */
void logError(int errorCode, const char* format, ...) {
    /* Store the error code */
    lastErrorCode = errorCode;
    
    /* Get error details */
    const ErrorCode* details = getErrorDetails(errorCode);
    LogLevel level = details ? details->severity : LOG_ERROR;
    
    /* Format the error message */
    char message[1024];
    va_list args;
    va_start(args, format);
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);
    
    /* Store the error message */
    strncpy(lastErrorMessage, message, sizeof(lastErrorMessage) - 1);
    lastErrorMessage[sizeof(lastErrorMessage) - 1] = '\0';
    
    /* Construct full message with error code and details */
    char fullMessage[1200];
    if (details) {
        snprintf(fullMessage, sizeof(fullMessage), "Error %d (%s): %s", 
                errorCode, details->message, message);
    } else {
        snprintf(fullMessage, sizeof(fullMessage), "Error %d: %s", errorCode, message);
    }
    
    /* Log the error */
    logMessage(level, "%s", fullMessage);
    
    /* Call error callback if registered */
    if (errorCallback != NULL) {
        errorCallback(errorCode, message);
    }
}

/* Get the current error code */
int getLastError(void) {
    return lastErrorCode;
}

/* Clear the current error */
void clearError(void) {
    lastErrorCode = ERR_SUCCESS;
    lastErrorMessage[0] = '\0';
}

/* Register a custom error callback */
void registerErrorCallback(ErrorCallback callback) {
    errorCallback = callback;
}

/* Error functions for specific modules */

void logAPIError(const char* message, const char* url, int statusCode) {
    logError(ERR_API_REQUEST_FAILED, "%s - URL: %s, Status: %d", 
             message, url ? url : "unknown", statusCode);
}

void logSystemError(const char* message) {
    logError(ERR_SYSTEM, "%s", message);
}

void logDataError(const char* message, const char* dataIdentifier) {
    logError(ERR_DATA_VALIDATION, "%s - Data: %s", 
             message, dataIdentifier ? dataIdentifier : "unknown");
}

void logCalculationError(const char* message, const char* calculationType) {
    logError(ERR_CALCULATION, "%s - Calculation: %s", 
             message, calculationType ? calculationType : "unknown");
}

/* Assertion function */
void assertCondition(int condition, const char* message, 
                    const char* file, int line, const char* function) {
    if (!condition) {
        logError(ERR_INVALID_PARAMETER, "Assertion failed: %s in %s:%d (%s)", 
                message, file, line, function);
        
        /* In debug builds, we might want to abort */
#ifdef EMERS_DEBUG
        abort();
#endif
    }
} 