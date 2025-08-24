#ifndef ANIMATION_SETTINGS_DIALOG_H
#define ANIMATION_SETTINGS_DIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>
#include <QSlider>
#include <QSpinBox>
#include <QGroupBox>
#include <QComboBox>
#include <QSettings>

class AnimationSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AnimationSettingsDialog(QWidget* parent = nullptr);

private slots:
    void onAnimationsEnabledToggled(bool enabled);
    void onLowResourceModeToggled(bool enabled);
    void onSoundEnabledToggled(bool enabled);
    void onAnimationSpeedChanged(int speed);
    void onOkClicked();
    void onCancelClicked();
    void onResetClicked();

private:
    // UI Components
    QVBoxLayout* m_layout;
    
    // Animation Settings
    QGroupBox* m_animationGroup;
    QCheckBox* m_animationsEnabledCheck;
    QCheckBox* m_lowResourceModeCheck;
    QCheckBox* m_soundEnabledCheck;
    
    // Animation Speed
    QGroupBox* m_speedGroup;
    QLabel* m_speedLabel;
    QSlider* m_speedSlider;
    QSpinBox* m_speedSpinBox;
    
    // Individual Animation Toggles
    QGroupBox* m_individualGroup;
    QCheckBox* m_nftRevealCheck;
    QCheckBox* m_tokenSparklineCheck;
    QCheckBox* m_portfolioLoadCheck;
    QCheckBox* m_shahDroppedCheck;
    
    // Buttons
    QHBoxLayout* m_buttonLayout;
    QPushButton* m_okButton;
    QPushButton* m_cancelButton;
    QPushButton* m_resetButton;
    
    // Settings
    QSettings m_settings;
    
    void setupUI();
    void loadSettings();
    void saveSettings();
    void connectSignals();
};

#endif // ANIMATION_SETTINGS_DIALOG_H
