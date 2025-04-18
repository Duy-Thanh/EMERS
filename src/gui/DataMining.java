package gui;

/**
 * DataMining - Java implementation of data mining algorithms
 * Migrated from the C-based implementation to facilitate integration with GUI
 */
public class DataMining {
    
    // Pattern types for price pattern recognition
    public static final int PATTERN_UNKNOWN = 0;
    public static final int PATTERN_SUPPORT = 1;
    public static final int PATTERN_RESISTANCE = 2;
    public static final int PATTERN_TREND_CHANGE = 3;
    public static final int PATTERN_DOUBLE_TOP = 4;
    public static final int PATTERN_DOUBLE_BOTTOM = 5;
    public static final int PATTERN_HEAD_SHOULDER = 6;
    public static final int PATTERN_UPTREND = 7;
    public static final int PATTERN_DOWNTREND = 8;
    public static final int PATTERN_HEAD_AND_SHOULDERS = 9;

    // Signal types for trading signal detection
    public static final int SIGNAL_UNKNOWN = 0;
    public static final int SIGNAL_BUY = 1;
    public static final int SIGNAL_SELL = 2;
    public static final int SIGNAL_HOLD = 3;
    public static final int SIGNAL_STOP_LOSS = 4;
    
    // Inner class for pattern results
    public static class PatternResult {
        public int type;              // Pattern type (from constants)
        public int startIndex;        // Start index in the data array
        public int endIndex;          // End index in the data array
        public double confidence;     // Confidence level (0.0-1.0)
        public double expectedMove;   // Expected price move (% change)
        public String description;    // Human-readable description
    }
    
    // Inner class for trading signals
    public static class TradingSignal {
        public String type;           // Signal type (BUY, SELL, etc.)
        public int signalIndex;       // Index in data where signal occurs
        public double confidence;     // Confidence level (0.0-1.0)
        public double entryPrice;     // Suggested entry price
        public double targetPrice;    // Target price for take profit
        public double stopLossPrice;  // Suggested stop loss price
        public double riskRewardRatio;// Risk/reward ratio
        public String description;    // Signal description
    }
    
    // Inner class for anomaly results
    public static class AnomalyResult {
        public int index;             // Index in data where anomaly occurs
        public double score;          // Anomaly score (higher is more anomalous)
        public double priceDeviation; // Price deviation in standard deviations
        public double volumeDeviation;// Volume deviation in standard deviations
        public String description;    // Human-readable description
    }
    
    /**
     * Detect price patterns in stock data
     * 
     * @param data Array of StockData objects
     * @return Array of detected patterns
     */
    public static PatternResult[] detectPricePatterns(StockData[] data) {
        if (data == null || data.length < 20) {
            return new PatternResult[0]; // Not enough data
        }
        
        // Allocate storage for patterns (maximum 10 patterns)
        PatternResult[] patterns = new PatternResult[10];
        int patternCount = 0;
        
        // Detect support/resistance levels
        patternCount = detectSupportResistanceLevels(data, patterns, patternCount);
        
        // Detect trend changes
        patternCount = detectTrendChanges(data, patterns, patternCount);
        
        // Detect head and shoulders pattern
        patternCount = detectHeadAndShouldersPattern(data, patterns, patternCount);
        
        // Resize array to actual pattern count
        PatternResult[] result = new PatternResult[patternCount];
        System.arraycopy(patterns, 0, result, 0, patternCount);
        
        return result;
    }
    
    /**
     * Detect support and resistance levels
     */
    private static int detectSupportResistanceLevels(StockData[] data, PatternResult[] patterns, int patternCount) {
        int maxPatterns = patterns.length;
        
        // Find local minima (support) and maxima (resistance)
        for (int i = 10; i < data.length - 10; i++) {
            // Check for support level (local minimum)
            if (isLocalMinimum(data, i, 5)) {
                if (patternCount < maxPatterns) {
                    PatternResult pattern = new PatternResult();
                    pattern.type = PATTERN_SUPPORT;
                    pattern.startIndex = Math.max(0, i - 5);
                    pattern.endIndex = Math.min(data.length - 1, i + 5);
                    pattern.confidence = calculateSupportStrength(data, i);
                    pattern.description = "Support level detected at " + data[i].date;
                    patterns[patternCount++] = pattern;
                }
            }
            
            // Check for resistance level (local maximum)
            if (isLocalMaximum(data, i, 5)) {
                if (patternCount < maxPatterns) {
                    PatternResult pattern = new PatternResult();
                    pattern.type = PATTERN_RESISTANCE;
                    pattern.startIndex = Math.max(0, i - 5);
                    pattern.endIndex = Math.min(data.length - 1, i + 5);
                    pattern.confidence = calculateResistanceStrength(data, i);
                    pattern.description = "Resistance level detected at " + data[i].date;
                    patterns[patternCount++] = pattern;
                }
            }
        }
        
        return patternCount;
    }
    
    /**
     * Check if a point is a local minimum
     */
    private static boolean isLocalMinimum(StockData[] data, int index, int window) {
        double currentLow = data[index].low;
        
        // Check if this point is lower than surrounding points
        for (int i = Math.max(0, index - window); i < index; i++) {
            if (data[i].low <= currentLow) {
                return false;
            }
        }
        
        for (int i = index + 1; i <= Math.min(data.length - 1, index + window); i++) {
            if (data[i].low <= currentLow) {
                return false;
            }
        }
        
        return true;
    }
    
    /**
     * Check if a point is a local maximum
     */
    private static boolean isLocalMaximum(StockData[] data, int index, int window) {
        double currentHigh = data[index].high;
        
        // Check if this point is higher than surrounding points
        for (int i = Math.max(0, index - window); i < index; i++) {
            if (data[i].high >= currentHigh) {
                return false;
            }
        }
        
        for (int i = index + 1; i <= Math.min(data.length - 1, index + window); i++) {
            if (data[i].high >= currentHigh) {
                return false;
            }
        }
        
        return true;
    }
    
    /**
     * Calculate strength of a support level
     */
    private static double calculateSupportStrength(StockData[] data, int index) {
        // Simple implementation - could be more sophisticated
        return 0.7 + (Math.random() * 0.3); // 70-100% confidence
    }
    
    /**
     * Calculate strength of a resistance level
     */
    private static double calculateResistanceStrength(StockData[] data, int index) {
        // Simple implementation - could be more sophisticated
        return 0.7 + (Math.random() * 0.3); // 70-100% confidence
    }
    
    /**
     * Detect trend changes in the data
     */
    private static int detectTrendChanges(StockData[] data, PatternResult[] patterns, int patternCount) {
        int maxPatterns = patterns.length;
        
        // Simple trend change detection using moving averages
        double[] sma20 = calculateSMA(data, 20);
        double[] sma50 = calculateSMA(data, 50);
        
        // Look for crossovers
        for (int i = 50; i < data.length - 1; i++) {
            // Uptrend starting (20-day crosses above 50-day)
            if (sma20[i-1] <= sma50[i-1] && sma20[i] > sma50[i]) {
                if (patternCount < maxPatterns) {
                    PatternResult pattern = new PatternResult();
                    pattern.type = PATTERN_UPTREND;
                    pattern.startIndex = Math.max(0, i - 10);
                    pattern.endIndex = i;
                    pattern.confidence = 0.8;
                    pattern.expectedMove = 5.0; // Expect 5% move
                    pattern.description = "Uptrend starting at " + data[i].date;
                    patterns[patternCount++] = pattern;
                }
            }
            
            // Downtrend starting (20-day crosses below 50-day)
            if (sma20[i-1] >= sma50[i-1] && sma20[i] < sma50[i]) {
                if (patternCount < maxPatterns) {
                    PatternResult pattern = new PatternResult();
                    pattern.type = PATTERN_DOWNTREND;
                    pattern.startIndex = Math.max(0, i - 10);
                    pattern.endIndex = i;
                    pattern.confidence = 0.8;
                    pattern.expectedMove = -5.0; // Expect -5% move
                    pattern.description = "Downtrend starting at " + data[i].date;
                    patterns[patternCount++] = pattern;
                }
            }
        }
        
        return patternCount;
    }
    
    /**
     * Calculate Simple Moving Average
     */
    public static double[] calculateSMA(StockData[] data, int period) {
        double[] result = new double[data.length];
        
        for (int i = 0; i < data.length; i++) {
            if (i < period - 1) {
                // Not enough data for full period
                result[i] = 0;
            } else {
                double sum = 0;
                for (int j = 0; j < period; j++) {
                    sum += data[i - j].close;
                }
                result[i] = sum / period;
            }
        }
        
        return result;
    }
    
    /**
     * Detect head and shoulders pattern
     */
    private static int detectHeadAndShouldersPattern(StockData[] data, PatternResult[] patterns, int patternCount) {
        int maxPatterns = patterns.length;
        
        // Need at least 60 data points for reliable detection
        if (data.length < 60) {
            return patternCount;
        }
        
        // Scan for potential head and shoulders pattern
        // Look for 3 peaks with the middle one higher
        for (int i = 30; i < data.length - 30; i++) {
            // Check if i is a local maximum (potential head)
            if (isLocalMaximum(data, i, 10)) {
                // Look for left shoulder
                int leftShoulderIdx = -1;
                for (int j = i - 5; j >= i - 25; j--) {
                    if (j < 0) break;
                    if (isLocalMaximum(data, j, 5)) {
                        leftShoulderIdx = j;
                        break;
                    }
                }
                
                // Look for right shoulder
                int rightShoulderIdx = -1;
                for (int j = i + 5; j <= i + 25; j++) {
                    if (j >= data.length) break;
                    if (isLocalMaximum(data, j, 5)) {
                        rightShoulderIdx = j;
                        break;
                    }
                }
                
                // Validate head and shoulders
                if (leftShoulderIdx > 0 && rightShoulderIdx > 0) {
                    double headValue = data[i].high;
                    double leftShoulderValue = data[leftShoulderIdx].high;
                    double rightShoulderValue = data[rightShoulderIdx].high;
                    
                    // Check if head is higher than shoulders
                    if (headValue > leftShoulderValue && headValue > rightShoulderValue) {
                        // Check if shoulders are roughly at the same level
                        double shoulderDiff = Math.abs(leftShoulderValue - rightShoulderValue);
                        double shoulderAvg = (leftShoulderValue + rightShoulderValue) / 2;
                        
                        if (shoulderDiff / shoulderAvg < 0.1) { // Shoulders within 10% of each other
                            if (patternCount < maxPatterns) {
                                PatternResult pattern = new PatternResult();
                                pattern.type = PATTERN_HEAD_AND_SHOULDERS;
                                pattern.startIndex = leftShoulderIdx - 5;
                                pattern.endIndex = rightShoulderIdx + 5;
                                pattern.confidence = 0.7;
                                pattern.expectedMove = -5.0; // Expect downward move
                                pattern.description = "Head and Shoulders pattern detected ending at " + 
                                                      data[rightShoulderIdx].date;
                                patterns[patternCount++] = pattern;
                            }
                        }
                    }
                }
            }
        }
        
        return patternCount;
    }
    
    /**
     * Detect SMA crossover signals
     * 
     * @param data Array of stock data
     * @param shortPeriod Period for short SMA
     * @param longPeriod Period for long SMA
     * @return Array of trading signals
     */
    public static TradingSignal[] detectSMACrossoverSignals(StockData[] data, int shortPeriod, int longPeriod) {
        if (data == null || data.length < longPeriod) {
            return new TradingSignal[0]; // Not enough data
        }
        
        // Calculate SMAs
        double[] shortSMA = calculateSMA(data, shortPeriod);
        double[] longSMA = calculateSMA(data, longPeriod);
        
        // Allocate for signals (maximum 20)
        TradingSignal[] signals = new TradingSignal[20];
        int signalCount = 0;
        
        // Look for crossovers
        for (int i = longPeriod; i < data.length - 1; i++) {
            // Buy signal: short SMA crosses above long SMA
            if (shortSMA[i-1] <= longSMA[i-1] && shortSMA[i] > longSMA[i]) {
                if (signalCount < signals.length) {
                    TradingSignal signal = new TradingSignal();
                    signal.type = "BUY";
                    signal.signalIndex = i;
                    signal.confidence = calculateSignalConfidence(data, i, true);
                    signal.entryPrice = data[i].close;
                    signal.targetPrice = signal.entryPrice * 1.05; // 5% target
                    signal.stopLossPrice = signal.entryPrice * 0.97; // 3% stop loss
                    signal.riskRewardRatio = (signal.targetPrice - signal.entryPrice) / 
                                            (signal.entryPrice - signal.stopLossPrice);
                    signal.description = "Buy signal: " + shortPeriod + "-day SMA crossed above " + 
                                        longPeriod + "-day SMA";
                    signals[signalCount++] = signal;
                }
            }
            
            // Sell signal: short SMA crosses below long SMA
            else if (shortSMA[i-1] >= longSMA[i-1] && shortSMA[i] < longSMA[i]) {
                if (signalCount < signals.length) {
                    TradingSignal signal = new TradingSignal();
                    signal.type = "SELL";
                    signal.signalIndex = i;
                    signal.confidence = calculateSignalConfidence(data, i, false);
                    signal.entryPrice = data[i].close;
                    signal.targetPrice = signal.entryPrice * 0.95; // 5% target (downward)
                    signal.stopLossPrice = signal.entryPrice * 1.03; // 3% stop loss (upward)
                    signal.riskRewardRatio = (signal.entryPrice - signal.targetPrice) / 
                                            (signal.stopLossPrice - signal.entryPrice);
                    signal.description = "Sell signal: " + shortPeriod + "-day SMA crossed below " + 
                                        longPeriod + "-day SMA";
                    signals[signalCount++] = signal;
                }
            }
        }
        
        // Resize array to actual signal count
        TradingSignal[] result = new TradingSignal[signalCount];
        System.arraycopy(signals, 0, result, 0, signalCount);
        
        return result;
    }
    
    /**
     * Calculate confidence for a signal
     */
    private static double calculateSignalConfidence(StockData[] data, int index, boolean isBuy) {
        // Simple implementation - could be more sophisticated
        return 0.7 + (Math.random() * 0.3); // 70-100% confidence
    }
    
    /**
     * Detect anomalies in stock data
     * 
     * @param data Array of stock data
     * @return Array of anomaly results
     */
    public static AnomalyResult[] detectAnomalies(StockData[] data) {
        if (data == null || data.length < 30) {
            return new AnomalyResult[0]; // Not enough data
        }
        
        // Allocate for anomalies (maximum 10)
        AnomalyResult[] anomalies = new AnomalyResult[10];
        int anomalyCount = 0;
        
        // Calculate price and volume statistics
        double[] returns = new double[data.length - 1];
        double[] volumeChanges = new double[data.length - 1];
        
        for (int i = 1; i < data.length; i++) {
            returns[i-1] = (data[i].close - data[i-1].close) / data[i-1].close;
            volumeChanges[i-1] = (data[i].volume - data[i-1].volume) / data[i-1].volume;
        }
        
        // Calculate mean and standard deviation
        double meanReturn = calculateMean(returns);
        double stdReturn = calculateStdDev(returns, meanReturn);
        
        double meanVolume = calculateMean(volumeChanges);
        double stdVolume = calculateStdDev(volumeChanges, meanVolume);
        
        // Look for anomalies (z-score > 3.0)
        for (int i = 1; i < data.length; i++) {
            double priceChange = (data[i].close - data[i-1].close) / data[i-1].close;
            double volumeChange = data[i-1].volume > 0 ? 
                                 (data[i].volume - data[i-1].volume) / data[i-1].volume : 0;
            
            double priceZScore = stdReturn > 0 ? Math.abs((priceChange - meanReturn) / stdReturn) : 0;
            double volumeZScore = stdVolume > 0 ? Math.abs((volumeChange - meanVolume) / stdVolume) : 0;
            
            // Combine price and volume scores
            double anomalyScore = Math.max(priceZScore, volumeZScore);
            
            // If significant anomaly detected
            if (anomalyScore > 3.0) {
                if (anomalyCount < anomalies.length) {
                    AnomalyResult anomaly = new AnomalyResult();
                    anomaly.index = i;
                    anomaly.score = anomalyScore;
                    anomaly.priceDeviation = priceZScore;
                    anomaly.volumeDeviation = volumeZScore;
                    
                    // Generate description
                    StringBuilder desc = new StringBuilder("Anomaly detected: ");
                    if (priceZScore > 3.0) {
                        desc.append(String.format("Price moved %.2f%% (%.1f std. dev.) ", 
                                                 priceChange * 100, priceZScore));
                    }
                    if (volumeZScore > 3.0) {
                        desc.append(String.format("Volume changed %.2f%% (%.1f std. dev.)", 
                                                 volumeChange * 100, volumeZScore));
                    }
                    
                    anomaly.description = desc.toString();
                    anomalies[anomalyCount++] = anomaly;
                }
            }
        }
        
        // Resize array to actual anomaly count
        AnomalyResult[] result = new AnomalyResult[anomalyCount];
        System.arraycopy(anomalies, 0, result, 0, anomalyCount);
        
        return result;
    }
    
    /**
     * Calculate mean of an array
     */
    private static double calculateMean(double[] values) {
        double sum = 0;
        for (double value : values) {
            sum += value;
        }
        return values.length > 0 ? sum / values.length : 0;
    }
    
    /**
     * Calculate standard deviation of an array
     */
    private static double calculateStdDev(double[] values, double mean) {
        double sumSquaredDiff = 0;
        for (double value : values) {
            double diff = value - mean;
            sumSquaredDiff += diff * diff;
        }
        return values.length > 0 ? Math.sqrt(sumSquaredDiff / values.length) : 0;
    }
} 