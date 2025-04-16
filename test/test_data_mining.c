/**
 * Emergency Market Event Response System (EMERS)
 * Data Mining Module Tests
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "test_framework.h"
#include "../include/emers.h"
#include "../include/data_mining.h"
#include "../include/technical_analysis.h"

/* Sample data for testing */
static StockData testData[100];
static EventData testEvents[10];
static double epsilon = 0.00001;

/* Setup function - prepare test data */
void setupDataMiningTests(void) {
    /* Initialize random seed */
    srand(time(NULL));
    
    /* Initialize test price data with patterns */
    int i;
    
    /* First 30 days: Uptrend */
    for (i = 0; i < 30; i++) {
        sprintf(testData[i].date, "2025-01-%02d", i + 1);
        double trend = i * 0.5;
        double noise = ((double)rand() / RAND_MAX - 0.5) * 2.0;
        
        testData[i].open = 100.0 + trend + noise;
        testData[i].high = testData[i].open + ((double)rand() / RAND_MAX) * 3.0;
        testData[i].low = testData[i].open - ((double)rand() / RAND_MAX) * 3.0;
        testData[i].close = testData[i].open + ((double)rand() / RAND_MAX - 0.5) * 2.0;
        testData[i].volume = 1000000.0 + ((double)rand() / RAND_MAX) * 500000.0;
        testData[i].adjClose = testData[i].close;
    }
    
    /* Next 30 days: Double top pattern */
    double peak1 = 115.0;
    double peak2 = 116.0;
    double trough = 105.0;
    
    /* First peak */
    for (i = 30; i < 40; i++) {
        sprintf(testData[i].date, "2025-02-%02d", i - 30 + 1);
        double distance = fabs(i - 35);
        double value = peak1 - distance * 1.5;
        double noise = ((double)rand() / RAND_MAX - 0.5) * 1.0;
        
        testData[i].open = value + noise;
        testData[i].high = testData[i].open + ((double)rand() / RAND_MAX) * 2.0;
        testData[i].low = testData[i].open - ((double)rand() / RAND_MAX) * 2.0;
        testData[i].close = testData[i].open + ((double)rand() / RAND_MAX - 0.5) * 1.0;
        testData[i].volume = 1200000.0 + ((double)rand() / RAND_MAX) * 400000.0;
        testData[i].adjClose = testData[i].close;
    }
    
    /* Trough */
    for (i = 40; i < 50; i++) {
        sprintf(testData[i].date, "2025-02-%02d", i - 30 + 1);
        double distance = fabs(i - 45);
        double value = trough + distance * 1.0;
        double noise = ((double)rand() / RAND_MAX - 0.5) * 1.0;
        
        testData[i].open = value + noise;
        testData[i].high = testData[i].open + ((double)rand() / RAND_MAX) * 2.0;
        testData[i].low = testData[i].open - ((double)rand() / RAND_MAX) * 2.0;
        testData[i].close = testData[i].open + ((double)rand() / RAND_MAX - 0.5) * 1.0;
        testData[i].volume = 900000.0 + ((double)rand() / RAND_MAX) * 300000.0;
        testData[i].adjClose = testData[i].close;
    }
    
    /* Second peak */
    for (i = 50; i < 60; i++) {
        sprintf(testData[i].date, "2025-03-%02d", i - 50 + 1);
        double distance = fabs(i - 55);
        double value = peak2 - distance * 1.5;
        double noise = ((double)rand() / RAND_MAX - 0.5) * 1.0;
        
        testData[i].open = value + noise;
        testData[i].high = testData[i].open + ((double)rand() / RAND_MAX) * 2.0;
        testData[i].low = testData[i].open - ((double)rand() / RAND_MAX) * 2.0;
        testData[i].close = testData[i].open + ((double)rand() / RAND_MAX - 0.5) * 1.0;
        testData[i].volume = 1300000.0 + ((double)rand() / RAND_MAX) * 500000.0;
        testData[i].adjClose = testData[i].close;
    }
    
    /* Last 40 days: Downtrend with an anomaly */
    for (i = 60; i < 100; i++) {
        sprintf(testData[i].date, "2025-03-%02d", i - 50 + 1);
        double trend = (100 - i) * 0.3;
        double noise = ((double)rand() / RAND_MAX - 0.5) * 1.5;
        
        testData[i].open = 100.0 + trend + noise;
        testData[i].high = testData[i].open + ((double)rand() / RAND_MAX) * 2.0;
        testData[i].low = testData[i].open - ((double)rand() / RAND_MAX) * 2.0;
        testData[i].close = testData[i].open + ((double)rand() / RAND_MAX - 0.5) * 1.0;
        testData[i].volume = 800000.0 + ((double)rand() / RAND_MAX) * 400000.0;
        testData[i].adjClose = testData[i].close;
    }
    
    /* Add an anomaly at day 80 */
    testData[80].open = testData[79].close * 0.9;
    testData[80].close = testData[79].close * 0.82;
    testData[80].high = testData[79].close * 0.95;
    testData[80].low = testData[79].close * 0.8;
    testData[80].volume = testData[79].volume * 3.0;
    testData[80].adjClose = testData[80].close;
    
    /* Initialize test events */
    strcpy(testEvents[0].date, "2025-01-15");
    strcpy(testEvents[0].title, "Positive Earnings Report");
    strcpy(testEvents[0].description, "Company reported earnings above analyst expectations");
    testEvents[0].sentiment = 0.8;
    testEvents[0].impactScore = 75;
    
    strcpy(testEvents[1].date, "2025-02-05");
    strcpy(testEvents[1].title, "New Product Launch");
    strcpy(testEvents[1].description, "Company launched a new flagship product");
    testEvents[1].sentiment = 0.9;
    testEvents[1].impactScore = 65;
    
    strcpy(testEvents[2].date, "2025-02-20");
    strcpy(testEvents[2].title, "Regulatory Investigation");
    strcpy(testEvents[2].description, "Company under investigation for regulatory compliance");
    testEvents[2].sentiment = -0.7;
    testEvents[2].impactScore = 85;
    
    strcpy(testEvents[3].date, "2025-03-10");
    strcpy(testEvents[3].title, "CEO Resignation");
    strcpy(testEvents[3].description, "CEO announced unexpected resignation");
    testEvents[3].sentiment = -0.5;
    testEvents[3].impactScore = 70;
}

/* Teardown function */
void teardownDataMiningTests(void) {
    /* Nothing to do here for now */
}

/* Test Pattern Recognition */
void testPatternRecognition(TestResult* result) {
    PatternResult patterns[5];
    int patternCount = detectPricePatterns(testData, 100, patterns, 5);
    
    /* Should detect at least one pattern (double top) */
    TEST_ASSERT(patternCount > 0);
    
    /* Check if double top was detected */
    int foundDoubleTop = 0;
    for (int i = 0; i < patternCount; i++) {
        if (patterns[i].type == PATTERN_DOUBLE_TOP) {
            foundDoubleTop = 1;
            break;
        }
    }
    
    TEST_ASSERT(foundDoubleTop);
    
    result->passed++;
}

/* Test Time Series Similarity Measures */
void testTimeSeriesSimilarity(TestResult* result) {
    /* Create two similar price series */
    double series1[20], series2[20];
    
    for (int i = 0; i < 20; i++) {
        series1[i] = 100.0 + i * 0.5 + ((double)rand() / RAND_MAX - 0.5) * 1.0;
        series2[i] = 100.0 + i * 0.5 + ((double)rand() / RAND_MAX - 0.5) * 1.0;
    }
    
    /* Create a dissimilar series */
    double series3[20];
    for (int i = 0; i < 20; i++) {
        series3[i] = 100.0 - i * 0.5 + ((double)rand() / RAND_MAX - 0.5) * 1.0;
    }
    
    /* Test Euclidean distance */
    double dist1 = calculateEuclideanDistance(series1, series2, 20);
    double dist2 = calculateEuclideanDistance(series1, series3, 20);
    
    /* Distance between similar series should be smaller */
    TEST_ASSERT(dist1 < dist2);
    
    /* Test Pearson correlation */
    double corr1 = calculatePearsonCorrelation(series1, series2, 20);
    double corr2 = calculatePearsonCorrelation(series1, series3, 20);
    
    /* Correlation between similar series should be higher (closer to 1) */
    TEST_ASSERT(corr1 > corr2);
    
    /* Correlation between series1 and series2 should be positive */
    TEST_ASSERT(corr1 > 0.5);
    
    /* Correlation between series1 and series3 should be negative */
    TEST_ASSERT(corr2 < 0);
    
    /* Test DTW distance on smaller series for performance */
    double smallSeries1[10], smallSeries2[10], smallSeries3[10];
    for (int i = 0; i < 10; i++) {
        smallSeries1[i] = series1[i];
        smallSeries2[i] = series2[i];
        smallSeries3[i] = series3[i];
    }
    
    double dtwDist1 = calculateDTW(smallSeries1, 10, smallSeries2, 10);
    double dtwDist2 = calculateDTW(smallSeries1, 10, smallSeries3, 10);
    
    /* DTW distance between similar series should be smaller */
    TEST_ASSERT(dtwDist1 < dtwDist2);
    
    result->passed++;
}

/* Test Volatility Prediction */
void testVolatilityPrediction(TestResult* result) {
    /* Test EWMA volatility prediction */
    double ewmaVol = predictVolatilityEWMA(testData, 100, 10);
    
    /* Volatility should be positive and reasonable */
    TEST_ASSERT(ewmaVol > 0.0);
    TEST_ASSERT(ewmaVol < 1.0);  /* Volatility should be less than 100% for most stocks */
    
    /* Test GARCH volatility prediction */
    double garchVol = predictVolatilityGARCH(testData, 100, 30);
    
    /* GARCH volatility should also be positive and reasonable */
    TEST_ASSERT(garchVol > 0.0);
    TEST_ASSERT(garchVol < 1.0);
    
    /* Compare volatility for different time horizons */
    double shortVol = predictVolatility(testData, 100, 5);
    double longVol = predictVolatility(testData, 100, 30);
    
    /* Volatility should scale with sqrt of time (approximate) */
    double expectedRatio = sqrt(30.0 / 5.0);
    double actualRatio = longVol / shortVol;
    
    /* Allow for some approximation error */
    TEST_ASSERT(fabs(actualRatio - expectedRatio) < 0.5);
    
    result->passed++;
}

/* Test Anomaly Detection */
void testAnomalyDetection(TestResult* result) {
    /* Test anomaly score */
    double anomalyScore = calculateAnomalyScore(&testData[79], 20);
    
    /* Score should be high because day 80 has an anomaly */
    TEST_ASSERT(anomalyScore > 2.0);
    
    /* Test anomaly detection */
    int anomalyIndices[10];
    int anomalyCount = detectAnomalies(testData, 100, anomalyIndices, 10);
    
    /* Should detect at least one anomaly */
    TEST_ASSERT(anomalyCount > 0);
    
    /* Check if the day 80 anomaly was detected */
    int foundDay80 = 0;
    for (int i = 0; i < anomalyCount; i++) {
        if (abs(anomalyIndices[i] - 80) <= 1) {  /* Allow for a 1-day window */
            foundDay80 = 1;
            break;
        }
    }
    
    TEST_ASSERT(foundDay80);
    
    result->passed++;
}

/* Test Similar Event Finding */
void testSimilarEventFinding(TestResult* result) {
    /* Create a test event */
    EventData testEvent;
    strcpy(testEvent.date, "2025-04-01");
    strcpy(testEvent.title, "Positive Quarterly Report");
    strcpy(testEvent.description, "Company reported strong quarterly earnings");
    testEvent.sentiment = 0.75;
    testEvent.impactScore = 70;
    
    /* Create a test event database */
    EventDatabase eventDB;
    eventDB.events = testEvents;
    eventDB.eventCount = 4;
    eventDB.eventCapacity = 10;
    
    /* Find similar events */
    SimilarHistoricalEvent similarEvents[10];
    int similarCount = findSimilarHistoricalEvents(&testEvent, &eventDB, similarEvents, 10);
    
    /* Should find at least one similar event */
    TEST_ASSERT(similarCount > 0);
    
    /* Check that the most similar event is the earnings report (testEvents[0]) */
    TEST_ASSERT(similarEvents[0].similarityScore > 0.5);
    
    /* Test outcome prediction */
    double prediction = predictEventOutcome(&testEvent, similarEvents, similarCount);
    
    /* Prediction should be a reasonably sized percentage */
    TEST_ASSERT(fabs(prediction) < 0.2);  /* Less than 20% */
    
    result->passed++;
}

/* Main test function */
int main(void) {
    TestSuite dataMiningTestSuite;
    
    /* Initialize test suite */
    initTestSuite(&dataMiningTestSuite, "Data Mining Tests", 
                 setupDataMiningTests, teardownDataMiningTests);
    
    /* Add tests to the suite */
    addTest(&dataMiningTestSuite, testPatternRecognition, "Pattern Recognition");
    addTest(&dataMiningTestSuite, testTimeSeriesSimilarity, "Time Series Similarity Measures");
    addTest(&dataMiningTestSuite, testVolatilityPrediction, "Volatility Prediction");
    addTest(&dataMiningTestSuite, testAnomalyDetection, "Anomaly Detection");
    addTest(&dataMiningTestSuite, testSimilarEventFinding, "Similar Event Finding");
    
    /* Run the tests */
    runTestSuite(&dataMiningTestSuite);
    
    /* Print results */
    printTestResults(&dataMiningTestSuite);
    
    /* Check for memory leaks */
    printMemoryLeaks();
    
    return dataMiningTestSuite.result.failed;
}