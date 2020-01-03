#ifndef BYTESETTINGSFORM_H
#define BYTESETTINGSFORM_H

#include <QWidget>

namespace Ui {
class ByteSettingsForm;
}

class ByteSettingsForm : public QWidget
{
    Q_OBJECT

public:
    explicit ByteSettingsForm(QWidget *parent = nullptr);
    //ByteSettingsForm(int _devNum, int _byteNum);
    int devNum;
    int byteNum;
    void open(int _devNum, int _byteNum);
    void getWordTypeFromProfile(int _devNum, int _byteNum);
    void cleanMaskList();
    void requestAllMasks();
    ~ByteSettingsForm();

signals:
    void setWordBit (int devNum,int byteNum,int arg1);
    void getWordType(int _devNum, int _byteNum);
    void createMask(int devNum, int byteNum);
    void editMask(int devNum, int byteNum, int row);
    void requestAllMaskToList(int devNum, int byteNum, int id4all = 999);

public slots:
    void returnWordType(int _devNum, int _byteNum, int wordType);
    void openMaskSettingsForm(int row, int column);
    void addMaskItem(int _devNum, int _byteNum, int _id, QString _paramName, int _paramMask, int _paramType, int _valueShift, float _valueKoef, bool _viewInLogFlag, int _wordType);
private slots:
    void on_bitBox_valueChanged(int arg1);

    void on_pushButton_clicked();

private:
    Ui::ByteSettingsForm *ui;
};

#endif // BYTESETTINGSFORM_H
