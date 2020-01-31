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
    void deleteMaskItem(int row);
    void updateMasksList(int _devNum, QString _devName, int _byteNum, QString _byteName, int _wordData, int _id, QString parameterName, int _binRawValue, float _endValue, bool viewInLogFlag);

    ~ByteSettingsForm();

signals:
    void setWordBit (int devNum,int byteNum,int arg1);
    void getWordType(int _devNum, int _byteNum);
    void createMask(int devNum, int byteNum);
    void editMask(int devNum, int byteNum, int row);
    void requestAllMaskToList(int devNum, int byteNum, int id4all = 999);
    void deleteMaskObj(int devNum, int byteNum, int id);
    void saveByteName(int devNum, int byteNum, QString byteName);

public slots:
    void returnWordType(int _devNum, int _byteNum, int wordType);
    void addMaskItem(int _devNum, QString _devName, int _byteNum, QString _byteName, int _id, QString _paramName, QString _paramMask, int _paramType, int _valueShift, float _valueKoef, bool _viewInLogFlag, int _wordType);
    void updateHexWordData(int _devNum, int _byteNum, QString _txt);
    void setWordName(int _devNum, int _byteNum, QString _byteName);
private slots:
    void on_bitBox_valueChanged(int arg1);

    void on_pushButton_clicked();

    void on_masksWidget_cellClicked(int row, int column);

    void on_wordNameEdit_editingFinished();

private:
    Ui::ByteSettingsForm *ui;
    QStringList lst = {"Parameter", "Value", "ID", "Delete"};
};

#endif // BYTESETTINGSFORM_H
