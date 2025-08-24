#ifndef SHAHCOIN_QT_RELEASENOTESDIALOG_H
#define SHAHCOIN_QT_RELEASENOTESDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QTextBrowser>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QTabWidget>
#include <QListWidget>
#include <QGroupBox>

class QTextBrowser;
class QPushButton;
class QCheckBox;
class QTabWidget;
class QListWidget;
class QGroupBox;

/**
 * @brief Release Notes Dialog for SHAHCOIN Core
 * 
 * Displays comprehensive release information including:
 * - Version history and changelog
 * - New features and improvements
 * - Bug fixes and security updates
 * - Migration notes and breaking changes
 * - What's new highlights
 */
class ReleaseNotesDialog : public QDialog
{
    Q_OBJECT

public:
    struct ReleaseInfo {
        QString version;
        QString codename;
        QString releaseDate;
        QString description;
        QStringList newFeatures;
        QStringList improvements;
        QStringList bugFixes;
        QStringList securityUpdates;
        QStringList breakingChanges;
        QString migrationNotes;
        bool isMajor;
        bool isLTS;
    };

    explicit ReleaseNotesDialog(QWidget* parent = nullptr);
    ~ReleaseNotesDialog();

    void setCurrentVersion(const QString& version);
    void setShowOnStartup(bool show);
    bool shouldShowOnStartup() const;
    void addRelease(const ReleaseInfo& release);
    void setReleases(const QList<ReleaseInfo>& releases);

public Q_SLOTS:
    void onVersionSelected(const QString& version);
    void onShowOnStartupToggled(bool checked);
    void onWhatsNewClicked();
    void onFullChangelogClicked();
    void onMigrationGuideClicked();
    void onDownloadClicked();

Q_SIGNALS:
    void showOnStartupChanged(bool show);
    void downloadRequested(const QString& version);

private:
    void setupUI();
    void setupReleases();
    void populateVersionList();
    void showReleaseInfo(const ReleaseInfo& release);
    void applyTheme();
    QString formatReleaseInfo(const ReleaseInfo& release) const;
    QString formatWhatsNew(const ReleaseInfo& release) const;
    QString formatFullChangelog(const ReleaseInfo& release) const;
    QString formatMigrationNotes(const ReleaseInfo& release) const;
    
    // UI Components
    QLabel* m_titleLabel;
    QLabel* m_currentVersionLabel;
    QListWidget* m_versionList;
    QTextBrowser* m_releaseInfoBrowser;
    QTabWidget* m_tabWidget;
    QTextBrowser* m_whatsNewBrowser;
    QTextBrowser* m_fullChangelogBrowser;
    QTextBrowser* m_migrationBrowser;
    QCheckBox* m_showOnStartupCheckBox;
    QPushButton* m_whatsNewButton;
    QPushButton* m_fullChangelogButton;
    QPushButton* m_migrationGuideButton;
    QPushButton* m_downloadButton;
    QPushButton* m_closeButton;
    
    // Data
    QString m_currentVersion;
    QList<ReleaseInfo> m_releases;
    ReleaseInfo m_currentRelease;
    bool m_showOnStartup;
    
    // Styling
    QString m_currentTheme;
};

/**
 * @brief What's New Widget
 * 
 * Highlights new features and improvements
 */
class WhatsNewWidget : public QWidget
{
    Q_OBJECT

public:
    explicit WhatsNewWidget(QWidget* parent = nullptr);
    void setReleaseInfo(const ReleaseNotesDialog::ReleaseInfo& release);

private:
    void setupUI();
    void populateFeatures();
    
    QVBoxLayout* m_layout;
    QLabel* m_titleLabel;
    QGroupBox* m_newFeaturesGroup;
    QGroupBox* m_improvementsGroup;
    QGroupBox* m_securityGroup;
    QListWidget* m_newFeaturesList;
    QListWidget* m_improvementsList;
    QListWidget* m_securityList;
    
    ReleaseNotesDialog::ReleaseInfo m_release;
};

/**
 * @brief Full Changelog Widget
 * 
 * Shows complete changelog with all details
 */
class FullChangelogWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FullChangelogWidget(QWidget* parent = nullptr);
    void setReleaseInfo(const ReleaseNotesDialog::ReleaseInfo& release);

private:
    void setupUI();
    void populateChangelog();
    
    QVBoxLayout* m_layout;
    QTextBrowser* m_changelogBrowser;
    
    ReleaseNotesDialog::ReleaseInfo m_release;
};

/**
 * @brief Migration Guide Widget
 * 
 * Shows migration notes and breaking changes
 */
class MigrationGuideWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MigrationGuideWidget(QWidget* parent = nullptr);
    void setReleaseInfo(const ReleaseNotesDialog::ReleaseInfo& release);

private:
    void setupUI();
    void populateMigrationNotes();
    
    QVBoxLayout* m_layout;
    QTextBrowser* m_migrationBrowser;
    QGroupBox* m_breakingChangesGroup;
    QListWidget* m_breakingChangesList;
    
    ReleaseNotesDialog::ReleaseInfo m_release;
};

#endif // SHAHCOIN_QT_RELEASENOTESDIALOG_H
