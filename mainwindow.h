#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QHBoxLayout>
#include <QPushButton>
#include "aboutdialog.h"
#include "controlboard.h"
#include "devsettingsform.h"
#include "bytesettingsform.h"
#include "device.h"
#include "masksettingsdialog.h"
#include <livegraph.h>
#include <QTableWidget>
//#include <QGestureEvent>
#include "logger.h"
#include "global.h"

QT_BEGIN_NAMESPACE

class QLabel;
//class QGestureEvent;
//class QSwipeGesture;

namespace Ui {
    class MainWindow;
}

QT_END_NAMESPACE

class Console;
class SettingsDialog;
class newconnect;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:

    explicit MainWindow(QWidget *parent = nullptr);
    void addConnection();
    newconnect *connection = nullptr;
    QLabel *statuslbl = nullptr;
    QLabel *crcerrorlbl = nullptr;
    QPushButton *aboutButton = nullptr;
    void openMaskSettingsDialog();
    void createDevice(int devNum);
    void loadProfile(s_parameterMask mask);
    void textLogWindow(QDateTime currentTime, QString string, bool redFlag);
    void cleanDevList();
    void updValueArea(s_parameterMask mask);
// void grabGestures(const QVector<Qt::GestureType> &gestures);
    void ValueArea_CellClicked(int row, int);
    QString appHomeDir;
    ~MainWindow();


signals:
    void devUpdate(QDateTime currentTime, int devNum, QVector<int> ddata);
    void getDevName(int devNum);
    void setDevName(int devNum, QString name);
    void returnDevNameAfterClose(int devNum, QString text);
    void sendMaskData(s_parameterMask mask);
    void getByteName(int devNum, int byteNum);
    void hideOtherDevButtons(bool, int _devNum);
    void dvsfAfterCloseClear();
    void prepareToSaveProfile();
    void saveProfile();
    void toJsonMap(s_parameterMask mask);
    void getJsonMap(int devNum);
    void setDevParamsCount(int devNum, int paramsCount);
    void toTxtLogger(QString);
    void emitCommand(QVector<quint8> command, bool newcommandflag);

public slots:

    void showStatusMessage(QString message);
    void addDeviceToList(QDateTime currentTime, QVector<int> ddata);
    void openDevSett(int devNum, QVector<int> data);
    void openByteSett(int devNum, int byteNum);
    void frontendDataSort(QDateTime currentTime, s_parameterMask mask);
    void devStatusMsg(QString _devName, QString status);
    void badCRCEvent(uint8_t calculatedCRC, QVector<int> dataFrame);
    void corruptedDataEvent(QVector<int> data);

private slots:

    void on_tabWidget_currentChanged(int);
    void onAboutButtonClicked(bool);
    void guiCommandHandler(int varNumber, bool action);

private:
    void initActionsConnections();
    //QDateTime returnTimestamp();
    QTimer *timer = new QTimer(this);
    void swipeTriggered(QString);
    int currentOpenTab = 0;
    void setCurrentOpenTab(int index);
    QList<Device*> vlayChildList;
    int devNum;
    bool thisDeviceHere = false;
    bool findRow;
    QString value2str;
    QString namesUnited;
    QTableWidget *valueTable = nullptr;
    QString tmp;
    const int oneMsgLeight = 18; //41 //18 для брнг опытная
private:
    AboutDialog aboutDialog;
    QLabel *m_status = nullptr;
    bool touchTrigger = false;
    int mouseStartX;
    int mouseStartY;
    int mouseStopX;
    int mouseStopY;
    void swipeCalc(QMouseEvent mouseev);
    int grabDevNum = 0;
    int grabByteNum = 0;
    int grabMaskId = 0;
    int CRCErrorCount = 0;

protected:
    Ui::MainWindow *m_ui = nullptr;
    virtual void resizeEvent(QResizeEvent *) override;
    bool event(QEvent *event) override;
    //bool eventFilter(QObject *obj, QEvent *ev) override;

public:
    devSettingsForm devSettForm;
    ByteSettingsForm byteSettForm;
    maskSettingsDialog maskSettForm;
    liveGraph graphiq;
    ControlBoard cBoard;
    Logger *logger = nullptr;
};

#endif // MAINWINDOW_H
