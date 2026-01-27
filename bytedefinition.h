#ifndef BYTEDEFINITION_H
#define BYTEDEFINITION_H

#include <QObject>
#include "global.h"

class byteDefinition : public QObject

{
    Q_OBJECT

public:
    byteDefinition();
    byteDefinition(int numDev, int byteNum, int data);
    QString byteName = ("<empty>");
    int wordType = 0; //0 - 8bit, 1 - 16bit, 2 - 32bit
    void initProfileStrings(int byteNum, int data);
    void calcWordData(int _devNum, QVector<int> data);
    int calcMaskID();
    int devNum;
    int th_byteNum = 0;
    int th_data = 0;
    uint32_t wordData;
    virtual ~byteDefinition();
signals:
    void returnWordType(int _devNum, int _byteNum, int wordType);
    void mask2FormTX(s_parameterMask mask);
    void maskData2FormTX(s_parameterMask mask);
    void requestMaskDataTX(s_parameterMask mask);
    void sendDataToProfileTX(s_parameterMask mask);
    void wordData2Mask(int devNum, int byteNum, int wordData);
    void allMasksToListTX(s_parameterMask mask);
    void deleteMaskObjTX(int devNum, int byteNum, int id);
    void param2FrontEndTX(s_parameterMask mask);
    void loadMaskTX(s_parameterMask mask);
    void returnByteName(int devNum, int byteNum, QString byteName);
    void returnMaskCountForThisByte(int devNum, int th_byteNum, int count);

public slots:
    void setWordBitRX(int _devNum, int _byteNum, int _argBit);
    void getWordType(int _devNum, int _byteNum);
    void createNewMask(int _devNum, int _byteNum);
    void mask2FormRX(s_parameterMask mask);
    void maskData2FormRX(s_parameterMask mask);
    void requestMaskDataRX(s_parameterMask mask);
    void sendDataToProfileRX(s_parameterMask mask);
    void allMasksToListRX(s_parameterMask maskr);
    void param2FrontEndRX(s_parameterMask mask);
    void loadMaskRX(s_parameterMask mask);
    void updateSlot(int _devNum, QVector<int> _data);
    void countMasks();

private:
    int tmpMaskId = 0;
};

#endif // BYTEDEFINITION_H
