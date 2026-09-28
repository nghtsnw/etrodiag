#include "bitmaskobj.h"
#include "maskmath.h"
#include <QDebug>

bitMaskObj::bitMaskObj()
{
    currentMask.parameterName = "Parameter";
    currentMask.parameterMask = "00000000";
    currentMask.parameterShift = 0;
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
    else if (r_devNum == currentMask.devNum && r_byteNum == currentMask.byteNum && r_id == kAllMasksMaskId) {
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
    if (currentMask.parameterMask != oldMask.parameterMask)
    { //если маска изменилась - заново её вычисляем
        recalcMask();
    }
    currentMask.parameterShift = maskmath::parameterShift(currentMask.parameterMask, currentMask.wordType);
}

void bitMaskObj::calculateValue(int _devNum, int _byteNum, uint32_t wordData)
{
    if (currentMask.devNum == _devNum && currentMask.byteNum == _byteNum)
    {
        if (currentMask.parameterMask != oldMask.parameterMask)
        { //если маска изменилась - заново её вычисляем
            recalcMask();
        }
        uint32_t value = (wordData & paramMaskInt);
        value = value >> currentMask.parameterShift; //сдвигаем нужные нам биты к началу
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
    paramMaskInt = maskmath::maskToInt(currentMask.parameterMask); //переводим маску из строки нулей и единиц в число
}

void bitMaskObj::deleteMaskObjectTX(int devNum, int byteNum, int id)
{
    if (currentMask.devNum == devNum && currentMask.byteNum == byteNum && currentMask.id == id)
    {
        deleteLater(); //самоудаление через очередь событий безопаснее, чем delete this
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
