#ifndef DEVSETTINGSFORM_H
#define DEVSETTINGSFORM_H

#include <QWidget>
#include <QLineEdit>

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
    QLineEdit *devNameForm = new QLineEdit;
    //void onByteBtnClicked();
    void updByteButtons(int id, QVector<int> data);
    void retranslateByteButtonSetStatus(int byteNum, bool status);
    void killChildren();
    ~devSettingsForm();

signals:
    void returnDevNameAfterEdit(int devNum, QString text);
    void openByteSettingsFormTX(int devNum, int byteNum);
    void updateBtnDataSIG(int id, QVector<int> data);
    void wordType2ByteBtn(int _devNum, int _byteNum, int wordType);
    void setByteButtonStatus(int byteNum, bool status);
    void initByteButtonsWordLeight(int id,int count);
    void wordDataFullHex(int devNum, int byteNum, QString);
public slots:
    //void onByteBtnClicked();
    void openByteSettingsFormRX(int devNum, int byteNum);
    void wordTypeChangeRX(int _devNum, int _byteNum, int wordType);
    //void retranslateByteButtonSetStatus(int byteNum, bool status);

private slots:
    void on_devNameInputForm_editingFinished();


private:
    Ui::devSettingsForm *m_ui;
};

#endif // DEVSETTINGSFORM_H
