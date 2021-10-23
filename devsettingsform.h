#ifndef DEVSETTINGSFORM_H
#define DEVSETTINGSFORM_H

#include <QWidget>
#include <QLineEdit>
#include "bitmaskstruct.h"

namespace Ui {
class devSettingsForm;
}

class devSettingsForm : public QWidget
{
    Q_OBJECT

public:
    explicit devSettingsForm(QWidget *parent = nullptr);
    void setDevName(int id, QString devName);
    int devNum;
    void initByteButtons(int id, QVector<int> data);
    void updByteButtons(int _devNum, QVector<int> data);
    void retranslateByteButtonSetStatus(int byteNum, bool status);
    void afterCloseClearing();
    ~devSettingsForm();

signals:
    void returnDevNameAfterEdit(int devNum, QString text);
    void openByteSettingsFormTX(int devNum, int byteNum);
    void updateBtnDataSIG(int id, QVector<int> data);
    void wordType2ByteBtn(int _devNum, int _byteNum, int wordType);
    void setByteButtonStatus(int byteNum, bool status);
    void initByteButtonsWordLeight(int id,int count);
    void wordDataFullHex(int devNum, int byteNum, QString);
    void inThisWordLivingMask(int _devNum, int _byteNum);

public slots:
    void wordTypeChangeRX(int _devNum, int _byteNum, int wordType);
    void liveDataSlot(bitMaskDataStruct &bitMask);

private slots:
    void on_devNameEditLine_editingFinished();

private:
    Ui::devSettingsForm *m_ui;
    bool readyToSetDevNameToForm = true;
};

#endif // DEVSETTINGSFORM_H
