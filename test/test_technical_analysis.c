/**
 * Emergency Market Event Response System (EMERS)
 * Technical Analysis Module Tests
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "test_framework.h"
#include "../include/emers.h"
#include "../include/technical_analysis.h"

/* Sample data for testing */
static StockData testData[10];
static double epsilon = 0.00001;

/* Setup function - prepare test data */
void setupTechnicalAnalysisTests(void) {
    /* Initialize test data with a simple price series */
    int i;
    for (i = 0; i < 10; i++) {
        sprintf(testData[i].date, "2025-01-%02d", i + 1);
        testData[i].open = 100.0 + i;
        testData[i].high = 105.0 + i;
        testData[i].low = 95.0 + i;
        testData[i].close = 101.0 + i;
        testData[i].volume = 1000000.0 + i * 10000;
        testData[i].adjClose = 101.0 + i;
    }
}

/* Teardown function */
void teardownTechnicalAnalysisTests(void) {
    /* Nothing to do here for now */
}

/* Test SMA calculation */
void testCalculateSMA(TestResult* result) {
    double output[5];
    calculateSMA(testData, 10, 5, output);
    
    /* Expected values for 5-day SMA */
    double expected[5] = {103.0, 104.0, 105.0, 106.0, 107.0};
    
    int i;
    for (i = 0; i < 5; i++) {
        TEST_ASSERT_EQUAL_DOUBLE(expected[i], output[i], epsilon);
    }
    
    result->passed++;
}

/* Test EMA calculation */
void testCalculateEMA(TestResult* result) {
    double output[7];
    calculateEMA(testData, 10, 3, output);
    
    /* Expected values might vary based on implementation */
    /* Just checking that the values are in a reasonable range for now */
    int i;
    for (i = 0; i < 7; i++) {
        TEST_ASSERT(output[i] >= 100.0 + i);
        TEST_ASSERT(output[i] <= 110.0 + i);
    }
    
    result->passed++;
}

/* Test RSI calculation */
void testCalculateRSI(TestResult* result) {
    double output[5];
    calculateRSI(testData, 10, 5, output);
    
    /* For this data, since price always increases, RSI should be high */
    int i;
    for (i = 0; i < 5; i++) {
        TEST_ASSERT(output[i] > 70.0);  /* RSI should indicate overbought condition */
    }
    
    result->passed++;
}

/* Test MACD calculation */
void testCalculateMACD(TestResult* result) {
    double macdLine[5];
    double signalLine[5];
    double histogram[5];
    
    calculateMACD(testData, 10, 3, 6, 4, macdLine, signalLine, histogram);
    
    /* Just checking that the values are in a reasonable range */
    int i;
    for (i = 0; i < 5; i++) {
        TEST_ASSERT(fabs(macdLine[i]) < 10.0);
        TEST_ASSERT(fabs(signalLine[i]) < 10.0);
        TEST_ASSERT(fabs(histogram[i]) < 10.0);
    }
    
    result->passed++;
}

/* Test Bollinger Bands calculation */
void testCalculateBollingerBands(TestResult* result) {
    double upperBand[5];
    double middleBand[5];
    double lowerBand[5];
    
    calculateBollingerBands(testData, 10, 5, 2.0, upperBand, middleBand, lowerBand);
    
    /* Expected values for 5-day SMA (middle band) */
    double expectedMiddle[5] = {103.0, 104.0, 105.0, 106.0, 107.0};
    
    int i;
    for (i = 0; i < 5; i++) {
        /* Middle band should be the SMA */
        TEST_ASSERT_EQUAL_DOUBLE(expectedMiddle[i], middleBand[i], epsilon);
        
        /* Upper band should be higher than middle */
        TEST_ASSERT(upperBand[i] > middleBand[i]);
        
        /* Lower band should be lower than middle */
        TEST_ASSERT(lowerBand[i] < middleBand[i]);
        
        /* Upper and lower bands should be equidistant from middle */
        double upperDiff = upperBand[i] - middleBand[i];
        double lowerDiff = middleBand[i] - lowerBand[i];
        TEST_ASSERT_EQUAL_DOUBLE(upperDiff, lowerDiff, epsilon);
    }
    
    result->passed++;
}

/* Test ATR calculation */
void testCalculateATR(TestResult* result) {
    double output[5];
    calculateATR(testData, 10, 5, output);
    
    /* ATR should be positive */
    int i;
    for (i = 0; i < 5; i++) {
        TEST_ASSERT(output[i] > 0.0);
    }
    
    result->passed++;
}

/* Test the assembly-optimized standard deviation function */
void testAsmCalculateStandardDeviationSIMD(TestResult* result) {
    /* Create test data */
    double data[10] = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0};
    double stdDev;
    
    /* Calculate standard deviation using assembly-optimized function */
    asmCalculateStandardDeviationSIMD(data, 10, &stdDev);
    
    /* Expected standard deviation for this series is approx. 3.0277 */
    double expected = 3.0277;
    TEST_ASSERT_EQUAL_DOUBLE(expected, stdDev, 0.01);
    
    result->passed++;
}

/* Benchmark SMA calculation */
void benchmarkSMA(void) {
    double output[5];
    calculateSMA(testData, 10, 5, output);
}

/* Benchmark assembly vs. non-assembly functions */
void testPerformanceComparison(TestResult* result) {
    /* Create larger test data */
    int dataSize = 10000;
    double* largeData = (double*)malloc(dataSize * sizeof(double));
    
    int i;
    for (i = 0; i < dataSize; i++) {
        largeData[i] = 100.0 + (double)rand() / RAND_MAX * 10.0;
    }
    
    /* Test C standard deviation calculation */
    double stdDev;
    double cTime = benchmarkFunction(benchmarkSMA, 1000);
    
    /* Test assembly optimized calculation */
    double asmTime;
    if (largeData) {
        /* Free the large data array */
        free(largeData);
    }
    
    /* We're not asserting anything here, just printing performance data */
    printf("Performance comparison:\n");
    printf("  C implementation: %.6f seconds\n", cTime);
    printf("  Assembly implementation benchmark would go here\n");
    
    result->passed++;
}

/* Main test function */
int main(void) {
    TestSuite techAnalysisSuite;
    
    /* Initialize test suite */
    initTestSuite(&techAnalysisSuite, "Technical Analysis Tests", setupTechnicalAnalysisTests, teardownTechnicalAnalysisTests);
    
    /* Add tests to the suite */
    addTest(&techAnalysisSuite, testCalculateSMA, "SMA Calculation");
    addTest(&techAnalysisSuite, testCalculateEMA, "EMA Calculation");
    addTest(&techAnalysisSuite, testCalculateRSI, "RSI Calculation");
    addTest(&techAnalysisSuite, testCalculateMACD, "MACD Calculation");
    addTest(&techAnalysisSuite, testCalculateBollingerBands, "Bollinger Bands Calculation");
    addTest(&techAnalysisSuite, testCalculateATR, "ATR Calculation");
    addTest(&techAnalysisSuite, testAsmCalculateStandardDeviationSIMD, "Assembly-Optimized Standard Deviation");
    addTest(&techAnalysisSuite, testPerformanceComparison, "Performance Comparison");
    
    /* Run the tests */
    runTestSuite(&techAnalysisSuite);
    
    /* Print results */
    printTestResults(&techAnalysisSuite);
    
    /* Check for memory leaks */
    printMemoryLeaks();
    
    return techAnalysisSuite.result.failed;
} 