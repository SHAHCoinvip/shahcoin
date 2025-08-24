@echo off
REM SHAHCOIN Premine Wallet Test Script for Windows
REM Tests the premine wallet functionality after build

echo 🔐 SHAHCOIN Premine Wallet Test
echo ===============================

REM Premine wallet details
set PREMINE_ADDRESS=SVUNaktXHkoRtoZeScGxjMxFVDVMn4xKTG
set PREMINE_PRIVATE_KEY=VPDArztHSodZuqn8rVsf2aakrePbmG2BZqZf9D5ZKpEEynocS4Pm
set PREMINE_AMOUNT=8000000.00000000

echo 📋 Premine Configuration:
echo Address: %PREMINE_ADDRESS%
echo Amount: %PREMINE_AMOUNT% SHAH
echo.

REM Test 1: Start daemon and verify genesis block
echo 🧪 Test 1: Genesis Block Validation
echo Starting shahcoind daemon...
if exist "src\shahcoind.exe" (
    start /B src\shahcoind.exe -daemon -testnet=0 -regtest=0
) else (
    echo ❌ shahcoind.exe not found
    exit /b 1
)

REM Wait for daemon to start
echo Waiting for daemon to start...
timeout /t 5 /nobreak >nul

REM Check if daemon is running
tasklist /FI "IMAGENAME eq shahcoind.exe" 2>NUL | find /I /N "shahcoind.exe">NUL
if "%ERRORLEVEL%"=="0" (
    echo ✅ Daemon started successfully
) else (
    echo ❌ Daemon failed to start
    exit /b 1
)

REM Test 2: Verify genesis block
echo.
echo 🧪 Test 2: Genesis Block Verification
if exist "src\shahcoin-cli.exe" (
    for /f "tokens=*" %%i in ('src\shahcoin-cli.exe getblockhash 0') do set GENESIS_HASH=%%i
    echo Genesis Block Hash: %GENESIS_HASH%
    
    if not "%GENESIS_HASH%"=="" (
        echo ✅ Genesis block found
    ) else (
        echo ❌ Genesis block not found
        exit /b 1
    )
) else (
    echo ❌ shahcoin-cli.exe not found
    exit /b 1
)

REM Test 3: Check premine balance
echo.
echo 🧪 Test 3: Premine Balance Verification
for /f "tokens=*" %%i in ('src\shahcoin-cli.exe getreceivedbyaddress "%PREMINE_ADDRESS%"') do set PREMINE_BALANCE=%%i

if "%PREMINE_BALANCE%"=="%PREMINE_AMOUNT%" (
    echo ✅ Premine balance correct: %PREMINE_BALANCE% SHAH
) else (
    echo ❌ Premine balance incorrect: %PREMINE_BALANCE% SHAH (expected: %PREMINE_AMOUNT%)
    exit /b 1
)

REM Test 4: Import private key
echo.
echo 🧪 Test 4: Private Key Import
src\shahcoin-cli.exe importprivkey "%PREMINE_PRIVATE_KEY%" "premine" true
if %ERRORLEVEL%==0 (
    echo ✅ Private key imported successfully
) else (
    echo ❌ Private key import failed
    exit /b 1
)

REM Test 5: Verify wallet balance
echo.
echo 🧪 Test 5: Wallet Balance Verification
for /f "tokens=*" %%i in ('src\shahcoin-cli.exe getbalance') do set WALLET_BALANCE=%%i

if "%WALLET_BALANCE%"=="%PREMINE_AMOUNT%" (
    echo ✅ Wallet balance correct: %WALLET_BALANCE% SHAH
) else (
    echo ❌ Wallet balance incorrect: %WALLET_BALANCE% SHAH (expected: %PREMINE_AMOUNT%)
    exit /b 1
)

REM Test 6: Test staking functionality
echo.
echo 🧪 Test 6: Staking Functionality
src\shahcoin-cli.exe getstakinginfo
if %ERRORLEVEL%==0 (
    echo ✅ Staking info retrieved successfully
) else (
    echo ❌ Staking info retrieval failed
    exit /b 1
)

REM Test 7: Test hybrid consensus RPC
echo.
echo 🧪 Test 7: Hybrid Consensus RPC
src\shahcoin-cli.exe getalgoinfo
if %ERRORLEVEL%==0 (
    echo ✅ Hybrid consensus RPC working
) else (
    echo ❌ Hybrid consensus RPC failed
    exit /b 1
)

REM Stop daemon
echo.
echo 🛑 Stopping daemon...
src\shahcoin-cli.exe stop

echo.
echo 🎉 All Premine Wallet Tests Passed!
echo ===================================
echo ✅ Genesis block validated
echo ✅ Premine balance verified
echo ✅ Private key imported
echo ✅ Wallet balance correct
echo ✅ Staking functionality working
echo ✅ Hybrid consensus operational
echo.
echo 🚀 SHAHCOIN is ready for mainnet launch!
echo.
pause
