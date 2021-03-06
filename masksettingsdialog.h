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
    int wordBit = 8;
    void updateBitButtonsAndCheckBoxes();
    QList<bitSetForm*> bitSetList;
    //QListIterator<bitSetForm*> bitSetListIt;
    void initBitButtonsAndCheckBoxes(int _wordType);
    void killChildren();
    void scanCheckboxesToMask();
    bool openDirectly = false;

public slots:
    void requestDataOnId(int _devNum, int _byteNum, int _id);
    //void sendDataOnId(int _devNum, int _byteNum, int _id);
    void sendMask2Profile();
    void getDataOnId(int _devNum, int _byteNum, int _id, QString _paramName, QString _paramMask, int _paramType, double _valueShift, double _valueKoef, bool _viewInLogFlag, int _wordType, bool _drawGraphFlag, QString _drawGraphColor);
    void liveDataSlot(int _devNum, QString _devName, int _byteNum, QString _byteName, uint32_t _wordData, int _id, QString parameterName, int _binRawValue, double _endValue, bool viewInLogFlag, bool isNewData, bool _drawGraphFlag, QString _drawGraphColor);

signals:
    void requestMaskData(int _devNum, int _byteNum, int _id);
    void sendMaskData(int _devNum, QString, int _byteNum, QString, int _id, QString _paramName, QString _paramMask, int _paramType, double _valueShift, double _valueKoef, bool _viewInLogFlag, int _wordType, bool _drawGraphFlag, QString _drawGraphColor);
    void getWordBit(int _devNum, int _byteNum);
    void setCheckBox(bool chk, int var);
    void wordData2bitSetForm(int i, bool wordDataIntArray);

private slots:
    void on_maskName_editingFinished();

    void on_shiftTxt_editingFinished();

    void on_koeffTxt_editingFinished();

    void on_logCheckBox_stateChanged(int);

    void on_drawGraphCheckBox_stateChanged(int);

    void on_checkAllButton_clicked();

    void on_uncheckAllButton_clicked();

private:
    Ui::maskSettingsDialog *ui;
    QColor drawColor;
    bool chkBoxStopSignal;
    QString binMaskInTxt;
};

#endif // MASKSETTINGSDIALOG_H
