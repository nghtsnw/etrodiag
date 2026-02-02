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
    //void readFromFile();
    QString appHomeDir;
    QDateTime returnTimestamp();
    bool permission2SaveMasks = false;
    QString currentProfileName;

    ~newconnect();

signals:

    void cleanDevListSig();
    void cleanGraph();
    void sendStatusStr(QString);
    void transmitData(QDateTime currentTime, QVector<int> snapshot);
    void badCRC(uint8_t calculatedCRC, QVector<int> snapshot);
    void corruptedData(QVector<int> data);
    void saveAllMasks();
    void loadMask(s_parameterMask mask);
    void loadProtocol(s_protocolDescription); //Загрузка при чтении протокола из файла
    void writeTextLog(bool);
    void writeJsonLog(bool);
    void writeBinLog(bool);
    void directly2logArea(QString);
    void sendRawData(QByteArray);
    void startLog();
    void stopLog();
    void profileName2log(QString);
    void setVisibleControlWindow(bool);
    void s_sendSettings(s_Settings);

public slots:

    void saveProfileSlot4Masks(s_parameterMask mask);
    void restoreWindowAfterApplySettings();
    void prepareToSaveProfile();
    void saveProfile();
    void receiveCommandFromGui(QVector<quint8> command, bool newcommandflag);

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
    QString message;
    bool createNewFileNamePermission = true;
    QString binFileName;
    s_Settings p_local;
    //QByteArray fsba;
    QByteArray arr4byteStream;
    //QTimer *timer = new QTimer(this);
    QTimer *timerAboveTxCommand = new QTimer(this);
    /*QList<QByteArray> fileSplitted;
    const int bytesPerOneShot = 20;
    int pos = 0;*/
    QString getProfileNameFromInfo(QFileInfo &info);
    bool newcommand = false;
    QVector<quint8> toTransmit;
    quint8 calcCrc(const QVector<quint8> &arr);
    s_protocolDescription protocol;

protected:
    virtual void resizeEvent(QResizeEvent *event);
};

#endif // NEWCONNECT_H
