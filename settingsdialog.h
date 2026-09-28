#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QWidget>
#include <QSerialPort>
#include "global.h"

QT_BEGIN_NAMESPACE

namespace Ui {
    class SettingsDialog;
}

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

    // API для кнопок в строке состояния главного окна
    s_Settings currentSettings();                 // синхронизирует модель с состоянием и возвращает её
    QString connectionSummary() const;            // краткая подпись текущих параметров связи
    bool isReadFromFile() const;                  // выбран режим чтения лога из файла
    QString selectedPortName() const;             // выбранный COM-порт или путь к логу
    QStringList availablePortNames() const;       // список доступных COM-портов
    QStringList profileNames() const;             // список профилей из каталога Profiles
    QString currentProfileName() const;
    void selectProfile(const QString &fileName);  // выбрать профиль (загрузит его)
    void createNewProfile();                      // диалог создания нового профиля
    void deleteCurrentProfile();                  // удалить выбранный профиль (с подтверждением)
    bool writeTxtEnabled() const;
    bool writeBinEnabled() const;
    bool writeJsonEnabled() const;
    void setWriteTxt(bool on);
    void setWriteBin(bool on);
    void setWriteJson(bool on);
    void setPortName(const QString &portName);    // выбор COM-порта
    void setReadFromFile(const QString &filePath);// выбор чтения лога из файла
    void applyConnection(int baud, int dataBits, int parity, int stopBits, int flowControl);
    void applyConnectionSettings(const s_Settings &s); // применить настройки из профиля

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
    void settingsChanged(); //настройки связи изменились программно или из профиля

public slots:
    void apply(); //применение настроек: сохранение профиля и закрытие окна (кнопка "Применить")

    void on_newProfileButton_clicked();

    void on_profileSelectBox_currentTextChanged(const QString &arg1);

    void on_deleteProfileButton_clicked();

private:
    void updateSettings();
    void fillProfileList();
    s_protocolDescription currentProtocol;
    void updateProtocol();
    void markerTextNormalisation(int numberByte, QString text);

private:
    Ui::SettingsDialog *m_ui = nullptr;
    s_Settings m_currentSettings;
    bool m_readFromFileMode = false; //режим "чтение из файла" - меняется только явным выбором пользователя
    bool m_writeTxt = false;
    bool m_writeBin = false;
    bool m_writeJson = false;
};

#endif // SETTINGSDIALOG_H
