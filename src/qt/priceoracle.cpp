#include "priceoracle.h"
#include <QSettings>

PriceOracle::PriceOracle(QObject* parent)
    : QObject(parent)
    , m_usdPerShah(0.10) // default fallback
{
    connect(&m_timer, &QTimer::timeout, this, &PriceOracle::refresh);
    loadFromSettings();
}

void PriceOracle::setManualPrice(double usdPerShah)
{
    if (usdPerShah <= 0) return;
    if (qFuzzyCompare(m_usdPerShah, usdPerShah)) return;
    m_usdPerShah = usdPerShah;
    emit priceUpdated(m_usdPerShah);
}

void PriceOracle::start(int intervalMs)
{
    if (intervalMs < 1000) intervalMs = 1000;
    m_timer.start(intervalMs);
    refresh();
}

void PriceOracle::stop()
{
    m_timer.stop();
}

void PriceOracle::refresh()
{
    double before = m_usdPerShah;
    loadFromSettings();
    if (!qFuzzyCompare(before, m_usdPerShah)) {
        emit priceUpdated(m_usdPerShah);
    }
}

void PriceOracle::loadFromSettings()
{
    QSettings s;
    double v = s.value("shahswap/usdPerShah", m_usdPerShah).toDouble();
    if (v > 0) m_usdPerShah = v;
}


