#ifndef DEVICE_H
#define DEVICE_H

#include <QMainWindow>
#include <QObject>
#include <QWidget>
#include <QPushButton>
#include "bytedefinition.h"
#include <QTimer>
#include <QVariantMap>

class Device : public QPushButton
{
    Q_OBJECT

public:
    explicit Device(QWidget *parent = nullptr);
    int devNum;
    QString devName = "Device name";
    QVector<int> currState;
    int currStateInt = 0;
    QVector<int> *oldState = new QVector<int>;
    Device(int id);
    void byteObjectsInit(QVector<int> &data);
    QVector<byteDefinition*> *byteObjArr = new QVector<byteDefinition*>;
    QVector<byteDefinition*> *byteObjArrOld = new QVector<byteDefinition*>;
    byteDefinition zeroByteDef;
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
    void maskData2FormTX(bitMaskDataStruct &bitMask);
    void requestMaskDataTX(int _devNum, int _byteNum, int _id);
    void sendDataToProfileTX(bitMaskDataStruct &bitMask);
    void allMasksToListTX(bitMaskDataStruct &bitMask);
    void deleteMaskObjTX(int devNum, int byteNum, int id);
    void param2FrontEndTX(bitMaskDataStruct &bitMask);
    void loadMaskTX(bitMaskDataStruct &bitMask);
    void byteObjUpdSig(int devNum, QVector<int> data);
    void devStatusMessage(QString _devname, QString status);
    void devParamsToJson(QVariantMap);
    void requestMaskCounting();

public slots:
    void updateData(int id, QVector<int> devdata);
    void setDeviceName(int id, QString name);
    void setWordTypeInByteProfile(int _devNum, int _byteNum, int _wordType);
    void getWordTypeFromProfileRetranslator(int _devNum, int _byteNum);
    void returnWordTypeRX(int _devNum, int _byteNum, int wordType);
    void createNewMaskRX(int _devNum, int _byteNum);
    void mask2FormRX(int _devNum, int _byteNum, int _id);
    void maskData2FormRX(bitMaskDataStruct &bitMask);
    void requestMaskDataRX(int _devNum, int _byteNum, int _id);
    void sendDataToProfileRX(bitMaskDataStruct &bitMask);
    void allMasksToListRX(bitMaskDataStruct &bitMask);
    void param2FrontEndRX(bitMaskDataStruct &bitMask);
    void requestMasks4Saving();
    void loadMaskRX(bitMaskDataStruct &bitMask);
    void hideDevButton(bool trueOrFalse, int _devNum);
    void changeButtonColor(QString _status);
    void setOfflineStatus();
    void jsonMap(bitMaskDataStruct &bitMask);
    void returnMaskCounting(int devNum, int byteNum, int count);

private:
    QString devStatus = "init";
    Q_DISABLE_COPY(Device)
    QTimer *timer = new QTimer(this);
    QVariantMap *devParams = new QVariantMap;
    QDateTime returnTimestamp();
    bool skippedFirstJsonSending = false;
    int devParamsCount = 0;
    int countMasks();
    QMap<int, int> maskCountMap;
    int calcMasksInDev();
};

#endif // DEVICE_H
