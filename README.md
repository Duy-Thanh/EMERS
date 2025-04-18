# StockPredict - Stock Analysis and Trading Signals

## Architecture Overview

StockPredict is a stock analysis application with a hybrid C/Java architecture:

- **C Backend**: Handles data fetching, cleaning, and preprocessing
- **Java GUI**: Performs data mining, visualization, and user interaction

## Key Components

### C Components
- **Data Fetching**: Downloads stock data from financial APIs
- **Preprocessing**: Cleans data, removes outliers, fills missing values
- **Date/Time Handling**: ISO 8601 timestamp parsing and conversion
- **Event Detection**: Market event monitoring and analysis

### Java Components
- **Data Mining**: Pattern recognition, signal detection, anomaly detection
- **GUI Interface**: Interactive charts, tables, and analysis views
- **Visualization**: Price charts, indicators, and analysis results
- **User Workflows**: Data loading, analysis execution, results review

## Setup and Running

1. **Build the C Backend**:
   ```
   cd src
   make
   ```

2. **Run the Java GUI**:
   ```
   ./run_gui.bat
   ```

## Data Directory
Stock data is stored in the `data` directory and is automatically detected by the GUI.

## Development Info
- C Code: Located in `src/` and `include/` directories
- Java GUI: Located in `src/gui/`
- Test files: Located in `test/` directory
- Data files: Located in `data/` directory

## License

This project is created for educational and research purposes only. Always consult with a financial advisor before making investment decisions based on any software analysis.

## Disclaimer

EMERS is a simulation and research tool. Financial markets are complex and unpredictable, and no software can guarantee accurate predictions or profitable trading strategies. Use at your own risk.