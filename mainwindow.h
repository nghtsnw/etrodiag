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
#include "logger.h"
#include "global.h"

QT_BEGIN_NAMESPACE

class QLabel;
class QProgressBar;
class QToolButton;
class QStackedWidget;

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
    QProgressBar *loadProgress = nullptr;
    QToolButton *profileButton = nullptr;
    QToolButton *portButton = nullptr;
    QToolButton *paramsButton = nullptr;
    QToolButton *logButton = nullptr;
    QPushButton *connectButton = nullptr;
    QStackedWidget *profileArea = nullptr;
    QLabel *profileInfoLabel = nullptr;
    bool serialConnected = false;
    void setupProfileArea();   // делит вкладку соединения, справа - информация профиля/редактор
    void updateProfileInfo();  // показывает данные выбранного профиля
    void onEditProfile();      // открывает в правой половине редактор профиля
    void openMaskSettingsDialog();
    void createDevice(int devNum);
    void loadProfile(s_parameterMask mask);
    void textLogWindow(QDateTime currentTime, QString string, bool redFlag);
    void cleanDevList();
    void updValueArea(s_parameterMask mask);
    void ValueArea_CellClicked(int row, int);
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
    void timeNavigationSliderPositionChanged(int);

public slots:

    void showStatusMessage(QString message);
    void setLogLoadProgress(int percent);
    void refreshConnectionButtons();
    void addDeviceToList(QDateTime currentTime, QVector<int> ddata);
    void openDevSett(int devNum, QVector<int> data);
    void openByteSett(int devNum, int byteNum);
    void frontendDataSort(QDateTime currentTime, s_parameterMask mask);
    void devStatusMsg(QString _devName, QString status);
    void badCRCEvent(uint8_t calculatedCRC, QVector<int> dataFrame);

private slots:

    void on_tabWidget_currentChanged(int);
    void onAboutButtonClicked(bool);
    void guiCommandHandler(int varNumber, bool action);

private:
    void initActionsConnections();
    void setupStatusBar();
    void fillProfileMenu();
    void fillPortMenu();
    void fillParamsMenu();
    void pollPorts();
    QTimer *portPollTimer = nullptr;
    QStringList knownPortList;
    QTimer *timer = new QTimer(this);
    int currentOpenTab = 0;
    void setCurrentOpenTab(int index);
    QList<Device*> vlayChildList;
    int devNum;
    bool thisDeviceHere = false;
    QTableWidget *valueTableForDevice(const QString &devName); //таблица значений устройства или nullptr
    QTableWidget *createValueTable(const QString &devName);    //создать таблицу и вкладку устройства
    void updateValueTableRow(QTableWidget *table, const s_parameterMask &mask); //обновить/добавить строку
    void closeMaskSettings(int devNum);                        //закрыть настройки маски
    void closeByteSettings();                                  //закрыть настройки байта
    void toggleDeviceSettings(int devNum, QVector<int> data);  //открыть/закрыть форму устройства
private:
    AboutDialog aboutDialog;
    int grabDevNum = 0;
    int grabByteNum = 0;
    int grabMaskId = 0;
    int CRCErrorCount = 0;

protected:
    Ui::MainWindow *m_ui = nullptr;
    virtual void resizeEvent(QResizeEvent *) override;
    virtual void closeEvent(QCloseEvent *) override;
    bool event(QEvent *event) override;

public:
    devSettingsForm devSettForm;
    ByteSettingsForm byteSettForm;
    maskSettingsDialog maskSettForm;
    liveGraph graphiq;
    ControlBoard cBoard;
    Logger *logger = nullptr;
};

#endif // MAINWINDOW_H
