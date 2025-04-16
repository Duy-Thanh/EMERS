/**
 * Emergency Market Event Response System (EMERS)
 * Model Validation Module Tests
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "test_framework.h"
#include "../include/emers.h"
#include "../include/model_validation.h"
#include "../include/error_handling.h"

/* Sample data for testing */
static Stock testStocks[3];
static StockData testData[10];
static EventData testEvents[5];
static double predictedPrices[10];
static double actualPrices[10];
static double epsilon = 0.00001;

/* Setup function - prepare test data */
void setupModelValidationTests(void) {
    /* Initialize error handling */
    initErrorHandling("test_log.txt", LOG_DEBUG, LOG_DEBUG);
    
    /* Initialize model validation */
    initModelValidation();
    
    /* Initialize test stocks */
    int i;
    for (i = 0; i < 3; i++) {
        sprintf(testStocks[i].symbol, "STOCK%d", i + 1);
        testStocks[i].dataCount = 10;
        testStocks[i].data = testData;
    }
    
    /* Initialize test data */
    for (i = 0; i < 10; i++) {
        sprintf(testData[i].date, "2025-01-%02d", i + 1);
        testData[i].open = 100.0 + i;
        testData[i].high = 105.0 + i;
        testData[i].low = 95.0 + i;
        testData[i].close = 101.0 + i;
        testData[i].volume = 1000000.0 + i * 10000;
        testData[i].adjClose = 101.0 + i;
    }
    
    /* Initialize test events */
    for (i = 0; i < 5; i++) {
        sprintf(testEvents[i].date, "2025-01-%02d", i * 2 + 1);
        testEvents[i].type = i % 3;  /* Alternate between event types */
        testEvents[i].severity = (i % 4) + 1;  /* Severity between 1-4 */
        testEvents[i].impact = (double)(i + 1) * 0.5;
        sprintf(testEvents[i].description, "Test Event %d", i + 1);
    }
    
    /* Initialize price predictions */
    for (i = 0; i < 10; i++) {
        /* Predicted prices follow actual prices with some error */
        predictedPrices[i] = testData[i].close * (1.0 + ((double)(rand() % 10 - 5) / 100.0));
        actualPrices[i] = testData[i].close;
    }
}

/* Teardown function */
void teardownModelValidationTests(void) {
    /* Clean up model validation */
    cleanupModelValidation();
    
    /* Clean up error handling */
    cleanupErrorHandling();
}

/* Test backtesting functionality */
void testBacktesting(TestResult* result) {
    /* Perform backtesting */
    BacktestResults* results = performBacktest(
        testStocks, 3, "2025-01-01", "2025-01-10", "TestStrategy");
    
    /* Validate the results are not NULL */
    TEST_ASSERT(results != NULL);
    
    /* Validate that some metrics were calculated */
    TEST_ASSERT(results->eventDetectionMetrics.accuracy > 0);
    TEST_ASSERT(results->profitLoss != 0);
    TEST_ASSERT(results->sharpeRatio != 0);
    
    /* Check that detailed report was created */
    TEST_ASSERT(results->detailedReport != NULL);
    TEST_ASSERT(strlen(results->detailedReport) > 0);
    
    /* Clean up */
    freeBacktestResults(results);
    
    result->passed++;
}

/* Test cross-validation functionality */
void testCrossValidation(TestResult* result) {
    /* Perform cross-validation */
    CrossValidationResults* results = performCrossValidation(
        testStocks, 3, 5, "TestModel");
    
    /* Validate the results are not NULL */
    TEST_ASSERT(results != NULL);
    
    /* Validate that the right number of folds were created */
    TEST_ASSERT_EQUAL_INT(5, results->foldCount);
    
    /* Check that average metrics were calculated */
    TEST_ASSERT(results->averageMetrics.accuracy > 0);
    TEST_ASSERT(results->standardDeviation >= 0);
    
    /* Check that best and worst metrics were identified */
    TEST_ASSERT(results->bestMetrics.accuracy >= results->averageMetrics.accuracy);
    TEST_ASSERT(results->worstMetrics.accuracy <= results->averageMetrics.accuracy);
    
    /* Clean up */
    freeCrossValidationResults(results);
    
    result->passed++;
}

/* Test calculation of event detection metrics */
void testEventDetectionMetrics(TestResult* result) {
    /* Create predicted events (a subset of test events with some differences) */
    EventData predictedEvents[4];
    memcpy(predictedEvents, testEvents, 3 * sizeof(EventData));
    
    /* Add one event that doesn't match test events */
    strcpy(predictedEvents[3].date, "2025-01-09");
    predictedEvents[3].type = 1;
    predictedEvents[3].severity = 2;
    predictedEvents[3].impact = 1.5;
    strcpy(predictedEvents[3].description, "Predicted Event 4");
    
    /* Calculate metrics */
    ValidationMetrics metrics = calculateEventDetectionMetrics(
        predictedEvents, 4, testEvents, 5);
    
    /* Validate metrics */
    TEST_ASSERT(metrics.accuracy > 0);
    TEST_ASSERT(metrics.precision > 0);
    TEST_ASSERT(metrics.recall > 0);
    TEST_ASSERT(metrics.f1Score > 0);
    
    result->passed++;
}

/* Test calculation of price prediction metrics */
void testPricePredictionMetrics(TestResult* result) {
    /* Calculate metrics */
    ValidationMetrics metrics = calculatePricePredictionMetrics(
        predictedPrices, actualPrices, 10);
    
    /* Validate metrics */
    TEST_ASSERT(metrics.meanAbsoluteError >= 0);
    TEST_ASSERT(metrics.rootMeanSquareError >= 0);
    TEST_ASSERT(metrics.r2Score <= 1.0);
    TEST_ASSERT(metrics.accuracy > 0);
    
    result->passed++;
}

/* Test saving and loading validation results */
void testSaveLoadResults(TestResult* result) {
    /* Create test metrics */
    ValidationMetrics originalMetrics = {0};
    originalMetrics.accuracy = 0.85;
    originalMetrics.precision = 0.92;
    originalMetrics.recall = 0.78;
    originalMetrics.f1Score = 0.84;
    originalMetrics.meanAbsoluteError = 1.25;
    originalMetrics.rootMeanSquareError = 1.75;
    originalMetrics.r2Score = 0.67;
    
    /* Save metrics to file */
    int saveResult = saveValidationResults(&originalMetrics, "test_metrics.csv");
    TEST_ASSERT(saveResult == 1);
    
    /* Load metrics from file */
    ValidationMetrics* loadedMetrics = loadValidationResults("test_metrics.csv");
    TEST_ASSERT(loadedMetrics != NULL);
    
    /* Compare metrics */
    TEST_ASSERT_EQUAL_DOUBLE(originalMetrics.accuracy, loadedMetrics->accuracy, epsilon);
    TEST_ASSERT_EQUAL_DOUBLE(originalMetrics.precision, loadedMetrics->precision, epsilon);
    TEST_ASSERT_EQUAL_DOUBLE(originalMetrics.recall, loadedMetrics->recall, epsilon);
    TEST_ASSERT_EQUAL_DOUBLE(originalMetrics.f1Score, loadedMetrics->f1Score, epsilon);
    TEST_ASSERT_EQUAL_DOUBLE(originalMetrics.meanAbsoluteError, loadedMetrics->meanAbsoluteError, epsilon);
    TEST_ASSERT_EQUAL_DOUBLE(originalMetrics.rootMeanSquareError, loadedMetrics->rootMeanSquareError, epsilon);
    TEST_ASSERT_EQUAL_DOUBLE(originalMetrics.r2Score, loadedMetrics->r2Score, epsilon);
    
    /* Clean up */
    free(loadedMetrics);
    
    result->passed++;
}

/* Test report generation */
void testReportGeneration(TestResult* result) {
    /* Create test metrics */
    ValidationMetrics metrics = {0};
    metrics.accuracy = 0.85;
    metrics.precision = 0.92;
    metrics.recall = 0.78;
    metrics.f1Score = 0.84;
    metrics.meanAbsoluteError = 1.25;
    metrics.rootMeanSquareError = 1.75;
    metrics.r2Score = 0.67;
    
    /* Generate report */
    char* report = generateValidationReport(&metrics, "TestModel");
    TEST_ASSERT(report != NULL);
    
    /* Check report content */
    TEST_ASSERT(strstr(report, "TestModel") != NULL);
    TEST_ASSERT(strstr(report, "0.8500") != NULL);  /* Accuracy */
    TEST_ASSERT(strstr(report, "0.9200") != NULL);  /* Precision */
    
    /* Clean up */
    free(report);
    
    result->passed++;
}

/* Test regression testing functionality */
void testRegressionTesting(TestResult* result) {
    /* First create a baseline file */
    ValidationMetrics baselineMetrics = {0};
    baselineMetrics.accuracy = 0.85;
    baselineMetrics.precision = 0.92;
    baselineMetrics.recall = 0.78;
    baselineMetrics.f1Score = 0.84;
    
    saveValidationResults(&baselineMetrics, "baseline_results.csv");
    
    /* Perform regression test */
    int testResult = performRegressionTest("baseline_results.csv", "TestModel");
    
    /* Check result - should return success (1) if current metrics are within threshold */
    TEST_ASSERT(testResult == 1);
    
    result->passed++;
}

/* Main test function */
int main(void) {
    TestSuite modelValidationSuite;
    
    /* Initialize test suite */
    initTestSuite(&modelValidationSuite, "Model Validation Tests", 
                  setupModelValidationTests, teardownModelValidationTests);
    
    /* Add tests to the suite */
    addTest(&modelValidationSuite, testBacktesting, "Backtesting Functionality");
    addTest(&modelValidationSuite, testCrossValidation, "Cross-Validation Functionality");
    addTest(&modelValidationSuite, testEventDetectionMetrics, "Event Detection Metrics Calculation");
    addTest(&modelValidationSuite, testPricePredictionMetrics, "Price Prediction Metrics Calculation");
    addTest(&modelValidationSuite, testSaveLoadResults, "Save and Load Validation Results");
    addTest(&modelValidationSuite, testReportGeneration, "Validation Report Generation");
    addTest(&modelValidationSuite, testRegressionTesting, "Regression Testing Functionality");
    
    /* Run the tests */
    runTestSuite(&modelValidationSuite);
    
    /* Print results */
    printTestResults(&modelValidationSuite);
    
    /* Check for memory leaks */
    printMemoryLeaks();
    
    return modelValidationSuite.result.failed;
} 