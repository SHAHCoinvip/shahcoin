// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Hamid Rad / SHAHCOIN Core

#include "smartunitdisplay.h"
#include <QApplication>
#include <QStyle>
#include <QMessageBox>
#include <QTextEdit>
#include <QScrollArea>
#include <QGridLayout>

SmartUnitDisplay::SmartUnitDisplay(QObject *parent)
    : QObject(parent)
    , m_smartUnitEnabled(true)
    , m_displayMode(SmartUnit)
    , m_usdPrice(0.0)
{
    initializeUnits();
    setupUI();
}

SmartUnitDisplay::~SmartUnitDisplay()
{
}

void SmartUnitDisplay::initializeUnits()
{
    // Sub-Units (below 1 SHAH)
    m_units[Shahi] = {"shahi", "Shahi", createUnitIconSVG(Shahi), 1, "Base unit (like satoshi)", "#6c757d"};
    m_units[NanoShah] = {"nano", "Nano Shah", createUnitIconSVG(NanoShah), 10, "10 Shahi", "#007bff"};
    m_units[MicroShah] = {"micro", "Micro Shah", createUnitIconSVG(MicroShah), 100, "100 Shahi", "#6f42c1"};
    m_units[MesinShah] = {"mesin", "Mesin Shah", createUnitIconSVG(MesinShah), 1000, "1,000 Shahi", "#fd7e14"};
    m_units[SiminShah] = {"simin", "Simin Shah", createUnitIconSVG(SiminShah), 10000, "10,000 Shahi", "#20c997"};
    m_units[ZarrinShah] = {"zarrin", "Zarrin Shah", createUnitIconSVG(ZarrinShah), 100000, "100,000 Shahi", "#ffc107"};
    m_units[Shahcent] = {"shahcent", "Shahcent", createUnitIconSVG(Shahcent), 1000000, "1,000,000 Shahi", "#dc3545"};
    m_units[DaricShah] = {"daric", "Daric Shah", createUnitIconSVG(DaricShah), 10000000, "10,000,000 Shahi", "#28a745"};
    m_units[SHAH] = {"shah", "SHAH", createUnitIconSVG(SHAH), 100000000, "100,000,000 Shahi", "#17a2b8"};

    // Mega-Units (above 1 SHAH)
    m_units[MetaShah] = {"meta", "Meta Shah", createUnitIconSVG(MetaShah), 1000000000, "10 SHAH", "#6f42c1"};
    m_units[BetaShah] = {"beta", "Beta Shah", createUnitIconSVG(BetaShah), 10000000000, "100 SHAH", "#fd7e14"};
    m_units[SigmaShah] = {"sigma", "Sigma Shah", createUnitIconSVG(SigmaShah), 100000000000, "1,000 SHAH", "#20c997"};
    m_units[MegaShah] = {"mega", "Mega Shah", createUnitIconSVG(MegaShah), 1000000000000, "10,000 SHAH", "#ffc107"};
    m_units[GigaShah] = {"giga", "Giga Shah", createUnitIconSVG(GigaShah), 10000000000000, "100,000 SHAH", "#dc3545"};
    m_units[AlphaShah] = {"alpha", "Alpha Shah", createUnitIconSVG(AlphaShah), 100000000000000, "1,000,000 SHAH", "#28a745"};
}

QString SmartUnitDisplay::formatBalance(qint64 shahiAmount, DisplayMode mode) const
{
    switch (mode) {
        case FullSHAH:
            return getFullSHAHValue(shahiAmount);
        case SmartUnit:
            return getSmartUnitBreakdown(shahiAmount);
        case USDEquivalent:
            return getUSDEquivalent(shahiAmount, m_usdPrice);
        default:
            return getSmartUnitBreakdown(shahiAmount);
    }
}

QString SmartUnitDisplay::getSmartUnitBreakdown(qint64 shahiAmount) const
{
    if (shahiAmount < 100000000) {
        // Below 1 SHAH - use sub-units
        return getSubUnitBreakdown(shahiAmount);
    } else {
        // Above 1 SHAH - use mega-units
        return getMegaUnitBreakdown(shahiAmount);
    }
}

QString SmartUnitDisplay::getSubUnitBreakdown(qint64 shahiAmount) const
{
    if (shahiAmount == 0) return "0 Shahi";

    QStringList parts;
    qint64 remaining = shahiAmount;

    // Start from largest sub-unit and work down
    QList<UnitType> subUnits = {DaricShah, Shahcent, ZarrinShah, SiminShah, MesinShah, MicroShah, NanoShah, Shahi};
    
    for (UnitType unit : subUnits) {
        qint64 unitValue = getShahiValue(unit);
        if (remaining >= unitValue) {
            qint64 count = remaining / unitValue;
            remaining = remaining % unitValue;
            
            QString unitName = getUnitInfo(unit).displayName;
            parts.append(QString("%1 %2").arg(count).arg(unitName));
        }
    }

    return parts.join(" + ");
}

QString SmartUnitDisplay::getMegaUnitBreakdown(qint64 shahiAmount) const
{
    if (shahiAmount < 100000000) return getSubUnitBreakdown(shahiAmount);

    qint64 shahAmount = shahiAmount / 100000000;
    qint64 remainingShahi = shahiAmount % 100000000;

    QStringList parts;

    // Handle mega-units
    QList<UnitType> megaUnits = {AlphaShah, GigaShah, MegaShah, SigmaShah, BetaShah, MetaShah};
    
    for (UnitType unit : megaUnits) {
        qint64 unitValue = getShahiValue(unit) / 100000000; // Convert to SHAH
        if (shahAmount >= unitValue) {
            qint64 count = shahAmount / unitValue;
            shahAmount = shahAmount % unitValue;
            
            QString unitName = getUnitInfo(unit).displayName;
            parts.append(QString("%1 %2").arg(formatNumber(count)).arg(unitName));
        }
    }

    // Add remaining SHAH if any
    if (shahAmount > 0) {
        parts.append(QString("%1 SHAH").arg(formatNumber(shahAmount)));
    }

    // Add remaining Shahi if any
    if (remainingShahi > 0) {
        parts.append(getSubUnitBreakdown(remainingShahi));
    }

    return parts.join(" + ");
}

QString SmartUnitDisplay::getFullSHAHValue(qint64 shahiAmount) const
{
    double shahValue = static_cast<double>(shahiAmount) / 100000000.0;
    return QString("%1 SHAH").arg(QString::number(shahValue, 'f', 8));
}

QString SmartUnitDisplay::getUSDEquivalent(qint64 shahiAmount, double usdPrice) const
{
    if (usdPrice <= 0.0) {
        return getSmartUnitBreakdown(shahiAmount) + " (USD price not available)";
    }
    
    double shahValue = static_cast<double>(shahiAmount) / 100000000.0;
    double usdValue = shahValue * usdPrice;
    
    return QString("$%1 USD").arg(QString::number(usdValue, 'f', 2));
}

SmartUnitDisplay::UnitInfo SmartUnitDisplay::getUnitInfo(UnitType type) const
{
    return m_units.value(type);
}

QMap<SmartUnitDisplay::UnitType, SmartUnitDisplay::UnitInfo> SmartUnitDisplay::getAllUnits() const
{
    return m_units;
}

QString SmartUnitDisplay::getUnitIcon(UnitType type) const
{
    return m_units.value(type).icon;
}

QString SmartUnitDisplay::getUnitColor(UnitType type) const
{
    return m_units.value(type).color;
}

bool SmartUnitDisplay::isSmartUnitEnabled() const
{
    return m_smartUnitEnabled;
}

void SmartUnitDisplay::setSmartUnitEnabled(bool enabled)
{
    if (m_smartUnitEnabled != enabled) {
        m_smartUnitEnabled = enabled;
        emit smartUnitToggled(enabled);
    }
}

SmartUnitDisplay::DisplayMode SmartUnitDisplay::getDisplayMode() const
{
    return m_displayMode;
}

void SmartUnitDisplay::setDisplayMode(DisplayMode mode)
{
    if (m_displayMode != mode) {
        m_displayMode = mode;
        emit displayModeChanged(mode);
    }
}

QWidget* SmartUnitDisplay::createSettingsWidget()
{
    QWidget* widget = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(widget);

    // Smart Unit Toggle
    QGroupBox* smartUnitGroup = new QGroupBox("🧠 Smart Unit Display");
    QVBoxLayout* smartUnitLayout = new QVBoxLayout(smartUnitGroup);

    QCheckBox* smartUnitCheck = new QCheckBox("Enable Smart Unit Mode");
    smartUnitCheck->setChecked(m_smartUnitEnabled);
    smartUnitCheck->setToolTip("Display balances using named units (e.g., '1 Sigma Shah' instead of '1,000 SHAH')");
    
    connect(smartUnitCheck, &QCheckBox::toggled, this, &SmartUnitDisplay::onSmartUnitToggled);
    
    smartUnitLayout->addWidget(smartUnitCheck);

    // Display Mode Selection
    QGroupBox* displayModeGroup = new QGroupBox("Display Mode");
    QVBoxLayout* displayModeLayout = new QVBoxLayout(displayModeGroup);

    QComboBox* displayModeCombo = new QComboBox();
    displayModeCombo->addItem("Smart Unit Mode", SmartUnit);
    displayModeCombo->addItem("Full SHAH Value", FullSHAH);
    displayModeCombo->addItem("USD Equivalent", USDEquivalent);
    displayModeCombo->setCurrentIndex(static_cast<int>(m_displayMode));
    
    connect(displayModeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &SmartUnitDisplay::onDisplayModeChanged);

    displayModeLayout->addWidget(displayModeCombo);

    // Unit Explorer Button
    QPushButton* explorerButton = new QPushButton("🔎 SHAH Unit Explorer");
    explorerButton->setToolTip("Learn about all SHAH units and their values");
    connect(explorerButton, &QPushButton::clicked, this, &SmartUnitDisplay::showUnitExplorer);

    layout->addWidget(smartUnitGroup);
    layout->addWidget(displayModeGroup);
    layout->addWidget(explorerButton);
    layout->addStretch();

    return widget;
}

QWidget* SmartUnitDisplay::createUnitExplorerDialog()
{
    QDialog* dialog = new QDialog();
    dialog->setWindowTitle("🔎 SHAH Unit Explorer");
    dialog->setModal(true);
    dialog->resize(800, 600);

    QVBoxLayout* layout = new QVBoxLayout(dialog);

    // Title
    QLabel* titleLabel = new QLabel("SHAH Smart Unit System");
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; margin-bottom: 10px;");
    layout->addWidget(titleLabel);

    // Description
    QLabel* descLabel = new QLabel("SHAH uses a hierarchical unit system for better readability and user experience.");
    descLabel->setWordWrap(true);
    descLabel->setStyleSheet("color: #666; margin-bottom: 20px;");
    layout->addWidget(descLabel);

    // Scrollable content
    QScrollArea* scrollArea = new QScrollArea();
    QWidget* scrollWidget = new QWidget();
    QVBoxLayout* scrollLayout = new QVBoxLayout(scrollWidget);

    // Sub-Units Section
    QGroupBox* subUnitsGroup = new QGroupBox("Sub-Units (Below 1 SHAH)");
    QGridLayout* subUnitsLayout = new QGridLayout(subUnitsGroup);

    int row = 0;
    QList<UnitType> subUnits = {Shahi, NanoShah, MicroShah, MesinShah, SiminShah, ZarrinShah, Shahcent, DaricShah};
    
    for (UnitType unit : subUnits) {
        UnitInfo info = getUnitInfo(unit);
        
        QLabel* iconLabel = new QLabel();
        iconLabel->setText(info.icon);
        iconLabel->setStyleSheet("font-size: 20px;");
        
        QLabel* nameLabel = new QLabel(info.displayName);
        nameLabel->setStyleSheet("font-weight: bold; color: " + info.color + ";");
        
        QLabel* valueLabel = new QLabel(QString("= %1 Shahi").arg(formatNumber(info.shahiValue)));
        QLabel* descLabel = new QLabel(info.description);
        descLabel->setStyleSheet("color: #666; font-size: 11px;");

        subUnitsLayout->addWidget(iconLabel, row, 0);
        subUnitsLayout->addWidget(nameLabel, row, 1);
        subUnitsLayout->addWidget(valueLabel, row, 2);
        subUnitsLayout->addWidget(descLabel, row, 3);
        
        row++;
    }

    scrollLayout->addWidget(subUnitsGroup);

    // Mega-Units Section
    QGroupBox* megaUnitsGroup = new QGroupBox("Mega-Units (Above 1 SHAH)");
    QGridLayout* megaUnitsLayout = new QGridLayout(megaUnitsGroup);

    row = 0;
    QList<UnitType> megaUnits = {SHAH, MetaShah, BetaShah, SigmaShah, MegaShah, GigaShah, AlphaShah};
    
    for (UnitType unit : megaUnits) {
        UnitInfo info = getUnitInfo(unit);
        
        QLabel* iconLabel = new QLabel();
        iconLabel->setText(info.icon);
        iconLabel->setStyleSheet("font-size: 20px;");
        
        QLabel* nameLabel = new QLabel(info.displayName);
        nameLabel->setStyleSheet("font-weight: bold; color: " + info.color + ";");
        
        QString valueText;
        if (unit == SHAH) {
            valueText = "= 100,000,000 Shahi";
        } else {
            qint64 shahValue = info.shahiValue / 100000000;
            valueText = QString("= %1 SHAH").arg(formatNumber(shahValue));
        }
        
        QLabel* valueLabel = new QLabel(valueText);
        QLabel* descLabel = new QLabel(info.description);
        descLabel->setStyleSheet("color: #666; font-size: 11px;");

        megaUnitsLayout->addWidget(iconLabel, row, 0);
        megaUnitsLayout->addWidget(nameLabel, row, 1);
        megaUnitsLayout->addWidget(valueLabel, row, 2);
        megaUnitsLayout->addWidget(descLabel, row, 3);
        
        row++;
    }

    scrollLayout->addWidget(megaUnitsGroup);

    // Examples Section
    QGroupBox* examplesGroup = new QGroupBox("Examples");
    QVBoxLayout* examplesLayout = new QVBoxLayout(examplesGroup);

    QStringList examples = {
        "110 Shahi → 1 Micro Shah + 1 Nano Shah",
        "1,540,000 Shahi → 1 Shahcent + 5 Simin Shah + 4 Mesin Shah",
        "1,000 SHAH → 1 Sigma Shah",
        "10,000 SHAH → 1 Mega Shah",
        "1,000,000 SHAH → 1 Alpha Shah"
    };

    for (const QString& example : examples) {
        QLabel* exampleLabel = new QLabel(example);
        exampleLabel->setStyleSheet("padding: 5px; background: #f8f9fa; border-radius: 4px; margin: 2px;");
        examplesLayout->addWidget(exampleLabel);
    }

    scrollLayout->addWidget(examplesGroup);

    scrollArea->setWidget(scrollWidget);
    scrollArea->setWidgetResizable(true);
    layout->addWidget(scrollArea);

    // Close button
    QPushButton* closeButton = new QPushButton("Close");
    connect(closeButton, &QPushButton::clicked, dialog, &QDialog::accept);
    
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(closeButton);
    layout->addLayout(buttonLayout);

    return dialog;
}

QLabel* SmartUnitDisplay::createBalanceLabel(qint64 shahiAmount)
{
    QLabel* label = new QLabel();
    
    if (m_smartUnitEnabled) {
        label->setText(getSmartUnitBreakdown(shahiAmount));
    } else {
        label->setText(getFullSHAHValue(shahiAmount));
    }
    
    label->setToolTip(createTooltip(shahiAmount));
    label->setStyleSheet("font-weight: bold; color: #333;");
    
    return label;
}

QString SmartUnitDisplay::createTooltip(qint64 shahiAmount) const
{
    QStringList tooltipParts;
    
    // Smart unit breakdown
    tooltipParts.append(QString("Smart: %1").arg(getSmartUnitBreakdown(shahiAmount)));
    
    // Full SHAH value
    tooltipParts.append(QString("Full: %1").arg(getFullSHAHValue(shahiAmount)));
    
    // USD equivalent if available
    if (m_usdPrice > 0.0) {
        tooltipParts.append(QString("USD: %1").arg(getUSDEquivalent(shahiAmount, m_usdPrice)));
    }
    
    return tooltipParts.join("\n");
}

qint64 SmartUnitDisplay::getShahiValue(UnitType type) const
{
    return m_units.value(type).shahiValue;
}

SmartUnitDisplay::UnitType SmartUnitDisplay::getBestUnit(qint64 shahiAmount) const
{
    // Find the best unit to display
    QList<UnitType> allUnits = {AlphaShah, GigaShah, MegaShah, SigmaShah, BetaShah, MetaShah, 
                               SHAH, DaricShah, Shahcent, ZarrinShah, SiminShah, MesinShah, 
                               MicroShah, NanoShah, Shahi};
    
    for (UnitType unit : allUnits) {
        if (shahiAmount >= getShahiValue(unit)) {
            return unit;
        }
    }
    
    return Shahi;
}

QString SmartUnitDisplay::formatNumber(qint64 value) const
{
    return QString::number(value).replace(QRegExp("(\\d)(?=(\\d{3})+(?!\\d))"), "\\1,");
}

void SmartUnitDisplay::onSmartUnitToggled(bool checked)
{
    setSmartUnitEnabled(checked);
}

void SmartUnitDisplay::onDisplayModeChanged(int index)
{
    DisplayMode mode = static_cast<DisplayMode>(index);
    setDisplayMode(mode);
}

void SmartUnitDisplay::showUnitExplorer()
{
    QDialog* dialog = qobject_cast<QDialog*>(createUnitExplorerDialog());
    if (dialog) {
        dialog->exec();
        dialog->deleteLater();
    }
}

void SmartUnitDisplay::setupUI()
{
    // Setup completed in constructor
}

QString SmartUnitDisplay::createUnitIconSVG(UnitType type) const
{
    switch (type) {
        case Shahi:
            return "●"; // Default SHAH dot
        case NanoShah:
            return "⚡"; // Tiny neon-blue particle
        case MicroShah:
            return "✨"; // Minimal spark
        case MesinShah:
            return "🔶"; // Bronze orb
        case SiminShah:
            return "⚪"; // Silver coin
        case ZarrinShah:
            return "🟡"; // Small golden disc
        case Shahcent:
            return "💎"; // Gem-like yellow core
        case DaricShah:
            return "🏛️"; // Historical coin
        case SHAH:
            return "🪙"; // Main SHAH coin
        case MetaShah:
            return "🔷"; // Digital hex coin
        case BetaShah:
            return "🟠"; // Bronze coin
        case SigmaShah:
            return "⚙️"; // Iron gear shape
        case MegaShah:
            return "🌟"; // Shiny coin with ripple
        case GigaShah:
            return "🔵"; // Electric blue core
        case AlphaShah:
            return "💠"; // Glowing platinum diamond
        default:
            return "●";
    }
}
