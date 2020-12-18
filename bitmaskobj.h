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
    QString paramName = "Parameter";
    int devNum;
    int byteNum;
    int id;
    QString paramMask = "00000000";
    QString paramMaskNew;
    int paramShift = 0;
    void calculateParamShift();
//    void calculateParamLeight();
    int paramLeght = 0;
    int paramType = 0;
    double valueShift = 0;
    double valueKoef = 1;
    int wordData = 0;
    int wordType = 0;
    int paramMask4calcShift = 0;
    void calculateValue(int _devNum, int _byteNum, uint32_t wordData);
    bool viewInLogFlag = true;
    bool drawGraphFlag = false;
    QString drawGraphColor;

signals:
    void maskToFormSIG(int devNum, int byteNum, int id, QString paramName, QString paramMask, int paramType, int valueShift, double valueKoef, bool viewInLogFlag, int wordType, bool drawGraphFlag, QString drawGraphColor);
    void mask2byteSettingsForm(int devNum, int byteNum, int id);
    void maskToListSIG(int devNum, int byteNum, int id, QString paramName, QString paramMask, int paramType, int valueShift, double valueKoef, bool viewInLogFlag, int wordType, bool drawGraphFlag, QString drawGraphColor);
    void param2FrontEnd(int devNum, int byteNum, int wordData, int id, QString paramName, int binRawValue, double endValue, bool viewInLogFlag, bool isNewData, bool _drawGraphFlag, QString _drawGraphColor);
public slots:
    void maskToForm(int _devNum, int _byteNum, int _id);
    void sendMaskToProfile(int _devNum, int _byteNum, int _id, QString _paramName, QString _paramMask, int _paramType, double _valueShift, double _valueKoef, bool _viewInLogFlag, bool _drawGraphFlag, QString _drawGraphColor);
    void allMasksToList(int _devNum, int _byteNum);
    void deleteMaskObjectTX(int _devNum, int _byteNum, int _id);
    void loadMaskRX(int _devNum, QString _devName, int _byteNum, QString _byteName, int _id, QString _paramName, QString _paramMask, int _paramType, double _valueShift, double _valueKoef, bool _viewInLogFlag, int _wordType, bool drawGraphFlag, QString drawGraphColor);

private:
    double oldEndValue = 1234.56;
    int wordDataSize = 0;
    bool isNewData = true;
    uint32_t paramMaskInt;
    void recalcMask();
};

#endif // BITMASKOBJ_H
