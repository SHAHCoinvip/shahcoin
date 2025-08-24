#ifndef NFT_FLOOR_PRICE_ALERTS_H
#define NFT_FLOOR_PRICE_ALERTS_H

#include <QObject>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QSlider>
#include <QProgressBar>
#include <QTableWidget>
#include <QHeaderView>
#include <QMenu>
#include <QAction>
#include <QChart>
#include <QChartView>
#include <QLineSeries>
#include <QBarSeries>
#include <QPieSeries>
#include <QValueAxis>
#include <QBarCategoryAxis>
#include <QDateTimeAxis>
#include <QSettings>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>
#include <QList>
#include <QMap>
#include <QString>
#include <QColor>
#include <QPoint>
#include <QSize>

// NFT Floor Price Alert Structures
struct NFTCollection {
    QString contractAddress;
    QString name;
    QString symbol;
    QString description;
    QString imageUrl;
    double currentFloorPrice;
    double previousFloorPrice;
    double change24h;
    double change7d;
    double change30d;
    int totalSupply;
    int holders;
    double volume24h;
    double volume7d;
    double volume30d;
    QString blockchain; // "Ethereum", "Polygon", "BSC", etc.
    QDateTime lastUpdated;
    bool isVerified;
    QString category; // "Art", "Gaming", "Music", "Sports", etc.
};

struct FloorPriceAlert {
    QString id;
    QString collectionAddress;
    QString collectionName;
    QString alertType; // "above", "below", "change_percent", "volume_spike"
    double targetPrice;
    double changePercent;
    double volumeThreshold;
    bool isActive;
    bool isTriggered;
    QDateTime createdAt;
    QDateTime lastTriggered;
    int triggerCount;
    QString notificationMethod; // "popup", "email", "push", "sound"
    QString description;
    double priority; // 0.0 to 1.0
};

struct AlertHistory {
    QString alertId;
    QString collectionAddress;
    QString collectionName;
    QString triggerReason;
    double oldPrice;
    double newPrice;
    double changePercent;
    QDateTime triggeredAt;
    bool wasAcknowledged;
    QString userNotes;
};

struct MarketData {
    QString collectionAddress;
    QDateTime timestamp;
    double floorPrice;
    double volume24h;
    int sales24h;
    double averagePrice;
    int uniqueBuyers;
    int uniqueSellers;
    double marketCap;
    double totalVolume;
};

// NFT Floor Price Alerts Manager Class
class NFTFloorPriceAlertsManager : public QObject
{
    Q_OBJECT

public:
    explicit NFTFloorPriceAlertsManager(QObject* parent = nullptr);
    ~NFTFloorPriceAlertsManager();

    // Core Alert Management
    void addAlert(const FloorPriceAlert& alert);
    void removeAlert(const QString& alertId);
    void updateAlert(const FloorPriceAlert& alert);
    void toggleAlert(const QString& alertId, bool active);
    
    // Collection Management
    void addCollection(const NFTCollection& collection);
    void removeCollection(const QString& contractAddress);
    void updateCollectionData(const QString& contractAddress);
    
    // Alert Checking
    void checkAllAlerts();
    void checkCollectionAlerts(const QString& collectionAddress);
    bool shouldTriggerAlert(const FloorPriceAlert& alert, const NFTCollection& collection);
    
    // Data Management
    void loadAlerts();
    void saveAlerts();
    void loadCollections();
    void saveCollections();
    void exportAlertHistory(const QString& filePath);
    
    // Settings Management
    void loadSettings();
    void saveSettings();
    
    // Getters and Setters
    bool isMonitoringEnabled() const { return m_monitoringEnabled; }
    int getCheckInterval() const { return m_checkInterval; }
    bool isSoundEnabled() const { return m_soundEnabled; }
    bool isPopupEnabled() const { return m_popupEnabled; }
    bool isEmailEnabled() const { return m_emailEnabled; }
    QString getDefaultNotificationMethod() const { return m_defaultNotificationMethod; }
    
    void setMonitoringEnabled(bool enabled);
    void setCheckInterval(int interval);
    void setSoundEnabled(bool enabled);
    void setPopupEnabled(bool enabled);
    void setEmailEnabled(bool enabled);
    void setDefaultNotificationMethod(const QString& method);
    
    QList<FloorPriceAlert> getAlerts() const { return m_alerts; }
    QList<NFTCollection> getCollections() const { return m_collections; }
    QList<AlertHistory> getAlertHistory() const { return m_alertHistory; }
    QList<MarketData> getMarketData(const QString& collectionAddress) const;

signals:
    void alertTriggered(const FloorPriceAlert& alert, const NFTCollection& collection, double oldPrice, double newPrice);
    void alertAdded(const FloorPriceAlert& alert);
    void alertRemoved(const QString& alertId);
    void alertUpdated(const FloorPriceAlert& alert);
    void collectionAdded(const NFTCollection& collection);
    void collectionRemoved(const QString& contractAddress);
    void collectionUpdated(const NFTCollection& collection);
    void monitoringStarted();
    void monitoringStopped();
    void settingsChanged();

private slots:
    void performPeriodicCheck();
    void onNetworkReplyFinished();
    void onPriceDataReceived(const QString& collectionAddress, double newPrice);

private:
    // Core Components
    QSettings m_settings;
    QTimer* m_checkTimer;
    QNetworkAccessManager* m_networkManager;
    
    // Settings
    bool m_monitoringEnabled;
    int m_checkInterval;
    bool m_soundEnabled;
    bool m_popupEnabled;
    bool m_emailEnabled;
    QString m_defaultNotificationMethod;
    
    // Data Storage
    QList<FloorPriceAlert> m_alerts;
    QList<NFTCollection> m_collections;
    QList<AlertHistory> m_alertHistory;
    QMap<QString, QList<MarketData>> m_marketData;
    
    // API Configuration
    QString m_apiBaseUrl;
    QString m_apiKey;
    QList<QString> m_supportedBlockchains;
    
    // Helper Methods
    void setupTimers();
    void setupNetworkMonitoring();
    void fetchCollectionData(const QString& contractAddress);
    void fetchFloorPrice(const QString& contractAddress);
    void processPriceUpdate(const QString& collectionAddress, double newPrice);
    void triggerAlert(const FloorPriceAlert& alert, const NFTCollection& collection, double oldPrice, double newPrice);
    void sendNotification(const FloorPriceAlert& alert, const NFTCollection& collection, const QString& message);
    void playAlertSound();
    void showAlertPopup(const QString& title, const QString& message);
    void sendEmailAlert(const QString& subject, const QString& body);
    QString generateAlertMessage(const FloorPriceAlert& alert, const NFTCollection& collection, double oldPrice, double newPrice);
    double calculatePriceChange(double oldPrice, double newPrice);
    bool isValidContractAddress(const QString& address);
    QString getBlockchainFromAddress(const QString& address);
};

// NFT Floor Price Alerts Widget
class NFTFloorPriceAlertsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit NFTFloorPriceAlertsWidget(NFTFloorPriceAlertsManager* manager, QWidget* parent = nullptr);
    ~NFTFloorPriceAlertsWidget();

    void refreshData();
    void showAlertDialog(const FloorPriceAlert& alert);

private slots:
    void onAddAlertClicked();
    void onAddCollectionClicked();
    void onSettingsClicked();
    void onExportClicked();
    void onRefreshClicked();
    void onAlertTriggered(const FloorPriceAlert& alert, const NFTCollection& collection, double oldPrice, double newPrice);
    void onAlertAdded(const FloorPriceAlert& alert);
    void onAlertRemoved(const QString& alertId);
    void onAlertUpdated(const FloorPriceAlert& alert);
    void onCollectionAdded(const NFTCollection& collection);
    void onCollectionRemoved(const QString& contractAddress);
    void onCollectionUpdated(const NFTCollection& collection);

private:
    // Core Components
    NFTFloorPriceAlertsManager* m_manager;
    
    // UI Components
    QVBoxLayout* m_mainLayout;
    QTabWidget* m_tabWidget;
    
    // Alerts Tab
    QWidget* m_alertsTab;
    QTableWidget* m_alertsTable;
    QPushButton* m_addAlertButton;
    QPushButton* m_editAlertButton;
    QPushButton* m_deleteAlertButton;
    QPushButton* m_toggleAlertButton;
    
    // Collections Tab
    QWidget* m_collectionsTab;
    QTableWidget* m_collectionsTable;
    QPushButton* m_addCollectionButton;
    QPushButton* m_refreshCollectionButton;
    QPushButton* m_removeCollectionButton;
    QChartView* m_floorPriceChart;
    
    // History Tab
    QWidget* m_historyTab;
    QTableWidget* m_historyTable;
    QPushButton* m_exportHistoryButton;
    QPushButton* m_clearHistoryButton;
    QChartView* m_historyChart;
    
    // Market Tab
    QWidget* m_marketTab;
    QChartView* m_volumeChart;
    QChartView* m_salesChart;
    QChartView* m_marketCapChart;
    
    // Control Panel
    QWidget* m_controlPanel;
    QPushButton* m_startMonitoringButton;
    QPushButton* m_stopMonitoringButton;
    QPushButton* m_settingsButton;
    QPushButton* m_exportButton;
    QLabel* m_statusLabel;
    QProgressBar* m_monitoringProgress;
    
    // Helper Methods
    void setupUI();
    void setupAlertsTab();
    void setupCollectionsTab();
    void setupHistoryTab();
    void setupMarketTab();
    void setupControlPanel();
    
    void updateAlertsTable();
    void updateCollectionsTable();
    void updateHistoryTable();
    void updateMarketCharts();
    void updateStatusDisplay();
    
    QChart* createFloorPriceChart(const QString& collectionAddress);
    QChart* createHistoryChart();
    QChart* createVolumeChart(const QString& collectionAddress);
    QChart* createSalesChart(const QString& collectionAddress);
    QChart* createMarketCapChart(const QString& collectionAddress);
    
    QString formatPrice(double price);
    QString formatPercentage(double percentage);
    QString formatNumber(int number);
    QString formatCurrency(double amount);
    QColor getChangeColor(double change);
    QColor getAlertPriorityColor(double priority);
};

// Add Alert Dialog
class AddAlertDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddAlertDialog(NFTFloorPriceAlertsManager* manager, QWidget* parent = nullptr);

private slots:
    void onCollectionChanged(const QString& collectionAddress);
    void onAlertTypeChanged(const QString& alertType);
    void onAddClicked();
    void onCancelClicked();

private:
    NFTFloorPriceAlertsManager* m_manager;
    
    // UI Components
    QVBoxLayout* m_layout;
    QComboBox* m_collectionCombo;
    QComboBox* m_alertTypeCombo;
    QLineEdit* m_targetPriceEdit;
    QLineEdit* m_changePercentEdit;
    QLineEdit* m_volumeThresholdEdit;
    QComboBox* m_notificationMethodCombo;
    QTextEdit* m_descriptionEdit;
    QSlider* m_prioritySlider;
    QLabel* m_priorityLabel;
    QPushButton* m_addButton;
    QPushButton* m_cancelButton;
    
    void setupUI();
    void populateCollections();
    void updateUI();
};

// Add Collection Dialog
class AddCollectionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddCollectionDialog(NFTFloorPriceAlertsManager* manager, QWidget* parent = nullptr);

private slots:
    void onAddClicked();
    void onCancelClicked();
    void onSearchClicked();

private:
    NFTFloorPriceAlertsManager* m_manager;
    
    // UI Components
    QVBoxLayout* m_layout;
    QLineEdit* m_contractAddressEdit;
    QLineEdit* m_nameEdit;
    QLineEdit* m_symbolEdit;
    QTextEdit* m_descriptionEdit;
    QComboBox* m_blockchainCombo;
    QComboBox* m_categoryCombo;
    QPushButton* m_searchButton;
    QPushButton* m_addButton;
    QPushButton* m_cancelButton;
    
    void setupUI();
    void searchCollection(const QString& contractAddress);
    void populateCollectionData(const NFTCollection& collection);
};

// Settings Dialog
class NFTAlertsSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NFTAlertsSettingsDialog(NFTFloorPriceAlertsManager* manager, QWidget* parent = nullptr);

private slots:
    void onMonitoringToggled(bool enabled);
    void onIntervalChanged(int seconds);
    void onSoundToggled(bool enabled);
    void onPopupToggled(bool enabled);
    void onEmailToggled(bool enabled);
    void onNotificationMethodChanged(const QString& method);
    void onOkClicked();
    void onCancelClicked();

private:
    NFTFloorPriceAlertsManager* m_manager;
    
    // UI Components
    QVBoxLayout* m_layout;
    QCheckBox* m_monitoringCheck;
    QSpinBox* m_intervalSpin;
    QCheckBox* m_soundCheck;
    QCheckBox* m_popupCheck;
    QCheckBox* m_emailCheck;
    QComboBox* m_notificationMethodCombo;
    QPushButton* m_okButton;
    QPushButton* m_cancelButton;
    
    void setupUI();
    void loadCurrentSettings();
};

#endif // NFT_FLOOR_PRICE_ALERTS_H
