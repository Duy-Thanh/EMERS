package gui;

import javax.swing.*;
import javax.swing.table.DefaultTableModel;
import javax.swing.table.TableModel;
import javax.swing.table.TableRowSorter;
import java.awt.*;
import java.awt.event.*;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import org.jfree.chart.ChartFactory;
import org.jfree.chart.ChartPanel;
import org.jfree.chart.JFreeChart;
import org.jfree.chart.plot.PlotOrientation;
import org.jfree.data.category.DefaultCategoryDataset;
import org.jfree.data.time.Day;
import org.jfree.data.time.TimeSeries;
import org.jfree.data.time.TimeSeriesCollection;
import org.jfree.data.xy.XYDataset;
import java.util.Arrays;
import java.util.Comparator;
import java.time.LocalDate;

/**
 * StockPredictGUI - Java-based GUI application for displaying StockPredict data mining results
 * 
 * This application provides a graphical interface to visualize:
 * - Stock price data with OHLC charts
 * - Detected price patterns and trading signals
 * - Technical indicators (SMA, EMA, MACD, etc.)
 * - Anomalies detected in the data
 */
public class StockPredictGUI extends JFrame {
    
    // Components
    private JTabbedPane mainTabbedPane;
    private JPanel chartPanel;
    private JPanel patternsPanel;
    private JPanel signalsPanel;
    private JPanel anomaliesPanel;
    private JPanel indicatorsPanel;
    
    // Tables
    private JTable patternsTable;
    private JTable signalsTable;
    private JTable anomaliesTable;
    
    // Menu items
    private JMenuItem openDataMenuItem;
    private JMenuItem exportResultsMenuItem;
    private JMenuItem exitMenuItem;
    
    // Data structures
    private List<StockData> stockDataList = new ArrayList<>();
    private List<PatternResult> patternsList = new ArrayList<>();
    private List<TradingSignal> signalsList = new ArrayList<>();
    private List<AnomalyResult> anomaliesList = new ArrayList<>();
    
    // Add the class variables to support the above code
    private JPanel indicatorChartPanel;
    private JCheckBox smaCheckbox;
    private JCheckBox emaCheckbox;
    private JCheckBox macdCheckbox;
    private JCheckBox rsiCheckbox;
    private JCheckBox bollingerCheckbox;
    private JTextField smaPeriodField;
    private JTextField emaPeriodField;
    private JTextField macdFastField;
    private JTextField macdSlowField;
    private JTextField macdSignalField;
    
    public StockPredictGUI() {
        super("StockPredict Data Mining Results Viewer");
        initializeUI();
    }
    
    private void initializeUI() {
        // Set up the frame
        setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        setSize(1024, 768);
        setLocationRelativeTo(null);
        
        // Create menu bar
        JMenuBar menuBar = new JMenuBar();
        JMenu fileMenu = new JMenu("File");
        
        openDataMenuItem = new JMenuItem("Open Data File...");
        exportResultsMenuItem = new JMenuItem("Export Results...");
        exitMenuItem = new JMenuItem("Exit");
        
        openDataMenuItem.addActionListener(e -> loadDataFile());
        exportResultsMenuItem.addActionListener(e -> exportResults());
        exitMenuItem.addActionListener(e -> System.exit(0));
        
        fileMenu.add(openDataMenuItem);
        fileMenu.add(exportResultsMenuItem);
        fileMenu.addSeparator();
        fileMenu.add(exitMenuItem);
        menuBar.add(fileMenu);
        
        JMenu viewMenu = new JMenu("View");
        JMenuItem refreshMenuItem = new JMenuItem("Refresh");
        refreshMenuItem.addActionListener(e -> refreshData());
        viewMenu.add(refreshMenuItem);
        menuBar.add(viewMenu);
        
        JMenu analysisMenu = new JMenu("Analysis");
        JMenuItem runAnalysisMenuItem = new JMenuItem("Run Analysis...");
        runAnalysisMenuItem.addActionListener(e -> runAnalysis());
        analysisMenu.add(runAnalysisMenuItem);
        menuBar.add(analysisMenu);
        
        JMenu helpMenu = new JMenu("Help");
        JMenuItem aboutMenuItem = new JMenuItem("About");
        aboutMenuItem.addActionListener(e -> showAboutDialog());
        helpMenu.add(aboutMenuItem);
        menuBar.add(helpMenu);
        
        setJMenuBar(menuBar);
        
        // Create main tabbed pane
        mainTabbedPane = new JTabbedPane();
        
        // Chart Panel
        chartPanel = createChartPanel();
        mainTabbedPane.addTab("Price Chart", chartPanel);
        
        // Patterns Panel
        patternsPanel = createPatternsPanel();
        mainTabbedPane.addTab("Price Patterns", patternsPanel);
        
        // Signals Panel
        signalsPanel = createSignalsPanel();
        mainTabbedPane.addTab("Trading Signals", signalsPanel);
        
        // Anomalies Panel
        anomaliesPanel = createAnomaliesPanel();
        mainTabbedPane.addTab("Anomalies", anomaliesPanel);
        
        // Indicators Panel
        indicatorsPanel = createIndicatorsPanel();
        mainTabbedPane.addTab("Technical Indicators", indicatorsPanel);
        
        // Add the tabbed pane to the frame
        add(mainTabbedPane, BorderLayout.CENTER);
        
        // Status bar
        JPanel statusPanel = new JPanel(new BorderLayout());
        statusPanel.setBorder(BorderFactory.createEtchedBorder());
        JLabel statusLabel = new JLabel("Ready");
        statusPanel.add(statusLabel, BorderLayout.WEST);
        add(statusPanel, BorderLayout.SOUTH);
    }
    
    private JPanel createChartPanel() {
        JPanel panel = new JPanel(new BorderLayout());
        
        // Placeholder for JFreeChart
        JLabel placeholderLabel = new JLabel("Price chart will be displayed here", JLabel.CENTER);
        placeholderLabel.setFont(new Font("Arial", Font.BOLD, 18));
        panel.add(placeholderLabel, BorderLayout.CENTER);
        
        // Controls panel
        JPanel controlsPanel = new JPanel();
        String[] timeframes = {"1 Day", "5 Days", "1 Month", "3 Months", "6 Months", "1 Year", "5 Years"};
        JComboBox<String> timeframeCombo = new JComboBox<>(timeframes);
        controlsPanel.add(new JLabel("Timeframe:"));
        controlsPanel.add(timeframeCombo);
        
        String[] chartTypes = {"Candlestick", "OHLC", "Line"};
        JComboBox<String> chartTypeCombo = new JComboBox<>(chartTypes);
        controlsPanel.add(new JLabel("Chart Type:"));
        controlsPanel.add(chartTypeCombo);
        
        JCheckBox showPatternsCheckbox = new JCheckBox("Show Patterns");
        JCheckBox showSignalsCheckbox = new JCheckBox("Show Signals");
        JCheckBox showAnomaliesCheckbox = new JCheckBox("Show Anomalies");
        
        controlsPanel.add(showPatternsCheckbox);
        controlsPanel.add(showSignalsCheckbox);
        controlsPanel.add(showAnomaliesCheckbox);
        
        panel.add(controlsPanel, BorderLayout.NORTH);
        
        return panel;
    }
    
    private JPanel createPatternsPanel() {
        JPanel panel = new JPanel(new BorderLayout());
        
        // Create table model with columns
        DefaultTableModel model = new DefaultTableModel();
        model.addColumn("ID");
        model.addColumn("Type");
        model.addColumn("Start Date");
        model.addColumn("End Date");
        model.addColumn("Confidence");
        model.addColumn("Expected Move");
        model.addColumn("Description");
        
        // Create table with sorting capability
        patternsTable = new JTable(model);
        patternsTable.setAutoCreateRowSorter(true); // Enable sorting
        patternsTable.setFillsViewportHeight(true);
        JScrollPane scrollPane = new JScrollPane(patternsTable);
        panel.add(scrollPane, BorderLayout.CENTER);
        
        // Controls panel
        JPanel controlsPanel = new JPanel();
        JButton viewPatternButton = new JButton("View Selected Pattern");
        viewPatternButton.addActionListener(e -> viewSelectedPattern());
        
        JButton exportButton = new JButton("Export Patterns");
        exportButton.addActionListener(e -> exportPatterns());
        
        controlsPanel.add(viewPatternButton);
        controlsPanel.add(exportButton);
        
        panel.add(controlsPanel, BorderLayout.SOUTH);
        
        return panel;
    }
    
    private JPanel createSignalsPanel() {
        JPanel panel = new JPanel(new BorderLayout());
        
        // Create table model with columns
        DefaultTableModel model = new DefaultTableModel();
        model.addColumn("ID");
        model.addColumn("Type");
        model.addColumn("Date");
        model.addColumn("Confidence");
        model.addColumn("Entry Price");
        model.addColumn("Target Price");
        model.addColumn("Stop Loss");
        model.addColumn("R/R Ratio");
        model.addColumn("Description");
        
        // Create table with sorting capability
        signalsTable = new JTable(model);
        signalsTable.setAutoCreateRowSorter(true); // Enable sorting
        signalsTable.setFillsViewportHeight(true);
        JScrollPane scrollPane = new JScrollPane(signalsTable);
        panel.add(scrollPane, BorderLayout.CENTER);
        
        // Controls panel
        JPanel controlsPanel = new JPanel();
        JButton viewSignalButton = new JButton("View Selected Signal");
        viewSignalButton.addActionListener(e -> viewSelectedSignal());
        
        JButton exportButton = new JButton("Export Signals");
        exportButton.addActionListener(e -> exportSignals());
        
        controlsPanel.add(viewSignalButton);
        controlsPanel.add(exportButton);
        
        panel.add(controlsPanel, BorderLayout.SOUTH);
        
        return panel;
    }
    
    private JPanel createAnomaliesPanel() {
        JPanel panel = new JPanel(new BorderLayout());
        
        // Create table model with columns
        DefaultTableModel model = new DefaultTableModel();
        model.addColumn("ID");
        model.addColumn("Date");
        model.addColumn("Score");
        model.addColumn("Price Deviation");
        model.addColumn("Volume Deviation");
        model.addColumn("Description");
        
        // Create table with sorting capability
        anomaliesTable = new JTable(model);
        anomaliesTable.setAutoCreateRowSorter(true); // Enable sorting
        anomaliesTable.setFillsViewportHeight(true);
        JScrollPane scrollPane = new JScrollPane(anomaliesTable);
        panel.add(scrollPane, BorderLayout.CENTER);
        
        // Controls panel
        JPanel controlsPanel = new JPanel();
        JButton viewAnomalyButton = new JButton("View Selected Anomaly");
        viewAnomalyButton.addActionListener(e -> viewSelectedAnomaly());
        
        JButton exportButton = new JButton("Export Anomalies");
        exportButton.addActionListener(e -> exportAnomalies());
        
        controlsPanel.add(viewAnomalyButton);
        controlsPanel.add(exportButton);
        
        panel.add(controlsPanel, BorderLayout.SOUTH);
        
        return panel;
    }
    
    private JPanel createIndicatorsPanel() {
        JPanel panel = new JPanel(new BorderLayout());
        
        // Split pane - indicator selector on left, chart on right
        JSplitPane splitPane = new JSplitPane(JSplitPane.HORIZONTAL_SPLIT);
        
        // Left panel - indicator selector
        JPanel indicatorSelectorPanel = new JPanel();
        indicatorSelectorPanel.setLayout(new BoxLayout(indicatorSelectorPanel, BoxLayout.Y_AXIS));
        
        // Add indicator checkboxes
        smaCheckbox = new JCheckBox("SMA");
        emaCheckbox = new JCheckBox("EMA");
        macdCheckbox = new JCheckBox("MACD");
        rsiCheckbox = new JCheckBox("RSI");
        bollingerCheckbox = new JCheckBox("Bollinger Bands");
        
        // Add input fields for parameters
        JPanel smaParamsPanel = new JPanel(new FlowLayout(FlowLayout.LEFT));
        smaParamsPanel.add(new JLabel("Period:"));
        smaPeriodField = new JTextField("20", 4);
        smaParamsPanel.add(smaPeriodField);
        
        JPanel emaParamsPanel = new JPanel(new FlowLayout(FlowLayout.LEFT));
        emaParamsPanel.add(new JLabel("Period:"));
        emaPeriodField = new JTextField("12", 4);
        emaParamsPanel.add(emaPeriodField);
        
        JPanel macdParamsPanel = new JPanel(new FlowLayout(FlowLayout.LEFT));
        macdParamsPanel.add(new JLabel("Fast:"));
        macdFastField = new JTextField("12", 3);
        macdParamsPanel.add(macdFastField);
        macdParamsPanel.add(new JLabel("Slow:"));
        macdSlowField = new JTextField("26", 3);
        macdParamsPanel.add(macdSlowField);
        macdParamsPanel.add(new JLabel("Signal:"));
        macdSignalField = new JTextField("9", 3);
        macdParamsPanel.add(macdSignalField);
        
        indicatorSelectorPanel.add(smaCheckbox);
        indicatorSelectorPanel.add(smaParamsPanel);
        indicatorSelectorPanel.add(emaCheckbox);
        indicatorSelectorPanel.add(emaParamsPanel);
        indicatorSelectorPanel.add(macdCheckbox);
        indicatorSelectorPanel.add(macdParamsPanel);
        indicatorSelectorPanel.add(rsiCheckbox);
        indicatorSelectorPanel.add(bollingerCheckbox);
        
        // Add refresh button
        JButton refreshButton = new JButton("Apply Indicators");
        refreshButton.addActionListener(e -> refreshIndicators());
        indicatorSelectorPanel.add(Box.createVerticalStrut(20));
        indicatorSelectorPanel.add(refreshButton);
        
        // Right panel - chart
        indicatorChartPanel = new JPanel(new BorderLayout());
        JLabel placeholderLabel = new JLabel("Indicator chart will be displayed here", JLabel.CENTER);
        placeholderLabel.setFont(new Font("Arial", Font.BOLD, 18));
        indicatorChartPanel.add(placeholderLabel, BorderLayout.CENTER);
        
        // Add panels to split pane
        splitPane.setLeftComponent(indicatorSelectorPanel);
        splitPane.setRightComponent(indicatorChartPanel);
        splitPane.setDividerLocation(250);
        
        panel.add(splitPane, BorderLayout.CENTER);
        
        return panel;
    }
    
    // Event handler methods
    
    /**
     * Opens a file chooser dialog and loads CSV stock data from the selected file
     * Mở hộp thoại chọn tệp và tải dữ liệu chứng khoán CSV từ tệp đã chọn
     * 
     * Process:
     * 1. Opens a file chooser dialog configured for CSV files
     * 2. If user selects a file, tries to load the stock data
     * 3. Updates the price chart with the loaded data
     * 4. Shows a confirmation message to the user
     * 
     * Quy trình:
     * 1. Mở hộp thoại chọn tệp được cấu hình cho các tệp CSV
     * 2. Nếu người dùng chọn một tệp, cố gắng tải dữ liệu chứng khoán
     * 3. Cập nhật biểu đồ giá với dữ liệu đã tải
     * 4. Hiển thị thông báo xác nhận cho người dùng
     */
    private void loadDataFile() {
        JFileChooser fileChooser = new JFileChooser("data"); // Set default directory to 'data'
        fileChooser.setDialogTitle("Open Stock Data File");
        fileChooser.setFileFilter(new javax.swing.filechooser.FileFilter() {
            public boolean accept(File f) {
                return f.isDirectory() || f.getName().toLowerCase().endsWith(".csv");
            }
            public String getDescription() {
                return "CSV Files (*.csv)";
            }
        });
        
        int result = fileChooser.showOpenDialog(this);
        
        if (result == JFileChooser.APPROVE_OPTION) {
            File selectedFile = fileChooser.getSelectedFile();
            try {
                // Read stock data from file
                // Đọc dữ liệu chứng khoán từ tệp
                loadStockDataFromFile(selectedFile);
                
                // Update the UI with the loaded data
                // Cập nhật giao diện người dùng với dữ liệu đã tải
                updatePriceChart();
                JOptionPane.showMessageDialog(this, 
                                             "Successfully loaded " + stockDataList.size() + " data points.",
                                             "Data Loaded", 
                                             JOptionPane.INFORMATION_MESSAGE);
            } catch (IOException e) {
                JOptionPane.showMessageDialog(this,
                                             "Error loading data file: " + e.getMessage(),
                                             "Error",
                                             JOptionPane.ERROR_MESSAGE);
            }
        }
    }
    
    /**
     * Loads stock data from a CSV file into the application's data model
     * Tải dữ liệu chứng khoán từ tệp CSV vào mô hình dữ liệu của ứng dụng
     * 
     * CSV format expected:
     * Date,Open,High,Low,Close,Volume
     * 
     * Định dạng CSV dự kiến:
     * Ngày,Mở,Cao,Thấp,Đóng,Khối lượng
     * 
     * @param file CSV file containing stock data (Tệp CSV chứa dữ liệu chứng khoán)
     * @throws IOException If there's an error reading the file (Nếu có lỗi khi đọc tệp)
     */
    private void loadStockDataFromFile(File file) throws IOException {
        stockDataList.clear();
        
        try (BufferedReader reader = new BufferedReader(new FileReader(file))) {
            String line;
            boolean firstLine = true;
            
            while ((line = reader.readLine()) != null) {
                if (firstLine) {
                    firstLine = false;
                    if (line.contains("Date") || line.contains("date")) {
                        continue; // Skip header
                    }
                }
                
                String[] parts = line.split(",");
                if (parts.length >= 6) {
                    StockData data = new StockData();
                    data.date = parts[0].trim();
                    
                    try {
                        data.open = Double.parseDouble(parts[1].trim());
                        data.high = Double.parseDouble(parts[2].trim());
                        data.low = Double.parseDouble(parts[3].trim());
                        data.close = Double.parseDouble(parts[4].trim());
                        data.volume = Double.parseDouble(parts[5].trim());
                        
                        stockDataList.add(data);
                    } catch (NumberFormatException e) {
                        System.err.println("Error parsing numeric data: " + line + " - " + e.getMessage());
                        // Skip this line and continue with the next
                    }
                } else {
                    System.err.println("Invalid data format (not enough columns): " + line);
                }
            }
        }
        
        System.out.println("Loaded " + stockDataList.size() + " data points from " + file.getName());
        
        // Check the first few entries for date format debugging
        // Kiểm tra một vài mục đầu tiên để gỡ lỗi định dạng ngày
        int debugCount = Math.min(stockDataList.size(), 5);
        for (int i = 0; i < debugCount; i++) {
            System.out.println("Sample date [" + i + "]: " + stockDataList.get(i).date);
        }
    }
    
    /**
     * Updates the price chart with the current stock data
     * Cập nhật biểu đồ giá với dữ liệu chứng khoán hiện tại
     * 
     * Process:
     * 1. Creates a time series from the stock data
     * 2. Generates a JFreeChart time series chart
     * 3. Configures the chart panel with zoom capabilities
     * 4. Updates the UI to display the new chart
     * 
     * Quy trình:
     * 1. Tạo chuỗi thời gian từ dữ liệu chứng khoán
     * 2. Tạo biểu đồ chuỗi thời gian JFreeChart
     * 3. Cấu hình bảng điều khiển biểu đồ với khả năng phóng to
     * 4. Cập nhật giao diện người dùng để hiển thị biểu đồ mới
     */
    private void updatePriceChart() {
        if (stockDataList.isEmpty()) {
            return;
        }
        
        // Create a time series for the stock data
        // Tạo chuỗi thời gian cho dữ liệu chứng khoán
        TimeSeries series = new TimeSeries("Price");
        
        // Parse dates and add to series
        // Phân tích ngày tháng và thêm vào chuỗi
        for (StockData data : stockDataList) {
            try {
                Day date = parseDateString(data.date);
                // Add to series
                series.add(date, data.close);
            } catch (Exception e) {
                // Log the error and continue with next data point
                System.err.println("Error parsing date: " + data.date + " - " + e.getMessage());
            }
        }
        
        // Create dataset and chart
        // Tạo bộ dữ liệu và biểu đồ
        TimeSeriesCollection dataset = new TimeSeriesCollection(series);
        JFreeChart chart = ChartFactory.createTimeSeriesChart(
            "Stock Price", // title
            "Date",        // x-axis label
            "Price",       // y-axis label
            dataset,       // data
            true,          // legend
            true,          // tooltips
            false          // urls
        );
        
        // Create chart panel
        // Tạo bảng điều khiển biểu đồ
        ChartPanel chartPanel = new ChartPanel(chart);
        chartPanel.setPreferredSize(new Dimension(800, 500));
        chartPanel.setDomainZoomable(true);
        chartPanel.setRangeZoomable(true);
        
        // Replace the placeholder in the chart panel
        // Thay thế giữ chỗ trong bảng điều khiển biểu đồ
        this.chartPanel.removeAll();
        this.chartPanel.add(chartPanel, BorderLayout.CENTER);
        this.chartPanel.revalidate();
        this.chartPanel.repaint();
    }
    
    /**
     * Parse a date string in various formats into a JFreeChart Day object
     * Handles: 
     * - ISO 8601 (YYYY-MM-DDThh:mm:ss)
     * - YYYY-MM-DD
     * - MM/DD/YYYY or DD/MM/YYYY
     * 
     * @param dateStr Date string to parse
     * @return Day object representing the date
     */
    private Day parseDateString(String dateStr) {
        if (dateStr == null || dateStr.isEmpty()) {
            return new Day(); // Default to current date if empty
        }
        
        try {
            // Handle ISO 8601 format: YYYY-MM-DDThh:mm:ss
            if (dateStr.contains("T")) {
                String[] parts = dateStr.split("T");
                if (parts.length >= 1) {
                    String[] dateParts = parts[0].split("-");
                    if (dateParts.length >= 3) {
                        int year = Integer.parseInt(dateParts[0]);
                        int month = Integer.parseInt(dateParts[1]);
                        int day = Integer.parseInt(dateParts[2]);
                        return new Day(day, month, year);
                    }
                }
                System.err.println("Invalid ISO date format: " + dateStr);
                return new Day(); // Default to current date
            } 
            // Handle YYYY-MM-DD format
            else if (dateStr.contains("-")) {
                String[] dateParts = dateStr.split("-");
                if (dateParts.length >= 3) {
                    int year = Integer.parseInt(dateParts[0]);
                    int month = Integer.parseInt(dateParts[1]);
                    int day = Integer.parseInt(dateParts[2]);
                    return new Day(day, month, year);
                }
                System.err.println("Invalid date format (dashes): " + dateStr);
                return new Day(); // Default to current date
            } 
            // Handle MM/DD/YYYY or DD/MM/YYYY format
            else if (dateStr.contains("/")) {
                String[] dateParts = dateStr.split("/");
                if (dateParts.length >= 3) {
                    // Assume MM/DD/YYYY format
                    int month = Integer.parseInt(dateParts[0]);
                    int day = Integer.parseInt(dateParts[1]);
                    int year = Integer.parseInt(dateParts[2]);
                    
                    // Handle possible two-digit year
                    if (year < 100) {
                        year += 2000;
                    }
                    
                    return new Day(day, month, year);
                }
                System.err.println("Invalid date format (slashes): " + dateStr);
                return new Day(); // Default to current date
            }
            
            // Unknown format
            System.err.println("Unknown date format: " + dateStr);
            return new Day(); // Default to current date
        } catch (Exception e) {
            System.err.println("Error parsing date: " + dateStr + " - " + e.getMessage());
            return new Day(); // Default to current date
        }
    }
    
    private void exportResults() {
        JFileChooser fileChooser = new JFileChooser();
        fileChooser.setDialogTitle("Export Results");
        int result = fileChooser.showSaveDialog(this);
        
        if (result == JFileChooser.APPROVE_OPTION) {
            File selectedFile = fileChooser.getSelectedFile();
            // Here you would implement the export logic
            JOptionPane.showMessageDialog(this, "Results exported to " + selectedFile.getName());
        }
    }
    
    private void refreshData() {
        updatePriceChart();
        updatePatternsList();
        updateSignalsList();
        updateAnomaliesList();
    }
    
    /**
     * Runs data mining analysis on the loaded stock data
     * Chạy phân tích khai thác dữ liệu trên dữ liệu chứng khoán đã tải
     * 
     * Process:
     * 1. Validates that stock data has been loaded
     * 2. Converts internal data model to the format needed by the DataMining class
     * 3. Detects price patterns, trading signals, and anomalies
     * 4. Updates the UI to display the analysis results
     * 
     * Quy trình:
     * 1. Xác nhận rằng dữ liệu chứng khoán đã được tải
     * 2. Chuyển đổi mô hình dữ liệu nội bộ sang định dạng cần thiết cho lớp DataMining
     * 3. Phát hiện các mẫu hình giá, tín hiệu giao dịch và bất thường
     * 4. Cập nhật giao diện người dùng để hiển thị kết quả phân tích
     */
    private void runAnalysis() {
        if (stockDataList.isEmpty()) {
            JOptionPane.showMessageDialog(this, 
                                         "Please load data before running analysis.",
                                         "No Data", 
                                         JOptionPane.WARNING_MESSAGE);
            return;
        }
        
        // Convert internal StockData to DataUtils.StockData using reflection
        // This avoids compatibility issues between different StockData classes
        // Chuyển đổi StockData nội bộ sang DataUtils.StockData sử dụng phản chiếu
        // Điều này tránh các vấn đề tương thích giữa các lớp StockData khác nhau
        Object[] stockDataArray = stockDataList.toArray();
        DataUtils.StockData[] dataArray = DataUtils.StockData.fromGUIStockDataArray(stockDataArray);
        
        // Use Java-based data mining implementation
        // Sử dụng triển khai khai thác dữ liệu dựa trên Java
        try {
            // Detect price patterns
            // Phát hiện mẫu hình giá
            DataUtils.PatternResult[] detectedPatterns = DataMining.detectPricePatterns(dataArray);
            patternsList.clear();
            for (DataUtils.PatternResult pattern : detectedPatterns) {
                PatternResult guiPattern = new PatternResult();
                
                // Map pattern type to string
                // Ánh xạ loại mẫu hình thành chuỗi
                switch(pattern.type) {
                    case DataMining.PATTERN_SUPPORT:
                        guiPattern.type = "Support";
                        break;
                    case DataMining.PATTERN_RESISTANCE:
                        guiPattern.type = "Resistance";
                        break;
                    case DataMining.PATTERN_UPTREND:
                        guiPattern.type = "Uptrend";
                        break;
                    case DataMining.PATTERN_DOWNTREND:
                        guiPattern.type = "Downtrend";
                        break;
                    case DataMining.PATTERN_HEAD_AND_SHOULDERS:
                        guiPattern.type = "Head & Shoulders";
                        break;
                    case DataMining.PATTERN_DOUBLE_TOP:
                        guiPattern.type = "Double Top";
                        break;
                    case DataMining.PATTERN_DOUBLE_BOTTOM:
                        guiPattern.type = "Double Bottom";
                        break;
                    default:
                        guiPattern.type = "Unknown";
                }
                
                guiPattern.startIndex = pattern.startIndex;
                guiPattern.endIndex = pattern.endIndex;
                guiPattern.confidence = pattern.confidence;
                guiPattern.expectedMove = pattern.expectedMove;
                guiPattern.description = pattern.description;
                
                patternsList.add(guiPattern);
            }
            
            // Detect SMA crossover signals (10-day and 30-day)
            // Phát hiện tín hiệu cắt nhau SMA (10 ngày và 30 ngày)
            DataUtils.TradingSignal[] detectedSignals = DataMining.detectSMACrossoverSignals(dataArray, 10, 30);
            signalsList.clear();
            for (DataUtils.TradingSignal dmSignal : detectedSignals) {
                TradingSignal signal = new TradingSignal();
                signal.type = dmSignal.type;
                signal.signalIndex = dmSignal.signalIndex;
                signal.confidence = dmSignal.confidence;
                signal.entryPrice = dmSignal.entryPrice;
                signal.targetPrice = dmSignal.targetPrice;
                signal.stopLossPrice = dmSignal.stopLossPrice;
                signal.riskRewardRatio = dmSignal.riskRewardRatio;
                signal.description = dmSignal.description;
                signalsList.add(signal);
            }
            
            // Detect anomalies
            // Phát hiện bất thường
            DataUtils.AnomalyResult[] detectedAnomalies = DataMining.detectAnomalies(dataArray);
            anomaliesList.clear();
            for (DataUtils.AnomalyResult dmAnomaly : detectedAnomalies) {
                AnomalyResult anomaly = new AnomalyResult();
                anomaly.index = dmAnomaly.index;
                anomaly.score = dmAnomaly.score;
                anomaly.priceDeviation = dmAnomaly.priceDeviation;
                anomaly.volumeDeviation = dmAnomaly.volumeDeviation;
                anomaly.description = dmAnomaly.description;
                anomaliesList.add(anomaly);
            }
            
            // Refresh the UI with the analysis results
            // Làm mới giao diện người dùng với kết quả phân tích
            refreshData();
            
            JOptionPane.showMessageDialog(this, 
                                         "Analysis completed successfully.\n" +
                                         "Found " + patternsList.size() + " patterns, " +
                                         signalsList.size() + " signals, and " +
                                         anomaliesList.size() + " anomalies.",
                                         "Analysis Complete", 
                                         JOptionPane.INFORMATION_MESSAGE);
            
        } catch (Exception e) {
            JOptionPane.showMessageDialog(this, 
                                         "Error during analysis: " + e.getMessage(),
                                         "Analysis Error", 
                                         JOptionPane.ERROR_MESSAGE);
            e.printStackTrace();
        }
    }
    
    private void updatePatternsList() {
        DefaultTableModel model = (DefaultTableModel) patternsTable.getModel();
        model.setRowCount(0); // Clear existing rows
        
        // Log the total number of patterns for debugging
        System.out.println("Total patterns detected: " + patternsList.size());
        
        for (int i = 0; i < patternsList.size(); i++) {
            PatternResult pattern = patternsList.get(i);
            
            String startDate = "";
            String endDate = "";
            
            if (pattern.startIndex < stockDataList.size()) {
                startDate = formatDateForDisplay(stockDataList.get(pattern.startIndex).date);
            }
            
            if (pattern.endIndex < stockDataList.size()) {
                endDate = formatDateForDisplay(stockDataList.get(pattern.endIndex).date);
            }
            
            model.addRow(new Object[]{
                i+1,
                pattern.type,
                startDate,
                endDate,
                pattern.confidence,
                pattern.expectedMove + "%",
                pattern.description
            });
        }
        
        // Set up a custom sorter for date columns
        patternsTable.setAutoCreateRowSorter(true);
        TableRowSorter<TableModel> sorter = new TableRowSorter<>(patternsTable.getModel());
        patternsTable.setRowSorter(sorter);
        
        // Add date comparators for the Start Date and End Date columns
        sorter.setComparator(2, Comparator.comparing(s -> {
            try {
                return s.toString().isEmpty() ? LocalDate.MIN : LocalDate.parse(s.toString().split("T")[0]);
            } catch (Exception e) {
                return LocalDate.MIN;
            }
        }));
        
        sorter.setComparator(3, Comparator.comparing(s -> {
            try {
                return s.toString().isEmpty() ? LocalDate.MIN : LocalDate.parse(s.toString().split("T")[0]);
            } catch (Exception e) {
                return LocalDate.MIN;
            }
        }));
    }
    
    private void updateSignalsList() {
        DefaultTableModel model = (DefaultTableModel) signalsTable.getModel();
        model.setRowCount(0); // Clear existing rows
        
        // Log the total number of signals for debugging
        System.out.println("Total trading signals detected: " + signalsList.size());
        
        for (int i = 0; i < signalsList.size(); i++) {
            TradingSignal signal = signalsList.get(i);
            
            String date = "";
            if (signal.signalIndex < stockDataList.size()) {
                date = formatDateForDisplay(stockDataList.get(signal.signalIndex).date);
            }
            
            model.addRow(new Object[]{
                i+1,
                signal.type,
                date,
                signal.confidence,
                String.format("%.2f", signal.entryPrice),
                String.format("%.2f", signal.targetPrice),
                String.format("%.2f", signal.stopLossPrice),
                String.format("%.1f", signal.riskRewardRatio),
                signal.description
            });
        }
        
        // Set up a custom sorter for date column
        signalsTable.setAutoCreateRowSorter(true);
        TableRowSorter<TableModel> sorter = new TableRowSorter<>(signalsTable.getModel());
        signalsTable.setRowSorter(sorter);
        
        // Add date comparator for the Date column
        sorter.setComparator(2, Comparator.comparing(s -> {
            try {
                return s.toString().isEmpty() ? LocalDate.MIN : LocalDate.parse(s.toString().split("T")[0]);
            } catch (Exception e) {
                return LocalDate.MIN;
            }
        }));
    }
    
    private void updateAnomaliesList() {
        DefaultTableModel model = (DefaultTableModel) anomaliesTable.getModel();
        model.setRowCount(0); // Clear existing rows
        
        // Log the total number of anomalies for debugging
        System.out.println("Total anomalies detected: " + anomaliesList.size());
        
        for (int i = 0; i < anomaliesList.size(); i++) {
            AnomalyResult anomaly = anomaliesList.get(i);
            
            String date = "";
            if (anomaly.index < stockDataList.size()) {
                date = formatDateForDisplay(stockDataList.get(anomaly.index).date);
            }
            
            model.addRow(new Object[]{
                i+1,
                date,
                String.format("%.2f", anomaly.score),
                String.format("%.2f", anomaly.priceDeviation),
                String.format("%.2f", anomaly.volumeDeviation),
                anomaly.description
            });
        }
        
        // Set up a custom sorter for date column
        anomaliesTable.setAutoCreateRowSorter(true);
        TableRowSorter<TableModel> sorter = new TableRowSorter<>(anomaliesTable.getModel());
        anomaliesTable.setRowSorter(sorter);
        
        // Add date comparator for the Date column
        sorter.setComparator(1, Comparator.comparing(s -> {
            try {
                return s.toString().isEmpty() ? LocalDate.MIN : LocalDate.parse(s.toString().split("T")[0]);
            } catch (Exception e) {
                return LocalDate.MIN;
            }
        }));
    }
    
    /**
     * Format a date string for display in tables and UI components
     * Handles both regular dates and ISO formatted dates
     * 
     * @param dateStr The date string to format
     * @return A formatted date string (e.g., "2023-01-15")
     */
    private String formatDateForDisplay(String dateStr) {
        if (dateStr == null || dateStr.isEmpty()) {
            return "";
        }
        
        try {
            // For ISO 8601 dates, extract just the date part
            if (dateStr.contains("T")) {
                String[] parts = dateStr.split("T");
                if (parts.length > 0) {
                    return parts[0]; // Return just the date part (YYYY-MM-DD)
                }
            }
            
            // For other formats, return as is
            return dateStr;
        } catch (Exception e) {
            System.err.println("Error formatting date: " + dateStr + " - " + e.getMessage());
            return dateStr;
        }
    }
    
    /**
     * Refreshes indicator charts based on selected indicators and parameters
     * Làm mới các biểu đồ chỉ báo dựa trên các chỉ báo và tham số đã chọn
     * 
     * Features:
     * 1. Creates time series for price and selected indicators
     * 2. Calculates indicators based on user-selected parameters
     * 3. Creates separate scales for indicators requiring different scales
     * 4. Generates JFreeChart with multiple datasets and renderers
     * 
     * Tính năng:
     * 1. Tạo chuỗi thời gian cho giá và các chỉ báo đã chọn
     * 2. Tính toán các chỉ báo dựa trên tham số do người dùng chọn
     * 3. Tạo các thang đo riêng cho các chỉ báo yêu cầu thang đo khác nhau
     * 4. Tạo JFreeChart với nhiều bộ dữ liệu và trình hiển thị
     */
    private void refreshIndicators() {
        if (stockDataList.isEmpty()) {
            JOptionPane.showMessageDialog(this, "No stock data loaded. Please load data first.", 
                "Error", JOptionPane.WARNING_MESSAGE);
            return;
        }
        
        try {
            // Create a time series for the stock price
            // Tạo chuỗi thời gian cho giá cổ phiếu
            TimeSeries priceSeries = new TimeSeries("Price");
            
            // Create datasets for selected indicators
            // Tạo bộ dữ liệu cho các chỉ báo đã chọn
            TimeSeriesCollection priceDataset = new TimeSeriesCollection();
            priceDataset.addSeries(priceSeries);
            
            // Additional dataset for indicators that need their own scale (RSI, MACD)
            // Bộ dữ liệu bổ sung cho các chỉ báo cần thang đo riêng (RSI, MACD)
            TimeSeriesCollection secondaryDataset = null;
            
            // Add price data
            // Thêm dữ liệu giá
            for (StockData data : stockDataList) {
                try {
                    Day date = parseDateString(data.date);
                    priceSeries.add(date, data.close);
                } catch (Exception e) {
                    System.err.println("Error parsing date: " + data.date + " - " + e.getMessage());
                }
            }
            
            // Check which indicators are selected and add them
            if (smaCheckbox.isSelected()) {
                try {
                    int period = Integer.parseInt(smaPeriodField.getText());
                    TimeSeries smaSeries = new TimeSeries("SMA(" + period + ")");
                    
                    // Calculate SMA
                    double[] closes = new double[stockDataList.size()];
                    for (int i = 0; i < stockDataList.size(); i++) {
                        closes[i] = stockDataList.get(i).close;
                    }
                    
                    for (int i = period - 1; i < stockDataList.size(); i++) {
                        double sum = 0;
                        for (int j = i - (period - 1); j <= i; j++) {
                            sum += closes[j];
                        }
                        double sma = sum / period;
                        
                        // Add to series
                        Day date = parseDateString(stockDataList.get(i).date);
                        smaSeries.add(date, sma);
                    }
                    
                    priceDataset.addSeries(smaSeries);
                } catch (NumberFormatException e) {
                    JOptionPane.showMessageDialog(this, "Invalid SMA period. Please enter a valid number.", 
                        "Error", JOptionPane.WARNING_MESSAGE);
                }
            }
            
            if (emaCheckbox.isSelected()) {
                try {
                    int period = Integer.parseInt(emaPeriodField.getText());
                    TimeSeries emaSeries = new TimeSeries("EMA(" + period + ")");
                    
                    // Calculate EMA
                    double[] closes = new double[stockDataList.size()];
                    for (int i = 0; i < stockDataList.size(); i++) {
                        closes[i] = stockDataList.get(i).close;
                    }
                    
                    // First EMA value is SMA
                    double sum = 0;
                    for (int i = 0; i < period; i++) {
                        sum += closes[i];
                    }
                    double ema = sum / period;
                    
                    // Add first EMA point
                    Day date = parseDateString(stockDataList.get(period - 1).date);
                    emaSeries.add(date, ema);
                    
                    // Calculate multiplier
                    double multiplier = 2.0 / (period + 1);
                    
                    // Calculate rest of EMA values
                    for (int i = period; i < stockDataList.size(); i++) {
                        ema = (closes[i] - ema) * multiplier + ema;
                        date = parseDateString(stockDataList.get(i).date);
                        emaSeries.add(date, ema);
                    }
                    
                    priceDataset.addSeries(emaSeries);
                } catch (NumberFormatException e) {
                    JOptionPane.showMessageDialog(this, "Invalid EMA period. Please enter a valid number.", 
                        "Error", JOptionPane.WARNING_MESSAGE);
                }
            }
            
            if (bollingerCheckbox.isSelected()) {
                try {
                    int period = 20; // Standard period
                    double stdDevMultiplier = 2.0; // Standard deviation multiplier
                    
                    TimeSeries upperBandSeries = new TimeSeries("Upper Bollinger Band");
                    TimeSeries middleBandSeries = new TimeSeries("Middle Bollinger Band");
                    TimeSeries lowerBandSeries = new TimeSeries("Lower Bollinger Band");
                    
                    // Calculate Bollinger Bands
                    double[] closes = new double[stockDataList.size()];
                    for (int i = 0; i < stockDataList.size(); i++) {
                        closes[i] = stockDataList.get(i).close;
                    }
                    
                    for (int i = period - 1; i < stockDataList.size(); i++) {
                        // Calculate SMA (middle band)
                        double sum = 0;
                        for (int j = i - (period - 1); j <= i; j++) {
                            sum += closes[j];
                        }
                        double sma = sum / period;
                        
                        // Calculate standard deviation
                        double sumSquaredDiff = 0;
                        for (int j = i - (period - 1); j <= i; j++) {
                            double diff = closes[j] - sma;
                            sumSquaredDiff += diff * diff;
                        }
                        double stdDev = Math.sqrt(sumSquaredDiff / period);
                        
                        // Calculate upper and lower bands
                        double upperBand = sma + (stdDevMultiplier * stdDev);
                        double lowerBand = sma - (stdDevMultiplier * stdDev);
                        
                        // Add to series
                        Day date = parseDateString(stockDataList.get(i).date);
                        upperBandSeries.add(date, upperBand);
                        middleBandSeries.add(date, sma);
                        lowerBandSeries.add(date, lowerBand);
                    }
                    
                    priceDataset.addSeries(upperBandSeries);
                    priceDataset.addSeries(middleBandSeries);
                    priceDataset.addSeries(lowerBandSeries);
                } catch (Exception e) {
                    JOptionPane.showMessageDialog(this, "Error calculating Bollinger Bands: " + e.getMessage(), 
                        "Error", JOptionPane.WARNING_MESSAGE);
                }
            }
            
            if (rsiCheckbox.isSelected()) {
                try {
                    if (secondaryDataset == null) {
                        secondaryDataset = new TimeSeriesCollection();
                    }
                    
                    int period = 14; // Standard RSI period
                    TimeSeries rsiSeries = new TimeSeries("RSI(" + period + ")");
                    
                    // Extract price data
                    double[] closes = new double[stockDataList.size()];
                    for (int i = 0; i < stockDataList.size(); i++) {
                        closes[i] = stockDataList.get(i).close;
                    }
                    
                    if (closes.length <= period) {
                        throw new IllegalArgumentException("Not enough data points for RSI calculation");
                    }
                    
                    // Calculate gains and losses
                    double[] gains = new double[closes.length];
                    double[] losses = new double[closes.length];
                    
                    for (int i = 1; i < closes.length; i++) {
                        double change = closes[i] - closes[i-1];
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
                    double rs = 0;
                    double rsi = 0;
                    
                    if (avgLoss > 0) {
                        rs = avgGain / avgLoss;
                        rsi = 100 - (100 / (1 + rs));
                    } else {
                        rsi = 100; // No losses, RSI = 100
                    }
                    
                    // Add first RSI value
                    Day date = parseDateString(stockDataList.get(period).date);
                    rsiSeries.add(date, rsi);
                    
                    // Calculate remaining RSI values
                    for (int i = period + 1; i < closes.length; i++) {
                        avgGain = ((avgGain * (period - 1)) + gains[i]) / period;
                        avgLoss = ((avgLoss * (period - 1)) + losses[i]) / period;
                        
                        if (avgLoss > 0) {
                            rs = avgGain / avgLoss;
                            rsi = 100 - (100 / (1 + rs));
                        } else {
                            rsi = 100; // No losses, RSI = a maximum 100
                        }
                        
                        date = parseDateString(stockDataList.get(i).date);
                        rsiSeries.add(date, rsi);
                    }
                    
                    secondaryDataset.addSeries(rsiSeries);
                } catch (Exception e) {
                    JOptionPane.showMessageDialog(this, "Error calculating RSI: " + e.getMessage(), 
                        "Error", JOptionPane.WARNING_MESSAGE);
                }
            }
            
            if (macdCheckbox.isSelected()) {
                try {
                    if (secondaryDataset == null) {
                        secondaryDataset = new TimeSeriesCollection();
                    }
                    
                    int fastPeriod = Integer.parseInt(macdFastField.getText());
                    int slowPeriod = Integer.parseInt(macdSlowField.getText());
                    int signalPeriod = Integer.parseInt(macdSignalField.getText());
                    
                    TimeSeries macdSeries = new TimeSeries("MACD Line");
                    TimeSeries signalSeries = new TimeSeries("Signal Line");
                    TimeSeries histogramSeries = new TimeSeries("Histogram");
                    
                    // Extract price data
                    double[] closes = new double[stockDataList.size()];
                    for (int i = 0; i < stockDataList.size(); i++) {
                        closes[i] = stockDataList.get(i).close;
                    }
                    
                    // Calculate fast EMA
                    double[] fastEMA = new double[closes.length];
                    // First value is SMA
                    double sum = 0;
                    for (int i = 0; i < fastPeriod && i < closes.length; i++) {
                        sum += closes[i];
                    }
                    fastEMA[fastPeriod-1] = sum / fastPeriod;
                    
                    // Calculate fast multiplier
                    double fastMultiplier = 2.0 / (fastPeriod + 1);
                    
                    // Calculate rest of fast EMA values
                    for (int i = fastPeriod; i < closes.length; i++) {
                        fastEMA[i] = (closes[i] - fastEMA[i-1]) * fastMultiplier + fastEMA[i-1];
                    }
                    
                    // Calculate slow EMA
                    double[] slowEMA = new double[closes.length];
                    // First value is SMA
                    sum = 0;
                    for (int i = 0; i < slowPeriod && i < closes.length; i++) {
                        sum += closes[i];
                    }
                    slowEMA[slowPeriod-1] = sum / slowPeriod;
                    
                    // Calculate slow multiplier
                    double slowMultiplier = 2.0 / (slowPeriod + 1);
                    
                    // Calculate rest of slow EMA values
                    for (int i = slowPeriod; i < closes.length; i++) {
                        slowEMA[i] = (closes[i] - slowEMA[i-1]) * slowMultiplier + slowEMA[i-1];
                    }
                    
                    // Calculate MACD Line = Fast EMA - Slow EMA
                    double[] macdLine = new double[closes.length];
                    for (int i = 0; i < closes.length; i++) {
                        macdLine[i] = 0; // Initialize
                    }
                    
                    // MACD line can only be calculated from the slow period onwards
                    for (int i = slowPeriod - 1; i < closes.length; i++) {
                        macdLine[i] = fastEMA[i] - slowEMA[i];
                    }
                    
                    // Calculate Signal Line (EMA of MACD Line)
                    double[] signalLine = new double[closes.length];
                    for (int i = 0; i < closes.length; i++) {
                        signalLine[i] = 0; // Initialize
                    }
                    
                    // First signal value is SMA of MACD
                    int signalStart = slowPeriod - 1 + signalPeriod - 1;
                    if (signalStart < closes.length) {
                        sum = 0;
                        for (int i = slowPeriod - 1; i <= signalStart; i++) {
                            sum += macdLine[i];
                        }
                        signalLine[signalStart] = sum / signalPeriod;
                        
                        // Calculate signal multiplier
                        double signalMultiplier = 2.0 / (signalPeriod + 1);
                        
                        // Calculate rest of signal values
                        for (int i = signalStart + 1; i < closes.length; i++) {
                            signalLine[i] = (macdLine[i] - signalLine[i-1]) * signalMultiplier + signalLine[i-1];
                        }
                    }
                    
                    // Calculate histogram
                    double[] histogram = new double[closes.length];
                    for (int i = 0; i < closes.length; i++) {
                        histogram[i] = macdLine[i] - signalLine[i];
                    }
                    
                    // Add to series from where all values are available
                    for (int i = signalStart; i < closes.length; i++) {
                        Day date = parseDateString(stockDataList.get(i).date);
                        macdSeries.add(date, macdLine[i]);
                        signalSeries.add(date, signalLine[i]);
                        histogramSeries.add(date, histogram[i]);
                    }
                    
                    secondaryDataset.addSeries(macdSeries);
                    secondaryDataset.addSeries(signalSeries);
                    secondaryDataset.addSeries(histogramSeries);
                } catch (NumberFormatException e) {
                    JOptionPane.showMessageDialog(this, "Invalid MACD parameters. Please enter valid numbers.", 
                        "Error", JOptionPane.WARNING_MESSAGE);
                } catch (Exception e) {
                    JOptionPane.showMessageDialog(this, "Error calculating MACD: " + e.getMessage(), 
                        "Error", JOptionPane.WARNING_MESSAGE);
                }
            }
            
            // Create the chart
            // Tạo biểu đồ
            JFreeChart chart;
            
            if (secondaryDataset != null && 
                (rsiCheckbox.isSelected() || macdCheckbox.isSelected())) {
                // Create a chart with two vertical axes for price and indicators
                // Tạo biểu đồ với hai trục dọc cho giá và chỉ báo
                chart = ChartFactory.createTimeSeriesChart(
                    "Technical Indicators", // title
                    "Date",                 // x-axis label
                    "Price",                // y-axis label
                    priceDataset,           // primary dataset
                    true,                   // legend
                    true,                   // tooltips
                    false                   // urls
                );
                
                // Get the plot and add a second axis for RSI/MACD
                // Lấy đồ thị và thêm trục thứ hai cho RSI/MACD
                org.jfree.chart.plot.XYPlot plot = chart.getXYPlot();
                org.jfree.chart.axis.NumberAxis axis2 = new org.jfree.chart.axis.NumberAxis("Indicator Value");
                axis2.setAutoRangeIncludesZero(false);
                plot.setRangeAxis(1, axis2);
                plot.setDataset(1, secondaryDataset);
                plot.mapDatasetToRangeAxis(1, 1);
                
                // Add renderer for second dataset
                // Thêm trình hiển thị cho bộ dữ liệu thứ hai
                org.jfree.chart.renderer.xy.XYLineAndShapeRenderer renderer2 = 
                    new org.jfree.chart.renderer.xy.XYLineAndShapeRenderer();
                plot.setRenderer(1, renderer2);
            } else {
                // Create a simple chart with just price and price-related indicators
                // Tạo biểu đồ đơn giản chỉ với giá và các chỉ báo liên quan đến giá
                chart = ChartFactory.createTimeSeriesChart(
                    "Technical Indicators", // title
                    "Date",                 // x-axis label
                    "Price",                // y-axis label
                    priceDataset,           // data
                    true,                   // legend
                    true,                   // tooltips
                    false                   // urls
                );
            }
            
            // Create chart panel
            // Tạo bảng điều khiển biểu đồ
            ChartPanel chartPanel = new ChartPanel(chart);
            chartPanel.setPreferredSize(new Dimension(800, 500));
            chartPanel.setDomainZoomable(true);
            chartPanel.setRangeZoomable(true);
            
            // Replace the placeholder in the indicator chart panel
            // Thay thế giữ chỗ trong bảng điều khiển biểu đồ chỉ báo
            indicatorChartPanel.removeAll();
            indicatorChartPanel.add(chartPanel, BorderLayout.CENTER);
            indicatorChartPanel.revalidate();
            indicatorChartPanel.repaint();
            
        } catch (Exception e) {
            e.printStackTrace();
            JOptionPane.showMessageDialog(this, "Error creating indicator chart: " + e.getMessage(), 
                "Error", JOptionPane.ERROR_MESSAGE);
        }
    }
    
    private void viewSelectedPattern() {
        int selectedRow = patternsTable.getSelectedRow();
        if (selectedRow >= 0) {
            // Convert view row index to model row index (necessary when table is sorted)
            int modelRow = patternsTable.convertRowIndexToModel(selectedRow);
            PatternResult pattern = patternsList.get(modelRow);
            showDetailDialog("Pattern Details", pattern.description);
        } else {
            JOptionPane.showMessageDialog(this, "Please select a pattern to view.");
        }
    }
    
    private void viewSelectedSignal() {
        int selectedRow = signalsTable.getSelectedRow();
        if (selectedRow >= 0) {
            // Convert view row index to model row index (necessary when table is sorted)
            int modelRow = signalsTable.convertRowIndexToModel(selectedRow);
            TradingSignal signal = signalsList.get(modelRow);
            showDetailDialog("Signal Details", signal.description);
        } else {
            JOptionPane.showMessageDialog(this, "Please select a signal to view.");
        }
    }
    
    private void viewSelectedAnomaly() {
        int selectedRow = anomaliesTable.getSelectedRow();
        if (selectedRow >= 0) {
            // Convert view row index to model row index (necessary when table is sorted)
            int modelRow = anomaliesTable.convertRowIndexToModel(selectedRow);
            AnomalyResult anomaly = anomaliesList.get(modelRow);
            showDetailDialog("Anomaly Details", anomaly.description);
        } else {
            JOptionPane.showMessageDialog(this, "Please select an anomaly to view.");
        }
    }
    
    private void showDetailDialog(String title, String message) {
        JDialog dialog = new JDialog(this, title, true);
        dialog.setLayout(new BorderLayout());
        
        JTextArea textArea = new JTextArea(message);
        textArea.setEditable(false);
        textArea.setFont(new Font("Arial", Font.PLAIN, 14));
        textArea.setLineWrap(true);
        textArea.setWrapStyleWord(true);
        
        JScrollPane scrollPane = new JScrollPane(textArea);
        dialog.add(scrollPane, BorderLayout.CENTER);
        
        JButton closeButton = new JButton("Close");
        closeButton.addActionListener(e -> dialog.dispose());
        
        JPanel buttonPanel = new JPanel();
        buttonPanel.add(closeButton);
        dialog.add(buttonPanel, BorderLayout.SOUTH);
        
        dialog.setSize(400, 300);
        dialog.setLocationRelativeTo(this);
        dialog.setVisible(true);
    }
    
    private void showAboutDialog() {
        JOptionPane.showMessageDialog(this,
                                     "StockPredict Data Mining Results Viewer\n" +
                                     "Version 1.0\n\n" +
                                     "A Java GUI for visualizing stock data mining results\n" +
                                     "from the StockPredict C codebase.",
                                     "About",
                                     JOptionPane.INFORMATION_MESSAGE);
    }
    
    private void exportPatterns() {
        JOptionPane.showMessageDialog(this, "Exporting patterns - This would save pattern data to a file.");
    }
    
    private void exportSignals() {
        JOptionPane.showMessageDialog(this, "Exporting signals - This would save signal data to a file.");
    }
    
    private void exportAnomalies() {
        JOptionPane.showMessageDialog(this, "Exporting anomalies - This would save anomaly data to a file.");
    }
    
    // Data classes
    
    /**
     * Stock data representation with OHLCV (Open, High, Low, Close, Volume) values
     * Biểu diễn dữ liệu chứng khoán với các giá trị OHLCV (Mở, Cao, Thấp, Đóng, Khối lượng)
     */
    class StockData {
        String date;   // Date in string format (Ngày dưới dạng chuỗi)
        double open;   // Opening price (Giá mở cửa)
        double high;   // Highest price (Giá cao nhất)
        double low;    // Lowest price (Giá thấp nhất)
        double close;  // Closing price (Giá đóng cửa)
        double volume; // Trading volume (Khối lượng giao dịch)
    }
    
    /**
     * Represents a detected price pattern in the stock data
     * Biểu diễn một mẫu hình giá được phát hiện trong dữ liệu chứng khoán
     */
    class PatternResult {
        String type;       // Pattern type name (Tên loại mẫu hình)
        int startIndex;    // Start index in the data array (Chỉ số bắt đầu trong mảng dữ liệu)
        int endIndex;      // End index in the data array (Chỉ số kết thúc trong mảng dữ liệu)
        double confidence; // Confidence level (0.0-1.0) (Mức độ tin cậy (0.0-1.0))
        double expectedMove; // Expected price move (% change) (Biến động giá dự kiến (% thay đổi))
        String description; // Human-readable description (Mô tả dễ đọc cho con người)
        
        // Constructor for creating from DataMining results
        // Hàm tạo để tạo từ kết quả DataMining
        public PatternResult() {
        }
        
        // Constructor for creating from DataUtils PatternResult
        // Hàm tạo để tạo từ DataUtils PatternResult
        public PatternResult(DataUtils.PatternResult source) {
            // Map int type to string type
            // Ánh xạ kiểu int thành kiểu chuỗi
            switch(source.type) {
                case DataMining.PATTERN_SUPPORT:
                    this.type = "Support";
                    break;
                case DataMining.PATTERN_RESISTANCE:
                    this.type = "Resistance";
                    break;
                case DataMining.PATTERN_UPTREND:
                    this.type = "Uptrend";
                    break;
                case DataMining.PATTERN_DOWNTREND:
                    this.type = "Downtrend";
                    break;
                case DataMining.PATTERN_HEAD_AND_SHOULDERS:
                    this.type = "Head & Shoulders";
                    break;
                case DataMining.PATTERN_DOUBLE_TOP:
                    this.type = "Double Top";
                    break;
                case DataMining.PATTERN_DOUBLE_BOTTOM:
                    this.type = "Double Bottom";
                    break;
                default:
                    this.type = "Unknown";
            }
            
            this.startIndex = source.startIndex;
            this.endIndex = source.endIndex;
            this.confidence = source.confidence;
            this.expectedMove = source.expectedMove;
            this.description = source.description;
        }
    }
    
    /**
     * Represents a detected trading signal (buy/sell/hold recommendation)
     * Biểu diễn một tín hiệu giao dịch được phát hiện (khuyến nghị mua/bán/giữ)
     */
    class TradingSignal {
        String type;        // Signal type (BUY, SELL, etc.) (Loại tín hiệu (MUA, BÁN, v.v.))
        int signalIndex;    // Index in data where signal occurs (Chỉ số trong dữ liệu nơi tín hiệu xuất hiện)
        double confidence;  // Confidence level (0.0-1.0) (Mức độ tin cậy (0.0-1.0))
        double entryPrice;  // Suggested entry price (Giá vào được đề xuất)
        double targetPrice; // Target price for take profit (Giá mục tiêu để chốt lời)
        double stopLossPrice; // Suggested stop loss price (Giá dừng lỗ được đề xuất)
        double riskRewardRatio; // Risk/reward ratio (Tỷ lệ rủi ro/phần thưởng)
        String description; // Signal description (Mô tả tín hiệu)
    }
    
    /**
     * Represents a detected anomaly in the stock data
     * Biểu diễn một bất thường được phát hiện trong dữ liệu chứng khoán
     */
    class AnomalyResult {
        int index;             // Index in data where anomaly occurs (Chỉ số trong dữ liệu nơi bất thường xuất hiện)
        double score;          // Anomaly score (higher is more anomalous) (Điểm bất thường (cao hơn là bất thường hơn))
        double priceDeviation; // Price deviation in standard deviations (Độ lệch giá tính bằng độ lệch chuẩn)
        double volumeDeviation; // Volume deviation in standard deviations (Độ lệch khối lượng tính bằng độ lệch chuẩn)
        String description;    // Human-readable description (Mô tả dễ đọc cho con người)
    }
    
    // Main method
    /**
     * Application entry point that initializes and shows the GUI
     * Điểm vào của ứng dụng để khởi tạo và hiển thị giao diện đồ họa
     * 
     * Process:
     * 1. Sets the system look and feel for native appearance
     * 2. Creates and displays the main GUI window
     * 3. Attempts to auto-load SPY data if available
     * 4. Runs initial analysis if data is successfully loaded
     * 
     * Quy trình:
     * 1. Đặt giao diện hệ thống cho diện mạo tự nhiên
     * 2. Tạo và hiển thị cửa sổ GUI chính
     * 3. Cố gắng tự động tải dữ liệu SPY nếu có sẵn
     * 4. Chạy phân tích ban đầu nếu dữ liệu được tải thành công
     * 
     * @param args Command line arguments (not used) (Các đối số dòng lệnh (không được sử dụng))
     */
    public static void main(String[] args) {
        // Set look and feel
        // Đặt giao diện và cảm nhận
        try {
            UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
        } catch (Exception e) {
            e.printStackTrace();
        }
        
        // Create and show GUI
        // Tạo và hiển thị GUI
        SwingUtilities.invokeLater(() -> {
            StockPredictGUI gui = new StockPredictGUI();
            gui.setVisible(true);
            
            // Auto-load SPY data
            // Tự động tải dữ liệu SPY
            try {
                // Look for SPY data file in the data directory
                // Tìm kiếm tệp dữ liệu SPY trong thư mục data
                File dataDir = new File("data");
                if (dataDir.exists() && dataDir.isDirectory()) {
                    File[] files = dataDir.listFiles((dir, name) -> name.toLowerCase().startsWith("spy") && name.toLowerCase().endsWith(".csv"));
                    if (files != null && files.length > 0) {
                        // Sort files to get the most recent one
                        // Sắp xếp các tệp để lấy tệp mới nhất
                        Arrays.sort(files, (f1, f2) -> f2.getName().compareTo(f1.getName()));
                        
                        // Load the most recent SPY data file
                        // Tải tệp dữ liệu SPY mới nhất
                        gui.loadStockDataFromFile(files[0]);
                        gui.updatePriceChart();
                        
                        // Run analysis
                        // Chạy phân tích
                        SwingUtilities.invokeLater(() -> {
                            gui.runAnalysis();
                        });
                        
                        System.out.println("Automatically loaded SPY data from: " + files[0].getName());
                    }
                }
            } catch (Exception e) {
                System.err.println("Error auto-loading SPY data: " + e.getMessage());
                e.printStackTrace();
            }
        });
    }
} 