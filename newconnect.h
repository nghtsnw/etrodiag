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
#include <bitmaskstruct.h>

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
    QList<bitMaskDataStruct*> maskVectorsList;
    void readProfile();
    void readFromFile();
    QString appHomeDir;
    QDateTime returnTimestamp();
    bool permission2SaveMasks = false;
    QString currentProfileName;

    ~newconnect();

signals:

    void cleanDevListSig();
    void cleanGraph();
    void sendStatusStr(QString);
    void transmitData(QVector<int> snapshot);
    void saveAllMasks();
    void loadMask(bitMaskDataStruct &bitMask);
//(int devNum, QString devName, int byteNum, QString byteName, int id, QString paramName, QString paramMask,
//int paramType, double valueShift, double valueKoef, bool viewInLogFlag, int wordType, bool _drawGraphFlag,
//QString _drawGraphColor);
    void writeTextLog(bool);
    void writeJsonLog(bool);
    void writeBinLog(bool);
    void directly2logArea(QString);
    void sendRawData(QByteArray);
    void startLog();
    void stopLog();
    void profileName2log(QString);

public slots:

    void saveProfileSlot4Masks(bitMaskDataStruct &bitMask);
//(int devNum, QString devName, int byteNum, QString byteName, int id, QString paramName,
//QString paramMask, int, double valueShift, double valueKoef, bool viewInLogFlag, int wordType,
//bool _drawGraphFlag, QString _drawGraphColor);
    void restoreWindowAfterApplySettings();
    void prepareToSaveProfile();
    void saveProfile();


private slots:
    void openSerialPort();
    void closeSerialPort();
    void writeData(const QByteArray &data);
    void readData();
    void handleError(QSerialPort::SerialPortError error);
    void transData(QVector<int> snapshot);
    void on_connectButton_clicked();
    void on_settingsButton_clicked();

private:
    Ui::newconnect *ui;
    QString message;
    //bool writeBinLog = false;
    bool createNewFileNamePermission = true;
    QString binFileName;
    SettingsDialog::Settings p_local;
    QByteArray fsba;
    QByteArray arr4byteStream;
    QTimer *timer = new QTimer(this);
    QList<QByteArray> fileSplitted;
    const int bytesPerOneShot = 20;
    int pos = 0;
    QString getProfileNameFromInfo(QFileInfo &info);

protected:
    virtual void resizeEvent(QResizeEvent *event);
};

#endif // NEWCONNECT_H
