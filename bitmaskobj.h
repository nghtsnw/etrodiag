#ifndef BITMASKOBJ_H
#define BITMASKOBJ_H

#include <QObject>

class bitMaskObj : public QObject
{
    Q_OBJECT
public:
    bitMaskObj();
    ~bitMaskObj();
    struct bitMaskDataStruct{
        int devNum;
        int byteNum;
        int id;
        int wordType = 0;
        uint32_t wordData = 0;
        QString paramName = "Parameter";
        QString paramMask = "00000000";
        int paramType = 0;
        double valueShift = 0;
        double valueKoef = 1;
        bool viewInLogFlag = true;
        bool drawGraphFlag = false;
        QString drawGraphColor;
        bool isNewData = true;
        int binRawValue;
        double endValue;
    };
    bitMaskDataStruct bitMask;
    void newMaskObj(int _devNum, int _byteNum, int _id);    
    QString paramMaskNew;
    int paramShift = 0;
    void calculateParamShift();
//    void calculateParamLeight();
//    int paramLeght = 0;

    int paramMask4calcShift = 0;
    void calculateValue(int _devNum, int _byteNum, uint32_t wordData);    


signals:
    void maskToFormSIG(bitMaskDataStruct bitMask);
    void mask2byteSettingsForm(int devNum, int byteNum, int id);
    void maskToListSIG(bitMaskDataStruct bitMask);
    void param2FrontEnd(bitMaskDataStruct bitMask);
public slots:
    void maskToForm(int _devNum, int _byteNum, int _id);
    void sendMaskToProfile(int _devNum, int _byteNum, int _id, QString _paramName, QString _paramMask, int _paramType, double _valueShift, double _valueKoef, bool _viewInLogFlag, bool _drawGraphFlag, QString _drawGraphColor);
    void allMasksToList(int _devNum, int _byteNum);
    void deleteMaskObjectTX(int _devNum, int _byteNum, int _id);
    void loadMaskRX(int _devNum, QString _devName, int _byteNum, QString _byteName, int _id, QString _paramName, QString _paramMask, int _paramType, double _valueShift, double _valueKoef, bool _viewInLogFlag, int _wordType, bool drawGraphFlag, QString drawGraphColor);

private:
    double oldEndValue = 1234.56;
    int wordDataSize = 0;    
    uint32_t paramMaskInt;
    void recalcMask();
};

#endif // BITMASKOBJ_H
