// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Hamid Rad / SHAHCOIN Core

#ifndef SMARTUNITDISPLAY_H
#define SMARTUNITDISPLAY_H

#include <QWidget>
#include <QLabel>
#include <QString>
#include <QMap>
#include <QCheckBox>
#include <QComboBox>
#include <QToolTip>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QDialog>

class SmartUnitDisplay : public QObject
{
    Q_OBJECT

public:
    // Unit types
    enum UnitType {
        Shahi,          // Base unit (1 SHAH = 100,000,000 Shahi)
        NanoShah,       // 10 Shahi
        MicroShah,      // 100 Shahi
        MesinShah,      // 1,000 Shahi
        SiminShah,      // 10,000 Shahi
        ZarrinShah,     // 100,000 Shahi
        Shahcent,       // 1,000,000 Shahi
        DaricShah,      // 10,000,000 Shahi
        SHAH,           // 100,000,000 Shahi
        MetaShah,       // 10 SHAH
        BetaShah,       // 100 SHAH
        SigmaShah,      // 1,000 SHAH
        MegaShah,       // 10,000 SHAH
        GigaShah,       // 100,000 SHAH
        AlphaShah       // 1,000,000 SHAH
    };

    // Unit information structure
    struct UnitInfo {
        QString name;
        QString displayName;
        QString icon;
        qint64 shahiValue;
        QString description;
        QString color;
    };

    // Display mode
    enum DisplayMode {
        FullSHAH,       // Show full SHAH value
        SmartUnit,      // Show smart unit breakdown
        USDEquivalent   // Show USD equivalent
    };

    explicit SmartUnitDisplay(QObject *parent = nullptr);
    ~SmartUnitDisplay();

    // Core conversion methods
    QString formatBalance(qint64 shahiAmount, DisplayMode mode = SmartUnit) const;
    QString getSmartUnitBreakdown(qint64 shahiAmount) const;
    QString getFullSHAHValue(qint64 shahiAmount) const;
    QString getUSDEquivalent(qint64 shahiAmount, double usdPrice = 0.0) const;

    // Unit information
    UnitInfo getUnitInfo(UnitType type) const;
    QMap<UnitType, UnitInfo> getAllUnits() const;
    QString getUnitIcon(UnitType type) const;
    QString getUnitColor(UnitType type) const;

    // Settings management
    bool isSmartUnitEnabled() const;
    void setSmartUnitEnabled(bool enabled);
    DisplayMode getDisplayMode() const;
    void setDisplayMode(DisplayMode mode);

    // UI components
    QWidget* createSettingsWidget();
    QWidget* createUnitExplorerDialog();
    QLabel* createBalanceLabel(qint64 shahiAmount);
    QString createTooltip(qint64 shahiAmount) const;

    // Utility methods
    qint64 getShahiValue(UnitType type) const;
    UnitType getBestUnit(qint64 shahiAmount) const;
    QString formatNumber(qint64 value) const;

signals:
    void smartUnitToggled(bool enabled);
    void displayModeChanged(DisplayMode mode);

private slots:
    void onSmartUnitToggled(bool checked);
    void onDisplayModeChanged(int index);
    void showUnitExplorer();

private:
    void initializeUnits();
    void setupUI();
    QString getSubUnitBreakdown(qint64 shahiAmount) const;
    QString getMegaUnitBreakdown(qint64 shahiAmount) const;
    QString createUnitIconSVG(UnitType type) const;

    QMap<UnitType, UnitInfo> m_units;
    bool m_smartUnitEnabled;
    DisplayMode m_displayMode;
    double m_usdPrice;
};

#endif // SMARTUNITDISPLAY_H
