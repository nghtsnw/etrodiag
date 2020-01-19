#ifndef BITMASKOBJ_H
#define BITMASKOBJ_H

#include <QObject>

class bitMaskObj : public QObject
{
    Q_OBJECT
public:
    bitMaskObj();
    ~bitMaskObj();
    void newMaskObj(int _devNum, int _byteNum, int _id);
    //void sendMaskToProfile(QString _paramName, int _paramMask, int _paramType, int _valueShift, float _valueKoef, int _wordData);
    QString paramName = "Parameter";
    int devNum;
    int byteNum;
    int id;
    QString paramMask = "00000000";
    int paramShift = 0;
    void calculateParamShift();
    void calculateParamLeight();
    int paramLeght = 0;
    int paramType = 0;
    int valueShift = 0;
    float valueKoef = 1;
    int wordData = 0;
    int wordType = 0;
    //void maskToForm();
    int paramMask4calcShift = 0;
    void calculateValue(int _devNum, int _byteNum, int wordData);
    bool viewInLogFlag = true;

signals:
    void maskToFormSIG(int devNum, int byteNum, int id, QString paramName, QString paramMask, int paramType, int valueShift, float valueKoef, bool viewInLogFlag, int wordType);
    void mask2byteSettingsForm(int devNum, int byteNum, int id);
    void maskToListSIG(int devNum, int byteNum, int id, QString paramName, QString paramMask, int paramType, int valueShift, float valueKoef, bool viewInLogFlag, int wordType);
    void param2FrontEnd(int devNum, int byteNum, int wordData, int id, QString paramName, int binRawValue, float endValue, bool viewInLogFlag);
public slots:
    void maskToForm(int _devNum, int _byteNum, int _id);
    void sendMaskToProfile(int _devNum, int _byteNum, int _id, QString _paramName, QString _paramMask, int _paramType, int _valueShift, float _valueKoef, bool _viewInLogFlag);
    void allMasksToList(int _devNum, int _byteNum);
    void deleteMaskObjectTX(int _devNum, int _byteNum, int _id);
};

#endif // BITMASKOBJ_H
