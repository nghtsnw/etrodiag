#ifndef BYTEDEFINITION_H
#define BYTEDEFINITION_H

#include <QObject>

class byteDefinition : public QObject

{
    Q_OBJECT

public:
    byteDefinition();
    byteDefinition(int numDev, int byteNum, int data);
    int wordType = 0; //0 - 8bit, 1 - 16bit, 2 - 32bit
    void initProfileStrings(int byteNum, int data);
    void calcWordData(int _devNum, QVector<int> data);
    int devNum;
    int th_byteNum = 0;
    int th_data = 0;
    int wordData;
    virtual ~byteDefinition();
signals:
    void returnWordType(int _devNum, int _byteNum, int wordType);
    void mask2FormTX(int _devNum, int _byteNum, int _id);
    void maskData2FormTX(int _devNum, int _byteNum, int _id, QString _paramName, int _paramMask, int _paramType, int _valueShift, float _valueKoef, bool _viewInLogFlag, int _wordType);
    void requestMaskDataTX(int _devNum, int _byteNum, int _id);
    void sendDataToProfileTX(int _devNum, int _byteNum, int _id, QString _paramName, int _paramMask, int _paramType, int _valueShift, float _valueKoef, bool _viewInLogFlag);
    void wordData2Mask(int wordData);
    void allMasksToListTX(int devNum, int byteNum, int id, QString paramName, int paramMask, int paramType, int valueShift, float valueKoef, bool viewInLogFlag, int wordType);
public slots:
    void setWordBitRX(int _devNum, int _byteNum, int _argBit);
    void getWordType(int _devNum, int _byteNum);
    void createNewMask(int _devNum, int _byteNum);
    void mask2FormRX(int _devNum, int _byteNum, int _id);
    void maskData2FormRX(int _devNum, int _byteNum, int _id, QString _paramName, int _paramMask, int _paramType, int _valueShift, float _valueKoef, bool _viewInLogFlag, int _wordType);
    void requestMaskDataRX(int _devNum, int _byteNum, int _id);
    void sendDataToProfileRX(int _devNum, int _byteNum, int _id, QString _paramName, int _paramMask, int _paramType, int _valueShift, float _valueKoef, bool _viewInLogFlag);
    void allMasksToListRX(int devNum, int byteNum, int id, QString paramName, int paramMask, int paramType, int valueShift, float valueKoef, bool viewInLogFlag, int wordType);
};

#endif // BYTEDEFINITION_H
