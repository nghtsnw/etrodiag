#ifndef BYTESETTINGSFORM_H
#define BYTESETTINGSFORM_H

#include <QWidget>
#include "bitmaskstruct.h"

namespace Ui {
class ByteSettingsForm;
}

class ByteSettingsForm : public QWidget
{
    Q_OBJECT

public:
    explicit ByteSettingsForm(QWidget *parent = nullptr);
    int devNum;
    int byteNum;
    void open(int _devNum, int _byteNum);
    void getWordTypeFromProfile(int _devNum, int _byteNum);
    void requestAllMasks();
    void deleteMaskItem(int row);
    void updateMasksList(bitMaskDataStruct &bitMask);
    void cleanForm();
    ~ByteSettingsForm();

signals:
    void setWordBit (int devNum,int byteNum,int arg1);
    void getWordType(int _devNum, int _byteNum);
    void createMask(int devNum, int byteNum);
    void editMask(int devNum, int byteNum, int row);
    void requestAllMaskToList(int devNum, int byteNum, int id4all = 999);
    void deleteMaskObj(int devNum, int byteNum, int id);

public slots:
    void returnWordType(int _devNum, int _byteNum, int wordType);
    void addMaskItem(bitMaskDataStruct &bitMask);
    void updateHexWordData(int _devNum, int _byteNum, QString _txt);

private slots:
    void on_bitBox_valueChanged(int arg1);

    void on_pushButton_clicked();

    void on_masksWidget_cellClicked(int row, int column);

private:
    Ui::ByteSettingsForm *ui;
    QStringList lst = {"Parameter", "Value", "ID", "Delete"};
    int arg2index;
};

#endif // BYTESETTINGSFORM_H
