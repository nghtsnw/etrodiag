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
    bitMask.id = _id;
    bitMask.devNum = _devNum;
    bitMask.byteNum = _byteNum;
    emit mask2byteSettingsForm(bitMask.devNum, bitMask.byteNum, bitMask.id);
    //после создания новой маски сразу посылаем сигнал на открытие формы masksettingsdialog, сообщая ей параметры маски которую нужно редактировать
}


void bitMaskObj::sendMaskToProfile(int _devNum, int _byteNum, int _id, QString _paramName, QString _paramMask, int _paramType, double _valueShift, double _valueKoef, bool _viewInLogflag, bool _drawGraphFlag, QString _drawGraphColor)
{//забор данных из формы masksettingsdialog и отправка в профиль bitmaskobj
    if (_devNum == bitMask.devNum && _byteNum == bitMask.byteNum && _id == bitMask.id)
    {
        bitMask.paramName = _paramName;
        paramMaskNew = _paramMask;
        recalcMask();
        calculateParamShift();        
//        calculateParamLeight();
        bitMask.paramType = _paramType;
        bitMask.valueShift = _valueShift;
        bitMask.valueKoef = _valueKoef;
        bitMask.viewInLogFlag = _viewInLogflag;
        bitMask.drawGraphFlag = _drawGraphFlag;
        bitMask.drawGraphColor = _drawGraphColor;
    }
}

void bitMaskObj::maskToForm(int _devNum, int _byteNum, int _id)
//ответный сигнал от masksettingsdialog с запросом всех параметров маски bitmaskobject
//по запросу формы настроек маски сообщаем ей все параметры маски
{
    if (_devNum == bitMask.devNum && _byteNum == bitMask.byteNum && _id == bitMask.id)
    emit maskToFormSIG(bitMask);
    else if (_devNum == bitMask.devNum && _byteNum == bitMask.byteNum && _id == 999)
        allMasksToList(_devNum, _byteNum); //если пришёл id 999, то вызывается функция на отправку сигнала от всех масок данного байта устройства в лист масок в bytesettingsform
}

void bitMaskObj::allMasksToList(int _devNum, int _byteNum)
//сигнал от bytesettingsform с запросом всех масок байта в список (id = 999)
{
    if (_devNum == bitMask.devNum && _byteNum == bitMask.byteNum)
    {
        qDebug() << "emit mask " << bitMask.paramName << " to saving";
        emit maskToListSIG(bitMask);
    }
}


void bitMaskObj::calculateParamShift()
{
    int wordTypeInt;
    if (bitMask.wordType == 0) wordTypeInt = 8;
    else if (bitMask.wordType == 1) wordTypeInt = 16;
    else if (bitMask.wordType == 2) wordTypeInt = 32;
    int n = 0;
    bool stopFlag = false;    
    if (bitMask.paramMask != paramMaskNew)
    {//если маска изменилась - заново её вычисляем
        recalcMask();
        bitMask.paramMask = paramMaskNew;
    }
    paramMask4calcShift = bitMask.paramMask.toInt(0,10);
    while(!stopFlag)
    {

        if (!(paramMask4calcShift & 0x01))
        {//вычисляем число сдвига, просто двигая маску к началу до первой единицы
                paramMask4calcShift = paramMask4calcShift >> 1;
                n++;                
        }
        else if (paramMask4calcShift & 0x01)
        {
            paramShift = n;
            stopFlag = true;
        }

        if (n > wordTypeInt)
        {            
            paramShift = 0;
             n = 0;
             stopFlag = true;
        }//Eсли за максимальные 32 бита цикл не прервался, прекращаем
    }
}

//void bitMaskObj::calculateParamLeight()
////считаем длину параметра, двигая маску дальше до первого нуля
//{
//    int i = 0;
//    int n = 0;
//    while (i==1)
//    {
//        if (paramMask4calcShift & 0x01)
//        {
//            paramMask4calcShift = paramMask4calcShift >> 1;
//            n++;
//        }
//        else
//        {
//            i = 1;
//            //qDebug() << "ParamLeight = " << n;
//            paramLeght = n;
//        }
//        if (n>32)
//        {
//             paramLeght = 0;
//             break;
//        }
//    }
//    //qDebug() << "ParamLeight = " << n;
//}

void bitMaskObj::calculateValue(int _devNum, int _byteNum, uint32_t wordData)
{
    if (bitMask.devNum == _devNum && bitMask.byteNum == _byteNum)
    {
        if (bitMask.paramMask != paramMaskNew)
        {//если маска изменилась - заново её вычисляем
            recalcMask();
            bitMask.paramMask = paramMaskNew;
        }
        uint32_t value = (wordData & paramMaskInt);
        value = value >> paramShift; //сдвигаем нужные нам биты к началу
        bitMask.binRawValue = value;
        bitMask.endValue = (value + bitMask.valueShift) * bitMask.valueKoef;
        if (bitMask.endValue == oldEndValue) bitMask.isNewData = false;
        else bitMask.isNewData = true;
        //if (endValue != oldEndValue || oldEndValue == 1234.56)
        bitMask.wordData = wordData;
        emit param2FrontEnd(bitMask);
        oldEndValue = bitMask.endValue;
    }
}

void bitMaskObj::recalcMask()
{
        paramMaskInt = 0;
        for (int i = paramMaskNew.size()-1, y = 0; i > -1; i--, y++) //переводим маску из строки нулей и единиц в число int
        {
            if (paramMaskNew.at(i) == '1')
                paramMaskInt+=pow(2,y);
        }
}

void bitMaskObj::deleteMaskObjectTX(int _devNum, int _byteNum, int _id)
{
    if (bitMask.devNum == _devNum && bitMask.byteNum == _byteNum && bitMask.id == _id)
    {
        this->~bitMaskObj();
    }
}

void bitMaskObj::loadMaskRX(int _devNum, QString _devName, int _byteNum, QString _byteName, int _id, QString _paramName, QString _paramMask, int _paramType, double _valueShift, double _valueKoef, bool _viewInLogFlag, int _wordType, bool _drawGraphFlag, QString _drawGraphColor)
{
    if (_devNum == bitMask.devNum && _byteNum == bitMask.byteNum && bitMask.id == _id)
    {
        bitMask.paramName = _paramName;
        paramMaskNew = _paramMask;
        recalcMask();
        calculateParamShift();
//        calculateParamLeight();
        bitMask.paramType = _paramType;
        bitMask.valueShift = _valueShift;
        bitMask.valueKoef = _valueKoef;
        bitMask.viewInLogFlag = _viewInLogFlag;
        bitMask.drawGraphFlag = _drawGraphFlag;
        bitMask.drawGraphColor = _drawGraphColor;
}
}
