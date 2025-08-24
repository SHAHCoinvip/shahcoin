#ifndef SHAHCOIN_QT_ABOUTDIALOG_H
#define SHAHCOIN_QT_ABOUTDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextBrowser>
#include <QTabWidget>

class QPixmap;

/**
 * @brief Enhanced About Dialog for SHAHCOIN Core
 * 
 * Displays comprehensive information about SHAHCOIN Core including:
 * - Shahcoin logo and branding
 * - App version and build information
 * - GitHub link and documentation
 * - Credits and contributors
 * - License information
 */
class AboutDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AboutDialog(QWidget* parent = nullptr);
    ~AboutDialog();

private Q_SLOTS:
    void onGitHubClicked();
    void onDocumentationClicked();
    void onLicenseClicked();
    void onCopyVersionClicked();

private:
    void setupUI();
    void setupBranding();
    void setupVersionInfo();
    void setupCredits();
    void setupLicense();
    void applyTheme();
    void copyToClipboard(const QString& text);

    // UI Elements
    QLabel* m_logo_label = nullptr;
    QLabel* m_title_label = nullptr;
    QLabel* m_tagline_label = nullptr;
    QLabel* m_version_label = nullptr;
    QLabel* m_build_label = nullptr;
    QPushButton* m_github_button = nullptr;
    QPushButton* m_docs_button = nullptr;
    QPushButton* m_license_button = nullptr;
    QPushButton* m_copy_version_button = nullptr;
    QTabWidget* m_tab_widget = nullptr;
    QTextBrowser* m_credits_browser = nullptr;
    QTextBrowser* m_license_browser = nullptr;
    QPushButton* m_close_button = nullptr;
    
    // Content
    QString m_version_text;
    QString m_build_text;
    QString m_credits_text;
    QString m_license_text;
};

#endif // SHAHCOIN_QT_ABOUTDIALOG_H
