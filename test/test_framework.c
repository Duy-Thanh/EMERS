/**
 * Emergency Market Event Response System (EMERS)
 * Test Framework Implementation
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "test_framework.h"

/* Memory tracking structure */
typedef struct MemoryBlock {
    void* ptr;
    size_t size;
    const char* file;
    int line;
    struct MemoryBlock* next;
} MemoryBlock;

static MemoryBlock* memoryBlocks = NULL;

/* Initialize a test suite */
void initTestSuite(TestSuite* suite, const char* name, void (*setup)(void), void (*teardown)(void)) {
    if (!suite) return;
    
    suite->name = strdup(name);
    suite->setup = setup;
    suite->teardown = teardown;
    suite->tests = NULL;
    suite->testNames = NULL;
    suite->testCount = 0;
    
    /* Initialize test results */
    suite->result.total = 0;
    suite->result.passed = 0;
    suite->result.failed = 0;
    suite->result.failedTestNames = NULL;
    suite->result.executionTime = 0.0;
}

/* Add a test to a test suite */
void addTest(TestSuite* suite, void (*test)(TestResult*), const char* testName) {
    if (!suite || !test || !testName) return;
    
    /* Reallocate the tests array */
    suite->tests = realloc(suite->tests, (suite->testCount + 1) * sizeof(void (*)(TestResult*)));
    suite->testNames = realloc(suite->testNames, (suite->testCount + 1) * sizeof(char*));
    
    if (!suite->tests || !suite->testNames) {
        fprintf(stderr, "Error: Memory allocation failed for adding test.\n");
        exit(EXIT_FAILURE);
    }
    
    /* Add the test */
    suite->tests[suite->testCount] = test;
    suite->testNames[suite->testCount] = strdup(testName);
    suite->testCount++;
}

/* Run a test suite */
void runTestSuite(TestSuite* suite) {
    if (!suite) return;
    
    printf("Running test suite: %s\n", suite->name);
    printf("----------------------------------------------------\n");
    
    /* Initialize or reset result */
    suite->result.total = suite->testCount;
    suite->result.passed = 0;
    suite->result.failed = 0;
    
    if (suite->result.failedTestNames) {
        int i;
        for (i = 0; i < suite->result.failed; i++) {
            free(suite->result.failedTestNames[i]);
        }
        free(suite->result.failedTestNames);
    }
    
    suite->result.failedTestNames = malloc(suite->testCount * sizeof(char*));
    if (!suite->result.failedTestNames) {
        fprintf(stderr, "Error: Memory allocation failed for test results.\n");
        exit(EXIT_FAILURE);
    }
    
    clock_t start, end;
    start = clock();
    
    int i;
    for (i = 0; i < suite->testCount; i++) {
        printf("Running test: %s... ", suite->testNames[i]);
        
        /* Setup test environment */
        if (suite->setup) {
            suite->setup();
        }
        
        /* Run the test */
        TestResult testResult = {1, 0, 0, NULL, 0.0};
        suite->tests[i](&testResult);
        
        /* Teardown test environment */
        if (suite->teardown) {
            suite->teardown();
        }
        
        /* Update suite result */
        if (testResult.failed == 0) {
            suite->result.passed++;
            printf("PASSED\n");
        } else {
            suite->result.failedTestNames[suite->result.failed] = strdup(suite->testNames[i]);
            suite->result.failed++;
            printf("FAILED\n");
        }
    }
    
    end = clock();
    suite->result.executionTime = ((double) (end - start)) / CLOCKS_PER_SEC;
    
    printf("----------------------------------------------------\n");
}

/* Print test results */
void printTestResults(const TestSuite* suite) {
    if (!suite) return;
    
    printf("\nTest Suite: %s - Results\n", suite->name);
    printf("----------------------------------------------------\n");
    printf("Total tests: %d\n", suite->result.total);
    printf("Passed: %d\n", suite->result.passed);
    printf("Failed: %d\n", suite->result.failed);
    printf("Execution time: %.3f seconds\n", suite->result.executionTime);
    
    if (suite->result.failed > 0) {
        printf("\nFailed tests:\n");
        int i;
        for (i = 0; i < suite->result.failed; i++) {
            printf("  - %s\n", suite->result.failedTestNames[i]);
        }
    }
    
    printf("----------------------------------------------------\n");
}

/* Benchmark a function */
double benchmarkFunction(void (*func)(void), int iterations) {
    if (!func || iterations <= 0) return 0.0;
    
    clock_t start, end;
    start = clock();
    
    int i;
    for (i = 0; i < iterations; i++) {
        func();
    }
    
    end = clock();
    return ((double) (end - start)) / CLOCKS_PER_SEC;
}

/* Memory tracking functions */
void* trackMalloc(size_t size, const char* file, int line) {
    void* ptr = malloc(size);
    if (!ptr) return NULL;
    
    MemoryBlock* block = (MemoryBlock*)malloc(sizeof(MemoryBlock));
    if (!block) {
        free(ptr);
        return NULL;
    }
    
    block->ptr = ptr;
    block->size = size;
    block->file = file;
    block->line = line;
    block->next = memoryBlocks;
    memoryBlocks = block;
    
    return ptr;
}

void trackFree(void* ptr, const char* file, int line) {
    if (!ptr) return;
    
    MemoryBlock** prev = &memoryBlocks;
    MemoryBlock* current = memoryBlocks;
    
    while (current != NULL) {
        if (current->ptr == ptr) {
            *prev = current->next;
            free(current);
            free(ptr);
            return;
        }
        
        prev = &(current->next);
        current = current->next;
    }
    
    /* If we get here, the pointer wasn't tracked */
    fprintf(stderr, "Error: Attempt to free untracked memory at %s:%d\n", file, line);
    free(ptr);
}

/* Get the number of memory leaks */
int getMemoryLeaks(void) {
    int count = 0;
    MemoryBlock* current = memoryBlocks;
    
    while (current != NULL) {
        count++;
        current = current->next;
    }
    
    return count;
}

/* Print memory leaks */
void printMemoryLeaks(void) {
    if (!memoryBlocks) {
        printf("No memory leaks detected.\n");
        return;
    }
    
    printf("Memory leaks detected:\n");
    printf("----------------------------------------------------\n");
    
    int count = 0;
    size_t totalBytes = 0;
    MemoryBlock* current = memoryBlocks;
    
    while (current != NULL) {
        printf("Leak #%d: %zu bytes at address %p, allocated in %s line %d\n",
               ++count, current->size, current->ptr, current->file, current->line);
        
        totalBytes += current->size;
        current = current->next;
    }
    
    printf("----------------------------------------------------\n");
    printf("Total: %d leaks, %zu bytes\n", count, totalBytes);
}

/* Clean up tracked memory (to be called at the end of the program) */
void cleanupMemoryTracking(void) {
    MemoryBlock* current = memoryBlocks;
    
    while (current != NULL) {
        MemoryBlock* next = current->next;
        free(current->ptr);
        free(current);
        current = next;
    }
    
    memoryBlocks = NULL;
} 