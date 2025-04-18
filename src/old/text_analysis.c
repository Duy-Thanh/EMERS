/**
 * @file text_analysis.c
 * @brief Implementation of text and news analysis functions
 */

#include "text_analysis.h"
#include "tiingo_api.h"
#include "emers.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <ctype.h>

// Simple dictionary of positive and negative words for sentiment analysis
static const char *POSITIVE_WORDS[] = {
    "gain", "growth", "profit", "positive", "increase", "up", "rising", "rose", "strong",
    "success", "successful", "bullish", "recovery", "improve", "improved", "rally", "surge",
    "outperform", "beat", "exceed", "exceeded", "opportunity", "optimistic", "advantage"
};

static const char *NEGATIVE_WORDS[] = {
    "loss", "decline", "drop", "fall", "fell", "down", "decrease", "negative", "weak",
    "poor", "bearish", "crash", "crisis", "risk", "threat", "concern", "concerned", "worried",
    "trouble", "underperform", "miss", "missed", "below", "fail", "failed", "warning", "danger"
};

static const int POSITIVE_WORDS_COUNT = sizeof(POSITIVE_WORDS) / sizeof(POSITIVE_WORDS[0]);
static const int NEGATIVE_WORDS_COUNT = sizeof(NEGATIVE_WORDS) / sizeof(NEGATIVE_WORDS[0]);

// Entity type keywords for rudimentary named entity recognition
static const char *PERSON_INDICATORS[] = {"Mr.", "Mrs.", "Ms.", "Dr.", "CEO", "Chairman", "President", "Director"};
static const char *ORG_INDICATORS[] = {"Inc", "Corp", "LLC", "Ltd", "Company", "Group", "Associates", "Bank"};
static const char *LOCATION_INDICATORS[] = {"in", "at", "from"};

static const int PERSON_INDICATORS_COUNT = sizeof(PERSON_INDICATORS) / sizeof(PERSON_INDICATORS[0]);
static const int ORG_INDICATORS_COUNT = sizeof(ORG_INDICATORS) / sizeof(ORG_INDICATORS[0]);
static const int LOCATION_INDICATORS_COUNT = sizeof(LOCATION_INDICATORS) / sizeof(LOCATION_INDICATORS[0]);

// Event type keywords
static const char *EVENT_KEYWORDS[][10] = {
    {"merger", "acquisition", "takeover", "buyout", "purchased", "acquired", "merged", "deal", "consolidation", "transaction"}, // MERGER_ACQUISITION
    {"earnings", "profit", "revenue", "eps", "income", "quarter", "quarterly", "financial", "results", "reported"}, // EARNINGS_REPORT
    {"scandal", "fraud", "lawsuit", "investigation", "probe", "legal", "court", "regulator", "sec", "violation"}, // CORPORATE_SCANDAL
    {"ceo", "executive", "chairman", "president", "chief", "officer", "leadership", "appointed", "resigned", "management"}, // LEADERSHIP_CHANGE
    {"dividend", "split", "buyback", "repurchase", "payout", "distribution", "yield", "share", "shareholder", "investor"}, // STOCK_SPLIT
    {"ipo", "offering", "public", "debut", "listing", "shares", "stock", "priced", "markets", "exchange"}, // IPO
    {"layoff", "fired", "redundancy", "cutback", "downsizing", "job", "workforce", "employee", "staff", "reduction"}, // LAYOFFS
    {"product", "launch", "new", "unveil", "announce", "release", "innovation", "technology", "feature", "breakthrough"}, // PRODUCT_LAUNCH
    {"partnership", "collaborate", "alliance", "agreement", "deal", "joint", "venture", "cooperation", "strategic", "partner"}, // PARTNERSHIP
    {"regulatory", "regulation", "law", "legislation", "compliance", "approval", "fda", "government", "agency", "policy"} // REGULATORY_CHANGE
};

static const int EVENT_KEYWORD_COUNT = 10; // Number of keywords per event type

// Global flags and data
static bool textAnalysisInitialized = false;

bool initTextAnalysis(void) {
    if (textAnalysisInitialized) {
        return true;  // Already initialized
    }
    
    // Random seed for deterministic but varied results in demo/mock implementation
    srand((unsigned int)time(NULL));
    
    textAnalysisInitialized = true;
    printf("Text analysis subsystem initialized.\n");
    return true;
}

void cleanupTextAnalysis(void) {
    if (!textAnalysisInitialized) {
        return;  // Not initialized
    }
    
    textAnalysisInitialized = false;
    printf("Text analysis subsystem cleaned up.\n");
}

bool analyzeSentiment(const char *text, SentimentResult *result) {
    if (!text || !result) {
        return false;
    }
    
    // Initialize result
    result->score = 0.0;
    result->confidence = 0.0;
    result->keywordCount = 0;
    
    // Simple bag-of-words sentiment analysis
    double positiveScore = 0.0;
    double negativeScore = 0.0;
    
    // Convert text to lowercase for case-insensitive matching
    char *lowerText = strdup(text);
    if (!lowerText) {
        return false;
    }
    
    for (size_t i = 0; i < strlen(lowerText); i++) {
        lowerText[i] = tolower(lowerText[i]);
    }
    
    // Count positive and negative words
    for (int i = 0; i < POSITIVE_WORDS_COUNT; i++) {
        char *ptr = lowerText;
        while ((ptr = strstr(ptr, POSITIVE_WORDS[i])) != NULL) {
            positiveScore += 1.0;
            
            // Add to keywords if not already there
            if (result->keywordCount < 10) {
                bool keywordExists = false;
                for (int k = 0; k < result->keywordCount; k++) {
                    if (strcmp(result->keywords[k], POSITIVE_WORDS[i]) == 0) {
                        keywordExists = true;
                        break;
                    }
                }
                
                if (!keywordExists) {
                    strncpy(result->keywords[result->keywordCount], POSITIVE_WORDS[i], 31);
                    result->keywords[result->keywordCount][31] = '\0';
                    result->keywordCount++;
                }
            }
            
            ptr += strlen(POSITIVE_WORDS[i]);
        }
    }
    
    for (int i = 0; i < NEGATIVE_WORDS_COUNT; i++) {
        char *ptr = lowerText;
        while ((ptr = strstr(ptr, NEGATIVE_WORDS[i])) != NULL) {
            negativeScore += 1.0;
            
            // Add to keywords if not already there
            if (result->keywordCount < 10) {
                bool keywordExists = false;
                for (int k = 0; k < result->keywordCount; k++) {
                    if (strcmp(result->keywords[k], NEGATIVE_WORDS[i]) == 0) {
                        keywordExists = true;
                        break;
                    }
                }
                
                if (!keywordExists) {
                    strncpy(result->keywords[result->keywordCount], NEGATIVE_WORDS[i], 31);
                    result->keywords[result->keywordCount][31] = '\0';
                    result->keywordCount++;
                }
            }
            
            ptr += strlen(NEGATIVE_WORDS[i]);
        }
    }
    
    free(lowerText);
    
    // Calculate final sentiment score
    double totalScore = positiveScore + negativeScore;
    
    if (totalScore > 0) {
        result->score = (positiveScore - negativeScore) / totalScore;
        
        // Higher confidence with more sentiment words
        result->confidence = fmin(totalScore / 10.0, 1.0);
    } else {
        // Neutral sentiment
        result->score = 0.0;
        result->confidence = 0.3; // Low confidence for neutral
    }
    
    return true;
}

int extractNamedEntities(const char *text, NamedEntity *entities, int maxEntities) {
    if (!text || !entities || maxEntities <= 0) {
        return 0;
    }
    
    int entityCount = 0;
    char *textCopy = strdup(text);
    if (!textCopy) {
        return 0;
    }
    
    // Split text into tokens
    char *token = strtok(textCopy, " \t\n.,;:!?()[]{}\"'");
    char prevToken[64] = "";
    
    while (token && entityCount < maxEntities) {
        // Check for person indicators
        for (int i = 0; i < PERSON_INDICATORS_COUNT; i++) {
            if (strcmp(token, PERSON_INDICATORS[i]) == 0 && strlen(prevToken) > 0) {
                strncpy(entities[entityCount].text, prevToken, 63);
                entities[entityCount].text[63] = '\0';
                strncpy(entities[entityCount].entityType, "PERSON", 15);
                entities[entityCount].entityType[15] = '\0';
                entityCount++;
                break;
            }
        }
        
        // Check for organization indicators
        for (int i = 0; i < ORG_INDICATORS_COUNT; i++) {
            if (strstr(token, ORG_INDICATORS[i]) != NULL) {
                strncpy(entities[entityCount].text, token, 63);
                entities[entityCount].text[63] = '\0';
                strncpy(entities[entityCount].entityType, "ORG", 15);
                entities[entityCount].entityType[15] = '\0';
                entityCount++;
                break;
            }
        }
        
        // Check for location indicators (simple heuristic)
        if (entityCount < maxEntities) {
            for (int i = 0; i < LOCATION_INDICATORS_COUNT; i++) {
                if (strcmp(prevToken, LOCATION_INDICATORS[i]) == 0 && 
                    isupper(token[0]) && strlen(token) > 2) {
                    strncpy(entities[entityCount].text, token, 63);
                    entities[entityCount].text[63] = '\0';
                    strncpy(entities[entityCount].entityType, "LOCATION", 15);
                    entities[entityCount].entityType[15] = '\0';
                    entityCount++;
                    break;
                }
            }
        }
        
        // Save current token for next iteration
        strncpy(prevToken, token, 63);
        prevToken[63] = '\0';
        
        // Get next token
        token = strtok(NULL, " \t\n.,;:!?()[]{}\"'");
    }
    
    free(textCopy);
    return entityCount;
}

int fetchNewsArticles(const char **symbols, int symbolCount, NewsArticle *articles, int maxArticles) {
    if (!symbols || symbolCount <= 0 || !articles || maxArticles <= 0) {
        return 0;
    }
    
    // Create comma-separated symbols string for Tiingo API
    char symbolsStr[256] = "";
    for (int i = 0; i < symbolCount; i++) {
        if (i > 0) {
            strcat(symbolsStr, ",");
        }
        strcat(symbolsStr, symbols[i]);
    }
    
    // Create event database to hold Tiingo news data
    EventDatabase eventDb;
    memset(&eventDb, 0, sizeof(EventDatabase));
    
    // Call Tiingo API to fetch real news data
    if (!fetchNewsFeed(symbolsStr, &eventDb)) {
        logError(ERR_API_REQUEST_FAILED, "Failed to fetch news from Tiingo API for symbols: %s", symbolsStr);
        return 0;
    }
    
    // Convert Tiingo event data to our NewsArticle format
    int articleCount = 0;
    for (int i = 0; i < eventDb.eventCount && articleCount < maxArticles; i++) {
        EventData *event = &eventDb.events[i];
        NewsArticle *article = &articles[articleCount];
        
        // Clear article
        memset(article, 0, sizeof(NewsArticle));
        
        // Copy title
        strncpy(article->title, event->title, sizeof(article->title) - 1);
        
        // Extract source from title or use default
        const char *sourceStart = strstr(event->title, " - ");
        if (sourceStart) {
            strncpy(article->source, sourceStart + 3, sizeof(article->source) - 1);
            
            // Truncate title at source marker
            size_t titleLen = sourceStart - event->title;
            if (titleLen < sizeof(article->title) - 1) {
                article->title[titleLen] = '\0';
            }
        } else {
            strncpy(article->source, "Tiingo News", sizeof(article->source) - 1);
        }
        
        // Create URL (Tiingo doesn't provide direct URLs in API)
        sprintf(article->url, "https://www.tiingo.com/news/%s", symbols[0]);
        
        // Copy date
        strncpy(article->date, event->date, sizeof(article->date) - 1);
        
        // Copy content/description
        strncpy(article->content, event->description, sizeof(article->content) - 1);
        
        // Analyze sentiment
        analyzeSentiment(article->content, &article->sentiment);
        
        // Extract entities
        article->entityCount = extractNamedEntities(
            article->content, 
            article->entities, 
            20);
        
        // Determine potential event type based on content keywords
        article->potentialEventType = UNKNOWN_EVENT;
        article->eventConfidence = 0.5;  // Default confidence
        
        // Try to determine event type from content
        double highestConfidence = 0.0;
        for (int typeIdx = 0; typeIdx < 10; typeIdx++) {
            int matchCount = 0;
            for (int keywordIdx = 0; keywordIdx < 10; keywordIdx++) {
                if (strstr(article->content, EVENT_KEYWORDS[typeIdx][keywordIdx]) != NULL) {
                    matchCount++;
                }
            }
            
            if (matchCount > 0) {
                double confidence = matchCount / 10.0 + 0.5;  // 0.5 to 1.5 based on matches
                if (confidence > 1.0) confidence = 1.0;
                
                if (confidence > highestConfidence) {
                    highestConfidence = confidence;
                    article->potentialEventType = (EventType)typeIdx;
                    article->eventConfidence = confidence;
                }
            }
        }
        
        articleCount++;
    }
    
    // Free allocated event database memory
    if (eventDb.events) {
        free(eventDb.events);
    }
    
    return articleCount;
}

int detectEventsFromNews(const NewsArticle *articles, int articleCount, MarketEvent *events, int maxEvents) {
    if (!articles || articleCount <= 0 || !events || maxEvents <= 0) {
        return 0;
    }
    
    int eventCount = 0;
    
    for (int i = 0; i < articleCount && eventCount < maxEvents; i++) {
        const NewsArticle *article = &articles[i];
        
        // Ignore articles with low event confidence
        if (article->eventConfidence < 0.6) {
            continue;
        }
        
        // Create a market event based on the article
        MarketEvent event;
        memset(&event, 0, sizeof(MarketEvent));
        
        // Set event type from article
        event.type = article->potentialEventType;
        
        // Get first stock symbol from article title
        char stockSymbol[10] = "";
        if (sscanf(article->title, "%*s %9s", stockSymbol) == 1) {
            strncpy(event.affectedStocks[0], stockSymbol, sizeof(event.affectedStocks[0]) - 1);
            event.affectedStockCount = 1;
        }
        
        // Calculate impact score based on sentiment and confidence
        event.impactScore = (int)(article->sentiment.score * 10.0 * article->eventConfidence);
        
        // Set timestamp from article date
        struct tm timeinfo = {0};
        
        // Windows doesn't have strptime, manually parse date in format "YYYY-MM-DD HH:MM:SS"
        if (sscanf(article->date, "%d-%d-%d %d:%d:%d", 
                  &timeinfo.tm_year, &timeinfo.tm_mon, &timeinfo.tm_mday,
                  &timeinfo.tm_hour, &timeinfo.tm_min, &timeinfo.tm_sec) == 6) {
            timeinfo.tm_year -= 1900;  // Years since 1900
            timeinfo.tm_mon -= 1;      // Months are 0-11
            event.timestamp = mktime(&timeinfo);
        } else {
            event.timestamp = time(NULL);  // Fallback to current time
        }
        
        // Copy description from article title
        strncpy(event.description, article->title, sizeof(event.description) - 1);
        
        // Set source from article source
        strncpy(event.source, article->source, sizeof(event.source) - 1);
        
        // Add event to array
        events[eventCount] = event;
        eventCount++;
    }
    
    return eventCount;
}

int calculateKeywordImportance(const NewsArticle *articles, int articleCount, char keywords[][32], int maxKeywords) {
    if (!articles || articleCount <= 0 || !keywords || maxKeywords <= 0) {
        return 0;
    }
    
    // This is a simplified version of TF-IDF for keywords
    // In a real implementation, we would use proper NLP libraries
    
    // Count word frequencies across all documents
    #define MAX_WORDS 1000
    char wordList[MAX_WORDS][32];
    int wordFreq[MAX_WORDS] = {0};
    int wordInDocCount[MAX_WORDS] = {0};  // For IDF calculation
    int uniqueWordCount = 0;
    
    // Process each article
    for (int i = 0; i < articleCount; i++) {
        const char *content = articles[i].content;
        
        // Make a copy for tokenization
        char *contentCopy = strdup(content);
        if (!contentCopy) {
            continue;
        }
        
        // Track words seen in this document (for IDF)
        bool wordSeenInDoc[MAX_WORDS] = {false};
        
        // Split text into tokens
        char *token = strtok(contentCopy, " \t\n.,;:!?()[]{}\"'");
        
        while (token) {
            // Skip short words and convert to lowercase
            if (strlen(token) >= 4) {
                char lowerToken[32];
                strncpy(lowerToken, token, 31);
                lowerToken[31] = '\0';
                
                for (size_t j = 0; j < strlen(lowerToken); j++) {
                    lowerToken[j] = tolower(lowerToken[j]);
                }
                
                // Check if word already in our list
                int wordIdx = -1;
                for (int j = 0; j < uniqueWordCount; j++) {
                    if (strcmp(wordList[j], lowerToken) == 0) {
                        wordIdx = j;
                        break;
                    }
                }
                
                // Add new word or update frequency
                if (wordIdx == -1 && uniqueWordCount < MAX_WORDS) {
                    strcpy(wordList[uniqueWordCount], lowerToken);
                    wordFreq[uniqueWordCount] = 1;
                    wordSeenInDoc[uniqueWordCount] = true;
                    uniqueWordCount++;
                } else if (wordIdx >= 0) {
                    wordFreq[wordIdx]++;
                    wordSeenInDoc[wordIdx] = true;
                }
            }
            
            token = strtok(NULL, " \t\n.,;:!?()[]{}\"'");
        }
        
        free(contentCopy);
        
        // Update document frequencies
        for (int j = 0; j < uniqueWordCount; j++) {
            if (wordSeenInDoc[j]) {
                wordInDocCount[j]++;
            }
        }
    }
    
    // Calculate TF-IDF scores
    double tfidfScores[MAX_WORDS] = {0.0};
    
    for (int i = 0; i < uniqueWordCount; i++) {
        double tf = (double)wordFreq[i] / (double)MAX_WORDS;
        double idf = log((double)articleCount / (double)(wordInDocCount[i] + 1));
        tfidfScores[i] = tf * idf;
    }
    
    // Sort words by TF-IDF score (simple bubble sort)
    for (int i = 0; i < uniqueWordCount - 1; i++) {
        for (int j = 0; j < uniqueWordCount - i - 1; j++) {
            if (tfidfScores[j] < tfidfScores[j + 1]) {
                // Swap scores
                double tempScore = tfidfScores[j];
                tfidfScores[j] = tfidfScores[j + 1];
                tfidfScores[j + 1] = tempScore;
                
                // Swap words
                char tempWord[32];
                strcpy(tempWord, wordList[j]);
                strcpy(wordList[j], wordList[j + 1]);
                strcpy(wordList[j + 1], tempWord);
                
                // Swap frequencies
                int tempFreq = wordFreq[j];
                wordFreq[j] = wordFreq[j + 1];
                wordFreq[j + 1] = tempFreq;
                
                // Swap document counts
                int tempDocCount = wordInDocCount[j];
                wordInDocCount[j] = wordInDocCount[j + 1];
                wordInDocCount[j + 1] = tempDocCount;
            }
        }
    }
    
    // Return top keywords
    int keywordCount = uniqueWordCount < maxKeywords ? uniqueWordCount : maxKeywords;
    
    for (int i = 0; i < keywordCount; i++) {
        strncpy(keywords[i], wordList[i], 31);
        keywords[i][31] = '\0';
    }
    
    return keywordCount;
}

double classifyNewsRelevance(const NewsArticle *article, const char *symbol) {
    if (!article || !symbol) {
        return 0.0;
    }
    
    double relevance = 0.0;
    
    // Check if symbol appears in title or content
    char *titleLower = strdup(article->title);
    char *contentLower = strdup(article->content);
    char symbolLower[16];
    
    if (!titleLower || !contentLower) {
        if (titleLower) free(titleLower);
        if (contentLower) free(contentLower);
        return 0.0;
    }
    
    strncpy(symbolLower, symbol, 15);
    symbolLower[15] = '\0';
    
    // Convert all to lowercase
    for (size_t i = 0; i < strlen(titleLower); i++) {
        titleLower[i] = tolower(titleLower[i]);
    }
    
    for (size_t i = 0; i < strlen(contentLower); i++) {
        contentLower[i] = tolower(contentLower[i]);
    }
    
    for (size_t i = 0; i < strlen(symbolLower); i++) {
        symbolLower[i] = tolower(symbolLower[i]);
    }
    
    // Symbol in title is a strong indicator
    if (strstr(titleLower, symbolLower) != NULL) {
        relevance += 0.6;
    }
    
    // Symbol in content adds some relevance
    if (strstr(contentLower, symbolLower) != NULL) {
        relevance += 0.3;
    }
    
    // Event confidence adds to relevance
    relevance += article->eventConfidence * 0.2;
    
    // Cap at 1.0
    if (relevance > 1.0) {
        relevance = 1.0;
    }
    
    free(titleLower);
    free(contentLower);
    
    return relevance;
}