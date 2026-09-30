#ifndef BYTEBUTTON_H
#define BYTEBUTTON_H

#include <QObject>
#include <QPushButton>
#include <QTimer>
#include <QLabel>

class byteButton : public QPushButton
{
    Q_OBJECT
public:
    byteButton();
    int byteNum;
    int devNum;
    int wordType = 0;
    bool maskInside = false;
    void setByteNum(int _devNum, int _byteNum);
    void setValueText(const QString &newValue); //значение в тексте кнопки; номер байта - метка в углу
    void onByteButtonClicked();
    void transformToWord(int wordType);
    QString Int2Hex(int num);
    ~byteButton();
signals:
    void openByteSettingsForm(int devNum, int byteNum);
    void setButton(int byteNum, bool status);
    void wordDataFullHex(int devNum, int byteNum, QString);

public slots:
    //void onByteButtonClicked();
    void updateBtnData(int _devNum, QVector<int> fullData);
    void setButtonStatus(int byteNum, bool status);
    void setWordType(int _devNum, int _byteNum, int _wordType);
    void changeButtonColor();
    void defaultButtonColor();
    void setMaskInThisWord(int _devNum, int _byteNum);

protected:
    void resizeEvent(QResizeEvent *event) override; //номер байта держим в углу при любом размере

private:
    QTimer *timer = new QTimer;
    QLabel *numLabel = nullptr; //номер байта в углу кнопки
    QString valueText; //последнее показанное значение (hex), без номера байта
    void placeNumLabel();
};

#endif // BYTEBUTTON_H
