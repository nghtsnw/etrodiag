#ifndef MASKSETTINGSDIALOG_H
#define MASKSETTINGSDIALOG_H
#include "bitsetform.h"
#include <QWidget>


namespace Ui {
class maskSettingsDialog;
}

class maskSettingsDialog : public QWidget
{
    Q_OBJECT

public:
    explicit maskSettingsDialog(QWidget *parent = nullptr);
    ~maskSettingsDialog();
    int devNum;
    int byteNum;
    int id;
    int wordType = 0;
    void updateBitButtonsAndCheckBoxes();
    QList<bitSetForm*> bitSetList;
    //QListIterator<bitSetForm*> bitSetListIt;
    void initBitButtonsAndCheckBoxes(int _wordType);
    void killChildren();

public slots:
    void requestDataOnId(int _devNum, int _byteNum, int _id);
    //void sendDataOnId(int _devNum, int _byteNum, int _id);
    void sendMask2Profile();
    void getDataOnId(int _devNum, int _byteNum, int _id, QString _paramName, int _paramMask, int _paramType, int _valueShift, float _valueKoef, bool _viewInLogFlag, int _wordType);

signals:
    void requestMaskData(int _devNum, int _byteNum, int _id);
    void sendMaskData(int _devNum, int _byteNum, int _id, QString _paramName, int _paramMask, int _paramType, int _valueShift, float _valueKoef, bool _viewInLogFlag, int _wordType);
    void getWordBit(int _devNum, int _byteNum);
private:
    Ui::maskSettingsDialog *ui;
};

#endif // MASKSETTINGSDIALOG_H
