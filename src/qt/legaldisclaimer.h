// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Hamid Rad / SHAHCOIN Core

#ifndef LEGALDISCLAIMER_H
#define LEGALDISCLAIMER_H

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QTextEdit>
#include <QDialog>

class LegalDisclaimer : public QWidget
{
    Q_OBJECT

public:
    explicit LegalDisclaimer(QWidget *parent = nullptr);
    ~LegalDisclaimer();

    // Create different types of legal notices
    QWidget* createFooterDisclaimer();
    QWidget* createAboutDisclaimer();
    QWidget* createCreationWarning();
    QDialog* createFullLegalDialog();

    // Get legal text templates
    QString getFooterText() const;
    QString getAboutText() const;
    QString getCreationWarningText() const;
    QString getFullLegalText() const;

    // Build signature methods
    QString getBuildSignature() const;
    QString getVersionInfo() const;

signals:
    void legalLinkClicked(const QString& link);

private slots:
    void onLegalLinkClicked();
    void showFullLegalDialog();

private:
    void setupUI();
    QString formatLegalText(const QString& text) const;
};

#endif // LEGALDISCLAIMER_H
