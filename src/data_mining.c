/**
 * Data Mining Module
 * Implementation of pattern recognition and data mining functions
 * FOCUSED VERSION - Data Preprocessing Only
 * 
 * Mô-đun Khai thác Dữ liệu
 * Triển khai các chức năng nhận dạng mẫu và khai thác dữ liệu
 * PHIÊN BẢN TẬP TRUNG - Chỉ Tiền xử lý Dữ liệu
 */

#include <stdio.h>      // Include standard input/output library (Thêm thư viện nhập/xuất chuẩn)
#include <stdlib.h>     // Include standard library for memory allocation (Thêm thư viện chuẩn cho phân bổ bộ nhớ)
#include <string.h>     // Include string manipulation library (Thêm thư viện xử lý chuỗi)
#include <math.h>       // Include math functions library (Thêm thư viện hàm toán học)
#include <time.h>       // Include time functions library (Thêm thư viện hàm thời gian)

#include "../include/emers.h"             // Include Emergency Market Event Response System header (Thêm header hệ thống phản ứng sự kiện thị trường khẩn cấp)
#include "../include/data_mining.h"       // Include data mining function declarations (Thêm khai báo hàm khai thác dữ liệu)
#include "../include/technical_analysis.h" // Include technical analysis functions (Thêm các hàm phân tích kỹ thuật)
#include "../include/error_handling.h"    // Include error handling utilities (Thêm tiện ích xử lý lỗi)
#include <float.h>      // Include floating point limits (Thêm giới hạn số thực dấu phẩy động)

/* Data Preprocessing Functions */
/* Các hàm tiền xử lý dữ liệu */

/**
 * Normalize price data using min-max scaling
 * 
 * Algorithm:
 * 1. Find min and max values for each dimension (open, high, low, close, volume)
 * 2. Apply min-max normalization formula: normalized = (value - min) / (max - min)
 * 3. This scales all values to [0,1] range
 * 4. Handle special case when min=max (constant data) by setting to 0.5
 * 
 * Formula:
 *   normalized = (value - min) / (max - min)
 * 
 * @param data Input stock data array containing raw price values
 * @param dataSize Number of data points in the array
 * @param normalizedData Output array for normalized data (must be pre-allocated)
 * @return 0 on success, negative error code on failure
 * 
 * Chuẩn hóa dữ liệu giá sử dụng phương pháp tỷ lệ min-max
 * 
 * Thuật toán:
 * 1. Tìm giá trị nhỏ nhất và lớn nhất cho mỗi chiều (mở, cao, thấp, đóng, khối lượng)
 * 2. Áp dụng công thức chuẩn hóa min-max: chuẩn hóa = (giá trị - nhỏ nhất) / (lớn nhất - nhỏ nhất)
 * 3. Điều này đưa tất cả giá trị về phạm vi [0,1]
 * 4. Xử lý trường hợp đặc biệt khi min=max (dữ liệu không đổi) bằng cách đặt thành 0.5
 * 
 * Công thức:
 *   chuẩn hóa = (giá trị - nhỏ nhất) / (lớn nhất - nhỏ nhất)
 * 
 * @param data Mảng dữ liệu chứng khoán đầu vào chứa giá trị giá thô
 * @param dataSize Số điểm dữ liệu trong mảng
 * @param normalizedData Mảng đầu ra cho dữ liệu đã chuẩn hóa (phải được cấp phát trước)
 * @return 0 khi thành công, mã lỗi âm khi thất bại
 */
int normalizeStockData(const StockData* data, int dataSize, StockData* normalizedData) {
    // Validate input parameters (Xác thực các tham số đầu vào)
    if (!data || !normalizedData || dataSize <= 0) {
        return ERR_INVALID_PARAMETER; // Return error for invalid parameters (Trả về lỗi cho tham số không hợp lệ)
    }
    
    /* Find min and max values for each dimension */
    /* Tìm giá trị nhỏ nhất và lớn nhất cho mỗi chiều */
    double minOpen = data[0].open;    // Initialize minimum open price with first element (Khởi tạo giá mở tối thiểu với phần tử đầu tiên)
    double maxOpen = data[0].open;    // Initialize maximum open price with first element (Khởi tạo giá mở tối đa với phần tử đầu tiên)
    double minHigh = data[0].high;    // Initialize minimum high price with first element (Khởi tạo giá cao tối thiểu với phần tử đầu tiên)
    double maxHigh = data[0].high;    // Initialize maximum high price with first element (Khởi tạo giá cao tối đa với phần tử đầu tiên)
    double minLow = data[0].low;      // Initialize minimum low price with first element (Khởi tạo giá thấp tối thiểu với phần tử đầu tiên)
    double maxLow = data[0].low;      // Initialize maximum low price with first element (Khởi tạo giá thấp tối đa với phần tử đầu tiên)
    double minClose = data[0].close;  // Initialize minimum close price with first element (Khởi tạo giá đóng tối thiểu với phần tử đầu tiên)
    double maxClose = data[0].close;  // Initialize maximum close price with first element (Khởi tạo giá đóng tối đa với phần tử đầu tiên)
    double minVolume = data[0].volume; // Initialize minimum volume with first element (Khởi tạo khối lượng tối thiểu với phần tử đầu tiên)
    double maxVolume = data[0].volume; // Initialize maximum volume with first element (Khởi tạo khối lượng tối đa với phần tử đầu tiên)
    
    // Loop through data to find min and max values (Lặp qua dữ liệu để tìm giá trị nhỏ nhất và lớn nhất)
    for (int i = 1; i < dataSize; i++) {
        /* Update min values */
        /* Cập nhật giá trị nhỏ nhất */
        if (data[i].open < minOpen) minOpen = data[i].open;       // Update minimum open if found smaller value (Cập nhật giá mở tối thiểu nếu tìm thấy giá trị nhỏ hơn)
        if (data[i].high < minHigh) minHigh = data[i].high;       // Update minimum high if found smaller value (Cập nhật giá cao tối thiểu nếu tìm thấy giá trị nhỏ hơn)
        if (data[i].low < minLow) minLow = data[i].low;           // Update minimum low if found smaller value (Cập nhật giá thấp tối thiểu nếu tìm thấy giá trị nhỏ hơn)
        if (data[i].close < minClose) minClose = data[i].close;   // Update minimum close if found smaller value (Cập nhật giá đóng tối thiểu nếu tìm thấy giá trị nhỏ hơn)
        if (data[i].volume < minVolume) minVolume = data[i].volume; // Update minimum volume if found smaller value (Cập nhật khối lượng tối thiểu nếu tìm thấy giá trị nhỏ hơn)
        
        /* Update max values */
        /* Cập nhật giá trị lớn nhất */
        if (data[i].open > maxOpen) maxOpen = data[i].open;       // Update maximum open if found larger value (Cập nhật giá mở tối đa nếu tìm thấy giá trị lớn hơn)
        if (data[i].high > maxHigh) maxHigh = data[i].high;       // Update maximum high if found larger value (Cập nhật giá cao tối đa nếu tìm thấy giá trị lớn hơn)
        if (data[i].low > maxLow) maxLow = data[i].low;           // Update maximum low if found larger value (Cập nhật giá thấp tối đa nếu tìm thấy giá trị lớn hơn)
        if (data[i].close > maxClose) maxClose = data[i].close;   // Update maximum close if found larger value (Cập nhật giá đóng tối đa nếu tìm thấy giá trị lớn hơn)
        if (data[i].volume > maxVolume) maxVolume = data[i].volume; // Update maximum volume if found larger value (Cập nhật khối lượng tối đa nếu tìm thấy giá trị lớn hơn)
    }
    
    /* Perform min-max normalization to [0,1] range */
    for (int i = 0; i < dataSize; i++) {
        /* Copy date and other fields */
        strcpy(normalizedData[i].date, data[i].date);
        normalizedData[i].adjClose = data[i].adjClose;
        
        /* Normalize price data */
        if (maxOpen != minOpen) {
            normalizedData[i].open = (data[i].open - minOpen) / (maxOpen - minOpen);
        } else {
            normalizedData[i].open = 0.5; /* Default for constant data */
        }
        
        if (maxHigh != minHigh) {
            normalizedData[i].high = (data[i].high - minHigh) / (maxHigh - minHigh);
        } else {
            normalizedData[i].high = 0.5;
        }
        
        if (maxLow != minLow) {
            normalizedData[i].low = (data[i].low - minLow) / (maxLow - minLow);
        } else {
            normalizedData[i].low = 0.5;
        }
        
        if (maxClose != minClose) {
            normalizedData[i].close = (data[i].close - minClose) / (maxClose - minClose);
        } else {
            normalizedData[i].close = 0.5;
        }
        
        if (maxVolume != minVolume) {
            normalizedData[i].volume = (data[i].volume - minVolume) / (maxVolume - minVolume);
        } else {
            normalizedData[i].volume = 0.5;
        }
    }
    
    return 0;
}

/**
 * Remove outliers using z-score method
 * 
 * Algorithm:
 * 1. Calculate mean for each dimension (open, high, low, close, volume)
 * 2. Calculate standard deviation for each dimension
 * 3. Calculate z-score for each data point: z = |value - mean| / stdDev
 * 4. Replace values where z-score > threshold with the mean
 * 
 * Formula:
 *   z-score = |value - mean| / stdDev
 *   mean = sum(values) / n
 *   stdDev = sqrt(sum((value - mean)²) / n)
 * 
 * @param data Input/output stock data array (outliers will be replaced in-place)
 * @param dataSize Number of data points
 * @param threshold Z-score threshold for outlier detection (typically 3.0)
 * @return Number of outliers detected and fixed, or negative error code on failure
 */
int removeOutliers(StockData* data, int dataSize, double threshold) {
    if (!data || dataSize <= 0 || threshold <= 0) {
        return ERR_INVALID_PARAMETER;
    }
    
    int outlierCount = 0;
    
    /* Loop through all data points to sum price values */
    /* Lặp qua tất cả các điểm dữ liệu để tính tổng giá trị giá */
    double sumOpen = 0, sumHigh = 0, sumLow = 0, sumClose = 0, sumVolume = 0;
    
    for (int i = 0; i < dataSize; i++) {
        sumOpen += data[i].open;      // Add current open price to sum (Cộng giá mở hiện tại vào tổng)
        sumHigh += data[i].high;      // Add current high price to sum (Cộng giá cao hiện tại vào tổng)
        sumLow += data[i].low;        // Add current low price to sum (Cộng giá thấp hiện tại vào tổng)
        sumClose += data[i].close;    // Add current close price to sum (Cộng giá đóng hiện tại vào tổng)
        sumVolume += data[i].volume;  // Add current volume to sum (Cộng khối lượng hiện tại vào tổng)
    }
    
    /* Calculate mean (average) values for each price dimension */
    /* Tính giá trị trung bình cho mỗi chiều giá */
    double meanOpen = sumOpen / dataSize;     // Average open price (Giá mở trung bình)
    double meanHigh = sumHigh / dataSize;     // Average high price (Giá cao trung bình)
    double meanLow = sumLow / dataSize;       // Average low price (Giá thấp trung bình)
    double meanClose = sumClose / dataSize;   // Average close price (Giá đóng trung bình)
    double meanVolume = sumVolume / dataSize; // Average volume (Khối lượng trung bình)
    
    /* Calculate standard deviations for each price dimension */
    /* Tính độ lệch chuẩn cho mỗi chiều giá */
    double sumSqOpen = 0, sumSqHigh = 0, sumSqLow = 0, sumSqClose = 0, sumSqVolume = 0; // Initialize sum of squared differences (Khởi tạo tổng bình phương độ lệch)
    
    /* Loop through data to calculate squared differences from mean */
    /* Lặp qua dữ liệu để tính các độ lệch bình phương từ trung bình */
    for (int i = 0; i < dataSize; i++) {
        /* Calculate differences between each value and its mean */
        /* Tính độ lệch giữa mỗi giá trị và giá trị trung bình của nó */
        double diffOpen = data[i].open - meanOpen;       // Difference from mean open (Độ lệch từ giá mở trung bình)
        double diffHigh = data[i].high - meanHigh;       // Difference from mean high (Độ lệch từ giá cao trung bình)
        double diffLow = data[i].low - meanLow;          // Difference from mean low (Độ lệch từ giá thấp trung bình)
        double diffClose = data[i].close - meanClose;    // Difference from mean close (Độ lệch từ giá đóng trung bình)
        double diffVolume = data[i].volume - meanVolume; // Difference from mean volume (Độ lệch từ khối lượng trung bình)
        
        /* Sum the squared differences for variance calculation */
        /* Tính tổng bình phương độ lệch để tính phương sai */
        sumSqOpen += diffOpen * diffOpen;       // Sum squared differences for open (Tổng bình phương độ lệch cho giá mở)
        sumSqHigh += diffHigh * diffHigh;       // Sum squared differences for high (Tổng bình phương độ lệch cho giá cao)
        sumSqLow += diffLow * diffLow;          // Sum squared differences for low (Tổng bình phương độ lệch cho giá thấp)
        sumSqClose += diffClose * diffClose;    // Sum squared differences for close (Tổng bình phương độ lệch cho giá đóng)
        sumSqVolume += diffVolume * diffVolume; // Sum squared differences for volume (Tổng bình phương độ lệch cho khối lượng)
    }
    
    /* Calculate standard deviation as square root of the average squared differences */
    /* Tính độ lệch chuẩn là căn bậc hai của trung bình bình phương độ lệch */
    double stdOpen = sqrt(sumSqOpen / dataSize);     // Standard deviation of open prices (Độ lệch chuẩn của giá mở)
    double stdHigh = sqrt(sumSqHigh / dataSize);     // Standard deviation of high prices (Độ lệch chuẩn của giá cao)
    double stdLow = sqrt(sumSqLow / dataSize);       // Standard deviation of low prices (Độ lệch chuẩn của giá thấp)
    double stdClose = sqrt(sumSqClose / dataSize);   // Standard deviation of close prices (Độ lệch chuẩn của giá đóng)
    double stdVolume = sqrt(sumSqVolume / dataSize); // Standard deviation of volume (Độ lệch chuẩn của khối lượng)
    
    /* Detect and replace outliers by checking each data point */
    /* Phát hiện và thay thế các giá trị ngoại lai bằng cách kiểm tra từng điểm dữ liệu */
    for (int i = 0; i < dataSize; i++) {
        /* Calculate z-scores for each price dimension */
        /* Tính điểm z-score cho mỗi chiều giá */
        /* Z-score measures how many standard deviations a value is from the mean */
        /* Z-score đo lường một giá trị cách xa bao nhiêu độ lệch chuẩn so với giá trị trung bình */
        
        /* Use absolute value to detect both positive and negative outliers */
        /* Sử dụng giá trị tuyệt đối để phát hiện cả ngoại lai dương và âm */
        
        /* Check for division by zero - if stdDev is 0, z-score is set to 0 */
        /* Kiểm tra phép chia cho 0 - nếu độ lệch chuẩn là 0, z-score được đặt thành 0 */
        double zOpen = stdOpen > 0 ? fabs((data[i].open - meanOpen) / stdOpen) : 0;       // Z-score for open price (Z-score cho giá mở)
        double zHigh = stdHigh > 0 ? fabs((data[i].high - meanHigh) / stdHigh) : 0;       // Z-score for high price (Z-score cho giá cao)
        double zLow = stdLow > 0 ? fabs((data[i].low - meanLow) / stdLow) : 0;            // Z-score for low price (Z-score cho giá thấp)
        double zClose = stdClose > 0 ? fabs((data[i].close - meanClose) / stdClose) : 0;   // Z-score for close price (Z-score cho giá đóng)
        double zVolume = stdVolume > 0 ? fabs((data[i].volume - meanVolume) / stdVolume) : 0; // Z-score for volume (Z-score cho khối lượng)
        
        /* Fix outliers by replacing with mean if z-score exceeds threshold */
        /* Sửa các giá trị ngoại lai bằng cách thay thế bằng giá trị trung bình nếu z-score vượt quá ngưỡng */
        if (zOpen > threshold) {
            data[i].open = meanOpen;   // Replace outlier open price with mean (Thay thế giá mở ngoại lai bằng giá trị trung bình)
            outlierCount++;            // Increment outlier counter (Tăng bộ đếm ngoại lai)
        }
        
        if (zHigh > threshold) {
            data[i].high = meanHigh;   // Replace outlier high price with mean (Thay thế giá cao ngoại lai bằng giá trị trung bình)
            outlierCount++;            // Increment outlier counter (Tăng bộ đếm ngoại lai)
        }
        
        if (zLow > threshold) {
            data[i].low = meanLow;     // Replace outlier low price with mean (Thay thế giá thấp ngoại lai bằng giá trị trung bình)
            outlierCount++;            // Increment outlier counter (Tăng bộ đếm ngoại lai)
        }
        
        if (zClose > threshold) {
            data[i].close = meanClose; // Replace outlier close price with mean (Thay thế giá đóng ngoại lai bằng giá trị trung bình)
            outlierCount++;            // Increment outlier counter (Tăng bộ đếm ngoại lai)
        }
        
        if (zVolume > threshold) {
            data[i].volume = meanVolume; // Replace outlier volume with mean (Thay thế khối lượng ngoại lai bằng giá trị trung bình)
            outlierCount++;              // Increment outlier counter (Tăng bộ đếm ngoại lai)
        }
    }
    
    return outlierCount;
}

/**
 * Fill missing data in stock data array using linear interpolation
 * 
 * Algorithm:
 * 1. Scan array for zero values (assumed to be missing)
 * 2. For each missing value, find nearest non-zero values before and after
 * 3. Apply linear interpolation based on position
 * 
 * Formula:
 *   interpolated = prev + weight * (next - prev)
 *   weight = (current_idx - prev_idx) / (next_idx - prev_idx)
 * 
 * @param data Input/output stock data array (missing values will be filled in-place)
 * @param dataSize Number of data points
 * @return Number of missing values filled, or negative error code on failure
 */
int fillMissingData(StockData* data, int dataSize) {
    /* Validate input parameters to prevent errors */
    /* Xác thực các tham số đầu vào để tránh lỗi */
    if (!data || dataSize <= 0) {
        return ERR_INVALID_PARAMETER; // Return error code for invalid parameters (Trả về mã lỗi cho tham số không hợp lệ)
    }
    
    int filledCount = 0; // Counter for number of filled values (Bộ đếm cho số giá trị đã được điền)
    
    /* Check for zero values (assumed missing) and interpolate */
    /* Kiểm tra các giá trị bằng 0 (được coi là thiếu) và nội suy */
    for (int i = 0; i < dataSize; i++) {
        /* Skip first and last points for simplicity */
        /* Bỏ qua các điểm đầu tiên và cuối cùng để đơn giản hóa */
        if (i == 0 || i == dataSize - 1) {
            continue; // Skip first and last data points (Bỏ qua điểm dữ liệu đầu tiên và cuối cùng)
        }
        
        /* Open price - process missing open price data */
        /* Giá mở - xử lý dữ liệu giá mở bị thiếu */
        if (data[i].open == 0.0) {
            /* Find previous non-zero value - look backwards */
            /* Tìm giá trị khác 0 trước đó - tìm ngược lại */
            int prev = i - 1; // Start with previous index (Bắt đầu với chỉ số trước đó)
            while (prev >= 0 && data[prev].open == 0.0) {
                prev--; // Keep moving backwards until non-zero value found (Tiếp tục lùi cho đến khi tìm thấy giá trị khác 0)
            }
            
            /* Find next non-zero value - look forwards */
            /* Tìm giá trị khác 0 tiếp theo - tìm tới trước */
            int next = i + 1; // Start with next index (Bắt đầu với chỉ số tiếp theo)
            while (next < dataSize && data[next].open == 0.0) {
                next++; // Keep moving forwards until non-zero value found (Tiếp tục tiến cho đến khi tìm thấy giá trị khác 0)
            }
            
            /* Interpolate if both bounds found - can only interpolate if we have valid values on both sides */
            /* Nội suy nếu tìm thấy cả hai giới hạn - chỉ có thể nội suy nếu có giá trị hợp lệ ở cả hai bên */
            if (prev >= 0 && next < dataSize) {
                /* Calculate weight based on position between prev and next */
                /* Tính trọng số dựa trên vị trí giữa prev và next */
                double weight = (double)(i - prev) / (next - prev); // Position ratio (0 to 1) (Tỷ lệ vị trí (0 đến 1))
                /* Linear interpolation formula: start + weight * (end - start) */
                /* Công thức nội suy tuyến tính: giá trị đầu + trọng số * (giá trị cuối - giá trị đầu) */
                data[i].open = data[prev].open + weight * (data[next].open - data[prev].open); // Interpolated value (Giá trị được nội suy)
                filledCount++; // Increment filled values counter (Tăng bộ đếm giá trị đã điền)
            }
        }
        
        /* High price - process missing high price data */
        /* Giá cao - xử lý dữ liệu giá cao bị thiếu */
        if (data[i].high == 0.0) {
            /* Similar interpolation logic for high */
            /* Logic nội suy tương tự cho giá cao */
            int prev = i - 1; // Start with previous index (Bắt đầu với chỉ số trước đó)
            while (prev >= 0 && data[prev].high == 0.0) {
                prev--; // Keep moving backwards until non-zero value found (Tiếp tục lùi cho đến khi tìm thấy giá trị khác 0)
            }
            
            int next = i + 1; // Start with next index (Bắt đầu với chỉ số tiếp theo)
            while (next < dataSize && data[next].high == 0.0) {
                next++; // Keep moving forwards until non-zero value found (Tiếp tục tiến cho đến khi tìm thấy giá trị khác 0)
            }
            
            if (prev >= 0 && next < dataSize) {
                double weight = (double)(i - prev) / (next - prev); // Position ratio (0 to 1) (Tỷ lệ vị trí (0 đến 1))
                data[i].high = data[prev].high + weight * (data[next].high - data[prev].high); // Apply linear interpolation (Áp dụng nội suy tuyến tính)
                filledCount++; // Increment count of filled data points (Tăng số lượng điểm dữ liệu đã điền)
            }
        }
        
        /* Low price - process missing low price data */
        /* Giá thấp - xử lý dữ liệu giá thấp bị thiếu */
        if (data[i].low == 0.0) {
            /* Similar interpolation logic for low */
            /* Logic nội suy tương tự cho giá thấp */
            int prev = i - 1; // Start with previous index (Bắt đầu với chỉ số trước đó)
            while (prev >= 0 && data[prev].low == 0.0) {
                prev--; // Keep moving backwards until non-zero value found (Tiếp tục lùi cho đến khi tìm thấy giá trị khác 0)
            }
            
            int next = i + 1; // Start with next index (Bắt đầu với chỉ số tiếp theo)
            while (next < dataSize && data[next].low == 0.0) {
                next++; // Keep moving forwards until non-zero value found (Tiếp tục tiến cho đến khi tìm thấy giá trị khác 0)
            }
            
            if (prev >= 0 && next < dataSize) {
                double weight = (double)(i - prev) / (next - prev); // Position ratio (0 to 1) (Tỷ lệ vị trí (0 đến 1))
                data[i].low = data[prev].low + weight * (data[next].low - data[prev].low); // Apply linear interpolation (Áp dụng nội suy tuyến tính)
                filledCount++; // Increment count of filled data points (Tăng số lượng điểm dữ liệu đã điền)
            }
        }
        
        /* Close price - process missing close price data */
        /* Giá đóng - xử lý dữ liệu giá đóng bị thiếu */
        if (data[i].close == 0.0) {
            /* Similar interpolation logic for close */
            /* Logic nội suy tương tự cho giá đóng */
            int prev = i - 1; // Start with previous index (Bắt đầu với chỉ số trước đó)
            while (prev >= 0 && data[prev].close == 0.0) {
                prev--; // Keep moving backwards until non-zero value found (Tiếp tục lùi cho đến khi tìm thấy giá trị khác 0)
            }
            
            int next = i + 1; // Start with next index (Bắt đầu với chỉ số tiếp theo)
            while (next < dataSize && data[next].close == 0.0) {
                next++; // Keep moving forwards until non-zero value found (Tiếp tục tiến cho đến khi tìm thấy giá trị khác 0)
            }
            
            if (prev >= 0 && next < dataSize) {
                double weight = (double)(i - prev) / (next - prev); // Position ratio (0 to 1) (Tỷ lệ vị trí (0 đến 1)) 
                data[i].close = data[prev].close + weight * (data[next].close - data[prev].close); // Apply linear interpolation (Áp dụng nội suy tuyến tính)
                filledCount++; // Increment count of filled data points (Tăng số lượng điểm dữ liệu đã điền)
            }
        }
        
        /* Volume - process missing volume data */
        /* Khối lượng - xử lý dữ liệu khối lượng bị thiếu */
        if (data[i].volume == 0.0) {
            /* Similar interpolation logic for volume */
            /* Logic nội suy tương tự cho khối lượng */
            int prev = i - 1; // Start with previous index (Bắt đầu với chỉ số trước đó)
            while (prev >= 0 && data[prev].volume == 0.0) {
                prev--; // Keep moving backwards until non-zero value found (Tiếp tục lùi cho đến khi tìm thấy giá trị khác 0)
            }
            
            int next = i + 1; // Start with next index (Bắt đầu với chỉ số tiếp theo)
            while (next < dataSize && data[next].volume == 0.0) {
                next++; // Keep moving forwards until non-zero value found (Tiếp tục tiến cho đến khi tìm thấy giá trị khác 0)
            }
            
            if (prev >= 0 && next < dataSize) {
                double weight = (double)(i - prev) / (next - prev); // Position ratio (0 to 1) (Tỷ lệ vị trí (0 đến 1))
                data[i].volume = data[prev].volume + weight * (data[next].volume - data[prev].volume); // Apply linear interpolation (Áp dụng nội suy tuyến tính)
                filledCount++; // Increment count of filled data points (Tăng số lượng điểm dữ liệu đã điền)
            }
        }
    }
    
    /* Return the total number of data points that were filled through interpolation */
    /* Trả về tổng số điểm dữ liệu đã được điền thông qua nội suy */
    return filledCount; // Return number of missing values that were filled (Trả về số giá trị bị thiếu đã được điền)
}

/**
 * Prepare input data for the data mining algorithms - Master function
 * 
 * Algorithm:
 * 1. Create temporary copy of input data
 * 2. Fill missing values using linear interpolation
 * 3. Remove outliers using z-score method (threshold=3.0)
 * 4. Normalize data if requested
 * 
 * The function combines three preprocessing steps:
 * - Missing data imputation
 * - Outlier detection and removal
 * - Optional normalization
 * 
 * @param inputData Input stock data array
 * @param inputSize Number of input data points
 * @param outputData Output prepared data array (must be pre-allocated)
 * @param shouldNormalize Whether to normalize the data (1=yes, 0=no)
 * @return 0 on success, negative error code on failure
 * 
 * Chuẩn bị dữ liệu đầu vào cho các thuật toán khai thác dữ liệu - Hàm chính
 * 
 * Thuật toán:
 * 1. Tạo bản sao tạm thời của dữ liệu đầu vào
 * 2. Điền các giá trị bị thiếu bằng nội suy tuyến tính
 * 3. Loại bỏ các giá trị ngoại lai bằng phương pháp z-score (ngưỡng=3.0)
 * 4. Chuẩn hóa dữ liệu nếu được yêu cầu
 * 
 * Hàm kết hợp ba bước tiền xử lý:
 * - Điền dữ liệu bị thiếu
 * - Phát hiện và loại bỏ các giá trị ngoại lai
 * - Chuẩn hóa tùy chọn
 */
int prepareDataForMining(const StockData* inputData, int inputSize, 
                         StockData* outputData, int shouldNormalize) {
    /* Validate input parameters to prevent errors */
    /* Xác thực các tham số đầu vào để tránh lỗi */
    if (!inputData || !outputData || inputSize <= 0) {
        return ERR_INVALID_PARAMETER; // Return error for invalid parameters (Trả về lỗi cho tham số không hợp lệ)
    }
    
    /* Create a temporary copy of the data for preprocessing */
    /* Tạo một bản sao tạm thời của dữ liệu để tiền xử lý */
    StockData* tmpData = (StockData*)malloc(inputSize * sizeof(StockData)); // Allocate memory for temporary data (Cấp phát bộ nhớ cho dữ liệu tạm thời)
    if (!tmpData) {
        return ERR_MEMORY_ALLOCATION; // Return error if memory allocation fails (Trả về lỗi nếu cấp phát bộ nhớ thất bại)
    }
    
    /* Copy input data to temporary array for safe processing */
    /* Sao chép dữ liệu đầu vào vào mảng tạm thời để xử lý an toàn */
    memcpy(tmpData, inputData, inputSize * sizeof(StockData)); // Copy all data (Sao chép tất cả dữ liệu)
    
    /* Preprocess the data - apply multiple preparation steps */
    /* Tiền xử lý dữ liệu - áp dụng nhiều bước chuẩn bị */
    
    /* Step 1: Fill missing data using linear interpolation */
    /* Bước 1: Điền dữ liệu thiếu bằng phương pháp nội suy tuyến tính */
    int filledCount = fillMissingData(tmpData, inputSize); // Fill missing values (Điền các giá trị bị thiếu)
    (void)filledCount;  /* Prevent unused variable warning (Ngăn cảnh báo biến không được sử dụng) */
    
    /* Step 2: Remove outliers using z-score method with threshold 3.0 */
    /* Bước 2: Loại bỏ ngoại lai sử dụng phương pháp z-score với ngưỡng 3.0 */
    int outlierCount = removeOutliers(tmpData, inputSize, 3.0); // Remove statistical outliers (Loại bỏ các ngoại lai thống kê)
    (void)outlierCount; /* Prevent unused variable warning (Ngăn cảnh báo biến không được sử dụng) */
    
    /* Step 3: Normalize data if requested */
    /* Bước 3: Chuẩn hóa dữ liệu nếu được yêu cầu */
    if (shouldNormalize) {
        normalizeStockData(tmpData, inputSize, outputData); // Normalize to [0,1] range (Chuẩn hóa về phạm vi [0,1])
    } else {
        memcpy(outputData, tmpData, inputSize * sizeof(StockData)); // Copy without normalization (Sao chép mà không chuẩn hóa)
    }
    
    /* Clean up temporary resources to prevent memory leaks */
    /* Giải phóng tài nguyên tạm thời để tránh rò rỉ bộ nhớ */
    free(tmpData); // Free the temporary data array (Giải phóng mảng dữ liệu tạm thời)
    
    return 0; // Return success (Trả về thành công)
}