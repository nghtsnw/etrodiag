#ifndef BITMASKOBJ_H
#define BITMASKOBJ_H

#include <QObject>
#include "bitmaskstruct.h"
class bitMaskObj : public QObject
{
    Q_OBJECT
public:
    bitMaskObj();
    ~bitMaskObj();
    bitMaskDataStruct bitMask; //тут все данные объекта
    void newMaskObj(int _devNum, int _byteNum, int _id);    
    QString paramMaskNew;
    int paramShift = 0;
    void calculateParamShift();
//    void calculateParamLeight();
//    int paramLeght = 0;
    int paramMask4calcShift = 0;
    void calculateValue(int _devNum, int _byteNum, uint32_t wordData);
signals:
    void maskToFormSIG(bitMaskDataStruct &bitMask);
    void mask2byteSettingsForm(int devNum, int byteNum, int id);
    void maskToListSIG(bitMaskDataStruct &bitMask);
    void param2FrontEnd(bitMaskDataStruct &bitMask);
public slots:
    void maskToForm(int _devNum, int _byteNum, int _id);
    void sendMaskToProfile(bitMaskDataStruct &bitMaskFromSettingsDialog);
    void allMasksToList(int _devNum, int _byteNum);
    void deleteMaskObjectTX(int _devNum, int _byteNum, int _id);
    void loadMaskRX(bitMaskDataStruct &bitMaskFromFile);
private:
    double oldEndValue = 1234.56;
    int wordDataSize = 0;    
    uint32_t paramMaskInt;
    void recalcMask();
};

#endif // BITMASKOBJ_H
