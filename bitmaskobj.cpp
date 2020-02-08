#include "bitmaskobj.h"
#include <QDebug>
#include <cmath>

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


void bitMaskObj::sendMaskToProfile(int _devNum, int _byteNum, int _id, QString _paramName, QString _paramMask, int _paramType, double _valueShift, double _valueKoef, bool _viewInLogflag)
{//забор данных из формы masksettingsdialog и отправка в профиль bitmaskobj
    if (_devNum == devNum && _byteNum == byteNum && _id == id)
    {
        paramName = _paramName;
        paramMask = _paramMask;
        calculateParamShift();        
        calculateParamLeight();        
        paramType = _paramType;
        valueShift = _valueShift;
        valueKoef = _valueKoef;
        viewInLogFlag = _viewInLogflag;        
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


void bitMaskObj::calculateParamShift()
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
            qDebug() << "ParamShift = " << n;
            paramShift = n;
        }
        if (n>32)
        {
             paramShift = 0;
             break;
        }//Eсли за максимальные 32 байта цикл не прервался, принудительно выходим
    }
    qDebug() << "ParamShift = " << n;
}

void bitMaskObj::calculateParamLeight()
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
            qDebug() << "ParamLeight = " << n;
            paramLeght = n;
        }
        if (n>32)
        {
             paramLeght = 0;
             break;
        }
    }
    qDebug() << "ParamLeight = " << n;
}

void bitMaskObj::calculateValue(int _devNum, int _byteNum, uint32_t wordData)
{
    if (devNum == _devNum && byteNum == _byteNum)
    {

        uint32_t paramMaskInt = 0;
        for (int i = paramMask.size()-1, y = 0; i > -1; i--, y++) //переводим маску из строки нулей и единиц в число int
        {//тодо: перенести это вычисление в masksettingsdialog, и отправлять сигналом уже готовое число а не строку
            if (paramMask.at(i) == '1')
               paramMaskInt+=pow(2,y);
        }
        uint32_t value = (wordData & paramMaskInt);
        value = value >> paramShift; //сдвигаем нужные нам биты к началу
        int binRawValue = value;
        qDebug() << "Value is " << value;
        double endValue = (value+valueShift)*valueKoef;
        emit param2FrontEnd(devNum, byteNum, wordData, id, paramName, binRawValue, endValue, viewInLogFlag);
        oldEndValue = endValue;
    }
}

void bitMaskObj::deleteMaskObjectTX(int _devNum, int _byteNum, int _id)
{
    if (devNum == _devNum && byteNum == _byteNum && id == _id)
    {
        qDebug() << "dev " << devNum << ", byte " << byteNum << ", id " << id << " bye bye";
        this->~bitMaskObj();
    }
}

void bitMaskObj::loadMaskRX(int _devNum, QString _devName, int _byteNum, QString _byteName, int _id, QString _paramName, QString _paramMask, int _paramType, double _valueShift, double _valueKoef, bool _viewInLogFlag, int _wordType)
{
    if (_devNum == devNum && _byteNum == byteNum && id == _id)
    {
        paramName = _paramName;
        paramMask = _paramMask;
        calculateParamShift();
        calculateParamLeight();
        paramType = _paramType;
        valueShift = _valueShift;
        valueKoef = _valueKoef;
        viewInLogFlag = _viewInLogFlag;
}
}
