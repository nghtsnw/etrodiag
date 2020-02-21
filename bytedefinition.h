#ifndef BYTEDEFINITION_H
#define BYTEDEFINITION_H

#include <QObject>

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
    void maskData2FormTX(int _devNum, int _byteNum, int _id, QString _paramName, QString _paramMask, int _paramType, double _valueShift, double _valueKoef, bool _viewInLogFlag, int _wordType);
    void requestMaskDataTX(int _devNum, int _byteNum, int _id);
    void sendDataToProfileTX(int _devNum, int _byteNum, int _id, QString _paramName, QString _paramMask, int _paramType, double _valueShift, double _valueKoef, bool _viewInLogFlag);
    void wordData2Mask(int devNum, int byteNum, int wordData);
    void allMasksToListTX(int devNum, int byteNum, QString byteName, int id, QString paramName, QString paramMask, int paramType, int valueShift, double valueKoef, bool viewInLogFlag, int wordType);
    void deleteMaskObjTX(int devNum, int byteNum, int id);
    void param2FrontEndTX(int devNum, int byteNum, QString byteName, uint32_t wordData, int id, QString parameterName, int binRawValue, double endValue, bool viewInLogFlag, bool isNewData);
    void loadMaskTX(int devNum, QString devName, int byteNum, QString byteName, int id, QString paramName, QString paramMask, int paramType, double valueShift, double valueKoef, bool viewInLogFlag, int wordType);
    void returnByteName(int devNum, int byteNum, QString byteName);

public slots:
    void setWordBitRX(int _devNum, int _byteNum, int _argBit);
    void getWordType(int _devNum, int _byteNum);
    void createNewMask(int _devNum, int _byteNum);
    void mask2FormRX(int _devNum, int _byteNum, int _id);
    void maskData2FormRX(int _devNum, int _byteNum, int _id, QString _paramName, QString _paramMask, int _paramType, double _valueShift, double _valueKoef, bool _viewInLogFlag, int _wordType);
    void requestMaskDataRX(int _devNum, int _byteNum, int _id);
    void sendDataToProfileRX(int _devNum, int _byteNum, int _id, QString _paramName, QString _paramMask, int _paramType, double _valueShift, double _valueKoef, bool _viewInLogFlag);
    void allMasksToListRX(int devNum, int byteNum, int id, QString paramName, QString paramMask, int paramType, double valueShift, double valueKoef, bool viewInLogFlag, int wordType);
    void param2FrontEndRX(int devNum, int byteNum, uint32_t wordData, int id, QString parameterName, int binRawValue, double endValue, bool viewInLogFlag, bool isNewData);
    void loadMaskRX(int devNum, QString devName, int byteNum, QString _byteName, int id, QString paramName, QString paramMask, int paramType, double valueShift, double valueKoef, bool viewInLogFlag, int wordType);
    void getByteNameRX(int _devNum, int _byteNum);
    void saveByteNameRX(int _devNum, int _byteNum, QString _byteName);
    void updateSlot(int _devNum, int _byteNum, QVector<int> _data);

private:
    int tmpMaskId = 0;
};

#endif // BYTEDEFINITION_H
