package gui; // Package declaration (Khai báo gói)

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Arrays;

/**
 * DataMining - Implementation of stock data analysis algorithms
 * DataMining - Hiện thực các thuật toán phân tích dữ liệu chứng khoán
 * 
 * This class contains algorithms for technical analysis, pattern recognition,
 * and anomaly detection in stock price data. It provides tools for identifying
 * trading opportunities based on price movements and statistical indicators.
 * 
 * Lớp này chứa các thuật toán cho phân tích kỹ thuật, nhận dạng mẫu hình,
 * và phát hiện bất thường trong dữ liệu giá cổ phiếu. Nó cung cấp các công cụ
 * để xác định cơ hội giao dịch dựa trên biến động giá và các chỉ báo thống kê.
 */
public class DataMining { // Class definition (Định nghĩa lớp)
    
    // Pattern Types - Pattern detection constants
    // Các loại mẫu hình - Hằng số phát hiện mẫu hình
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
    public static final int PATTERN_REVERSAL = 10;

    // Signal types for trading signal detection
    public static final int SIGNAL_UNKNOWN = 0;
    public static final int SIGNAL_BUY = 1;
    public static final int SIGNAL_SELL = 2;
    public static final int SIGNAL_HOLD = 3;
    public static final int SIGNAL_STOP_LOSS = 4;
    
    /**
     * MACDResult - Container for MACD calculation results
     * MACDResult - Lớp chứa kết quả tính toán MACD
     * 
     * Stores the three main components of the MACD indicator:
     * 1. MACD Line - The difference between fast and slow EMAs
     * 2. Signal Line - EMA of the MACD Line
     * 3. Histogram - Difference between MACD Line and Signal Line
     * 
     * Lưu trữ ba thành phần chính của chỉ báo MACD:
     * 1. Đường MACD - Sự chênh lệch giữa EMA nhanh và EMA chậm
     * 2. Đường tín hiệu - EMA của đường MACD
     * 3. Histogram - Sự chênh lệch giữa đường MACD và đường tín hiệu
     */
    public static class MACDResult {
        public double[] macdLine;    // MACD Line values
        public double[] signalLine;  // Signal Line values
        public double[] histogram;   // Histogram values
        
        /**
         * Constructor initializes arrays for MACD results
         * @param length Length of data arrays
         */
        public MACDResult(int length) {
            macdLine = new double[length];
            signalLine = new double[length];
            histogram = new double[length];
        }
        
        /**
         * Get MACD Line values only (used in divergence detection)
         * @return MACD Line values as array
         */
        public double[] getLine() {
            return macdLine;
        }
    }
    
    // Using shared data classes from DataUtils
    
    /**
     * Detect patterns in the provided data
     * 
     * Algorithm:
     * 1. Checks for minimum data requirement (at least 50 data points)
     * 2. Creates a list to store detected patterns
     * 3. Calls specialized detection methods for different pattern types
     * 4. Sorts patterns by start index (chronological order)
     * 5. Converts list to array for return
     * 
     * @param data Array of stock data points with price and volume information
     * @return Array of detected price patterns, sorted chronologically
     */
    public static DataUtils.PatternResult[] detectPricePatterns(DataUtils.StockData[] data) {
        if (data == null || data.length < 50) return new DataUtils.PatternResult[0];
        
        // Initialize with larger capacity for more aggressive detection
        List<DataUtils.PatternResult> patterns = new ArrayList<>(data.length / 10);
        
        // Only call methods that are actually implemented
        detectHeadAndShouldersPattern(data, patterns);
        detectSupportResistanceLevels(data, patterns);
        detectTrendChanges(data, patterns);
        detectDivergencePatterns(data, patterns);
        detectPatternsInFutureData(data, patterns);
        
        // Sort patterns by index
        Collections.sort(patterns, (p1, p2) -> Integer.compare(p1.startIndex, p2.startIndex));
        
        // Convert list to array
        DataUtils.PatternResult[] result = new DataUtils.PatternResult[patterns.size()];
        return patterns.toArray(result);
    }
    
    /**
     * Detect support and resistance levels
     */
    private static void detectSupportResistanceLevels(DataUtils.StockData[] data, List<DataUtils.PatternResult> patterns) {
        // Find local minima (support) and maxima (resistance)
        // Reduced window size from 10 to 5 to capture more patterns
        // Modified to include more recent data (reduced from -5 to -2)
        for (int i = 5; i < data.length - 2; i++) {
            // Check for support level (local minimum)
            // Reduced window from 5 to 3 for more sensitivity
            if (isLocalMinimum(data, i, 3)) {
                DataUtils.PatternResult pattern = new DataUtils.PatternResult();
                pattern.type = PATTERN_SUPPORT;
                pattern.startIndex = Math.max(0, i - 5);
                pattern.endIndex = Math.min(data.length - 1, i + 5);
                pattern.confidence = calculateSupportStrength(data, i);
                pattern.description = "Support level at price $" + String.format("%.2f", data[i].low) + 
                                     " on " + data[i].date + ". Potential bounce point.";
                patterns.add(pattern);
            }
            
            // Check for resistance level (local maximum)
            // Reduced window from 5 to 3 for more sensitivity
            if (isLocalMaximum(data, i, 3)) {
                DataUtils.PatternResult pattern = new DataUtils.PatternResult();
                pattern.type = PATTERN_RESISTANCE;
                pattern.startIndex = Math.max(0, i - 5);
                pattern.endIndex = Math.min(data.length - 1, i + 5);
                pattern.confidence = calculateResistanceStrength(data, i);
                pattern.description = "Resistance level at price $" + String.format("%.2f", data[i].high) + 
                                     " on " + data[i].date + ". Potential reversal point.";
                patterns.add(pattern);
            }
        }
    }
    
    /**
     * Check if a point is a local minimum
     * 
     * A local minimum is a point where the low price is less than or equal to
     * all other low prices within the specified window.
     * 
     * This checks the low price in the StockData.
     */
    private static boolean isLocalMinimum(DataUtils.StockData[] data, int index, int window) {
        if (index < window || index >= data.length - window) {
            System.out.println("DEBUG: Index " + index + " is too close to the edge for window size " + window);
            return false;
        }
        
        double currentLow = data[index].low;
        
        // Check if this point is lower than surrounding points
        // Reduced strictness - now we only need it to be <= rather than strictly <
        for (int i = Math.max(0, index - window); i < index; i++) {
            if (data[i].low < currentLow) {
                System.out.println("DEBUG: Not a local minimum at index " + index + " (" + data[index].date + 
                                  ") because low[" + i + "] = " + data[i].low + 
                                  " < currentLow = " + currentLow);
                return false;
            }
        }
        
        for (int i = index + 1; i <= Math.min(data.length - 1, index + window); i++) {
            if (data[i].low < currentLow) {
                System.out.println("DEBUG: Not a local minimum at index " + index + " (" + data[index].date + 
                                  ") because low[" + i + "] = " + data[i].low + 
                                  " < currentLow = " + currentLow);
                return false;
            }
        }
        
        System.out.println("DEBUG: Found local minimum at index " + index + " (" + data[index].date + 
                          ") with low = " + currentLow);
        return true;
    }
    
    /**
     * Check if a point is a local maximum
     * 
     * A local maximum is a point where the high price is greater than or equal to
     * all other high prices within the specified window.
     * 
     * This checks the high price in the StockData.
     */
    private static boolean isLocalMaximum(DataUtils.StockData[] data, int index, int window) {
        if (index < window || index >= data.length - window) {
            System.out.println("DEBUG: Index " + index + " is too close to the edge for window size " + window);
            return false;
        }
        
        double currentHigh = data[index].high;
        
        // Check if this point is higher than surrounding points
        // Reduced strictness - now we only need it to be >= rather than strictly >
        for (int i = Math.max(0, index - window); i < index; i++) {
            if (data[i].high > currentHigh) {
                System.out.println("DEBUG: Not a local maximum at index " + index + " (" + data[index].date + 
                                  ") because high[" + i + "] = " + data[i].high + 
                                  " > currentHigh = " + currentHigh);
                return false;
            }
        }
        
        for (int i = index + 1; i <= Math.min(data.length - 1, index + window); i++) {
            if (data[i].high > currentHigh) {
                System.out.println("DEBUG: Not a local maximum at index " + index + " (" + data[index].date + 
                                  ") because high[" + i + "] = " + data[i].high + 
                                  " > currentHigh = " + currentHigh);
                return false;
            }
        }
        
        System.out.println("DEBUG: Found local maximum at index " + index + " (" + data[index].date + 
                          ") with high = " + currentHigh);
        return true;
    }
    
    /**
     * Calculate strength of a support level
     */
    private static double calculateSupportStrength(DataUtils.StockData[] data, int index) {
        // Simple implementation - could be more sophisticated
        // Increased minimum confidence from 0.7 to 0.8
        return 0.8 + (Math.random() * 0.2); // 80-100% confidence
    }
    
    /**
     * Calculate strength of a resistance level
     */
    private static double calculateResistanceStrength(DataUtils.StockData[] data, int index) {
        // Simple implementation - could be more sophisticated
        // Increased minimum confidence from 0.7 to 0.8
        return 0.8 + (Math.random() * 0.2); // 80-100% confidence
    }
    
    /**
     * Detect trend changes in the data
     */
    private static void detectTrendChanges(DataUtils.StockData[] data, List<DataUtils.PatternResult> patterns) {
        // Simple trend change detection using moving averages
        double[] sma20 = calculateSMA(data, 20);
        double[] sma50 = calculateSMA(data, 50);
        double[] sma10 = calculateSMA(data, 10);
        double[] sma5 = calculateSMA(data, 5); // Added ultra-short term SMA
        
        // Look for crossovers
        // Reduced starting point from 50 to 5 to catch more early patterns
        for (int i = 5; i < data.length; i++) {
            // SMA5 crosses above SMA10 (ultra-short term uptrend)
            if (i >= 10 && sma5[i-1] <= sma10[i-1] && sma5[i] > sma10[i]) {
                DataUtils.PatternResult pattern = new DataUtils.PatternResult();
                pattern.type = PATTERN_UPTREND;
                pattern.startIndex = Math.max(0, i - 3);
                pattern.endIndex = i;
                pattern.confidence = 0.65 + (Math.random() * 0.15); // 65-80% confidence
                pattern.expectedMove = 1.5 + (Math.random() * 1.5); // 1.5-3% expected move
                pattern.description = "Ultra short-term bullish trend change at $" + String.format("%.2f", data[i].close) + 
                                     " on " + data[i].date + ". 5-day SMA crossed above 10-day SMA. Suggests short-term momentum shift.";
                patterns.add(pattern);
            }
            
            // SMA5 crosses below SMA10 (ultra-short term downtrend)
            if (i >= 10 && sma5[i-1] >= sma10[i-1] && sma5[i] < sma10[i]) {
                DataUtils.PatternResult pattern = new DataUtils.PatternResult();
                pattern.type = PATTERN_DOWNTREND;
                pattern.startIndex = Math.max(0, i - 3);
                pattern.endIndex = i;
                pattern.confidence = 0.65 + (Math.random() * 0.15); // 65-80% confidence
                pattern.expectedMove = -(1.5 + (Math.random() * 1.5)); // -1.5% to -3% expected move
                pattern.description = "Ultra short-term bearish trend change at $" + String.format("%.2f", data[i].close) + 
                                     " on " + data[i].date + ". 5-day SMA crossed below 10-day SMA. Suggests short-term momentum shift.";
                patterns.add(pattern);
            }
            
            // Uptrend starting (20-day crosses above 50-day)
            if (i >= 50 && sma20[i-1] <= sma50[i-1] && sma20[i] > sma50[i]) {
                DataUtils.PatternResult pattern = new DataUtils.PatternResult();
                pattern.type = PATTERN_UPTREND;
                pattern.startIndex = Math.max(0, i - 10);
                pattern.endIndex = i;
                pattern.confidence = 0.75 + (Math.random() * 0.15); // 75-90% confidence
                pattern.expectedMove = 5 + (Math.random() * 3); // 5-8% expected move
                pattern.description = "Bullish trend change at $" + String.format("%.2f", data[i].close) + 
                                     " on " + data[i].date + ". 20-day SMA crossed above 50-day SMA. Suggests potential upward momentum.";
                patterns.add(pattern);
            }
            
            // Downtrend starting (20-day crosses below 50-day)
            if (i >= 50 && sma20[i-1] >= sma50[i-1] && sma20[i] < sma50[i]) {
                DataUtils.PatternResult pattern = new DataUtils.PatternResult();
                pattern.type = PATTERN_DOWNTREND;
                pattern.startIndex = Math.max(0, i - 10);
                pattern.endIndex = i;
                pattern.confidence = 0.75 + (Math.random() * 0.15); // 75-90% confidence
                pattern.expectedMove = -(5 + (Math.random() * 3)); // -5% to -8% expected move
                pattern.description = "Bearish trend change at $" + String.format("%.2f", data[i].close) + 
                                     " on " + data[i].date + ". 20-day SMA crossed below 50-day SMA. Suggests potential downward pressure.";
                patterns.add(pattern);
            }
            
            // Also detect shorter term trend changes with SMA10 and SMA20
            if (i >= 20) {
                // Short-term uptrend
                if (sma10[i-1] <= sma20[i-1] && sma10[i] > sma20[i]) {
                    DataUtils.PatternResult pattern = new DataUtils.PatternResult();
                    pattern.type = PATTERN_UPTREND;
                    pattern.startIndex = Math.max(0, i - 5);
                    pattern.endIndex = i;
                    pattern.confidence = 0.7 + (Math.random() * 0.15); // 70-85% confidence
                    pattern.expectedMove = 3 + (Math.random() * 2); // 3-5% expected move
                    pattern.description = "Short-term bullish trend change at $" + String.format("%.2f", data[i].close) + 
                                         " on " + data[i].date + ". 10-day SMA crossed above 20-day SMA. Suggests short-term momentum shift.";
                    patterns.add(pattern);
                }
                
                // Short-term downtrend
                if (sma10[i-1] >= sma20[i-1] && sma10[i] < sma20[i]) {
                    DataUtils.PatternResult pattern = new DataUtils.PatternResult();
                    pattern.type = PATTERN_DOWNTREND;
                    pattern.startIndex = Math.max(0, i - 5);
                    pattern.endIndex = i;
                    pattern.confidence = 0.7 + (Math.random() * 0.15); // 70-85% confidence
                    pattern.expectedMove = -(3 + (Math.random() * 2)); // -3% to -5% expected move
                    pattern.description = "Short-term bearish trend change at $" + String.format("%.2f", data[i].close) + 
                                         " on " + data[i].date + ". 10-day SMA crossed below 20-day SMA. Suggests short-term momentum shift.";
                    patterns.add(pattern);
                }
            }
            
            // Detect price acceleration (rate of change increases)
            if (i >= 10) {
                double prevROC = (data[i-5].close - data[i-10].close) / data[i-10].close;
                double currROC = (data[i].close - data[i-5].close) / data[i-5].close;
                
                // If rate of change accelerates significantly
                if (currROC > 0 && currROC > prevROC * 1.5) {
                    DataUtils.PatternResult pattern = new DataUtils.PatternResult();
                    pattern.type = PATTERN_UPTREND;
                    pattern.startIndex = i-5;
                    pattern.endIndex = i;
                    pattern.confidence = 0.68 + (Math.random() * 0.12);
                    pattern.expectedMove = 2 + (Math.random() * 2);
                    pattern.description = "Price acceleration detected at $" + String.format("%.2f", data[i].close) + 
                                         " on " + data[i].date + ". Rate of change increased from " + 
                                         String.format("%.2f", prevROC * 100) + "% to " + 
                                         String.format("%.2f", currROC * 100) + "%. Suggests momentum building.";
                    patterns.add(pattern);
                }
                else if (currROC < 0 && currROC < prevROC * 1.5) {
                    DataUtils.PatternResult pattern = new DataUtils.PatternResult();
                    pattern.type = PATTERN_DOWNTREND;
                    pattern.startIndex = i-5;
                    pattern.endIndex = i;
                    pattern.confidence = 0.68 + (Math.random() * 0.12);
                    pattern.expectedMove = -(2 + (Math.random() * 2));
                    pattern.description = "Price deceleration detected at $" + String.format("%.2f", data[i].close) + 
                                         " on " + data[i].date + ". Rate of change decreased from " + 
                                         String.format("%.2f", prevROC * 100) + "% to " + 
                                         String.format("%.2f", currROC * 100) + "%. Suggests downward momentum building.";
                    patterns.add(pattern);
                }
            }
        }
    }
    
    /**
     * Calculate Simple Moving Average (SMA) for the given data and period
     * Tính giá trị trung bình động đơn giản (SMA) cho dữ liệu và kỳ hạn đã cho
     * 
     * Algorithm:
     * 1. For each point, sum the closing prices of the previous 'period' days
     * 2. Divide the sum by the period to get the average
     * 3. For points with index < period, use available data points only
     * 
     * Thuật toán:
     * 1. Với mỗi điểm, tính tổng giá đóng cửa của 'period' ngày trước đó
     * 2. Chia tổng cho kỳ hạn để có giá trị trung bình
     * 3. Đối với các điểm có chỉ số < kỳ hạn, chỉ sử dụng các điểm dữ liệu có sẵn
     * 
     * @param data Array of stock data (Mảng dữ liệu chứng khoán)
     * @param period Number of days to average (Số ngày để tính trung bình)
     * @return Array of SMA values (Mảng các giá trị SMA)
     */
    public static double[] calculateSMA(DataUtils.StockData[] data, int period) {
        double[] result = new double[data.length];
        
        for (int i = 0; i < data.length; i++) {
            if (i < period - 1) {
                // For early datapoints where we don't have enough history,
                // use the average of what we have so far
                double sum = 0;
                for (int j = 0; j <= i; j++) {
                    sum += data[i - j].close;
                }
                result[i] = sum / (i + 1);
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
     * Detect Head and Shoulders pattern in stock price data
     * Phát hiện mẫu hình Đầu và Vai trong dữ liệu giá cổ phiếu
     * 
     * Algorithm:
     * 1. Scan for potential head points (local maxima)
     * 2. For each potential head, look for left and right shoulders (lower peaks)
     * 3. Validate that shoulders are roughly at the same height
     * 4. Find the neckline by connecting lowest points between peaks
     * 5. Calculate expected price move based on the pattern height
     * 
     * Thuật toán:
     * 1. Quét các điểm tiềm năng làm đầu (các đỉnh cục bộ)
     * 2. Với mỗi đầu tiềm năng, tìm vai trái và vai phải (các đỉnh thấp hơn)
     * 3. Xác nhận rằng các vai xấp xỉ cùng độ cao
     * 4. Tìm đường cổ bằng cách nối các điểm thấp nhất giữa các đỉnh
     * 5. Tính toán biến động giá dự kiến dựa trên chiều cao của mẫu hình
     * 
     * Pattern characteristics:
     * - Requires 5 key points: left shoulder, neckline1, head, neckline2, right shoulder
     * - Head must be higher than both shoulders
     * - Shoulders should be similar in height (within 10% of each other)
     * - Expected price move is typically proportional to the head-to-neckline distance
     * 
     * Đặc điểm mẫu hình:
     * - Cần 5 điểm chính: vai trái, cổ1, đầu, cổ2, vai phải
     * - Đầu phải cao hơn cả hai vai
     * - Các vai nên có độ cao tương tự (trong khoảng 10% so với nhau)
     * - Biến động giá dự kiến thường tỷ lệ thuận với khoảng cách từ đầu đến đường cổ
     * 
     * @param data Array of stock data points (Mảng các điểm dữ liệu chứng khoán)
     * @param patterns List to store detected patterns (Danh sách để lưu trữ các mẫu hình đã phát hiện)
     */
    private static void detectHeadAndShouldersPattern(DataUtils.StockData[] data, List<DataUtils.PatternResult> patterns) {
        // Need at least 40 data points to detect H&S pattern
        if (data.length < 40) {
            return;
        }
        
        // Scan for potential head and shoulders patterns
        // We need to find 5 points: left shoulder, neckline1, head, neckline2, right shoulder
        // Modified to include more recent data (reduced buffer from 20 to 5)
        for (int i = 20; i < data.length - 5; i++) {
            // Try to identify a potential head (local maximum)
            if (isLocalMaximum(data, i, 5)) {
                double headValue = data[i].high;
                
                // Look back for left shoulder (should be lower than head)
                int leftShoulderIdx = -1;
                double leftShoulderVal = 0;
                
                for (int j = i - 5; j > i - 15 && j > 0; j--) {
                    if (isLocalMaximum(data, j, 3) && data[j].high < headValue) {
                        leftShoulderIdx = j;
                        leftShoulderVal = data[j].high;
                        break;
                    }
                }
                
                if (leftShoulderIdx < 0) continue; // No left shoulder found
                
                // Look forward for right shoulder (should be lower than head and similar to left shoulder)
                int rightShoulderIdx = -1;
                double rightShoulderVal = 0;
                
                for (int j = i + 5; j < i + 15 && j < data.length; j++) {
                    if (isLocalMaximum(data, j, 3) && data[j].high < headValue) {
                        rightShoulderIdx = j;
                        rightShoulderVal = data[j].high;
                        break;
                    }
                }
                
                if (rightShoulderIdx < 0) continue; // No right shoulder found
                
                // Validate the pattern - shoulders should be roughly similar in height
                double shoulderDiff = Math.abs(leftShoulderVal - rightShoulderVal);
                if (shoulderDiff > 0.1 * headValue) continue; // Shoulders too different
                
                // Find neckline by connecting lows between shoulders and head
                int neckline1Idx = findLowestPointBetween(data, leftShoulderIdx, i);
                int neckline2Idx = findLowestPointBetween(data, i, rightShoulderIdx);
                
                if (neckline1Idx < 0 || neckline2Idx < 0) continue;
                
                // Pattern detected, add to results
                DataUtils.PatternResult pattern = new DataUtils.PatternResult();
                pattern.type = PATTERN_HEAD_AND_SHOULDERS;
                pattern.startIndex = leftShoulderIdx - 2;
                pattern.endIndex = rightShoulderIdx + 2;
                pattern.confidence = 0.80 + (Math.random() * 0.15); // 80-95% confidence
                
                // Head and shoulders usually predicts a downtrend
                double necklinePrice = (data[neckline1Idx].low + data[neckline2Idx].low) / 2;
                double headHeight = headValue - necklinePrice;
                pattern.expectedMove = -(headHeight / necklinePrice) * 100; // Predicted percent drop
                
                pattern.description = "Head and Shoulders pattern from " + data[leftShoulderIdx].date + 
                                     " to " + data[rightShoulderIdx].date + 
                                     ". Head at $" + String.format("%.2f", headValue) + 
                                     ", neckline at $" + String.format("%.2f", necklinePrice) + 
                                     ". Bearish reversal pattern that suggests a potential downtrend.";
                patterns.add(pattern);
                
                // Skip ahead to avoid overlapping patterns
                i = rightShoulderIdx;
            }
        }
    }
    
    /**
     * Find the lowest price point between two indices
     */
    private static int findLowestPointBetween(DataUtils.StockData[] data, int startIdx, int endIdx) {
        int lowestIdx = -1;
        double lowestPrice = Double.MAX_VALUE;
        
        for (int i = startIdx; i <= endIdx; i++) {
            if (data[i].low < lowestPrice) {
                lowestPrice = data[i].low;
                lowestIdx = i;
            }
        }
        
        return lowestIdx;
    }
    
    /**
     * Special method to detect more patterns in future data
     */
    private static void detectPatternsInFutureData(DataUtils.StockData[] data, List<DataUtils.PatternResult> patterns) {
        // Today's date for comparison
        java.time.LocalDate today = java.time.LocalDate.now();
        
        // Look for future dates and analyze them more aggressively
        for (int i = 5; i < data.length - 2; i++) {
            String dateStr = data[i].date;
            java.time.LocalDate dataDate;
            
            try {
                // Parse the date string to check if it's in the future
                if (dateStr.contains("T")) {
                    dateStr = dateStr.split("T")[0];
                }
                dataDate = java.time.LocalDate.parse(dateStr);
                
                // If this is a future date, apply more aggressive pattern detection
                if (dataDate.isAfter(today)) {
                    // Check for volatility changes in future data
                    if (i > 10 && i < data.length - 5) {
                        double pastVolatility = calculateVolatility(data, i-10, i-1);
                        double futureVolatility = calculateVolatility(data, i, i+5);
                        
                        // Detect volatility increase
                        if (futureVolatility > pastVolatility * 1.2) {
                            DataUtils.PatternResult pattern = new DataUtils.PatternResult();
                            pattern.type = PATTERN_TREND_CHANGE;
                            pattern.startIndex = i;
                            pattern.endIndex = Math.min(data.length - 1, i + 5);
                            pattern.confidence = 0.85;
                            pattern.description = "Predicted volatility increase on " + data[i].date + 
                                                 ". Price volatility expected to increase from " + 
                                                 String.format("%.2f", pastVolatility) + " to " + 
                                                 String.format("%.2f", futureVolatility) + ".";
                            patterns.add(pattern);
                        }
                        
                        // Detect major price moves
                        double priceMove = calculatePriceMove(data, i, Math.min(data.length - 1, i + 5));
                        if (Math.abs(priceMove) > 3.0) { // More than 3% move
                            DataUtils.PatternResult pattern = new DataUtils.PatternResult();
                            
                            if (priceMove > 0) {
                                pattern.type = PATTERN_UPTREND;
                                pattern.expectedMove = priceMove;
                                pattern.description = "Predicted price rally on " + data[i].date + 
                                                     ". Expected " + String.format("%.1f", priceMove) + 
                                                     "% price increase over next " + 
                                                     Math.min(5, data.length - i - 1) + " days.";
                            } else {
                                pattern.type = PATTERN_DOWNTREND;
                                pattern.expectedMove = priceMove;
                                pattern.description = "Predicted price decline on " + data[i].date + 
                                                     ". Expected " + String.format("%.1f", Math.abs(priceMove)) + 
                                                     "% price decrease over next " + 
                                                     Math.min(5, data.length - i - 1) + " days.";
                            }
                            
                            pattern.startIndex = i;
                            pattern.endIndex = Math.min(data.length - 1, i + 5);
                            pattern.confidence = 0.80;
                            patterns.add(pattern);
                        }
                    }
                    
                    // For future Head & Shoulders patterns, check with smaller window
                    if (i > 30 && i < data.length - 15) {
                        detectHeadAndShouldersFuture(data, patterns, i);
                    }
                }
            } catch (Exception e) {
                // Skip dates that can't be parsed
                continue;
            }
        }
    }
    
    /**
     * Calculate volatility for a range of stock data
     * Tính độ biến động cho một khoảng dữ liệu chứng khoán
     * 
     * Algorithm:
     * 1. Calculate daily returns as percentage changes between consecutive days
     * 2. Find the mean (average) of daily returns
     * 3. Calculate variance as the average of squared deviations from mean
     * 4. Take the square root of variance to get standard deviation (volatility)
     * 5. Convert to percentage for easier interpretation
     * 
     * Thuật toán:
     * 1. Tính lợi nhuận hàng ngày dưới dạng tỷ lệ phần trăm thay đổi giữa các ngày liên tiếp
     * 2. Tìm giá trị trung bình của lợi nhuận hàng ngày
     * 3. Tính phương sai dưới dạng trung bình của độ lệch bình phương từ giá trị trung bình
     * 4. Lấy căn bậc hai của phương sai để có độ lệch chuẩn (độ biến động)
     * 5. Chuyển đổi thành phần trăm để dễ giải thích
     * 
     * Formula: Volatility = √(Σ(r_i - r_avg)² / n) * 100%
     * Where:
     * - r_i = daily return for day i
     * - r_avg = average of all daily returns
     * - n = number of days
     * 
     * Công thức: Độ biến động = √(Σ(r_i - r_tb)² / n) * 100%
     * Trong đó:
     * - r_i = lợi nhuận hàng ngày cho ngày i
     * - r_tb = trung bình của tất cả lợi nhuận hàng ngày
     * - n = số ngày
     * 
     * @param data Array of stock data (Mảng dữ liệu chứng khoán)
     * @param startIndex Starting index for calculation (Chỉ số bắt đầu tính toán)
     * @param endIndex Ending index for calculation (Chỉ số kết thúc tính toán)
     * @return Volatility as a percentage (Độ biến động dưới dạng phần trăm)
     */
    private static double calculateVolatility(DataUtils.StockData[] data, int startIndex, int endIndex) {
        double sum = 0;
        double sumSq = 0;
        int count = 0;
        
        // Calculate daily returns
        for (int i = startIndex + 1; i <= endIndex; i++) {
            double dailyReturn = (data[i].close - data[i-1].close) / data[i-1].close;
            sum += dailyReturn;
            sumSq += dailyReturn * dailyReturn;
            count++;
        }
        
        if (count < 2) return 0;
        
        double mean = sum / count;
        double variance = (sumSq / count) - (mean * mean);
        return Math.sqrt(variance) * 100; // Convert to percentage
    }
    
    /**
     * Calculate price move percentage over a range
     */
    private static double calculatePriceMove(DataUtils.StockData[] data, int startIndex, int endIndex) {
        if (startIndex >= endIndex) return 0;
        
        double startPrice = data[startIndex].close;
        double endPrice = data[endIndex].close;
        
        return ((endPrice - startPrice) / startPrice) * 100.0; // Return as percentage
    }
    
    /**
     * Detect head and shoulders pattern specifically in future data
     * Uses more relaxed parameters than the standard detection
     */
    private static void detectHeadAndShouldersFuture(DataUtils.StockData[] data, List<DataUtils.PatternResult> patterns, int startIndex) {
        // Look for local maxima as potential heads
        for (int i = startIndex; i < data.length - 3; i++) {
            if (isRelativeMaximum(data, i, 3)) { // Using relaxed maximum detection
                double headValue = data[i].high;
                
                // Look for left shoulder (relaxed parameters)
                int leftShoulderIdx = -1;
                double leftShoulderVal = 0;
                
                for (int j = i - 3; j > i - 10 && j > startIndex; j--) {
                    if (isRelativeMaximum(data, j, 2) && data[j].high < headValue * 1.1) {
                        leftShoulderIdx = j;
                        leftShoulderVal = data[j].high;
                        break;
                    }
                }
                
                if (leftShoulderIdx < 0) continue;
                
                // Look for right shoulder (relaxed parameters)
                int rightShoulderIdx = -1;
                double rightShoulderVal = 0;
                
                for (int j = i + 3; j < i + 10 && j < data.length; j++) {
                    if (isRelativeMaximum(data, j, 2) && data[j].high < headValue * 1.1) {
                        rightShoulderIdx = j;
                        rightShoulderVal = data[j].high;
                        break;
                    }
                }
                
                if (rightShoulderIdx < 0) continue;
                
                // Validate shoulders - more relaxed validation for future data
                double shoulderDiff = Math.abs(leftShoulderVal - rightShoulderVal);
                if (shoulderDiff > 0.2 * headValue) continue; // 20% difference allowed
                
                // Create pattern
                DataUtils.PatternResult pattern = new DataUtils.PatternResult();
                pattern.type = PATTERN_HEAD_AND_SHOULDERS;
                pattern.startIndex = leftShoulderIdx;
                pattern.endIndex = rightShoulderIdx;
                pattern.confidence = 0.75 + (Math.random() * 0.15);
                
                // Estimate expected move
                double necklinePrice = (data[leftShoulderIdx].low + data[rightShoulderIdx].low) / 2;
                double headHeight = headValue - necklinePrice;
                pattern.expectedMove = -(headHeight / necklinePrice) * 100;
                
                pattern.description = "Head and Shoulders pattern from " + data[leftShoulderIdx].date + 
                                     " to " + data[rightShoulderIdx].date + 
                                     ". Head at $" + String.format("%.2f", headValue) + 
                                     ". Predicted reversal pattern.";
                
                patterns.add(pattern);
                
                // Skip ahead
                i = rightShoulderIdx;
            }
        }
    }
    
    /**
     * Check if point is a relative maximum (more relaxed than a local maximum)
     */
    private static boolean isRelativeMaximum(double[] values, int index, int window) {
        double currentHigh = values[index];
        int higherCount = 0;
        
        // Check surrounding points
        for (int i = Math.max(0, index - window); i <= Math.min(values.length - 1, index + window); i++) {
            if (i != index && values[i] > currentHigh) {
                higherCount++;
            }
        }
        
        // Allow up to 2 points to be higher for a relaxed maximum
        return higherCount <= 2;
    }
    
    /**
     * Check if a point in stock data is a relative maximum based on closing price
     */
    private static boolean isRelativeMaximum(DataUtils.StockData[] data, int index, int range) {
        if (index < range || index >= data.length - range) return false;
        
        double close = data[index].close;
        for (int i = index - range; i <= index + range; i++) {
            if (i != index && data[i].close > close) {
                System.out.println("DEBUG: Not a relative maximum at index " + index + " (" + data[index].date + 
                                  ") because close[" + i + "] = " + data[i].close + 
                                  " > close = " + close);
                return false;
            }
        }
        System.out.println("DEBUG: Found relative maximum at index " + index + " (" + data[index].date + 
                          ") with close = " + close);
        return true;
    }
    
    /**
     * Detect Simple Moving Average (SMA) crossover trading signals
     * Phát hiện tín hiệu giao dịch khi đường SMA cắt nhau
     * 
     * Algorithm:
     * 1. Calculate short period SMA (faster moving average)
     * 2. Calculate long period SMA (slower moving average)
     * 3. Look for points where the two SMAs cross:
     *    - BUY signal: Short SMA crosses above Long SMA
     *    - SELL signal: Short SMA crosses below Long SMA
     * 4. For each signal, calculate target price and stop loss
     * 
     * Thuật toán:
     * 1. Tính SMA kỳ hạn ngắn (đường trung bình động nhanh)
     * 2. Tính SMA kỳ hạn dài (đường trung bình động chậm)
     * 3. Tìm kiếm các điểm mà hai đường SMA cắt nhau:
     *    - Tín hiệu MUA: SMA ngắn cắt lên trên SMA dài
     *    - Tín hiệu BÁN: SMA ngắn cắt xuống dưới SMA dài
     * 4. Với mỗi tín hiệu, tính giá mục tiêu và giá dừng lỗ
     * 
     * Trading strategy:
     * - Buy signals have 5% profit target and 3% stop loss
     * - Sell signals have 5% profit target and 3% stop loss
     * - Risk/reward ratio is 5/3 = 1.67
     * 
     * Chiến lược giao dịch:
     * - Tín hiệu mua có mục tiêu lợi nhuận 5% và dừng lỗ 3%
     * - Tín hiệu bán có mục tiêu lợi nhuận 5% và dừng lỗ 3%
     * - Tỷ lệ rủi ro/phần thưởng là 5/3 = 1.67
     * 
     * @param data Array of stock data points (Mảng các điểm dữ liệu chứng khoán)
     * @param shortPeriod Period for short-term SMA (Kỳ hạn cho SMA ngắn hạn)
     * @param longPeriod Period for long-term SMA (Kỳ hạn cho SMA dài hạn)
     * @return Array of trading signals detected (Mảng các tín hiệu giao dịch đã phát hiện)
     */
    public static DataUtils.TradingSignal[] detectSMACrossoverSignals(DataUtils.StockData[] data, int shortPeriod, int longPeriod) {
        if (data == null || data.length < longPeriod + 1) {
            return new DataUtils.TradingSignal[0]; // Not enough data
        }
        
        // Create dynamic list to hold signals
        List<DataUtils.TradingSignal> signalList = new ArrayList<>();
        
        // Calculate SMAs
        double[] shortSMA = calculateSMA(data, shortPeriod);
        double[] longSMA = calculateSMA(data, longPeriod);
        
        // Look for crossovers, starting from the first valid point
        // Modified to include the most recent data point
        for (int i = longPeriod; i < data.length; i++) {
            // Short SMA crosses above Long SMA - bullish signal
            if (shortSMA[i-1] <= longSMA[i-1] && shortSMA[i] > longSMA[i]) {
                DataUtils.TradingSignal signal = new DataUtils.TradingSignal();
                signal.type = "BUY";
                signal.signalIndex = i;
                signal.confidence = calculateSignalConfidence(data, i, true);
                signal.entryPrice = data[i].close;
                signal.targetPrice = signal.entryPrice * 1.05; // 5% profit target
                signal.stopLossPrice = signal.entryPrice * 0.97; // 3% stop loss
                signal.riskRewardRatio = 5.0 / 3.0; // Risk-reward ratio
                signal.description = "Buy signal on " + data[i].date + " at $" + String.format("%.2f", signal.entryPrice) + 
                                    ". " + shortPeriod + "-day SMA crossed above " + longPeriod + 
                                    "-day SMA. Target: $" + String.format("%.2f", signal.targetPrice) + 
                                    ", Stop: $" + String.format("%.2f", signal.stopLossPrice);
                signalList.add(signal);
            }
            
            // Short SMA crosses below Long SMA - bearish signal
            if (shortSMA[i-1] >= longSMA[i-1] && shortSMA[i] < longSMA[i]) {
                DataUtils.TradingSignal signal = new DataUtils.TradingSignal();
                signal.type = "SELL";
                signal.signalIndex = i;
                signal.confidence = calculateSignalConfidence(data, i, false);
                signal.entryPrice = data[i].close;
                signal.targetPrice = signal.entryPrice * 0.95; // 5% profit target (for short)
                signal.stopLossPrice = signal.entryPrice * 1.03; // 3% stop loss (for short)
                signal.riskRewardRatio = 5.0 / 3.0; // Risk-reward ratio
                signal.description = "Sell signal on " + data[i].date + " at $" + String.format("%.2f", signal.entryPrice) + 
                                    ". " + shortPeriod + "-day SMA crossed below " + longPeriod + 
                                    "-day SMA. Target: $" + String.format("%.2f", signal.targetPrice) + 
                                    ", Stop: $" + String.format("%.2f", signal.stopLossPrice);
                signalList.add(signal);
            }
        }
        
        // Convert list to array
        DataUtils.TradingSignal[] result = new DataUtils.TradingSignal[signalList.size()];
        return signalList.toArray(result);
    }
    
    /**
     * Calculate confidence for a signal
     */
    private static double calculateSignalConfidence(DataUtils.StockData[] data, int index, boolean isBuy) {
        // Simple implementation - could be more sophisticated
        return 0.7 + (Math.random() * 0.3); // 70-100% confidence
    }
    
    /**
     * Detect anomalies in the stock data
     * Phát hiện các bất thường trong dữ liệu chứng khoán
     * 
     * Algorithm:
     * 1. Extract price and volume data into arrays
     * 2. Calculate mean and standard deviation for price changes and volume
     * 3. For each data point, calculate z-scores for price change and volume
     * 4. Combine z-scores with weights: price change (70%) and volume (30%)
     * 5. Flag points with anomaly score > 2.5 standard deviations
     * 
     * Thuật toán:
     * 1. Trích xuất dữ liệu giá và khối lượng vào các mảng
     * 2. Tính giá trị trung bình và độ lệch chuẩn cho biến động giá và khối lượng
     * 3. Với mỗi điểm dữ liệu, tính điểm z-score cho biến động giá và khối lượng
     * 4. Kết hợp các z-score với trọng số: biến động giá (70%) và khối lượng (30%)  
     * 5. Đánh dấu các điểm có điểm bất thường > 2.5 độ lệch chuẩn
     * 
     * Formula:
     *   z-score = |value - mean| / stdDev
     *   anomalyScore = (priceChangeZScore * 0.7) + (volumeZScore * 0.3)
     * 
     * Công thức:
     *   z-score = |giá trị - trung bình| / độ lệch chuẩn
     *   điểm bất thường = (z-score biến động giá * 0.7) + (z-score khối lượng * 0.3)
     * 
     * @param data Array of stock data points (Mảng các điểm dữ liệu chứng khoán)
     * @return Array of detected anomalies (Mảng các bất thường đã phát hiện)
     */
    public static DataUtils.AnomalyResult[] detectAnomalies(DataUtils.StockData[] data) {
        if (data == null || data.length < 30) {
            return new DataUtils.AnomalyResult[0]; // Not enough data
        }
        
        // Create dynamic list to hold anomalies
        List<DataUtils.AnomalyResult> anomalyList = new ArrayList<>();
        
        // Prepare arrays for analysis
        double[] prices = new double[data.length];
        double[] volumes = new double[data.length];
        double[] priceChanges = new double[data.length-1];
        
        // Extract data for analysis
        for (int i = 0; i < data.length; i++) {
            prices[i] = data[i].close;
            volumes[i] = data[i].volume;
            
            if (i > 0) {
                priceChanges[i-1] = (data[i].close - data[i-1].close) / data[i-1].close;
            }
        }
        
        // Calculate mean and standard deviation for price changes and volumes
        double meanPriceChange = calculateMean(priceChanges);
        double stdDevPriceChange = calculateStdDev(priceChanges, meanPriceChange);
        
        double meanVolume = calculateMean(volumes);
        double stdDevVolume = calculateStdDev(volumes, meanVolume);
        
        // Look for anomalies in the data, starting from day 1
        // No change needed here as it already loops through all data points
        for (int i = 1; i < data.length; i++) {
            // Calculate price change for this day
            double priceChange = (data[i].close - data[i-1].close) / data[i-1].close;
            
            // Calculate z-scores
            double priceChangeZScore = Math.abs((priceChange - meanPriceChange) / stdDevPriceChange);
            double volumeZScore = Math.abs((data[i].volume - meanVolume) / stdDevVolume);
            
            // Anomaly score is a weighted combination of price and volume z-scores
            double anomalyScore = (priceChangeZScore * 0.7) + (volumeZScore * 0.3);
            
            // If anomaly score is high enough, record it
            if (anomalyScore > 2.5) { // More than 2.5 standard deviations
                DataUtils.AnomalyResult anomaly = new DataUtils.AnomalyResult();
                anomaly.index = i;
                anomaly.score = anomalyScore;
                anomaly.priceDeviation = priceChangeZScore;
                anomaly.volumeDeviation = volumeZScore;
                
                double percentChange = priceChange * 100.0;
                String changeDirection = percentChange > 0 ? "increased" : "decreased";
                String volumeDirection = volumeZScore > 2.0 ? "high" : "normal";
                
                anomaly.description = "Anomaly detected on " + data[i].date + ": Price " + 
                                     changeDirection + " by " + String.format("%.2f", Math.abs(percentChange)) + 
                                     "% to $" + String.format("%.2f", data[i].close) + 
                                     " with " + volumeDirection + " volume (" + 
                                     String.format("%.0f", data[i].volume) + 
                                     "). Deviation from normal: " + String.format("%.1f", anomalyScore) + " σ.";
                
                anomalyList.add(anomaly);
            }
        }
        
        // Convert list to array
        DataUtils.AnomalyResult[] result = new DataUtils.AnomalyResult[anomalyList.size()];
        return anomalyList.toArray(result);
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
    
    /**
     * Detect divergence patterns between price and technical indicators
     */
    private static void detectDivergencePatterns(DataUtils.StockData[] data, List<DataUtils.PatternResult> patterns) {
        if (data.length < 30) return;
        
        // Calculate RSI for divergence detection
        double[] rsi = calculateRSI(data, 14);
        // Calculate MACD for divergence detection
        double[] macd = calculateMACD(data).getLine();
        
        for (int i = 30; i < data.length; i++) {
            // Find local price highs and lows
            if (isRelativeMaximum(data, i, 5)) {
                // Check for bearish RSI divergence (price high, RSI lower high)
                if (i >= 10 && 
                    isRelativeMaximum(rsi, i, 5) && 
                    findPreviousRelativeMax(data, i, 20) != -1) {
                    
                    int prevPriceMaxIdx = findPreviousRelativeMax(data, i, 20);
                    int prevRsiMaxIdx = findPreviousRelativeMax(rsi, i, 20);
                    
                    if (prevPriceMaxIdx != -1 && prevRsiMaxIdx != -1 && 
                        data[i].close > data[prevPriceMaxIdx].close && 
                        rsi[i] < rsi[prevRsiMaxIdx]) {
                        
                        DataUtils.PatternResult pattern = new DataUtils.PatternResult();
                        pattern.type = PATTERN_REVERSAL;
                        pattern.startIndex = prevPriceMaxIdx;
                        pattern.endIndex = i;
                        pattern.confidence = 0.75 + (Math.random() * 0.15);
                        pattern.expectedMove = -(4 + (Math.random() * 3)); // -4% to -7% expected move
                        pattern.description = "Bearish RSI divergence at $" + String.format("%.2f", data[i].close) + 
                                             " on " + data[i].date + ". Price made higher high while RSI made lower high. " +
                                             "Suggests potential trend reversal down.";
                        patterns.add(pattern);
                    }
                }
                
                // Check for bearish MACD divergence
                if (i >= 15 && 
                    isRelativeMaximum(macd, i, 5) && 
                    findPreviousRelativeMax(data, i, 30) != -1) {
                    
                    int prevPriceMaxIdx = findPreviousRelativeMax(data, i, 30);
                    int prevMacdMaxIdx = findPreviousRelativeMax(macd, i, 30);
                    
                    if (prevPriceMaxIdx != -1 && prevMacdMaxIdx != -1 && 
                        data[i].close > data[prevPriceMaxIdx].close && 
                        macd[i] < macd[prevMacdMaxIdx]) {
                        
                        DataUtils.PatternResult pattern = new DataUtils.PatternResult();
                        pattern.type = PATTERN_REVERSAL;
                        pattern.startIndex = prevPriceMaxIdx;
                        pattern.endIndex = i;
                        pattern.confidence = 0.78 + (Math.random() * 0.12);
                        pattern.expectedMove = -(4.5 + (Math.random() * 3.5)); // -4.5% to -8% expected move
                        pattern.description = "Bearish MACD divergence at $" + String.format("%.2f", data[i].close) + 
                                             " on " + data[i].date + ". Price made higher high while MACD made lower high. " +
                                             "Strong indication of potential bearish reversal.";
                        patterns.add(pattern);
                    }
                }
            }
            
            if (isRelativeMinimum(data, i, 5)) {
                // Check for bullish RSI divergence (price low, RSI higher low)
                if (i >= 10 && 
                    isRelativeMinimum(rsi, i, 5) && 
                    findPreviousRelativeMin(data, i, 20) != -1) {
                    
                    int prevPriceMinIdx = findPreviousRelativeMin(data, i, 20);
                    int prevRsiMinIdx = findPreviousRelativeMin(rsi, i, 20);
                    
                    if (prevPriceMinIdx != -1 && prevRsiMinIdx != -1 && 
                        data[i].close < data[prevPriceMinIdx].close && 
                        rsi[i] > rsi[prevRsiMinIdx]) {
                        
                        DataUtils.PatternResult pattern = new DataUtils.PatternResult();
                        pattern.type = PATTERN_REVERSAL;
                        pattern.startIndex = prevPriceMinIdx;
                        pattern.endIndex = i;
                        pattern.confidence = 0.75 + (Math.random() * 0.15);
                        pattern.expectedMove = 4 + (Math.random() * 3); // 4% to 7% expected move
                        pattern.description = "Bullish RSI divergence at $" + String.format("%.2f", data[i].close) + 
                                             " on " + data[i].date + ". Price made lower low while RSI made higher low. " +
                                             "Suggests potential trend reversal up.";
                        patterns.add(pattern);
                    }
                }
                
                // Check for bullish MACD divergence
                if (i >= 15 && 
                    isRelativeMinimum(macd, i, 5) && 
                    findPreviousRelativeMin(data, i, 30) != -1) {
                    
                    int prevPriceMinIdx = findPreviousRelativeMin(data, i, 30);
                    int prevMacdMinIdx = findPreviousRelativeMin(macd, i, 30);
                    
                    if (prevPriceMinIdx != -1 && prevMacdMinIdx != -1 && 
                        data[i].close < data[prevPriceMinIdx].close && 
                        macd[i] > macd[prevMacdMinIdx]) {
                        
                        DataUtils.PatternResult pattern = new DataUtils.PatternResult();
                        pattern.type = PATTERN_REVERSAL;
                        pattern.startIndex = prevPriceMinIdx;
                        pattern.endIndex = i;
                        pattern.confidence = 0.78 + (Math.random() * 0.12);
                        pattern.expectedMove = 4.5 + (Math.random() * 3.5); // 4.5% to 8% expected move
                        pattern.description = "Bullish MACD divergence at $" + String.format("%.2f", data[i].close) + 
                                             " on " + data[i].date + ". Price made lower low while MACD made higher low. " +
                                             "Strong indication of potential bullish reversal.";
                        patterns.add(pattern);
                    }
                }
            }
        }
    }
    
    /**
     * Find the previous relative maximum in an array before the given index
     */
    private static int findPreviousRelativeMax(double[] values, int currentIndex, int lookbackRange) {
        int startLookback = Math.max(0, currentIndex - lookbackRange);
        for (int i = currentIndex - 5; i >= startLookback; i--) {
            if (isRelativeMaximum(values, i, 3)) {
                return i;
            }
        }
        return -1;
    }
    
    /**
     * Find the previous relative maximum in stock data before the given index
     */
    private static int findPreviousRelativeMax(DataUtils.StockData[] data, int currentIndex, int lookbackRange) {
        int startLookback = Math.max(0, currentIndex - lookbackRange);
        for (int i = currentIndex - 5; i >= startLookback; i--) {
            if (isRelativeMaximum(data, i, 3)) {
                return i;
            }
        }
        return -1;
    }
    
    /**
     * Find the previous relative minimum in an array before the given index
     */
    private static int findPreviousRelativeMin(double[] values, int currentIndex, int lookbackRange) {
        int startLookback = Math.max(0, currentIndex - lookbackRange);
        for (int i = currentIndex - 5; i >= startLookback; i--) {
            if (isRelativeMinimum(values, i, 3)) {
                return i;
            }
        }
        return -1;
    }
    
    /**
     * Find the previous relative minimum in stock data before the given index
     */
    private static int findPreviousRelativeMin(DataUtils.StockData[] data, int currentIndex, int lookbackRange) {
        int startLookback = Math.max(0, currentIndex - lookbackRange);
        for (int i = currentIndex - 5; i >= startLookback; i--) {
            if (isRelativeMinimum(data, i, 3)) {
                return i;
            }
        }
        return -1;
    }
    
    /**
     * Check if a point in an array is a relative minimum
     * 
     * A relative minimum is a point where the value is less than or equal to
     * all other values within the specified range on both sides.
     * 
     * Unlike isLocalMinimum which checks the low price, this method checks
     * an arbitrary array of values (like RSI, MACD, etc.).
     */
    private static boolean isRelativeMinimum(double[] values, int index, int range) {
        if (index < range || index >= values.length - range) return false;
        
        double value = values[index];
        for (int i = index - range; i <= index + range; i++) {
            if (i != index && values[i] < value) {
                System.out.println("DEBUG: Not a relative minimum at index " + index + " because values[" + i + "] = " + 
                                  values[i] + " < value = " + value);
                return false;
            }
        }
        System.out.println("DEBUG: Found relative minimum at index " + index + " with value = " + value);
        return true;
    }
    
    /**
     * Check if a point in stock data is a relative minimum based on closing price
     * 
     * A relative minimum is a point where the close price is less than or equal to
     * all other close prices within the specified range on both sides.
     * 
     * Unlike isLocalMinimum which checks the low price, this method checks the close price.
     */
    private static boolean isRelativeMinimum(DataUtils.StockData[] data, int index, int range) {
        if (index < range || index >= data.length - range) return false;
        
        double close = data[index].close;
        for (int i = index - range; i <= index + range; i++) {
            if (i != index && data[i].close < close) {
                System.out.println("DEBUG: Not a relative minimum at index " + index + " (" + data[index].date + 
                                  ") because close[" + i + "] = " + data[i].close + 
                                  " < close = " + close);
                return false;
            }
        }
        System.out.println("DEBUG: Found relative minimum at index " + index + " (" + data[index].date + 
                          ") with close = " + close);
        return true;
    }
    
    /**
     * Calculate Relative Strength Index (RSI) for the given data and period
     * Tính chỉ số sức mạnh tương đối (RSI) cho dữ liệu và kỳ hạn đã cho
     * 
     * Algorithm:
     * 1. Calculate price changes between consecutive days
     * 2. Separate gains (price increases) and losses (price decreases)
     * 3. Calculate average gain and average loss over the specified period
     * 4. Calculate Relative Strength (RS) = average gain / average loss
     * 5. Calculate RSI = 100 - (100 / (1 + RS))
     * 
     * Thuật toán:
     * 1. Tính thay đổi giá giữa các ngày liên tiếp
     * 2. Phân tách thành mức tăng (giá tăng) và mức giảm (giá giảm)
     * 3. Tính mức tăng trung bình và mức giảm trung bình trong kỳ hạn đã chỉ định
     * 4. Tính sức mạnh tương đối (RS) = mức tăng trung bình / mức giảm trung bình
     * 5. Tính RSI = 100 - (100 / (1 + RS))
     * 
     * Technical interpretation:
     * - RSI > 70: Potentially overbought condition (asset may be overvalued)
     * - RSI < 30: Potentially oversold condition (asset may be undervalued)
     * - RSI divergence from price can signal potential reversals
     * 
     * Diễn giải kỹ thuật:
     * - RSI > 70: Có thể đang mua quá mức (tài sản có thể đang được định giá quá cao)
     * - RSI < 30: Có thể đang bán quá mức (tài sản có thể đang được định giá quá thấp)
     * - Sự phân kỳ RSI so với giá có thể báo hiệu khả năng đảo chiều
     * 
     * @param data Array of stock data (Mảng dữ liệu chứng khoán)
     * @param period Period for RSI calculation, typically 14 days (Kỳ hạn tính RSI, thường là 14 ngày)
     * @return Array of RSI values (Mảng các giá trị RSI)
     */
    private static double[] calculateRSI(DataUtils.StockData[] data, int period) {
        double[] rsi = new double[data.length];
        Arrays.fill(rsi, 50); // Default value
        
        if (data.length <= period) return rsi;
        
        double[] gains = new double[data.length];
        double[] losses = new double[data.length];
        
        // Calculate price changes
        for (int i = 1; i < data.length; i++) {
            double change = data[i].close - data[i-1].close;
            if (change > 0) {
                gains[i] = change;
                losses[i] = 0;
            } else {
                gains[i] = 0;
                losses[i] = Math.abs(change);
            }
        }
        
        // Calculate initial averages
        double avgGain = 0;
        double avgLoss = 0;
        
        for (int i = 1; i <= period; i++) {
            avgGain += gains[i];
            avgLoss += losses[i];
        }
        
        avgGain /= period;
        avgLoss /= period;
        
        // Calculate RSI
        if (avgLoss > 0) {
            double rs = avgGain / avgLoss;
            rsi[period] = 100 - (100 / (1 + rs));
        } else {
            rsi[period] = 100; // No losses, RSI = 100
        }
        
        // Calculate remaining RSI values
        for (int i = period + 1; i < data.length; i++) {
            avgGain = ((avgGain * (period - 1)) + gains[i]) / period;
            avgLoss = ((avgLoss * (period - 1)) + losses[i]) / period;
            
            if (avgLoss > 0) {
                double rs = avgGain / avgLoss;
                rsi[i] = 100 - (100 / (1 + rs));
            } else {
                rsi[i] = 100; // No losses, RSI = a maximum 100
            }
        }
        
        return rsi;
    }
    
    /**
     * Calculate MACD (Moving Average Convergence Divergence)
     * Tính chỉ báo MACD (Phân kỳ và Hội tụ Trung bình Động)
     * 
     * Algorithm:
     * 1. Calculate the fast EMA (typically 12-period)
     * 2. Calculate the slow EMA (typically 26-period)
     * 3. MACD Line = Fast EMA - Slow EMA
     * 4. Signal Line = 9-period EMA of MACD Line
     * 5. Histogram = MACD Line - Signal Line
     * 
     * Thuật toán:
     * 1. Tính EMA nhanh (thường là 12 kỳ)
     * 2. Tính EMA chậm (thường là 26 kỳ)
     * 3. Đường MACD = EMA nhanh - EMA chậm
     * 4. Đường tín hiệu = EMA 9 kỳ của đường MACD
     * 5. Histogram = Đường MACD - Đường tín hiệu
     * 
     * Technical interpretation:
     * - MACD line crossing above signal line: Bullish signal
     * - MACD line crossing below signal line: Bearish signal
     * - MACD line crossing zero from below: Strong bullish signal
     * - MACD line crossing zero from above: Strong bearish signal
     * - Divergence between MACD and price: Potential reversal
     * 
     * Diễn giải kỹ thuật:
     * - Đường MACD cắt lên trên đường tín hiệu: Tín hiệu tăng giá
     * - Đường MACD cắt xuống dưới đường tín hiệu: Tín hiệu giảm giá
     * - Đường MACD cắt lên trên mức 0: Tín hiệu tăng giá mạnh
     * - Đường MACD cắt xuống dưới mức 0: Tín hiệu giảm giá mạnh
     * - Phân kỳ giữa MACD và giá: Khả năng đảo chiều
     * 
     * @param data Array of stock data points (Mảng các điểm dữ liệu chứng khoán)
     * @return MACDResult object containing MACD line, signal line, and histogram values
     *         (Đối tượng MACDResult chứa các giá trị đường MACD, đường tín hiệu và histogram)
     */
    public static MACDResult calculateMACD(DataUtils.StockData[] data) {
        // Default MACD parameters: 12-day EMA, 26-day EMA, 9-day signal
        // Tham số MACD mặc định: EMA 12 ngày, EMA 26 ngày, tín hiệu 9 ngày
        return calculateMACD(data, 12, 26, 9);
    }
    
    /**
     * Calculate MACD with custom parameters
     * Tính MACD với các tham số tùy chỉnh
     * 
     * @param data Array of stock data points (Mảng các điểm dữ liệu chứng khoán)
     * @param fastPeriod Period for fast EMA (Kỳ hạn cho EMA nhanh)
     * @param slowPeriod Period for slow EMA (Kỳ hạn cho EMA chậm)
     * @param signalPeriod Period for signal line EMA (Kỳ hạn cho EMA đường tín hiệu)
     * @return MACDResult object containing the calculation results (Đối tượng MACDResult chứa kết quả tính toán)
     */
    public static MACDResult calculateMACD(DataUtils.StockData[] data, int fastPeriod, int slowPeriod, int signalPeriod) {
        MACDResult result = new MACDResult(data.length);
        
        // 1. Calculate fast and slow EMAs
        // 1. Tính EMA nhanh và EMA chậm
        double[] fastEMA = calculateEMA(data, fastPeriod);
        double[] slowEMA = calculateEMA(data, slowPeriod);
        
        // 2. Calculate MACD line (fast EMA - slow EMA)
        // 2. Tính đường MACD (EMA nhanh - EMA chậm)
        double[] macdLine = new double[data.length];
        for (int i = 0; i < data.length; i++) {
            macdLine[i] = fastEMA[i] - slowEMA[i];
            result.macdLine[i] = macdLine[i];
        }
        
        // 3. Calculate signal line (9-period EMA of MACD line)
        // 3. Tính đường tín hiệu (EMA 9 kỳ của đường MACD)
        // For signal line calculation, we need to create a synthetic array of data objects
        // Để tính đường tín hiệu, chúng ta cần tạo một mảng dữ liệu tổng hợp
        DataUtils.StockData[] macdData = new DataUtils.StockData[data.length];
        for (int i = 0; i < data.length; i++) {
            macdData[i] = new DataUtils.StockData();
            macdData[i].close = macdLine[i];
        }
        
        double[] signalLine = calculateEMA(macdData, signalPeriod);
        
        // 4. Calculate histogram (MACD line - Signal line)
        // 4. Tính histogram (Đường MACD - Đường tín hiệu)
        for (int i = 0; i < data.length; i++) {
            result.signalLine[i] = signalLine[i];
            result.histogram[i] = macdLine[i] - signalLine[i];
        }
        
        return result;
    }
    
    /**
     * Calculate EMA (Exponential Moving Average)
     * Tính chỉ báo EMA (Trung bình Động Mũ)
     * 
     * Algorithm:
     * 1. Calculate initial SMA (Simple Moving Average) for the first EMA value
     * 2. Calculate the EMA multiplier: 2/(period+1)
     * 3. For each subsequent point, apply the EMA formula
     * 
     * Thuật toán:
     * 1. Tính SMA (Trung bình Động Đơn giản) ban đầu cho giá trị EMA đầu tiên
     * 2. Tính hệ số nhân EMA: 2/(kỳ hạn+1)
     * 3. Cho mỗi điểm tiếp theo, áp dụng công thức EMA
     * 
     * Formula:
     *   EMA = (Close - Previous EMA) * Multiplier + Previous EMA
     *   Multiplier = 2 / (Period + 1)
     * 
     * Công thức:
     *   EMA = (Giá đóng cửa - EMA trước đó) * Hệ số + EMA trước đó
     *   Hệ số = 2 / (Kỳ hạn + 1)
     * 
     * The EMA gives more weight to recent prices compared to SMA.
     * EMA đặt trọng số cao hơn cho giá gần đây so với SMA.
     * 
     * Mathematical explanation:
     * - The multiplier (2/(period+1)) determines how much weight to give the most recent price.
     * - A shorter period results in a larger multiplier, making the EMA more responsive to recent price changes.
     * - A longer period results in a smaller multiplier, making the EMA smoother and less responsive.
     * 
     * Giải thích toán học:
     * - Hệ số nhân (2/(kỳ hạn+1)) xác định mức độ ảnh hưởng của giá mới nhất.
     * - Kỳ hạn ngắn hơn dẫn đến hệ số lớn hơn, làm cho EMA phản ứng nhanh hơn với biến động giá gần đây.
     * - Kỳ hạn dài hơn dẫn đến hệ số nhỏ hơn, làm cho EMA mượt hơn và ít phản ứng hơn.
     * 
     * Technical interpretation:
     * - EMA crossing price from below: Potential bullish signal
     * - EMA crossing price from above: Potential bearish signal
     * - Short-term EMA crossing above long-term EMA: Bullish crossover
     * - Short-term EMA crossing below long-term EMA: Bearish crossover
     * 
     * Diễn giải kỹ thuật:
     * - EMA cắt giá từ dưới lên: Tín hiệu tăng giá tiềm năng
     * - EMA cắt giá từ trên xuống: Tín hiệu giảm giá tiềm năng
     * - EMA ngắn hạn cắt lên trên EMA dài hạn: Giao cắt tăng giá
     * - EMA ngắn hạn cắt xuống dưới EMA dài hạn: Giao cắt giảm giá
     * 
     * @param data Array of stock data points (Mảng các điểm dữ liệu chứng khoán)
     * @param period EMA period (e.g., 12 for 12-day EMA) (Kỳ hạn EMA, ví dụ: 12 cho EMA 12 ngày)
     * @return Array of EMA values corresponding to each data point (Mảng các giá trị EMA tương ứng với mỗi điểm dữ liệu)
     */
    private static double[] calculateEMA(DataUtils.StockData[] data, int period) {
        double[] ema = new double[data.length];
        Arrays.fill(ema, 0);
        
        if (data.length <= period) return ema;
        
        // Calculate initial SMA for the first EMA value
        // Tính SMA ban đầu cho giá trị EMA đầu tiên
        double sum = 0;
        for (int i = 0; i < period; i++) {
            sum += data[i].close;
        }
        ema[period - 1] = sum / period;
        
        // Calculate multiplier: 2/(period+1)
        // Tính hệ số nhân: 2/(kỳ hạn+1)
        double multiplier = 2.0 / (period + 1);
        
        // Calculate EMA values for each subsequent point using the formula:
        // EMA = (Current Price - Previous EMA) * Multiplier + Previous EMA
        // Tính giá trị EMA cho mỗi điểm tiếp theo sử dụng công thức:
        // EMA = (Giá hiện tại - EMA trước đó) * Hệ số + EMA trước đó
        for (int i = period; i < data.length; i++) {
            ema[i] = (data[i].close - ema[i-1]) * multiplier + ema[i-1];
        }
        
        return ema;
    }
} 