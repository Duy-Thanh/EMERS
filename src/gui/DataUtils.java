package gui; // Package declaration (Khai báo gói)

/**
 * DataUtils - Shared data structures for stock analysis
 * DataUtils - Các cấu trúc dữ liệu được chia sẻ cho phân tích chứng khoán
 * 
 * This class contains common data structures used across different components
 * of the application to prevent duplicate class definitions and compatibility issues.
 * 
 * Lớp này chứa các cấu trúc dữ liệu phổ biến được sử dụng trong các thành phần
 * khác nhau của ứng dụng để ngăn chặn định nghĩa lớp trùng lặp và vấn đề tương thích.
 */
public class DataUtils { // Class definition (Định nghĩa lớp)
    
    /**
     * StockData - Represents a single data point of stock market data
     * StockData - Đại diện cho một điểm dữ liệu của dữ liệu thị trường chứng khoán
     */
    public static class StockData { // Inner class definition for stock data (Định nghĩa lớp con cho dữ liệu chứng khoán)
        public String date; // Date of the stock data point (Ngày của điểm dữ liệu chứng khoán)
        public double open; // Opening price (Giá mở cửa)
        public double high; // Highest price during the period (Giá cao nhất trong kỳ)
        public double low;  // Lowest price during the period (Giá thấp nhất trong kỳ)
        public double close; // Closing price (Giá đóng cửa)
        public double volume; // Trading volume (Khối lượng giao dịch)
        public double adjClose; // Adjusted closing price (Giá đóng cửa đã điều chỉnh)
        
        // Default constructor (Hàm tạo mặc định)
        public StockData() {
        }
        
        // Copy constructor from GUI StockData (Hàm tạo sao chép từ GUI StockData)
        public static StockData fromGUIStockData(Object guiStockData) {
            try { // Try block for exception handling (Khối try để xử lý ngoại lệ)
                // Use reflection to handle various StockData classes (Sử dụng phản chiếu để xử lý các lớp StockData khác nhau)
                StockData result = new StockData(); // Create new StockData object (Tạo đối tượng StockData mới)
                java.lang.reflect.Field dateField = guiStockData.getClass().getDeclaredField("date"); // Get date field (Lấy trường date)
                dateField.setAccessible(true); // Make private field accessible (Làm cho trường private có thể truy cập)
                result.date = (String)dateField.get(guiStockData); // Get date value (Lấy giá trị date)
                
                java.lang.reflect.Field openField = guiStockData.getClass().getDeclaredField("open"); // Get open field (Lấy trường open)
                openField.setAccessible(true); // Make private field accessible (Làm cho trường private có thể truy cập)
                result.open = (Double)openField.get(guiStockData); // Get open value (Lấy giá trị open)
                
                java.lang.reflect.Field highField = guiStockData.getClass().getDeclaredField("high"); // Get high field (Lấy trường high)
                highField.setAccessible(true); // Make private field accessible (Làm cho trường private có thể truy cập)
                result.high = (Double)highField.get(guiStockData); // Get high value (Lấy giá trị high)
                
                java.lang.reflect.Field lowField = guiStockData.getClass().getDeclaredField("low"); // Get low field (Lấy trường low)
                lowField.setAccessible(true); // Make private field accessible (Làm cho trường private có thể truy cập)
                result.low = (Double)lowField.get(guiStockData); // Get low value (Lấy giá trị low)
                
                java.lang.reflect.Field closeField = guiStockData.getClass().getDeclaredField("close"); // Get close field (Lấy trường close)
                closeField.setAccessible(true); // Make private field accessible (Làm cho trường private có thể truy cập)
                result.close = (Double)closeField.get(guiStockData); // Get close value (Lấy giá trị close)
                
                java.lang.reflect.Field volumeField = guiStockData.getClass().getDeclaredField("volume"); // Get volume field (Lấy trường volume)
                volumeField.setAccessible(true); // Make private field accessible (Làm cho trường private có thể truy cập)
                result.volume = (Double)volumeField.get(guiStockData); // Get volume value (Lấy giá trị volume)
                
                // Handle adjClose which might not exist in all StockData classes (Xử lý adjClose có thể không tồn tại trong tất cả các lớp StockData)
                try {
                    java.lang.reflect.Field adjCloseField = guiStockData.getClass().getDeclaredField("adjClose"); // Get adjClose field (Lấy trường adjClose)
                    adjCloseField.setAccessible(true); // Make private field accessible (Làm cho trường private có thể truy cập)
                    result.adjClose = (Double)adjCloseField.get(guiStockData); // Get adjClose value (Lấy giá trị adjClose)
                } catch (NoSuchFieldException e) { // Catch block for NoSuchFieldException (Khối catch cho NoSuchFieldException)
                    // If adjClose doesn't exist, use regular close (Nếu adjClose không tồn tại, sử dụng close thông thường)
                    result.adjClose = result.close; // Set adjClose to close (Đặt adjClose bằng close)
                }
                
                return result; // Return the created StockData object (Trả về đối tượng StockData đã tạo)
            } catch (Exception e) { // Catch block for any exception (Khối catch cho bất kỳ ngoại lệ nào)
                System.err.println("Error converting StockData: " + e.getMessage()); // Print error message (In thông báo lỗi)
                e.printStackTrace(); // Print stack trace (In dấu vết ngăn xếp)
                return null; // Return null on error (Trả về null khi có lỗi)
            }
        }
        
        /**
         * Create an array of StockData from an array of GUI StockData objects
         * Tạo một mảng StockData từ một mảng đối tượng GUI StockData
         */
        public static StockData[] fromGUIStockDataArray(Object[] guiStockDataArray) {
            StockData[] result = new StockData[guiStockDataArray.length]; // Create new array with same length (Tạo mảng mới có cùng độ dài)
            for (int i = 0; i < guiStockDataArray.length; i++) { // Loop through the input array (Lặp qua mảng đầu vào)
                result[i] = fromGUIStockData(guiStockDataArray[i]); // Convert each element (Chuyển đổi từng phần tử)
            }
            return result; // Return the created array (Trả về mảng đã tạo)
        }
    }
    
    /**
     * PatternResult - Represents a detected price pattern
     * PatternResult - Đại diện cho một mẫu giá được phát hiện
     */
    public static class PatternResult { // Inner class definition for pattern results (Định nghĩa lớp con cho kết quả mẫu)
        public int type;              // Pattern type (from constants) (Loại mẫu (từ hằng số))
        public int startIndex;        // Start index in the data array (Chỉ mục bắt đầu trong mảng dữ liệu)
        public int endIndex;          // End index in the data array (Chỉ mục kết thúc trong mảng dữ liệu)
        public double confidence;     // Confidence level (0.0-1.0) (Mức độ tin cậy (0.0-1.0))
        public double expectedMove;   // Expected price move (% change) (Sự thay đổi giá dự kiến (% thay đổi))
        public String description;    // Human-readable description (Mô tả dễ đọc cho con người)
    }
    
    /**
     * TradingSignal - Represents a trading signal
     * TradingSignal - Đại diện cho tín hiệu giao dịch
     */
    public static class TradingSignal { // Inner class definition for trading signals (Định nghĩa lớp con cho tín hiệu giao dịch)
        public String type;           // Signal type (BUY, SELL, etc.) (Loại tín hiệu (MUA, BÁN, v.v.))
        public int signalIndex;       // Index in data where signal occurs (Chỉ mục trong dữ liệu nơi tín hiệu xuất hiện)
        public double confidence;     // Confidence level (0.0-1.0) (Mức độ tin cậy (0.0-1.0))
        public double entryPrice;     // Suggested entry price (Giá vào đề xuất)
        public double targetPrice;    // Target price for take profit (Giá mục tiêu để chốt lời)
        public double stopLossPrice;  // Suggested stop loss price (Giá dừng lỗ đề xuất)
        public double riskRewardRatio;// Risk/reward ratio (Tỷ lệ rủi ro/phần thưởng)
        public String description;    // Signal description (Mô tả tín hiệu)
    }
    
    /**
     * AnomalyResult - Represents an anomaly in stock data
     * AnomalyResult - Đại diện cho một bất thường trong dữ liệu chứng khoán
     */
    public static class AnomalyResult { // Inner class definition for anomaly results (Định nghĩa lớp con cho kết quả bất thường)
        public int index;             // Index in data where anomaly occurs (Chỉ mục trong dữ liệu nơi bất thường xuất hiện)
        public double score;          // Anomaly score (higher is more anomalous) (Điểm bất thường (cao hơn là bất thường hơn))
        public double priceDeviation; // Price deviation in standard deviations (Độ lệch giá tính bằng độ lệch chuẩn)
        public double volumeDeviation;// Volume deviation in standard deviations (Độ lệch khối lượng tính bằng độ lệch chuẩn)
        public String description;    // Human-readable description (Mô tả dễ đọc cho con người)
    }
} 