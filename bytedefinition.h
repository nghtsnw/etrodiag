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
    void mask2FormTX(int _devNum, int _byteNum, int _id);
    void maskData2FormTX(s_parameterMask mask);
    void requestMaskDataTX(int devNum, int byteNum, int id);
    void sendDataToProfileTX(s_parameterMask mask);
    void wordData2Mask(int devNum, int byteNum, int wordData);
    void allMasksToListTX(s_parameterMask answer);
    void deleteMaskObjTX(int devNum, int byteNum, int id);
    void param2FrontEndTX(s_parameterMask mask);
    void loadMaskTX(s_parameterMask mask);
    void returnByteName(int devNum, int byteNum, QString byteName);
    void returnMaskCountForThisByte(int devNum, int th_byteNum, int count);
    void sendDataToProfileRX(s_parameterMask mask);
    void requestMaskDataRX(int devNum, int byteNum, int id);

public slots:
    void setWordBitRX(int _devNum, int _byteNum, int _argBit);
    void getWordType(int _devNum, int _byteNum);
    void createNewMask(int _devNum, int _byteNum);
    void loadMaskRX(s_parameterMask mask);
    void updateSlot(int _devNum, QVector<int> _data);
    void countMasks();
};

#endif // BYTEDEFINITION_H
