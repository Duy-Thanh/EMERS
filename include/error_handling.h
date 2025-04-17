/**
 * Emergency Market Event Response System (EMERS)
 * Error Handling & Logging Module
 */

#ifndef ERROR_HANDLING_H
#define ERROR_HANDLING_H

#include <stdio.h>
#include <stdarg.h>
#include <time.h>

/* Error severity levels */
typedef enum {
    LOG_DEBUG = 0,  /* Detailed debugging information */
    LOG_INFO,       /* General information */
    LOG_WARNING,    /* Warning, but system can continue */
    LOG_ERROR,      /* Error condition, some operations may fail */
    LOG_CRITICAL    /* Critical error, system cannot continue */
} LogLevel;

/* Error categories */
typedef enum {
    ERR_NONE = 0,       /* No error */
    ERR_MEMORY,         /* Memory allocation/deallocation errors */
    ERR_IO,             /* Input/output errors (file, network) */
    ERR_API,            /* External API errors */
    ERR_DATA,           /* Data processing errors */
    ERR_CALCULATION,    /* Calculation errors */
    ERR_SYSTEM,         /* System/OS errors */
    ERR_PARAM,          /* Invalid parameter errors */
    ERR_INIT,           /* Initialization errors */
    ERR_UNKNOWN         /* Unknown errors */
} ErrorCategory;

/* Error code structure */
typedef struct {
    int code;               /* Numeric error code */
    ErrorCategory category; /* Error category */
    LogLevel severity;      /* Error severity level */
    const char* message;    /* Error message */
} ErrorCode;

/* Common error codes */
#define ERR_SUCCESS                 0
#define ERR_NULL_POINTER            1001
#define ERR_OUT_OF_MEMORY           1002
#define ERR_INVALID_PARAMETER       1003
#define ERR_API_INITIALIZATION      2001
#define ERR_API_REQUEST_FAILED      2002
#define ERR_API_RESPONSE_INVALID    2003
#define ERR_API_RESPONSE_EMPTY      2004
#define ERR_CURL_FAILED             2005
#define ERR_FILE_NOT_FOUND          3001
#define ERR_FILE_OPEN_FAILED        3002
#define ERR_FILE_READ_FAILED        3003
#define ERR_FILE_WRITE_FAILED       3004
#define ERR_FILE_ACCESS             3005
#define ERR_FILE_READ               3006
#define ERR_CALCULATION_OVERFLOW    4001
#define ERR_CALCULATION_UNDERFLOW   4002
#define ERR_CALCULATION_DIVIDE_ZERO 4003
#define ERR_DATA_VALIDATION         5001
#define ERR_DATA_INSUFFICIENT       5002
#define ERR_DATA_CORRUPTED          5003

/* Data mining specific error codes */
#define ERR_DATA_FETCH_FAILED       5101
#define ERR_INSUFFICIENT_DATA       5102
#define ERR_CACHE_WRITE_FAILED      5103
#define ERR_CACHE_READ_FAILED       5104
#define ERR_ANALYSIS_FAILED         5105

/* Initialize the error handling and logging system */
int initErrorHandling(const char* logFilePath, LogLevel minFileLevel, LogLevel minConsoleLevel);

/* Clean up the error handling and logging system */
void cleanupErrorHandling(void);

/* Set the minimum log level for console output */
void setConsoleLogLevel(LogLevel level);

/* Set the minimum log level for file output */
void setFileLogLevel(LogLevel level);

/* Log a message */
void logMessage(LogLevel level, const char* format, ...);

/* Log a warning message */
void logWarning(const char* format, ...);

/* Log an error with an error code */
void logError(int errorCode, const char* format, ...);

/* Get the current error code */
int getLastError(void);

/* Clear the current error */
void clearError(void);

/* Register a custom error callback */
typedef void (*ErrorCallback)(int errorCode, const char* message);
void registerErrorCallback(ErrorCallback callback);

/* Error functions for specific modules */
void logAPIError(const char* message, const char* url, int statusCode);
void logSystemError(const char* message);
void logDataError(const char* message, const char* dataIdentifier);
void logCalculationError(const char* message, const char* calculationType);

/* Assertion function */
void assertCondition(int condition, const char* message, 
                    const char* file, int line, const char* function);

/* Assertion macro */
#ifdef NDEBUG
#define EMERS_ASSERT(condition, message) ((void)0)
#else
#define EMERS_ASSERT(condition, message) \
    assertCondition((condition), (message), __FILE__, __LINE__, __FUNCTION__)
#endif

#endif /* ERROR_HANDLING_H */