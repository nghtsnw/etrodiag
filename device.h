#ifndef DEVICE_H
#define DEVICE_H

#include <QMainWindow>
#include <QObject>
#include <QWidget>
#include <QPushButton>
#include "bytedefinition.h"
#include "dynamicbaseprofile.h"

class Device : public QPushButton
{
    Q_OBJECT

public:
    explicit Device(QWidget *parent = nullptr);
    int devNum;
    dynamicBaseProfile *devPrf = new dynamicBaseProfile;
    QVector<int> currState;
    QVector<int> *oldState = new QVector<int>;
    Device(int id, QVector<int> data);
    void byteObjectsInit(QVector<int> data);
    QVector<byteDefinition*> *byteObjArr = new QVector<byteDefinition*>;
    QVector<byteDefinition*> *byteObjArrOld = new QVector<byteDefinition*>;
    void byteObjectsUpd(QVector<int> data);
    byteDefinition zeroByteDef;
    //void getDeviceName();
    void clickedF();

signals:
    void txtToGui(QString);
    void openDevSettSig(int devNum, QVector<int> data);
    void returnDeviceName(int devNum, QString);
    void setWordBitTX(int _devNum, int _byteNum, int _wordType);
    void getWordTypeTX(int _devNum, int _byteNum);
    void returnWordTypeTX(int _devNum, int _byteNum, int wordType);
    void createNewMaskTX(int _devNum, int _byteNum);
    void mask2FormTX(int _devNum, int _byteNum, int _id);
    void maskData2FormTX(int _devNum, int _byteNum, int _id, QString _paramName, QString _paramMask, int _paramType, int _valueShift, float _valueKoef, bool _viewInLogFlag, int wordType);
    void requestMaskDataTX(int _devNum, int _byteNum, int _id);
    void sendDataToProfileTX(int _devNum, int _byteNum, int _id, QString _paramName, QString _paramMask, int _paramType, int _valueShift, float _valueKoef, bool _viewInLogFlag);
    void allMasksToListTX(int devNum, int byteNum, int id, QString paramName, QString paramMask, int paramType, int valueShift, float valueKoef, bool viewInLogFlag, int wordType);
    void deleteMaskObjTX(int devNum, int byteNum, int id);

public slots:
    void getDeviceName(int devNum);
    void updateData(int id, QVector<int> devdata);
    void setDeviceName(int id, QString name);
    void setWordTypeInByteProfile(int _devNum, int _byteNum, int _wordType);
    void getWordTypeFromProfileRetranslator(int _devNum, int _byteNum);
    void returnWordTypeRX(int _devNum, int _byteNum, int wordType);
    void createNewMaskRX(int _devNum, int _byteNum);
    void mask2FormRX(int _devNum, int _byteNum, int _id);
    void maskData2FormRX(int _devNum, int _byteNum, int _id, QString _paramName, QString _paramMask, int _paramType, int _valueShift, float _valueKoef, bool _viewInLogFlag, int wordType);
    void requestMaskDataRX(int _devNum, int _byteNum, int _id);
    void sendDataToProfileRX(int _devNum, int _byteNum, int _id, QString _paramName, QString _paramMask, int _paramType, int _valueShift, float _valueKoef, bool _viewInLogFlag, int _wordType);
    void allMasksToListRX(int devNum, int byteNum, int id, QString paramName, QString paramMask, int paramType, int valueShift, float valueKoef, bool viewInLogFlag, int wordType);
private:
    Q_DISABLE_COPY(Device)

};

#endif // DEVICE_H
