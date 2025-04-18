package gui;

/**
 * DataUtils - Shared data structures for stock analysis
 * 
 * This class contains common data structures used across different components
 * of the application to prevent duplicate class definitions and compatibility issues.
 */
public class DataUtils {
    
    /**
     * StockData - Represents a single data point of stock market data
     */
    public static class StockData {
        public String date;
        public double open;
        public double high;
        public double low;
        public double close;
        public double volume;
        public double adjClose;
        
        // Default constructor
        public StockData() {
        }
        
        // Copy constructor from GUI StockData
        public static StockData fromGUIStockData(Object guiStockData) {
            try {
                // Use reflection to handle various StockData classes
                StockData result = new StockData();
                java.lang.reflect.Field dateField = guiStockData.getClass().getDeclaredField("date");
                dateField.setAccessible(true);
                result.date = (String)dateField.get(guiStockData);
                
                java.lang.reflect.Field openField = guiStockData.getClass().getDeclaredField("open");
                openField.setAccessible(true);
                result.open = (Double)openField.get(guiStockData);
                
                java.lang.reflect.Field highField = guiStockData.getClass().getDeclaredField("high");
                highField.setAccessible(true);
                result.high = (Double)highField.get(guiStockData);
                
                java.lang.reflect.Field lowField = guiStockData.getClass().getDeclaredField("low");
                lowField.setAccessible(true);
                result.low = (Double)lowField.get(guiStockData);
                
                java.lang.reflect.Field closeField = guiStockData.getClass().getDeclaredField("close");
                closeField.setAccessible(true);
                result.close = (Double)closeField.get(guiStockData);
                
                java.lang.reflect.Field volumeField = guiStockData.getClass().getDeclaredField("volume");
                volumeField.setAccessible(true);
                result.volume = (Double)volumeField.get(guiStockData);
                
                // Handle adjClose which might not exist in all StockData classes
                try {
                    java.lang.reflect.Field adjCloseField = guiStockData.getClass().getDeclaredField("adjClose");
                    adjCloseField.setAccessible(true);
                    result.adjClose = (Double)adjCloseField.get(guiStockData);
                } catch (NoSuchFieldException e) {
                    // If adjClose doesn't exist, use regular close
                    result.adjClose = result.close;
                }
                
                return result;
            } catch (Exception e) {
                System.err.println("Error converting StockData: " + e.getMessage());
                e.printStackTrace();
                return null;
            }
        }
        
        /**
         * Create an array of StockData from an array of GUI StockData objects
         */
        public static StockData[] fromGUIStockDataArray(Object[] guiStockDataArray) {
            StockData[] result = new StockData[guiStockDataArray.length];
            for (int i = 0; i < guiStockDataArray.length; i++) {
                result[i] = fromGUIStockData(guiStockDataArray[i]);
            }
            return result;
        }
    }
    
    /**
     * PatternResult - Represents a detected price pattern
     */
    public static class PatternResult {
        public int type;              // Pattern type (from constants)
        public int startIndex;        // Start index in the data array
        public int endIndex;          // End index in the data array
        public double confidence;     // Confidence level (0.0-1.0)
        public double expectedMove;   // Expected price move (% change)
        public String description;    // Human-readable description
    }
    
    /**
     * TradingSignal - Represents a trading signal
     */
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
    
    /**
     * AnomalyResult - Represents an anomaly in stock data
     */
    public static class AnomalyResult {
        public int index;             // Index in data where anomaly occurs
        public double score;          // Anomaly score (higher is more anomalous)
        public double priceDeviation; // Price deviation in standard deviations
        public double volumeDeviation;// Volume deviation in standard deviations
        public String description;    // Human-readable description
    }
} 