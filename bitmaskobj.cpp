#include "bitmaskobj.h"
#include <QDebug>
#include <cmath>

bitMaskObj::bitMaskObj()
{
    currentMask.parameterName = "Parameter";
    currentMask.parameterMask = "00000000";
    currentMask.parameterShift = 0;
    currentMask.parameterLeight = 0;
    currentMask.wordType = 0;
    currentMask.valueShift = 0;
    currentMask.valueKoef = 1;
    currentMask.wordData = 0;
    currentMask.wordType = 0;
    currentMask.viewInLogFlag = true;
    currentMask.drawGraphFlag = false;
}

bitMaskObj::~bitMaskObj()
{
}

void bitMaskObj::newMaskObj(s_parameterMask mask)
{
    currentMask.id = mask.id;
    currentMask.devNum = mask.devNum;
    currentMask.byteNum = mask.byteNum;
    //emit mask2byteSettingsForm(mask);
    //после создания новой маски сразу посылаем сигнал на открытие формы masksettingsdialog, сообщая ей параметры маски которую нужно редактировать
}


void bitMaskObj::sendMaskToProfile(s_parameterMask mask)
{ //забор данных из формы masksettingsdialog и отправка в профиль bitmaskobj
    if (mask.devNum == currentMask.devNum && mask.byteNum == currentMask.byteNum && mask.id == currentMask.id)
    {
        oldMask = currentMask;
        currentMask = mask;
        recalcMask();
        calculateParamShift();
    }
}

void bitMaskObj::maskToForm(int r_devNum, int r_byteNum, int r_id)
//ответный сигнал от masksettingsdialog с запросом всех параметров маски bitmaskobject
//по запросу формы настроек маски сообщаем ей все параметры маски
{
    if (r_devNum == currentMask.devNum && r_byteNum == currentMask.byteNum && r_id == currentMask.id) {
        emit maskToFormSIG(currentMask);
    }
    else if (r_devNum == currentMask.devNum && r_byteNum == currentMask.byteNum && r_id == 999) {
        allMasksToList(r_devNum, r_byteNum); //если пришёл id 999, то вызывается функция на отправку сигнала от всех масок данного байта устройства в лист масок в bytesettingsform
    }
}

void bitMaskObj::allMasksToList(int r_devNum, int r_byteNum)
//сигнал от bytesettingsform с запросом всех масок байта в список (id = 999)
{
    if (r_devNum == currentMask.devNum && r_byteNum == currentMask.byteNum)
    {
        qDebug() << "emit mask " << currentMask.parameterName << " to saving";
        emit maskToListSIG(currentMask);
    }
}

void bitMaskObj::calculateParamShift()
{
    int wordTypeInt;
    switch (currentMask.wordType) {
    case 0:
        wordTypeInt = 8;
        break;
    case 1:
        wordTypeInt = 16;
        break;
    case 2:
        wordTypeInt = 32;
        break;
    default:
        break;
    }
    int n = 0;
    bool stopFlag = false;
    if (currentMask.parameterMask != oldMask.parameterMask)
    { //если маска изменилась - заново её вычисляем
        recalcMask();
        //currentMask.parameterMask = oldMask.parameterMask;
    }
    paramMask4calcShift = currentMask.parameterMask.toInt(0, 10);
    while(!stopFlag)
    {
        if (!(paramMask4calcShift & 0x01))
        { //вычисляем число сдвига, просто двигая маску к началу до первой единицы
            paramMask4calcShift = paramMask4calcShift >> 1;
            n++;
        }
        else if (paramMask4calcShift & 0x01)
        {
            currentMask.parameterShift = n;
            stopFlag = true;
        }
        if (n > wordTypeInt)
        {
            currentMask.parameterShift = 0;
            n = 0;
            stopFlag = true;
        }//Eсли за максимальные 32 бита цикл не прервался, прекращаем
    }
}

//void bitMaskObj::calculateParamLeight()
////считаем длину параметра, двигая маску дальше до первого нуля
//{
// int i = 0;
// int n = 0;
// while (i==1)
// {
// if (paramMask4calcShift & 0x01)
// {
// paramMask4calcShift = paramMask4calcShift >> 1;
// n++;
// }
// else
// {
// i = 1;
// //qDebug() << "ParamLeight = " << n;
// paramLeght = n;
// }
// if (n>32)
// {
// paramLeght = 0;
// break;
// }
// }
// //qDebug() << "ParamLeight = " << n;
//}

void bitMaskObj::calculateValue(int _devNum, int _byteNum, uint32_t wordData)
{
    if (currentMask.devNum == _devNum && currentMask.byteNum == _byteNum)
    {
        if (currentMask.parameterMask != oldMask.parameterMask)
        { //если маска изменилась - заново её вычисляем
            recalcMask();
            currentMask.parameterMask = oldMask.parameterMask;
        }
        uint32_t value = (wordData & paramMaskInt);
        value = value >> currentMask.parameterShift; //сдвигаем нужные нам биты к началу
        //int binRawValue = value;
        double endValue = (value + currentMask.valueShift) * currentMask.valueKoef;
        if (endValue == oldEndValue) {
            isNewData = false;
        }
        else {
            isNewData = true;
        }
        //if (endValue != oldEndValue || oldEndValue == 1234.56)
        oldEndValue = endValue;
        currentMask.endValue = endValue;
        emit param2FrontEnd(currentMask);
    }
}

void bitMaskObj::recalcMask()
{
    paramMaskInt = 0;
    for (int i = currentMask.parameterMask.size() - 1, y = 0; i > -1; i--, y++) //переводим маску из строки нулей и единиц в число int
    {
        if (currentMask.parameterMask.at(i) == '1') {
            paramMaskInt += pow(2, y);
        }
    }
}

void bitMaskObj::deleteMaskObjectTX(int devNum, int byteNum, int id)
{
    if (currentMask.devNum == devNum && currentMask.byteNum == byteNum && currentMask.id == id)
    {
        this->~bitMaskObj();
    }
}

void bitMaskObj::loadMaskRX(s_parameterMask mask)
{
    if (currentMask.devNum == mask.devNum && currentMask.byteNum == mask.byteNum && currentMask.id == mask.id)
    {
        currentMask = mask;
        recalcMask();
        calculateParamShift();
    }
}
