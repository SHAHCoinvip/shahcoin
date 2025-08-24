# Shahcoin Qt Wallet - Vault Recovery Alerts Documentation

## Overview

The **Vault Recovery Alerts** system is a comprehensive wallet health monitoring and recovery solution designed to detect and resolve wallet.dat issues automatically. This feature provides proactive protection against wallet corruption, data loss, and accessibility problems.

## 🚨 Key Features

### **Automatic Health Monitoring**
- **Background Detection**: Continuously monitors wallet.dat file health
- **Smart Scheduling**: Performs checks at configurable intervals (default: 24 hours)
- **Startup Verification**: Automatically checks wallet health on application startup
- **Non-Intrusive**: Runs in background without disrupting user experience

### **Comprehensive Issue Detection**
- **Missing Wallet**: Detects when wallet.dat file is not found
- **File Corruption**: Identifies corrupted wallet files using Berkeley DB magic bytes
- **Version Issues**: Detects outdated wallet formats requiring updates
- **Lock Detection**: Identifies when wallet is locked by other processes
- **Accessibility**: Checks file permissions and read/write access

### **Recovery Options**
- **🔄 Restore from Backup**: Import wallet from backup file (.dat, .bak)
- **🔍 Rescan Wallet**: Trigger blockchain rescan for missing transactions
- **🆕 Create New Wallet**: Generate new wallet if recovery isn't possible
- **🔧 Repair Wallet**: Attempt automatic repair of corrupted files
- **⚠️ Ignore Warning**: Temporarily dismiss non-critical warnings

## 🔧 Technical Implementation

### **Architecture**
```
VaultRecoveryAlerts
├── WalletHealthChecker (Background Worker)
├── Detection Engine
│   ├── File Existence Check
│   ├── Corruption Detection
│   ├── Version Validation
│   └── Lock Detection
├── Recovery Engine
│   ├── Backup Restoration
│   ├── Wallet Rescan
│   ├── New Wallet Creation
│   └── File Repair
└── UI Components
    ├── Status Display
    ├── Recovery Options
    └── Progress Tracking
```

### **Detection Methods**

#### **File Existence Check**
```cpp
bool checkWalletFileExists(const QString& walletPath)
{
    QFileInfo fileInfo(walletPath);
    return fileInfo.exists() && fileInfo.isFile();
}
```

#### **Corruption Detection**
```cpp
bool checkWalletFileCorruption(const QString& walletPath)
{
    // Check file size (minimum 4KB)
    if (file.size() < 4096) return true;
    
    // Verify Berkeley DB magic bytes
    // Big endian: 00 05 31 62
    // Little endian: 62 31 05 00
    uint32_t magic = 0;
    memcpy(&magic, header.data() + 12, 4);
    return (magic != 0x00053162 && magic != 0x62310500);
}
```

#### **Lock Detection**
```cpp
bool checkWalletFileLock(const QString& walletPath)
{
    QFile file(walletPath);
    if (!file.open(QIODevice::ReadWrite)) {
        return true; // File is locked or inaccessible
    }
    file.close();
    return false;
}
```

### **Recovery Procedures**

#### **Backup Restoration**
1. **File Selection**: User selects backup file (.dat, .bak)
2. **Current Backup**: Creates backup of existing wallet (if any)
3. **File Copy**: Copies backup to wallet location
4. **Verification**: Validates restored wallet integrity

#### **Wallet Rescan**
1. **Trigger Rescan**: Initiates blockchain rescan
2. **Progress Tracking**: Shows rescan progress
3. **Transaction Recovery**: Rebuilds transaction history
4. **Balance Update**: Updates wallet balances

#### **New Wallet Creation**
1. **Confirmation**: User confirms new wallet creation
2. **Backup Creation**: Creates backup of existing wallet
3. **New Wallet**: Generates fresh wallet.dat
4. **Setup Wizard**: Guides through initial setup

## 🎯 User Interface

### **Main Dialog**
- **Status Display**: Clear visual indication of wallet health
- **Detailed Information**: Expandable details about detected issues
- **Recovery Options**: Context-sensitive action buttons
- **Progress Tracking**: Real-time operation status

### **Status Indicators**
- **✅ Healthy**: Wallet is functioning normally
- **❌ Missing**: Wallet file not found
- **⚠️ Corrupted**: Wallet file is damaged
- **🔄 Outdated**: Wallet needs rescan
- **🔒 Locked**: Wallet is in use by another process

### **Recovery Buttons**
- **🔄 Restore from Backup**: Import from backup file
- **🔍 Rescan Wallet**: Rebuild transaction history
- **🆕 Create New Wallet**: Generate new wallet
- **🔧 Repair Wallet**: Attempt automatic repair
- **⚠️ Ignore Warning**: Dismiss current warning

## ⚙️ Configuration

### **Settings**
```ini
[VaultRecoveryAlerts]
AutoCheckEnabled=true
CheckIntervalHours=24
LastCheckTime=2024-01-01T12:00:00
```

### **Automatic Check Schedule**
- **Default Interval**: 24 hours
- **Startup Check**: 5 seconds after application launch
- **Configurable**: User can adjust check frequency
- **Smart Timing**: Avoids checks during active operations

## 🔒 Security Features

### **Safe Recovery**
- **Backup Creation**: Always creates backup before modifications
- **File Validation**: Verifies file integrity after operations
- **Permission Checks**: Ensures proper file access rights
- **Process Safety**: Prevents conflicts with running operations

### **Data Protection**
- **No Private Key Exposure**: Recovery operations don't expose sensitive data
- **Secure File Handling**: Uses Qt's secure file operations
- **Error Handling**: Graceful failure with detailed error messages
- **Audit Trail**: Logs all recovery operations

## 📊 Integration

### **Main GUI Integration**
```cpp
// Automatic health monitoring
void ShahcoinGUI::setupWalletHealthMonitoring()
{
    if (VaultRecoveryAlerts::shouldCheckWalletHealth()) {
        QTimer::singleShot(5000, this, [this]() {
            VaultRecoveryAlerts::WalletStatus status = 
                VaultRecoveryAlerts::detectWalletStatus();
            if (status != VaultRecoveryAlerts::StatusOK) {
                openVaultRecoveryAlerts();
            }
        });
    }
}
```

### **Menu Integration**
- **Tools Menu**: "🚨 Vault Recovery Alerts" option
- **Keyboard Shortcut**: Configurable hotkey access
- **Status Bar**: Optional health indicator
- **Tray Icon**: Notification for critical issues

### **Signal Integration**
```cpp
// Connect recovery completion signals
connect(&dlg, &VaultRecoveryAlerts::walletRecoveryCompleted, 
        this, [this](bool success, const QString& message) {
    if (success) {
        walletFrame->currentWalletModel()->refresh();
    }
});
```

## 🚀 Usage Scenarios

### **Scenario 1: Missing Wallet**
**Problem**: User's wallet.dat file is missing
**Detection**: System detects file absence
**Recovery**: User selects "Restore from Backup" or "Create New Wallet"
**Result**: Wallet restored or new wallet created

### **Scenario 2: Corrupted Wallet**
**Problem**: Wallet.dat file is corrupted due to incomplete shutdown
**Detection**: System detects invalid Berkeley DB magic bytes
**Recovery**: User selects "Restore from Backup" or "Repair Wallet"
**Result**: Wallet restored from backup or repaired

### **Scenario 3: Outdated Wallet**
**Problem**: Wallet needs rescan after blockchain sync
**Detection**: System detects missing transaction data
**Recovery**: User selects "Rescan Wallet"
**Result**: Transaction history rebuilt

### **Scenario 4: Locked Wallet**
**Problem**: Another Shahcoin instance is using the wallet
**Detection**: System detects file lock
**Recovery**: User closes other instances or selects "Repair Wallet"
**Result**: Wallet becomes accessible

## 🔧 Advanced Features

### **Background Processing**
- **Threaded Operations**: All checks run in background threads
- **Non-Blocking UI**: User interface remains responsive
- **Progress Updates**: Real-time operation status
- **Error Recovery**: Graceful handling of operation failures

### **Smart Detection**
- **Multiple Wallet Support**: Handles multiple wallet files
- **Directory Scanning**: Automatically finds wallet locations
- **Version Compatibility**: Checks wallet format compatibility
- **Cross-Platform**: Works on Windows, macOS, and Linux

### **Customization**
- **Check Intervals**: Configurable health check frequency
- **Auto-Recovery**: Optional automatic recovery for minor issues
- **Notification Settings**: Customizable alert preferences
- **Logging**: Detailed operation logging for troubleshooting

## 📈 Performance Considerations

### **Optimization**
- **Lazy Loading**: Only loads components when needed
- **Caching**: Caches detection results to avoid repeated checks
- **Resource Management**: Efficient memory and CPU usage
- **Background Processing**: Non-blocking operations

### **Monitoring**
- **Check Frequency**: Configurable to balance security and performance
- **Resource Usage**: Minimal impact on system resources
- **Startup Time**: Fast initialization without blocking startup
- **Memory Footprint**: Efficient memory usage

## 🔮 Future Enhancements

### **Planned Features**
- **Cloud Backup Integration**: Automatic cloud backup verification
- **Advanced Repair**: More sophisticated corruption repair algorithms
- **Predictive Analysis**: AI-powered issue prediction
- **Remote Recovery**: Network-based recovery options

### **Integration Opportunities**
- **Blockchain Explorer**: Integration with blockchain data
- **Hardware Wallet**: Support for hardware wallet recovery
- **Multi-Signature**: Recovery for multi-signature wallets
- **Enterprise Features**: Advanced enterprise recovery options

## 📝 Troubleshooting

### **Common Issues**

#### **False Positives**
- **Solution**: Adjust detection sensitivity in settings
- **Workaround**: Use "Ignore Warning" for known false positives
- **Prevention**: Regular backup creation

#### **Recovery Failures**
- **Diagnosis**: Check detailed error messages
- **Solution**: Try alternative recovery methods
- **Support**: Contact support with error logs

#### **Performance Impact**
- **Optimization**: Adjust check intervals
- **Disable**: Temporarily disable automatic checks
- **Schedule**: Run checks during low-usage periods

### **Log Files**
- **Location**: Application data directory
- **Format**: Structured JSON logs
- **Content**: Operation details and error information
- **Retention**: Configurable log retention period

## 📚 API Reference

### **Main Class: VaultRecoveryAlerts**
```cpp
class VaultRecoveryAlerts : public QDialog
{
    Q_OBJECT
public:
    enum WalletStatus { StatusOK, StatusMissing, StatusCorrupted, 
                       StatusOutdated, StatusLocked, StatusUnknown };
    enum RecoveryAction { RestoreFromBackup, RescanWallet, 
                         CreateNewWallet, RepairWallet, IgnoreWarning };
    
    // Main methods
    void checkWalletHealth();
    void showRecoveryDialog(WalletStatus status, const QString& details);
    void performRecoveryAction(RecoveryAction action);
    
    // Static methods
    static bool shouldCheckWalletHealth();
    static void setLastHealthCheck();
    static WalletStatus detectWalletStatus(const QString& walletPath = QString());
    
signals:
    void walletRecoveryCompleted(bool success, const QString& message);
    void walletHealthCheckCompleted(WalletStatus status, const QString& details);
};
```

### **Background Worker: WalletHealthChecker**
```cpp
class WalletHealthChecker : public QObject
{
    Q_OBJECT
public:
    static VaultRecoveryAlerts::WalletStatus checkWalletHealth(const QString& walletPath);
    
signals:
    void healthCheckCompleted(VaultRecoveryAlerts::WalletStatus status, const QString& details);
    
public slots:
    void performHealthCheck(const QString& walletPath);
};
```

## 🎯 Conclusion

The **Vault Recovery Alerts** system provides comprehensive wallet health monitoring and recovery capabilities, ensuring users can quickly identify and resolve wallet issues. With automatic detection, multiple recovery options, and seamless integration, this feature significantly enhances wallet reliability and user confidence.

The system is designed to be:
- **Proactive**: Detects issues before they become critical
- **Comprehensive**: Covers all common wallet problems
- **User-Friendly**: Simple interface with clear recovery options
- **Secure**: Protects user data during recovery operations
- **Efficient**: Minimal performance impact with background processing

This feature represents a significant advancement in wallet security and reliability, providing users with the tools they need to maintain healthy wallets and recover from issues quickly and safely.
