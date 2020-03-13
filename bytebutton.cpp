#include "bytebutton.h"
#include "QDebug"
#include <QTimer>

byteButton::byteButton()
{
    connect(this, &byteButton::clicked, this, &byteButton::onByteButtonClicked);
    connect (timer, &QTimer::timeout, this, &byteButton::defaultButtonColor);
}

void byteButton::setByteNum(int _devNum, int _byteNum)
{
    devNum = _devNum;
    byteNum = _byteNum;
}

byteButton::~byteButton()
{

}

void byteButton::updateBtnData(int _devNum, QVector<int> fullData)
{
    QString txttmp;
    if (devNum == _devNum && this->isEnabled())
    {        
    if (wordType == 0)
    {
        txttmp = (Int2Hex(fullData.at(byteNum)));
    }
    else if (wordType == 1)
    {
        txttmp = ((Int2Hex(fullData.at(byteNum+1))+':'+(Int2Hex(fullData.at(byteNum)))));
    }
    else if (wordType == 2)
    {
        txttmp = ((Int2Hex(fullData.at(byteNum+3))+':'+(Int2Hex(fullData.at(byteNum+2)))+':'
                             +(Int2Hex(fullData.at(byteNum+1))+':'+(Int2Hex(fullData.at(byteNum))))));
    }
    if (txttmp != this->text())
    {
        byteButton::setText(txttmp);
        changeButtonColor();
    }
    }
    if (!this->isEnabled()) this->setText("--->");
    emit wordDataFullHex(devNum, byteNum, this->text());
}

void byteButton::onByteButtonClicked()
{
    emit openByteSettingsForm(devNum, byteNum);
    qDebug() << "openByteSettingsForm("<<devNum<<", "<<byteNum<<")";
}

void byteButton::transformToWord(int _wordType)
{
   if (this->isEnabled())
   {
    if (_wordType == 0) //8bit
    {
          emit setButton(byteNum+1,1);
          emit setButton(byteNum+2,1);
          emit setButton(byteNum+3,1);
    }
    else if (_wordType == 1) //16bit
    {
        emit setButton(byteNum+1,0);
        emit setButton(byteNum+2,1);
        emit setButton(byteNum+3,1);
    }
    else if (_wordType == 2) //32bit
    {
        emit setButton(byteNum+1,0);
        emit setButton(byteNum+2,0);
        emit setButton(byteNum+3,0);
    }
   }
}

void byteButton::setButtonStatus(int _byteNum, bool status)
{
    if (byteNum == _byteNum)
    {
        this->setEnabled(status);
    }
}

void byteButton::setWordType(int _devNum, int _byteNum, int _wordType)
{
    if (devNum == _devNum && byteNum == _byteNum)
    {
        wordType = _wordType;
        transformToWord(wordType);
    }
}

void byteButton::setMaskInThisWord(int _devNum, int _byteNum)
{
    if (_byteNum == byteNum && _devNum == devNum && !maskInside)
        maskInside = true;
}

void byteButton::changeButtonColor()
{
    this->setStyleSheet("QPushButton{background:#00FF00;}");
    timer->setInterval(500);
    timer->start();
}
void byteButton::defaultButtonColor()
{
    timer->stop();
    if (maskInside == false)
    this->setStyleSheet("QPushButton{background:none;}");
    else if (maskInside == true)
        this->setStyleSheet("QPushButton{background:#FFFF00;}");
}

QString byteButton::Int2Hex(int num) //для удобства конвертирования из инт в хекс
{
   return  QString("%1").arg(num,0,16).toUpper();
}
