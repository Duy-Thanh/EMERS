/**
 * @file text_analysis.h
 * @brief Text and news analysis functions for EMERS
 */

#ifndef TEXT_ANALYSIS_H
#define TEXT_ANALYSIS_H

#include "emers.h"
#include <stdbool.h>

/**
 * @brief Sentiment analysis result structure
 */
typedef struct {
    double score;         // -1.0 (very negative) to 1.0 (very positive)
    double confidence;    // 0.0 to 1.0
    char keywords[10][32]; // Top sentiment-driving keywords
    int keywordCount;
} SentimentResult;

/**
 * @brief Named entity structure
 */
typedef struct {
    char text[64];
    char entityType[16];  // PERSON, ORG, LOCATION, etc.
} NamedEntity;

/**
 * @brief News article structure
 */
typedef struct {
    char title[256];
    char source[64];
    char url[256];
    char content[4096];
    char date[32];
    SentimentResult sentiment;
    NamedEntity entities[20];
    int entityCount;
    EventType potentialEventType;
    double eventConfidence;
} NewsArticle;

/**
 * @brief Initialize the text analysis subsystem
 * 
 * @return true if initialized successfully, false otherwise
 */
bool initTextAnalysis(void);

/**
 * @brief Clean up the text analysis subsystem
 */
void cleanupTextAnalysis(void);

/**
 * @brief Analyze sentiment of text
 * 
 * @param text The text to analyze
 * @param result Pointer to store the sentiment result
 * @return true if analysis was successful, false otherwise
 */
bool analyzeSentiment(const char *text, SentimentResult *result);

/**
 * @brief Extract named entities from text
 * 
 * @param text The text to analyze
 * @param entities Array to store extracted entities
 * @param maxEntities Maximum number of entities to extract
 * @return Number of entities extracted
 */
int extractNamedEntities(const char *text, NamedEntity *entities, int maxEntities);

/**
 * @brief Fetch news articles related to specified stocks
 * 
 * @param symbols Array of stock symbols
 * @param symbolCount Number of symbols
 * @param articles Array to store fetched articles
 * @param maxArticles Maximum number of articles to fetch
 * @return Number of articles fetched
 */
int fetchNewsArticles(const char **symbols, int symbolCount, NewsArticle *articles, int maxArticles);

/**
 * @brief Detect potential market events from news articles
 * 
 * @param articles Array of news articles
 * @param articleCount Number of articles
 * @param events Array to store detected events
 * @param maxEvents Maximum number of events to detect
 * @return Number of events detected
 */
int detectEventsFromNews(const NewsArticle *articles, int articleCount, MarketEvent *events, int maxEvents);

/**
 * @brief Calculate keyword importance using TF-IDF
 * 
 * @param articles Array of news articles
 * @param articleCount Number of articles
 * @param keywords Array to store important keywords
 * @param maxKeywords Maximum number of keywords to identify
 * @return Number of keywords identified
 */
int calculateKeywordImportance(const NewsArticle *articles, int articleCount, char keywords[][32], int maxKeywords);

/**
 * @brief Classify news article relevance to stock performance
 * 
 * @param article The news article to classify
 * @param symbol Stock symbol
 * @return Relevance score from 0.0 (irrelevant) to 1.0 (highly relevant)
 */
double classifyNewsRelevance(const NewsArticle *article, const char *symbol);

#endif /* TEXT_ANALYSIS_H */ 