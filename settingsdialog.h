#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QWidget>
#include <QSerialPort>
#include <QLineEdit>
#include "global.h"

QT_BEGIN_NAMESPACE

namespace Ui {
    class SettingsDialog;
}

class QIntValidator;

QT_END_NAMESPACE

class SettingsDialog : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog();
    QString selectedProfile;
    QString appHomeDir;
    s_Settings settings() const;

signals:
    void restoreConsoleAndButtons();
    void prepareToSaveProfile();
    void saveProfile();
    void writeTextLog(bool);
    void writeBinLog(bool);
    void writeJsonLog(bool);
    void loadProtocol(s_protocolDescription); //Загрузка принятых значений в поля
    void setProtocol(s_protocolDescription); //Применение текущих значений
    void loadSelectedProfile();

private slots:
    void showPortInfo(int idx);
    void apply();
    void checkCustomBaudRatePolicy(int idx);
    void checkCustomDevicePathPolicy(int idx);

    void on_newProfileButton_clicked();

    void on_profileSelectBox_currentTextChanged(const QString &arg1);

    void on_deleteProfileButton_clicked();

    void on_readOnlyCheckBox_stateChanged(int);

    void on_writeBinChkBox_stateChanged(int);

    void on_writeTxtChkBox_stateChanged(int);

    void on_writeJsonChkBox_stateChanged(int);


private:
    void fillPortsParameters();
    void fillPortsInfo();
    void updateSettings();
    void fillProfileList();
    void portBoxEvent(int currentText);
    s_protocolDescription currentProtocol;
    void updateProtocol();
    void markerTextNormalisation(int numberByte, QString text);

private:
    Ui::SettingsDialog *m_ui = nullptr;
    s_Settings m_currentSettings;
    QIntValidator *m_intValidator = nullptr;
};

#endif // SETTINGSDIALOG_H
