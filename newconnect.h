#ifndef NEWCONNECT_H
#define NEWCONNECT_H

#include <QWidget>
#include "console.h"
#include "settingsdialog.h"
#include <QMainWindow>
#include <QtWidgets>
#include "mainwindow.h"
#include "getstream.h"
#include "dataprofiler.h"
#include "txtmaskobj.h"
#include "global.h"
#include <QDateTime>

namespace Ui {
    class newconnect;
}

class newconnect : public QWidget
{
    Q_OBJECT

public:
    explicit newconnect(QWidget *parent = nullptr);
    SettingsDialog *m_settings = nullptr;
    QSerialPort *m_serial = nullptr;
    Console *m_console = nullptr;
    void showStatusMessage(QString message);
    QLabel *m_status = nullptr;
    getStream *gstream = nullptr;
    dataprofiler *datapool = nullptr;
    QList<txtmaskobj*> maskVectorsList;
    void readProfile();

    bool permission2SaveMasks = false;
    QString currentProfileName;
    bool isReaderBusy() const { return readerBusy; } //идёт чтение лога из файла
    bool hasProfileChanges() const { return profileModified; } //есть несохранённые изменения профиля
    ~newconnect();

signals:

    void cleanDevListSig();
    void connected();
    void readFromFileSignal(bool);
    void sendStatusStr(QString);
    void transmitData(QDateTime currentTime, QVector<int> snapshot);
    void badCRC(uint8_t calculatedCRC, QVector<int> snapshot);
    // void corruptedData(QVector<int> data);
    void saveAllMasks();
    void loadMask(s_parameterMask mask);
    void loadProtocol(s_protocolDescription); //Загрузка при чтении протокола из файла
    void setProtocol(s_protocolDescription);
    void writeTextLog(bool);
    void writeJsonLog(bool);
    void writeBinLog(bool);
    void directly2logArea(QString);
    void sendRawData(QByteArray);
    void sendRawDataWithTime(QMap<QDateTime, QVector<uint8_t >> );
    void startLog();
    void disconnected();
    void profileName2log(QString);
    void profileLoaded(); //профиль прочитан целиком (протокол, настройки и все маски разосланы)
    void setVisibleControlWindow(bool);
    void setTime(QDateTime);
    void pushByteToProfiler(uint8_t);
    void putIntDataToConsole(QVector<uint8_t>);
    void logLoadProgress(int percent);
    void connectButtonTextChanged(QString); //текст кнопки подключения в строке состояния
    void loadSettings(s_Settings);          //настройки связи, считанные из профиля

public slots:

    void saveProfileSlot4Masks(s_parameterMask mask);
    void restoreWindowAfterApplySettings();
    void prepareToSaveProfile();
    void saveProfile();
    void receiveCommandFromGui(QVector<quint8> command, bool newcommandflag);
    void readFromFile(QMap<QDateTime, QVector<uint8_t >> );
    void toggleConnection(); //подключиться/отключиться или прочитать лог
    void editProfile();      //открыть окно редактирования профиля
    void commitProfileChanges();   //заменить профиль временным файлом (старый уйдёт в .bak)
    void discardProfileChanges();  //отказаться от изменений профиля (удалить .tmp)
    void offerToSaveProfile();     //предложить сохранить изменённый профиль

private slots:
    void openSerialPort();
    void closeSerialPort();
    void writeData(const QByteArray &data);
    void readData();
    void handleError(QSerialPort::SerialPortError error);
    void on_connectButton_clicked();
    void on_settingsButton_clicked();
    void sendCommand();

private:
    Ui::newconnect *ui;
    void wireConnection(); //связи порта, консоли и разбора кадров
    void wireSettings();   //связи протокола и настроек
    void wireProfile();    //связи профиля и отправки команд
    bool readerBusy = false;
    bool profileModified = false; //профиль изменён, изменения лежат в <профиль>.eag.tmp
    QTimer *timerAboveTxCommand = new QTimer(this);
    QString getProfileNameFromInfo(QFileInfo &info);
    bool newcommand = false;
    QVector<quint8> toTransmit;
    quint8 calcCrc(const QVector<quint8> &arr);

    QMap<QDateTime, QVector<uint8_t> > p_dataWithTime;

protected:
    virtual void resizeEvent(QResizeEvent *event);
};

#endif // NEWCONNECT_H
