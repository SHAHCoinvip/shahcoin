@echo off
REM SHAHCOIN Mainnet Build Script for Windows
REM Finalizes the SHAHCOIN blockchain for mainnet launch
REM Project: /mnt/c/Users/hamid/shahcoin-core/shahcoin

echo 🚀 SHAHCOIN Mainnet Build Finalization
echo ======================================

REM Clean previous builds
echo 🧹 Cleaning previous builds...
if exist Makefile (
    make clean
    make distclean
)

REM Configure for mainnet-only build
echo ⚙️  Configuring for mainnet-only build...
if exist autogen.sh (
    bash autogen.sh
) else (
    echo Using existing configure script
)

REM Configure build
if exist configure (
    configure --enable-wallet --with-gui=qt5 --disable-tests --disable-bench --disable-gui-tests --prefix=/usr/local
) else (
    echo Configure script not found, using default configuration
)

REM Build the binaries
echo 🔨 Building SHAHCOIN binaries...
if exist Makefile (
    make -j%NUMBER_OF_PROCESSORS%
) else (
    echo Makefile not found, build configuration failed
    exit /b 1
)

REM Verify binaries
echo ✅ Verifying binaries...
if exist "src\shahcoind.exe" (
    echo ✅ shahcoind built successfully
) else (
    echo ❌ shahcoind build failed
    exit /b 1
)

if exist "src\qt\shahcoin-qt.exe" (
    echo ✅ shahcoin-qt built successfully
) else (
    echo ❌ shahcoin-qt build failed
    exit /b 1
)

if exist "src\shahcoin-cli.exe" (
    echo ✅ shahcoin-cli built successfully
) else (
    echo ❌ shahcoin-cli build failed
    exit /b 1
)

echo.
echo 🎉 SHAHCOIN Mainnet Build Complete!
echo ==================================
echo Binaries ready for mainnet launch:
echo - shahcoind.exe (daemon)
echo - shahcoin-qt.exe (GUI wallet)
echo - shahcoin-cli.exe (command line interface)
echo.
echo Next steps:
echo 1. Test genesis block validation
echo 2. Import premine private key
echo 3. Verify 8M SHAH balance
echo 4. Test staking functionality
echo 5. Launch mainnet
echo.
pause
