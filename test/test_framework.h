/**
 * Emergency Market Event Response System (EMERS)
 * Test Framework Header
 */

#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../include/emers.h"

/* Test result structure */
typedef struct {
    int total;
    int passed;
    int failed;
    char** failedTestNames;
    double executionTime;
} TestResult;

/* Test suite structure */
typedef struct {
    char* name;
    void (*setup)(void);
    void (*teardown)(void);
    void (**tests)(TestResult*);
    char** testNames;
    int testCount;
    TestResult result;
} TestSuite;

/* Macros for test assertions */
#define TEST_ASSERT(condition) do { \
    if (!(condition)) { \
        printf("Assertion failed: %s, file %s, line %d\n", #condition, __FILE__, __LINE__); \
        result->failed++; \
        return; \
    } \
} while (0)

#define TEST_ASSERT_EQUAL_INT(expected, actual) do { \
    if ((expected) != (actual)) { \
        printf("Assertion failed: %d != %d, file %s, line %d\n", expected, actual, __FILE__, __LINE__); \
        result->failed++; \
        return; \
    } \
} while (0)

#define TEST_ASSERT_EQUAL_DOUBLE(expected, actual, epsilon) do { \
    if (fabs((expected) - (actual)) > (epsilon)) { \
        printf("Assertion failed: %f != %f, file %s, line %d\n", expected, actual, __FILE__, __LINE__); \
        result->failed++; \
        return; \
    } \
} while (0)

#define TEST_ASSERT_EQUAL_STRING(expected, actual) do { \
    if (strcmp((expected), (actual)) != 0) { \
        printf("Assertion failed: \"%s\" != \"%s\", file %s, line %d\n", expected, actual, __FILE__, __LINE__); \
        result->failed++; \
        return; \
    } \
} while (0)

/* Function to initialize a test suite */
void initTestSuite(TestSuite* suite, const char* name, void (*setup)(void), void (*teardown)(void));

/* Function to add a test to a test suite */
void addTest(TestSuite* suite, void (*test)(TestResult*), const char* testName);

/* Function to run a test suite */
void runTestSuite(TestSuite* suite);

/* Function to print test results */
void printTestResults(const TestSuite* suite);

/* Benchmark functions */
double benchmarkFunction(void (*func)(void), int iterations);

/* Memory leak detection */
void* trackMalloc(size_t size, const char* file, int line);
void trackFree(void* ptr, const char* file, int line);
int getMemoryLeaks(void);
void printMemoryLeaks(void);

#define TRACK_MALLOC(size) trackMalloc(size, __FILE__, __LINE__)
#define TRACK_FREE(ptr) trackFree(ptr, __FILE__, __LINE__)

#endif /* TEST_FRAMEWORK_H */ 