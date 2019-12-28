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
    ~ByteSettingsForm();

signals:
    void setWordBit (int devNum,int byteNum,int arg1);
    void getWordType(int _devNum, int _byteNum);
    void createMask(int devNum, int byteNum);

public slots:
    void returnWordType(int _devNum, int _byteNum, int wordType);
private slots:
    void on_bitBox_valueChanged(int arg1);

    void on_pushButton_clicked();

private:
    Ui::ByteSettingsForm *ui;
};

#endif // BYTESETTINGSFORM_H
