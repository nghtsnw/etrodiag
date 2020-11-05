#ifndef DEVICE_H
#define DEVICE_H

#include <QMainWindow>
#include <QObject>
#include <QWidget>
#include <QPushButton>
#include "bytedefinition.h"
#include <QTimer>

class Device : public QPushButton
{
    Q_OBJECT

public:
    explicit Device(QWidget *parent = nullptr);
    int devNum;
    QString devName = "Device name";
    //dynamicBaseProfile *devPrf = new dynamicBaseProfile;
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
    void maskData2FormTX(int _devNum, int _byteNum, int _id, QString _paramName, QString _paramMask, int _paramType, double _valueShift, double _valueKoef, bool _viewInLogFlag, int wordType, bool _drawGraphFlag, QString _drawGraphColor);
    void requestMaskDataTX(int _devNum, int _byteNum, int _id);
    void sendDataToProfileTX(int _devNum, int _byteNum, int _id, QString _paramName, QString _paramMask, int _paramType, double _valueShift, double _valueKoef, bool _viewInLogFlag, bool _drawGraphFlag, QString _drawGraphColor);
    void allMasksToListTX(int devNum, QString devName, int byteNum, QString byteName, int id, QString paramName, QString paramMask, int paramType, double valueShift, double valueKoef, bool viewInLogFlag, int wordType, bool drawGraphFlag, QString drawGraphColor);
    void deleteMaskObjTX(int devNum, int byteNum, int id);
    void param2FrontEndTX(int devNum, QString devName, int byteNum, QString byteName, uint32_t wordData, int id, QString parameterName, int binRawValue, double endValue, bool viewInLogFlag, bool isNewData, bool drawGraphFlag, QString drawGraphColor);
    void loadMaskTX(int devNum, QString devName, int byteNum, QString byteName, int id, QString paramName, QString paramMask, int paramType, double valueShift, double valueKoef, bool viewInLogFlag, int wordType, bool drawGraphFlag, QString drawGraphColor);
    void getByteNameTX(int _devNum, int byteNum);
    void returnByteNameTX(int devNum, int byteNum, QString byteName);
    void saveByteNameTX(int _devNum, int _byteNum, QString _byteName);
    void byteObjUpdSig(int devNum, QVector<int> data);
    void devStatusMessage(QString _devname, QString status);

public slots:
    //void getDeviceName(int devNum);
    void updateData(int id, QVector<int> devdata);
    void setDeviceName(int id, QString name);
    void setWordTypeInByteProfile(int _devNum, int _byteNum, int _wordType);
    void getWordTypeFromProfileRetranslator(int _devNum, int _byteNum);
    void returnWordTypeRX(int _devNum, int _byteNum, int wordType);
    void createNewMaskRX(int _devNum, int _byteNum);
    void mask2FormRX(int _devNum, int _byteNum, int _id);
    void maskData2FormRX(int _devNum, int _byteNum, int _id, QString _paramName, QString _paramMask, int _paramType, double _valueShift, double _valueKoef, bool _viewInLogFlag, int wordType, bool _drawGraphFlag, QString _drawGraphColor);
    void requestMaskDataRX(int _devNum, int _byteNum, int _id);
    void sendDataToProfileRX(int _devNum, QString, int _byteNum, QString, int _id, QString _paramName, QString _paramMask, int _paramType, double _valueShift, double _valueKoef, bool _viewInLogFlag, int, bool _drawGraphFlag, QString _drawGraphColor);
    void allMasksToListRX(int devNum, int byteNum, QString byteName, int id, QString paramName, QString paramMask, int paramType, double valueShift, double valueKoef, bool viewInLogFlag, int wordType, bool drawGraphFlag, QString drawGraphColor);
    void param2FrontEndRX(int devNum, int byteNum, QString byteName, uint32_t wordData, int id, QString parameterName, int binRawValue, double endValue, bool viewInLogFlag, bool isNewData, bool _drawGraphFlag, QString _drawGraphColor);
    void requestMasks4Saving();
    void loadMaskRX(int devNum, QString devName, int byteNum, QString byteName, int id, QString paramName, QString paramMask, int paramType, double valueShift, double valueKoef, bool viewInLogFlag, int wordType, bool drawGraphFlag, QString drawGraphColor);
    void getByteNameRX(int _devNum, int byteNum);
    void saveByteNameRX(int _devNum, int _byteNum, QString _byteName);
    void hideDevButton(bool trueOrFalse, int _devNum);
    void changeButtonColor(QString _status);
    void setOfflineStatus();

private:
    QString devStatus = "init";
    Q_DISABLE_COPY(Device)
    QTimer *timer = new QTimer(this);

};

#endif // DEVICE_H
