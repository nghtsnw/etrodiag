#ifndef DEVICE_H
#define DEVICE_H

#include <QMainWindow>
#include <QObject>
#include <QWidget>
#include <QPushButton>
#include "bytedefinition.h"
#include <QTimer>
#include <QVariantMap>
#include "global.h"
#include "qdatetime.h"

class ProtocolSettings;

class Device : public QPushButton
{
    Q_OBJECT

public:
    explicit Device(QWidget *parent = nullptr);
    enum class State { Init, Offline, Online }; //состояние устройства вместо сравнения переведённых строк
    int devNum;
    QString devName = "Device name";
    QVector<int> currentState;
    Device(int id);
    void byteObjectsInit(QVector<int> &data);
    void getDeviceName(int id);
    void clickedF();
    bool byteObjReady = false;
    void devOnlineWatchdog(int msec);

signals:
    void txtToGui(QString);
    void openDevSettSig(int devNum, QVector<int> data);
    void returnDeviceName(int devNum, QString);
    void setWordBitTX(int _devNum, int _byteNum, int _wordType);
    void getWordTypeTX(int _devNum, int _byteNum);
    void returnWordTypeTX(int _devNum, int _byteNum, int wordType);
    void createNewMaskTX(int _devNum, int _byteNum);
    void mask2FormTX(int _devNum, int _byteNum, int _id);
    void maskData2FormTX(s_parameterMask mask);
    void requestMaskDataTX(int _devNum, int _byteNum, int _id);
    void sendDataToProfileTX(s_parameterMask mask);
    void allMasksToListTX(s_parameterMask mask);
    void deleteMaskObjTX(int devNum, int byteNum, int id);
    void param2FrontEndTX(QDateTime currentTime, s_parameterMask mask);
    void loadMaskTX(s_parameterMask mask);
    void byteObjUpdSig(int devNum, QVector<int> data);
    void devStatusMessage(QString _devname, QString status);
    void devParamsToJson(QVariantMap);
    void requestMaskCounting();


public slots:
    void updateData(QDateTime currentTime, int id, QVector<int> devdata);
    void setDeviceName(int id, QString name);
    void requestMasks4Saving();
    void loadMaskRX(s_parameterMask mask);
    void hideDevButton(bool trueOrFalse, int _devNum);
    void changeButtonColor(State _status);
    void setOfflineStatus();
    void jsonMap(s_parameterMask mask);
    void returnMaskCounting(int devNum, int byteNum, int count);
    void setModel(const ProtocolSettings *model); //протокол читаем из модели профиля

private:
    State devStatus = State::Init;
    Q_DISABLE_COPY(Device)
    QTimer *timer = new QTimer(this);
    QVariantMap *devParams = new QVariantMap;
    QDateTime currentTime;
    int devParamsCount = 0;
    int countMasks(); //пересчитывает кэш количества масок и возвращает его
    int maskCountCache = 0;
    QMap<int, int> maskCountMap;
    int calcMasksInDev();
    const ProtocolSettings *m_model = nullptr;
};

#endif // DEVICE_H
