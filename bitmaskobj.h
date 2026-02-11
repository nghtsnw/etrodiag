#ifndef BITMASKOBJ_H
#define BITMASKOBJ_H

#include <QObject>
#include "global.h"

class bitMaskObj : public QObject
{
    Q_OBJECT
public:
    bitMaskObj();
    ~bitMaskObj();
    s_parameterMask currentMask;
    s_parameterMask oldMask;
    void newMaskObj(s_parameterMask mask);
    QString paramMaskNew;
    int paramMask4calcShift = 0;
    void calculateParamShift();
// void calculateParamLeight();
    void calculateValue(int _devNum, int _byteNum, uint32_t wordData);

signals:
    void maskToFormSIG(s_parameterMask mask);
// void mask2byteSettingsForm(s_parameterMask mask);
    void maskToListSIG(s_parameterMask mask);
    void param2FrontEnd(s_parameterMask mask);
public slots:
    void maskToForm(int devNum, int byteNum, int id);
    void sendMaskToProfile(s_parameterMask mask);
    void allMasksToList(int devNum, int byteNum);
    void deleteMaskObjectTX(int devNum, int byteNum, int id);
    void loadMaskRX(s_parameterMask mask);

private:
    double oldEndValue = 1234.56;
    int wordDataSize = 0;
    bool isNewData = true;
    uint32_t paramMaskInt;
    void recalcMask();
};

#endif // BITMASKOBJ_H
