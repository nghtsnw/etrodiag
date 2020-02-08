#include "bytebutton.h"
#include "QDebug"

byteButton::byteButton()
{
    connect(this, &byteButton::clicked, this, &byteButton::onByteButtonClicked);
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

    if (devNum == _devNum && this->isEnabled())
    {
    if (wordType == 0)
    {
        byteButton::setText(Int2Hex(fullData.at(byteNum)));
    }
    else if (wordType == 1)
    {
        byteButton::setText((Int2Hex(fullData.at(byteNum))+':'+(Int2Hex(fullData.at(byteNum+1)))));
    }
    else if (wordType == 2)
    {
        byteButton::setText((Int2Hex(fullData.at(byteNum))+':'+(Int2Hex(fullData.at(byteNum+1)))+':'
                             +(Int2Hex(fullData.at(byteNum+2))+':'+(Int2Hex(fullData.at(byteNum+3))))));
    }
    }
    if (!this->isEnabled()) this->setText("--->");
    emit wordDataFullHex(devNum, byteNum, this->text());
}

void byteButton::onByteButtonClicked()
{
    emit openByteSettingsForm(devNum, byteNum);
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

QString byteButton::Int2Hex(int num) //для удобства конвертирования из инт в хекс
{
   return  QString("%1").arg(num,0,16).toUpper();
}
