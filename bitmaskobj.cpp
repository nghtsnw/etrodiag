#include "bitmaskobj.h"
#include <QDebug>

bitMaskObj::bitMaskObj()
{

}

bitMaskObj::~bitMaskObj()
{

}

void bitMaskObj::newMaskObj(int _devNum, int _byteNum, int _id)
{
    id = _id;
    devNum = _devNum;
    byteNum = _byteNum;
    emit mask2byteSettingsForm(devNum, byteNum, id);
    //после создания новой маски сразу посылаем сигнал на открытие формы masksettingsdialog, сообщая ей параметры маски которую нужно редактировать
}

void bitMaskObj::sendMaskToProfile(int _devNum, int _byteNum, int _id, QString _paramName, QString _paramMask, int _paramType, int _valueShift, float _valueKoef, bool _viewInLogflag)
{//забор данных из формы masksettingsdialog и отправка в профиль bitmaskobj
    if (_devNum == devNum && _byteNum == byteNum && _id == id)
    {
        paramName = _paramName;
        paramMask = _paramMask;
        paramShift = calculateParamShift();
        paramLeght = calculateParamLeight();
        paramType = _paramType;
        valueShift = _valueShift;
        valueKoef = _valueKoef;
        //wordData = _wordData;
    }
}

void bitMaskObj::maskToForm(int _devNum, int _byteNum, int _id)
//ответный сигнал от masksettingsdialog с запросом всех параметров маски bitmaskobject
//по запросу формы настроек маски сообщаем ей все параметры маски
{
    if (_devNum == devNum && _byteNum == byteNum && _id == id)
    emit maskToFormSIG(devNum, byteNum, id, paramName, paramMask, paramType, valueShift, valueKoef, viewInLogFlag, wordType);
    else if (_devNum == devNum && _byteNum == byteNum && _id == 999)
        allMasksToList(_devNum, _byteNum); //если пришёл id 999, то вызывается функция на отправку сигнала от всех масок данного байта устройства в лист масок в bytesettingsform
}

void bitMaskObj::allMasksToList(int _devNum, int _byteNum)
//сигнал от bytesettingsform с запросом всех масок байта в список (id = 999)
{
    if (_devNum == devNum && _byteNum == byteNum)
    emit maskToListSIG(devNum, byteNum, id, paramName, paramMask, paramType, valueShift, valueKoef, viewInLogFlag, wordType);
}


int bitMaskObj::calculateParamShift()
{
    int n = 0;
    int i = 0;
    paramMask4calcShift = paramMask.toInt(0,10);
    while (i!=1)
    {
        if (!(paramMask4calcShift & 0x01))
        {//вычисляем число сдвига, просто двигая маску к началу
                paramMask4calcShift = paramMask4calcShift >> 1;
                n++;
        }
        else
        {
            i = 1;
            return n;
        }
         if (n>32) break; //Eсли за максимальные 32 байта цикл не прервался, принудительно выходим
    }
}

int bitMaskObj::calculateParamLeight()
//считаем длину параметра, двигая маску дальше до первого нуля
{
    int i = 0;
    int n = 0;
    while (i==1)
    {
        if (paramMask4calcShift & 0x01)
        {
            paramMask4calcShift = paramMask4calcShift >> 1;
            n++;
        }
        else
        {
            i = 1;
            return n;
        }
         if (n>32) break; //Переделать
    }
}

void bitMaskObj::calculateValue(int wordData)
{
    int value = (wordData >> paramShift); //сдвигаем нужные нам биты к началу
    int mask4calcValue = 1; //Сдвигая единичку влево, будем по этой маске гасить лишние биты.
    mask4calcValue = mask4calcValue << paramLeght; //сдвигаем маску влево на длину нужных нам данных, чтоб их не трогать
    for (int var = paramLeght; var < sizeof(wordData); var++)
    { //забиваем нулями всё что дальше наших данных
        if (value && mask4calcValue) //если этот бит равен единице...
        {
            value |= mask4calcValue; //инвертируем его
        }
        mask4calcValue = mask4calcValue << 1;
    }
    int binRawValue = value;
    qDebug() << "Value is " << (value+valueShift)*valueKoef;
    float endValue = (value+valueShift)*valueKoef;
}

void bitMaskObj::deleteMaskObjectTX(int _devNum, int _byteNum, int _id)
{
    if (devNum == _devNum && byteNum == _byteNum && id == _id)
    {
        qDebug() << "dev " << devNum << ", byte " << byteNum << ", id " << id << " bye bye";
        this->~bitMaskObj();
    }
}
