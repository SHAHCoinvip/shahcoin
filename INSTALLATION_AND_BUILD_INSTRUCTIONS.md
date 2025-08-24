# SHAHCOIN Installation and Build Instructions

## Project Structure
```
/mnt/c/Users/hamid/shahcoin-core/
└── shahcoin/                    # Main project directory
    ├── src/                     # Core source code
    ├── doc/                     # Documentation
    ├── contrib/                 # Contrib scripts and tools
    ├── test/                    # Test files
    ├── build_msvc/              # Windows build files
    ├── sdk/                     # SDK packages
    ├── sdk-core/                # Core SDK modules
    ├── web/                     # Web components
    ├── api/                     # API services
    └── game-plugins/            # Game engine integrations
```

## Prerequisites

### Linux (Ubuntu/Debian)
```bash
# Install dependencies
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    libtool \
    autotools-dev \
    automake \
    pkg-config \
    bsdmainutils \
    curl \
    git \
    libevent-dev \
    libboost-all-dev \
    libssl-dev \
    libzmq3-dev \
    libminiupnpc-dev \
    python3 \
    ca-certificates \
    qt5-default \
    qtbase5-dev \
    qttools5-dev-tools
```

### macOS
```bash
# Install Homebrew if not already installed
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install dependencies
brew install \
    automake \
    berkeley-db4 \
    boost \
    libevent \
    libtool \
    miniupnpc \
    pkg-config \
    python3 \
    qt5 \
    zeromq
```

### Windows
```bash
# Install Visual Studio 2019 or later with C++ development tools
# Install Qt5 development environment
# Install MSYS2 for additional tools
```

## Building SHAHCOIN

### 1. Clone the Repository
```bash
git clone https://github.com/SHAHCoinvip/shahcoin
cd shahcoin
```

### 2. Generate Build Configuration
```bash
# Linux/macOS
./autogen.sh

# Windows (in MSYS2)
./autogen.sh
```

### 3. Configure the Build
```bash
# Basic configuration
./configure --enable-wallet --with-gui=qt5

# Full configuration with all features
./configure \
    --enable-wallet \
    --with-gui=qt5 \
    --enable-debug \
    --with-boost-libdir=/usr/lib/x86_64-linux-gnu \
    --prefix=/usr/local
```

### 4. Build the Project
```bash
# Build with all available cores
make -j$(nproc)

# Or specify number of cores (e.g., 4)
make -j4
```

### 5. Install (Optional)
```bash
# Install to system directories
sudo make install

# Or install to custom directory
make install DESTDIR=/opt/shahcoin
```

## Building Specific Components

### Qt GUI Wallet
```bash
# Build only the Qt wallet
cd src/qt
make shahcoin-qt
```

### Command Line Tools
```bash
# Build daemon
make shahcoind

# Build CLI
make shahcoin-cli

# Build transaction tool
make shahcoin-tx
```

### Development Build
```bash
# Enable debug symbols and development features
./configure \
    --enable-debug \
    --enable-wallet \
    --with-gui=qt5 \
    --enable-tests \
    --enable-bench
```

## Testing

### Run Unit Tests
```bash
# Run all tests
make check

# Run specific test suite
src/test/test_shahcoin

# Run Qt tests
src/qt/test/test_shahcoin-qt
```

### Run Functional Tests
```bash
# Run functional tests
test/functional/test_runner.py

# Run specific functional test
test/functional/test_runner.py wallet_basic
```

## Development Setup

### 1. Install Development Dependencies
```bash
# Install Node.js dependencies for web components
npm install

# Install Python dependencies for testing
pip3 install -r test/requirements.txt
```

### 2. Configure Development Environment
```bash
# Create development configuration
cp share/examples/shahcoin.conf ~/.shahcoin/

# Edit configuration for development
nano ~/.shahcoin/shahcoin.conf
```

### 3. Start Development Server
```bash
# Start web development server
npm run dev

# Start API development server
cd api/dev-api
npm run dev
```

## Docker Build

### Build Docker Image
```bash
# Build SHAHCOIN Docker image
docker build -t shahcoin:latest .

# Run SHAHCOIN in Docker
docker run -it --rm shahcoin:latest
```

### Docker Compose (Development)
```bash
# Start development environment
docker-compose up -d

# View logs
docker-compose logs -f
```

## Game Plugin Development

### Unity Plugin
```bash
# Build Unity plugin
cd game-plugins/unity-shahcoin
dotnet build

# Test Unity integration
cd ../unity
npm test
```

### Unreal Plugin
```bash
# Build Unreal plugin
cd game-plugins/unreal-shahcoin
# Follow Unreal Engine plugin build process
```

### Godot Plugin
```bash
# Build Godot plugin
cd game-plugins/godot-shahcoin
# Follow Godot plugin build process
```

## Troubleshooting

### Common Build Issues

#### 1. Missing Dependencies
```bash
# Linux: Install missing packages
sudo apt-get install -y [missing-package]

# macOS: Install via Homebrew
brew install [missing-package]

# Windows: Install via MSYS2
pacman -S [missing-package]
```

#### 2. Qt Issues
```bash
# Ensure Qt5 is properly installed
qmake --version

# Set Qt5 environment variables
export QT5DIR=/usr/lib/qt5
export PATH=$QT5DIR/bin:$PATH
```

#### 3. Boost Issues
```bash
# Check Boost installation
pkg-config --modversion boost

# Set Boost library path
export BOOST_LIBRARYDIR=/usr/lib/x86_64-linux-gnu
```

### Performance Optimization

#### Build Optimization
```bash
# Use all available cores
make -j$(nproc)

# Enable parallel compilation
export MAKEFLAGS="-j$(nproc)"

# Use ccache for faster rebuilds
export USE_CCACHE=1
export CCACHE_DIR=~/.ccache
```

#### Runtime Optimization
```bash
# Optimize for your CPU
./configure --enable-native-b2

# Enable specific optimizations
./configure --enable-asm --enable-avx2
```

## Deployment

### Production Build
```bash
# Clean build for production
make clean
make distclean

# Configure for production
./configure \
    --enable-wallet \
    --with-gui=qt5 \
    --disable-tests \
    --disable-bench \
    --disable-gui-tests \
    --prefix=/usr/local

# Build optimized binaries
make -j$(nproc)
```

### Package Creation
```bash
# Create distribution package
make dist

# Create Windows installer
cd build_msvc
make package
```

## Support

- **Documentation**: https://shah.vip/dev
- **GitHub Issues**: https://github.com/SHAHCoinvip/shahcoin/issues
- **Contact**: info@shah.vip

## License

This project is licensed under the Apache 2.0 License.

© 2025 Hamid Rad – All Rights Reserved. SHAHCOIN® is a registered trademark.

---

*Last Updated: 2025*
*SHAHCOIN® is a registered trademark of Hamid Rad*
