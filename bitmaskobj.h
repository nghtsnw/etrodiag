#ifndef BITMASKOBJ_H
#define BITMASKOBJ_H

#include <QObject>

class bitMaskObj : public QObject
{
    Q_OBJECT
public:
    bitMaskObj();
    void newMaskObj(int _devNum, int _byteNum, int _id);
    //void sendMaskToProfile(QString _paramName, int _paramMask, int _paramType, int _valueShift, float _valueKoef, int _wordData);
    QString paramName;
    int devNum;
    int byteNum;
    int id;
    int paramMask = 0;
    int paramShift = 0;
    int calculateParamShift();
    int calculateParamLeight();
    int paramLeght = 0;
    int paramType = 0;
    int valueShift = 0;
    float valueKoef = 0;
    int wordData = 0;
    int wordType = 0;
    //void maskToForm();
    int paramMask4calcShift = 0;
    void calculateValue(int wordData);
    bool viewInLogFlag = true;

signals:
    void maskToFormSIG(int devNum, int byteNum, int id, QString paramName, int paramMask, int paramType, int valueShift, float valueKoef, bool viewInLogFlag, int wordType);
    void mask2byteSettingsForm(int devNum, int byteNum, int id);
public slots:
    void maskToForm(int _devNum, int _byteNum, int _id);
    void sendMaskToProfile(int _devNum, int _byteNum, int _id, QString _paramName, int _paramMask, int _paramType, int _valueShift, float _valueKoef, bool _viewInLogFlag);
};

#endif // BITMASKOBJ_H
