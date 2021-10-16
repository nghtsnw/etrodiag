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

signals:
    void returnWordType(int _devNum, int _byteNum, int wordType);
    void mask2FormTX(int _devNum, int _byteNum, int _id);
    void maskData2FormTX(bitMaskDataStruct bitMask);
    void requestMaskDataTX(int _devNum, int _byteNum, int _id);
    void sendDataToProfileTX(bitMaskDataStruct bitMask);
    void wordData2Mask(int devNum, int byteNum, int wordData);
    void allMasksToListTX(bitMaskDataStruct bitMask);
    void deleteMaskObjTX(int devNum, int byteNum, int id);
    void param2FrontEndTX(bitMaskDataStruct bitMask);
    void loadMaskTX(bitMaskDataStruct bitMask);
    void returnByteName(int devNum, int byteNum, QString byteName);
    void returnMaskCountForThisByte(int devNum, int th_byteNum, int count);

public slots:
    void setWordBitRX(int _devNum, int _byteNum, int _argBit);
    void getWordType(int _devNum, int _byteNum);
    void createNewMask(int _devNum, int _byteNum);
    void mask2FormRX(int _devNum, int _byteNum, int _id);
    void maskData2FormRX(bitMaskDataStruct &bitMask);
    void requestMaskDataRX(int _devNum, int _byteNum, int _id);
    void sendDataToProfileRX(bitMaskDataStruct &bitMask);
    void allMasksToListRX(bitMaskDataStruct &bitMask);
    void param2FrontEndRX(bitMaskDataStruct &bitMask);
    void loadMaskRX(bitMaskDataStruct &bitMask);
    void updateSlot(int _devNum, QVector<int> _data);
    void countMasks();

private:
    int tmpMaskId = 0;
};

#endif // BYTEDEFINITION_H
