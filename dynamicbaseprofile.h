#ifndef DYNAMICBASEPROFILE_H
#define DYNAMICBASEPROFILE_H

#include <QObject>

class dynamicBaseProfile : public QObject
{
    Q_OBJECT
public:
    explicit dynamicBaseProfile(QObject *parent = nullptr);
    void addDataDev(int devNum, QVector<int> data);
    QString deviceName = ("New ETRO device");
    QString getDeviceName();
    void setDeviceName(QString name);
signals:
//    void setWordBitTX(int devNum, int byteNum, int argBit);
//    void getWordType(int devNum, int byteNum);
//    void returnWordTypeTX(int _devNum, int _byteNum, int wordType);
//    void createNewMaskTX(int _devNum, int _byteNum);
//    void mask2FormTX(int _devNum, int _byteNum, int _id);
//    void maskData2FormTX(int _devNum, int _byteNum, int _id, QString _paramName, int _paramMask, int _paramType, int _valueShift, float _valueKoef, bool _viewInLogFlag);
//    void requestMaskDataTX(int _devNum, int _byteNum, int _id);
//    void sendDataToProfileTX(int _devNum, int _byteNum, int _id, QString _paramName, int _paramMask, int _paramType, int _valueShift, float _valueKoef, bool _viewInLogFlag);

public slots:
//    void setWordBitRX(int devNum, int byteNum, int argBit);
//    void getWordTypeFromProfileRetranslator(int devNum, int byteNum);
//    void returnWordTypeRX(int _devNum, int _byteNum, int wordType);
//    void createNewMaskRX(int _devNum, int _byteNum);
//    void mask2FormRX(int _devNum, int _byteNum, int _id);
//    void maskData2FormRX(int _devNum, int _byteNum, int _id, QString _paramName, int _paramMask, int _paramType, int _valueShift, float _valueKoef, bool _viewInLogFlag);
//    void requestMaskDataRX(int _devNum, int _byteNum, int _id);
//    void sendDataToProfileRX(int _devNum, int _byteNum, int _id, QString _paramName, int _paramMask, int _paramType, int _valueShift, float _valueKoef, bool _viewInLogFlag);

private:

};

#endif // DYNAMICBASEPROFILE_H
