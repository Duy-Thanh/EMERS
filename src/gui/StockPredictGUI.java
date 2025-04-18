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
        JCheckBox smaCheckbox = new JCheckBox("SMA");
        JCheckBox emaCheckbox = new JCheckBox("EMA");
        JCheckBox macdCheckbox = new JCheckBox("MACD");
        JCheckBox rsiCheckbox = new JCheckBox("RSI");
        JCheckBox bollingerCheckbox = new JCheckBox("Bollinger Bands");
        
        // Add input fields for parameters
        JPanel smaParamsPanel = new JPanel(new FlowLayout(FlowLayout.LEFT));
        smaParamsPanel.add(new JLabel("Period:"));
        JTextField smaPeriodField = new JTextField("20", 4);
        smaParamsPanel.add(smaPeriodField);
        
        JPanel emaParamsPanel = new JPanel(new FlowLayout(FlowLayout.LEFT));
        emaParamsPanel.add(new JLabel("Period:"));
        JTextField emaPeriodField = new JTextField("12", 4);
        emaParamsPanel.add(emaPeriodField);
        
        JPanel macdParamsPanel = new JPanel(new FlowLayout(FlowLayout.LEFT));
        macdParamsPanel.add(new JLabel("Fast:"));
        JTextField macdFastField = new JTextField("12", 3);
        macdParamsPanel.add(macdFastField);
        macdParamsPanel.add(new JLabel("Slow:"));
        JTextField macdSlowField = new JTextField("26", 3);
        macdParamsPanel.add(macdSlowField);
        macdParamsPanel.add(new JLabel("Signal:"));
        JTextField macdSignalField = new JTextField("9", 3);
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
        JPanel indicatorChartPanel = new JPanel(new BorderLayout());
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
                loadStockDataFromFile(selectedFile);
                
                // Update the UI with the loaded data
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
        int debugCount = Math.min(stockDataList.size(), 5);
        for (int i = 0; i < debugCount; i++) {
            System.out.println("Sample date [" + i + "]: " + stockDataList.get(i).date);
        }
    }
    
    private void updatePriceChart() {
        if (stockDataList.isEmpty()) {
            return;
        }
        
        // Create a time series for the stock data
        TimeSeries series = new TimeSeries("Price");
        
        // Parse dates and add to series
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
        ChartPanel chartPanel = new ChartPanel(chart);
        chartPanel.setPreferredSize(new Dimension(800, 500));
        chartPanel.setDomainZoomable(true);
        chartPanel.setRangeZoomable(true);
        
        // Replace the placeholder in the chart panel
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
        Object[] stockDataArray = stockDataList.toArray();
        DataUtils.StockData[] dataArray = DataUtils.StockData.fromGUIStockDataArray(stockDataArray);
        
        // Use Java-based data mining implementation
        try {
            // Detect price patterns
            DataUtils.PatternResult[] detectedPatterns = DataMining.detectPricePatterns(dataArray);
            patternsList.clear();
            for (DataUtils.PatternResult pattern : detectedPatterns) {
                PatternResult guiPattern = new PatternResult();
                
                // Map pattern type to string
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
    
    private void refreshIndicators() {
        JOptionPane.showMessageDialog(this, "Refreshing indicators - This would integrate with the C codebase.");
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
    
    class StockData {
        String date;
        double open;
        double high;
        double low;
        double close;
        double volume;
    }
    
    class PatternResult {
        String type;       // Pattern type name
        int startIndex;    // Start index in the data array
        int endIndex;      // End index in the data array
        double confidence; // Confidence level (0.0-1.0)
        double expectedMove; // Expected price move (% change)
        String description; // Human-readable description
        
        // Constructor for creating from DataMining results
        public PatternResult() {
        }
        
        // Constructor for creating from DataUtils PatternResult
        public PatternResult(DataUtils.PatternResult source) {
            // Map int type to string type
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
    
    class TradingSignal {
        String type;        // Signal type (BUY, SELL, etc.)
        int signalIndex;    // Index in data where signal occurs
        double confidence;  // Confidence level (0.0-1.0)
        double entryPrice;  // Suggested entry price
        double targetPrice; // Target price for take profit
        double stopLossPrice; // Suggested stop loss price
        double riskRewardRatio; // Risk/reward ratio
        String description; // Signal description
    }
    
    class AnomalyResult {
        int index;             // Index in data where anomaly occurs
        double score;          // Anomaly score (higher is more anomalous)
        double priceDeviation; // Price deviation in standard deviations
        double volumeDeviation; // Volume deviation in standard deviations
        String description;    // Human-readable description
    }
    
    // Main method
    public static void main(String[] args) {
        // Set look and feel
        try {
            UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
        } catch (Exception e) {
            e.printStackTrace();
        }
        
        // Create and show GUI
        SwingUtilities.invokeLater(() -> {
            StockPredictGUI gui = new StockPredictGUI();
            gui.setVisible(true);
            
            // Auto-load SPY data
            try {
                // Look for SPY data file in the data directory
                File dataDir = new File("data");
                if (dataDir.exists() && dataDir.isDirectory()) {
                    File[] files = dataDir.listFiles((dir, name) -> name.toLowerCase().startsWith("spy") && name.toLowerCase().endsWith(".csv"));
                    if (files != null && files.length > 0) {
                        // Sort files to get the most recent one
                        Arrays.sort(files, (f1, f2) -> f2.getName().compareTo(f1.getName()));
                        
                        // Load the most recent SPY data file
                        gui.loadStockDataFromFile(files[0]);
                        gui.updatePriceChart();
                        
                        // Run analysis
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