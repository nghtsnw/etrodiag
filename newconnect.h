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

    QString appHomeDir;
    QDateTime returnTimestamp();
    bool permission2SaveMasks = false;
    QString currentProfileName;

    ~newconnect();

signals:

    void cleanDevListSig();
    void connected();
    //void readFromFile();
    void readFromFileSignal();
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
    void setVisibleControlWindow(bool);
    void s_sendSettings(s_Settings);
    void setTime(QDateTime);
    void pushByteToProfiler(uint8_t);
    void putIntDataToConsole(QVector<uint8_t>);

public slots:

    void saveProfileSlot4Masks(s_parameterMask mask);
    void restoreWindowAfterApplySettings();
    void prepareToSaveProfile();
    void saveProfile();
    void receiveCommandFromGui(QVector<quint8> command, bool newcommandflag);
    void readFromFile(QMap<QDateTime, QVector<uint8_t >> );

private slots:
    void openSerialPort();
    void closeSerialPort();
    void writeData(const QByteArray &data);
    void readData();
    void handleError(QSerialPort::SerialPortError error);
    void on_connectButton_clicked();
    void on_settingsButton_clicked();
    void sendCommand();
    void readFromFilePortions();

private:
    Ui::newconnect *ui;
    QString message;
    bool createNewFileNamePermission = true;
    QString binFileName;
    s_Settings p_local;
    //QByteArray fsba;
    QByteArray arr4byteStream;
    QTimer *timer = new QTimer(this);
    QTimer *timerAboveTxCommand = new QTimer(this);
    /*QList<QByteArray> fileSplitted;
    const int bytesPerOneShot = 20;
    int pos = 0;*/
    QString getProfileNameFromInfo(QFileInfo &info);
    bool newcommand = false;
    QVector<quint8> toTransmit;
    quint8 calcCrc(const QVector<quint8> &arr);
    s_protocolDescription protocol;

    QList<QDateTime> timeKeys;
    QListIterator<QDateTime> *p_timeKeysIterator = nullptr;
    QMap<QDateTime, QVector<uint8_t> > p_dataWithTime;

protected:
    virtual void resizeEvent(QResizeEvent *event);
};

#endif // NEWCONNECT_H
