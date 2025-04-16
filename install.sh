#!/bin/bash
# EMERS Installation Script for Unix-based systems

echo "Emergency Market Event Response System (EMERS) Installation"
echo "=========================================================="

# Check if running as root
if [ "$EUID" -ne 0 ]; then
  echo "Please run as root for a system-wide installation, or use --local for a local installation"
  echo "Usage: sudo ./install.sh or ./install.sh --local"
  exit 1
fi

# Default installation settings
INSTALL_PREFIX="/usr/local"
BUILD_TYPE="Release"
LOCAL_INSTALL=0

# Process command line arguments
for arg in "$@"; do
  case $arg in
    --local)
      INSTALL_PREFIX="$HOME/.local"
      LOCAL_INSTALL=1
      ;;
    --debug)
      BUILD_TYPE="Debug"
      ;;
    --prefix=*)
      INSTALL_PREFIX="${arg#*=}"
      ;;
    *)
      echo "Unknown argument: $arg"
      echo "Usage: ./install.sh [--local] [--debug] [--prefix=<path>]"
      exit 1
      ;;
  esac
done

# Check dependencies
echo "Checking dependencies..."
MISSING_DEPS=0

check_command() {
  if ! command -v $1 &> /dev/null; then
    echo " - $1 not found!"
    MISSING_DEPS=1
  else
    echo " - $1 found: $(command -v $1)"
  fi
}

check_command gcc
check_command make
check_command curl

if [ $MISSING_DEPS -eq 1 ]; then
  echo "Please install the missing dependencies and try again."
  echo "On Debian/Ubuntu systems, run: sudo apt-get install build-essential curl"
  echo "On RedHat/Fedora systems, run: sudo dnf install gcc make curl"
  exit 1
fi

# Create build directory
echo "Creating build directory..."
mkdir -p build
cd build

# Build the project
echo "Building EMERS..."
make clean
CFLAGS="-DEMERS_$BUILD_TYPE" make

if [ $? -ne 0 ]; then
  echo "Build failed! Please check the error messages."
  exit 1
fi

# Create installation directories
echo "Creating installation directories..."
mkdir -p "$INSTALL_PREFIX/bin"
mkdir -p "$INSTALL_PREFIX/share/emers"
mkdir -p "$INSTALL_PREFIX/share/doc/emers"

# Install binary
echo "Installing EMERS binary..."
cp -f bin/emers "$INSTALL_PREFIX/bin/"

# Install documentation
echo "Installing documentation..."
cp -f ../README.md "$INSTALL_PREFIX/share/doc/emers/"

# Create configuration directory
if [ $LOCAL_INSTALL -eq 1 ]; then
  CONFIG_DIR="$HOME/.config/emers"
else
  CONFIG_DIR="/etc/emers"
fi

mkdir -p "$CONFIG_DIR"

# Create default configuration file
echo "Creating default configuration..."
cat > "$CONFIG_DIR/emers.conf" << EOF
# EMERS Configuration File

# API Settings
api_key = 
api_timeout = 30

# Data Settings
default_lookback_days = 180
cache_directory = ${CONFIG_DIR}/cache

# Logging Settings
log_level = info
log_file = ${CONFIG_DIR}/emers.log

# Model Settings
event_detection_threshold = 0.75
risk_confidence_level = 0.95
EOF

# Set appropriate permissions
if [ $LOCAL_INSTALL -eq 1 ]; then
  chmod 700 "$CONFIG_DIR"
  chmod 600 "$CONFIG_DIR/emers.conf"
else
  chmod 755 "$CONFIG_DIR"
  chmod 644 "$CONFIG_DIR/emers.conf"
fi

echo "Installation complete!"
echo "EMERS binary installed to: $INSTALL_PREFIX/bin/emers"
echo "Configuration file created at: $CONFIG_DIR/emers.conf"
echo "Please edit the configuration file to set your Tiingo API key."

# Create uninstall script
cat > uninstall.sh << EOF
#!/bin/bash
# EMERS Uninstall Script

echo "Uninstalling EMERS..."
rm -f "$INSTALL_PREFIX/bin/emers"
rm -rf "$INSTALL_PREFIX/share/emers"
rm -rf "$INSTALL_PREFIX/share/doc/emers"

if [ -d "$CONFIG_DIR" ]; then
  echo "Configuration directory found at: $CONFIG_DIR"
  echo "Do you want to remove it? (y/n)"
  read -n 1 -r
  echo
  if [[ \$REPLY =~ ^[Yy]$ ]]; then
    rm -rf "$CONFIG_DIR"
    echo "Configuration directory removed."
  else
    echo "Configuration directory preserved."
  fi
fi

echo "Uninstallation complete!"
EOF

chmod +x uninstall.sh
echo "Uninstall script created at: $(pwd)/uninstall.sh" 