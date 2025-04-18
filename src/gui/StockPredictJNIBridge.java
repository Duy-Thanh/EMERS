package gui;

import java.util.List;
import java.util.ArrayList;

/**
 * JNI Bridge between Java GUI and C data mining functions
 * 
 * This class provides native method declarations for calling
 * C functions from the StockPredict library.
 */
public class StockPredictJNIBridge {
    
    // Load the native library when the class is loaded
    static {
        try {
            System.loadLibrary("stockpredict_jni");
        } catch (UnsatisfiedLinkError e) {
            System.err.println("Native library stockpredict_jni not found or could not be loaded.");
            System.err.println("Error: " + e.getMessage());
        }
    }
    
    /**
     * Initialize the JNI bridge
     * @return 0 on success, negative on error
     */
    public native int initializeBridge();
    
    /**
     * Clean up resources used by the JNI bridge
     */
    public native void cleanupBridge();
    
    /**
     * Detect price patterns in stock data
     * 
     * @param dates Array of date strings
     * @param opens Array of opening prices
     * @param highs Array of high prices
     * @param lows Array of low prices
     * @param closes Array of closing prices
     * @param volumes Array of volume data
     * @param dataSize Number of data points
     * @return Array of detected patterns in format [type, startIndex, endIndex, confidence, expectedMove]
     */
    public native double[][] detectPricePatterns(String[] dates, double[] opens, double[] highs, 
                                              double[] lows, double[] closes, double[] volumes, 
                                              int dataSize);
    
    /**
     * Detect SMA crossover signals
     * 
     * @param dates Array of date strings
     * @param opens Array of opening prices
     * @param highs Array of high prices
     * @param lows Array of low prices
     * @param closes Array of closing prices
     * @param volumes Array of volume data
     * @param dataSize Number of data points
     * @param shortPeriod Short period for SMA
     * @param longPeriod Long period for SMA
     * @return Array of detected signals in format [type, index, confidence, entryPrice, targetPrice, stopLossPrice, riskRewardRatio]
     */
    public native double[][] detectSMACrossoverSignals(String[] dates, double[] opens, double[] highs, 
                                                   double[] lows, double[] closes, double[] volumes, 
                                                   int dataSize, int shortPeriod, int longPeriod);
    
    /**
     * Detect EMA crossover signals
     * 
     * @param dates Array of date strings
     * @param opens Array of opening prices
     * @param highs Array of high prices
     * @param lows Array of low prices
     * @param closes Array of closing prices
     * @param volumes Array of volume data
     * @param dataSize Number of data points
     * @param shortPeriod Short period for EMA
     * @param longPeriod Long period for EMA
     * @return Array of detected signals in format [type, index, confidence, entryPrice, targetPrice, stopLossPrice, riskRewardRatio]
     */
    public native double[][] detectEMACrossoverSignals(String[] dates, double[] opens, double[] highs, 
                                                   double[] lows, double[] closes, double[] volumes, 
                                                   int dataSize, int shortPeriod, int longPeriod);
    
    /**
     * Detect MACD crossover signals
     * 
     * @param dates Array of date strings
     * @param opens Array of opening prices
     * @param highs Array of high prices
     * @param lows Array of low prices
     * @param closes Array of closing prices
     * @param volumes Array of volume data
     * @param dataSize Number of data points
     * @param fastPeriod Fast period for MACD
     * @param slowPeriod Slow period for MACD
     * @param signalPeriod Signal period for MACD
     * @return Array of detected signals in format [type, index, confidence, entryPrice, targetPrice, stopLossPrice, riskRewardRatio]
     */
    public native double[][] detectMACDCrossoverSignals(String[] dates, double[] opens, double[] highs, 
                                                    double[] lows, double[] closes, double[] volumes, 
                                                    int dataSize, int fastPeriod, int slowPeriod, int signalPeriod);
    
    /**
     * Detect anomalies in stock data
     * 
     * @param dates Array of date strings
     * @param opens Array of opening prices
     * @param highs Array of high prices
     * @param lows Array of low prices
     * @param closes Array of closing prices
     * @param volumes Array of volume data
     * @param dataSize Number of data points
     * @return Array of detected anomalies in format [index, score, priceDeviation, volumeDeviation]
     */
    public native double[][] detectAnomalies(String[] dates, double[] opens, double[] highs, 
                                         double[] lows, double[] closes, double[] volumes, 
                                         int dataSize);
    
    /**
     * Calculate technical indicators
     * 
     * @param dates Array of date strings
     * @param opens Array of opening prices
     * @param highs Array of high prices
     * @param lows Array of low prices
     * @param closes Array of closing prices
     * @param volumes Array of volume data
     * @param dataSize Number of data points
     * @param indicators Array of indicator types to calculate
     * @param periods Array of periods for each indicator
     * @return 2D array of indicator values, one row per indicator with values for each data point
     */
    public native double[][] calculateIndicators(String[] dates, double[] opens, double[] highs, 
                                             double[] lows, double[] closes, double[] volumes, 
                                             int dataSize, int[] indicators, int[] periods);
    
    /**
     * Helper method to convert stock data objects to arrays for JNI calls
     * 
     * @param stockDataList List of stock data objects
     * @return Array containing arrays for each data component [dates, opens, highs, lows, closes, volumes]
     */
    public Object[] convertStockDataToArrays(List<StockPredictGUI.StockData> stockDataList) {
        int size = stockDataList.size();
        
        String[] dates = new String[size];
        double[] opens = new double[size];
        double[] highs = new double[size];
        double[] lows = new double[size];
        double[] closes = new double[size];
        double[] volumes = new double[size];
        
        for (int i = 0; i < size; i++) {
            StockPredictGUI.StockData data = stockDataList.get(i);
            dates[i] = data.date;
            opens[i] = data.open;
            highs[i] = data.high;
            lows[i] = data.low;
            closes[i] = data.close;
            volumes[i] = data.volume;
        }
        
        return new Object[] { dates, opens, highs, lows, closes, volumes, size };
    }
    
    /**
     * Convert pattern data from JNI calls to Java objects
     * 
     * @param patternData 2D array of pattern data
     * @return List of pattern objects
     */
    public List<PatternResult> convertPatternData(double[][] patternData) {
        List<PatternResult> patterns = new ArrayList<>();
        
        if (patternData == null) {
            return patterns;
        }
        
        for (double[] data : patternData) {
            if (data.length >= 5) {
                PatternResult pattern = new PatternResult();
                pattern.type = getPatternTypeName((int)data[0]);
                pattern.startIndex = (int)data[1];
                pattern.endIndex = (int)data[2];
                pattern.confidence = data[3];
                pattern.expectedMove = data[4];
                patterns.add(pattern);
            }
        }
        
        return patterns;
    }
    
    /**
     * Convert signal data from JNI calls to Java objects
     * 
     * @param signalData 2D array of signal data
     * @return List of signal objects
     */
    public List<TradingSignal> convertSignalData(double[][] signalData) {
        List<TradingSignal> signals = new ArrayList<>();
        
        if (signalData == null) {
            return signals;
        }
        
        for (double[] data : signalData) {
            if (data.length >= 7) {
                TradingSignal signal = new TradingSignal();
                signal.type = getSignalTypeName((int)data[0]);
                signal.signalIndex = (int)data[1];
                signal.confidence = data[2];
                signal.entryPrice = data[3];
                signal.targetPrice = data[4];
                signal.stopLossPrice = data[5];
                signal.riskRewardRatio = data[6];
                signals.add(signal);
            }
        }
        
        return signals;
    }
    
    /**
     * Convert anomaly data from JNI calls to Java objects
     * 
     * @param anomalyData 2D array of anomaly data
     * @return List of anomaly objects
     */
    public List<AnomalyResult> convertAnomalyData(double[][] anomalyData) {
        List<AnomalyResult> anomalies = new ArrayList<>();
        
        if (anomalyData == null) {
            return anomalies;
        }
        
        for (double[] data : anomalyData) {
            if (data.length >= 4) {
                AnomalyResult anomaly = new AnomalyResult();
                anomaly.index = (int)data[0];
                anomaly.score = data[1];
                anomaly.priceDeviation = data[2];
                anomaly.volumeDeviation = data[3];
                anomalies.add(anomaly);
            }
        }
        
        return anomalies;
    }
    
    // Helper methods for conversion between numeric codes and string names
    
    private String getPatternTypeName(int patternType) {
        switch (patternType) {
            case 1: return "Support";
            case 2: return "Resistance";
            case 3: return "Trend Change";
            case 4: return "Double Top";
            case 5: return "Double Bottom";
            case 6: return "Head & Shoulder";
            case 7: return "Uptrend";
            case 8: return "Downtrend";
            case 9: return "Head & Shoulders";
            default: return "Unknown";
        }
    }
    
    private String getSignalTypeName(int signalType) {
        switch (signalType) {
            case 1: return "BUY";
            case 2: return "SELL";
            case 3: return "HOLD";
            case 4: return "STOP LOSS";
            default: return "UNKNOWN";
        }
    }
    
    // Data classes to match the ones in StockPredictGUI
    
    static class PatternResult {
        String type;
        int startIndex;
        int endIndex;
        double confidence;
        double expectedMove;
        String description;
    }
    
    static class TradingSignal {
        String type;
        int signalIndex;
        double confidence;
        double entryPrice;
        double targetPrice;
        double stopLossPrice;
        double riskRewardRatio;
        String description;
    }
    
    static class AnomalyResult {
        int index;
        double score;
        double priceDeviation;
        double volumeDeviation;
        String description;
    }
} 