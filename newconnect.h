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
    void saveProfile();
    bool permission2SaveMasks = false;
    void prepareToSaveProfile();
    ~newconnect();

signals:

void sendStatusStr(QString);
void transmitData(QVector<int> snapshot);
void saveAllMasks();

public slots:

void saveProfileSlot4Masks(int devNum, QString devName, int byteNum, QString byteName, int id, QString paramName, QString paramMask, int paramType, double valueShift, double valueKoef, bool viewInLogFlag, int wordType);


private slots:
    void on_pushButton_clicked();
    void openSerialPort();
    void closeSerialPort();
//    void about();
    void writeData(const QByteArray &data);
    void readData();
    void handleError(QSerialPort::SerialPortError error);
    void transData(QVector<int> snapshot);

    void on_pushButton_2_clicked();

private:
    Ui::newconnect *ui;
    QString message;
};

#endif // NEWCONNECT_H
