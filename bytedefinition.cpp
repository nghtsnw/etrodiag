#include "bytedefinition.h"
#include <QDebug>
#include "bitmaskobj.h"
#include <QByteArray>
#include <QDataStream>
#include "cmath"
//Создаётся для каждого байта при инициализации устройства.
//Данные отсюда будут подтягиваться в гуй параметров байта, и сюда же сохраняться.
//Вместе с параметрами устройства (dynamicbaseprofile) данные будут сохраняться в файл.
byteDefinition::byteDefinition()//(QObject *parent)
{
    devNum = 0;
    th_byteNum = 0;
    th_data = 0;
}

byteDefinition::byteDefinition(int numDev, int byteNum, int data)
{
    devNum = numDev;
    th_byteNum = byteNum;
    th_data = data;
}

 byteDefinition::~byteDefinition()
{

}

void byteDefinition::updateSlot(int _devNum, int _byteNum, QVector<int> _data)
{
    if (devNum == _devNum && th_byteNum == _byteNum)
    {
    th_data = _data.at(_byteNum);
    calcWordData(devNum, _data);
    }
}

void byteDefinition::setWordBitRX(int _devNum, int _byteNum, int _argBit)
{//по изменению битбокса в форме bytesettingsform, отправляем значение в byteDefinition
    if (devNum == _devNum && th_byteNum == _byteNum)
    {
        wordType = _argBit;
    }
}

void byteDefinition::getWordType(int _devNum, int _byteNum)
{//при создании формы bytesettingsform, отправляем запрос на длину слова в bytedefinition
    if (devNum == _devNum && th_byteNum == _byteNum)
    emit returnWordType(_devNum, _byteNum, wordType);    
}

void byteDefinition::getByteNameRX(int _devNum, int _byteNum)
{
    if (devNum == _devNum && th_byteNum == _byteNum)
        emit returnByteName(_devNum, _byteNum, byteName);
}

void byteDefinition::saveByteNameRX(int _devNum, int _byteNum, QString _byteName)
{
    if (devNum == _devNum && th_byteNum == _byteNum)
        byteName = _byteName;
}

void byteDefinition::createNewMask(int _devNum, int _byteNum)
{//по нажатию кнопки добавления маски в bytesettingsform, отправляем сигнал в bytedefinition на создание маски
    //найти всех детей типа bitMaskObj, что-бы присвоить маске айди
    if (_devNum == devNum && _byteNum == th_byteNum)
    {
        int id = calcMaskID();
        tmpMaskId = id;
        bitMaskObj *mask = new bitMaskObj;
        mask->wordType = wordType;
        mask->setParent(this);
        connect (mask, &bitMaskObj::mask2byteSettingsForm, this, &byteDefinition::mask2FormRX); //открытие формы
        connect (this, &byteDefinition::requestMaskDataTX, mask, &bitMaskObj::maskToForm);//запрос от формы
        connect (mask, &bitMaskObj::maskToFormSIG, this, &byteDefinition::maskData2FormRX);//ответ форме
        connect (this, &byteDefinition::sendDataToProfileTX, mask, &bitMaskObj::sendMaskToProfile);
        connect (this, &byteDefinition::wordData2Mask, mask, &bitMaskObj::calculateValue);
        connect (mask, &bitMaskObj::maskToListSIG, this, &byteDefinition::allMasksToListRX);
        connect (this, &byteDefinition::sendDataToProfileTX, mask, &bitMaskObj::sendMaskToProfile);
        connect (this, &byteDefinition::deleteMaskObjTX, mask, &bitMaskObj::deleteMaskObjectTX);
        connect (mask, &bitMaskObj::param2FrontEnd, this, &byteDefinition::param2FrontEndRX);
        connect (this, &byteDefinition::loadMaskTX, mask, &bitMaskObj::loadMaskRX);
        mask->newMaskObj(devNum, th_byteNum, id);
    }
}

void byteDefinition::loadMaskRX(int devNum, QString devName, int byteNum, QString _byteName, int id, QString paramName, QString paramMask, int paramType, double valueShift, double valueKoef, bool viewInLogFlag, int wordType)
{
    byteName = _byteName;
    setWordBitRX(devNum,byteNum,wordType);
    createNewMask(devNum, byteNum);
    emit loadMaskTX(devNum, devName, byteNum, byteName, tmpMaskId, paramName, paramMask, paramType, valueShift, valueKoef, viewInLogFlag, wordType);
}

int byteDefinition::calcMaskID()
{
    int id = 0;
    bool notFoundFlag = 0;
    QList<bitMaskObj*> bytedefChildList = this->findChildren<bitMaskObj*>();
    QListIterator<bitMaskObj*> bytedefChildListIt(bytedefChildList);

    if (bytedefChildList.size() != 0)
    {
    for (int n = 0; notFoundFlag == 0 ; n++)
    {
        notFoundFlag = 1;
        while (bytedefChildListIt.hasNext())
        {//прогоняем число n по всем id
            if (n == bytedefChildListIt.next()->id)
            notFoundFlag = 0;//если маска с таким id хоть раз попалась, то скидываем флаг
        }
        bytedefChildListIt.toFront();
        //если после работы цикла флаг остался в состоянии 1, то назначаем ненайденый id новой маске
        //цикл for прекратится по условию достижения notFoundFlag != 0
        if (notFoundFlag == 1)
        {
            id = n;
        }
    }
    }
    return id;
}

void byteDefinition::mask2FormRX(int _devNum, int _byteNum, int _id)
{//после создания новой маски сразу посылаем сигнал на открытие формы masksettingsdialog, сообщая ей параметры маски которую нужно редактировать
    if (devNum == _devNum && th_byteNum == _byteNum)
    emit mask2FormTX(_devNum, _byteNum, _id);
}

void byteDefinition::requestMaskDataRX(int _devNum, int _byteNum, int _id)
{//ответный сигнал от masksettingsdialog с запросом всех параметров маски bitmaskobject
    if (devNum == _devNum && th_byteNum == _byteNum)
    emit requestMaskDataTX(_devNum, _byteNum, _id);
}

void byteDefinition::maskData2FormRX(int _devNum, int _byteNum, int _id, QString _paramName, QString _paramMask, int _paramType, double _valueShift, double _valueKoef, bool _viewInLogFlag, int _wordType)
{//ответный сигнал со всеми данными маски bitmaskobj в masksettingsdialog
    if (devNum == _devNum && th_byteNum == _byteNum)
    emit maskData2FormTX(_devNum, _byteNum, _id, _paramName, _paramMask, _paramType, _valueShift, _valueKoef, _viewInLogFlag, _wordType);
}

void byteDefinition::sendDataToProfileRX(int _devNum, int _byteNum, int _id, QString _paramName, QString _paramMask, int _paramType, double _valueShift, double _valueKoef, bool _viewInLogFlag)
{//забор данных из формы masksettingsdialog и отправка в профиль bitmaskobj
    if (devNum == _devNum && th_byteNum == _byteNum)
    emit sendDataToProfileTX(_devNum, _byteNum, _id, _paramName, _paramMask, _paramType, _valueShift, _valueKoef, _viewInLogFlag);
}

void byteDefinition::allMasksToListRX(int devNum, int byteNum, int id, QString paramName, QString paramMask, int paramType, double valueShift, double valueKoef, bool viewInLogFlag, int wordType)
{//сигнал от bitmaskobj предназначенный для bytesettingsform, для наполнения листа масок всеми имеющимися у этого байта
    emit allMasksToListTX(devNum, byteNum, byteName, id, paramName, paramMask, paramType, valueShift, valueKoef, viewInLogFlag, wordType);
}

void byteDefinition::calcWordData(int _devNum, QVector<int> data)
{//формируем слово из полных данных устройства и заданной длины, и рассылаем слово маскам
    wordData = 0;
    uint32_t mask = 1;
    int bytex = 0;
    int step = 0;

    if (_devNum == devNum)
    {
    if (wordType == 0)
        wordData = (data.at(th_byteNum));
    else if (wordType == 1)
    {
        for (int y = 0; y <= 1; y++)
        {
            bytex = (data.at(th_byteNum+y));
        for (int i = 0, mask = 1; i <= 7; i++, step++, mask = mask << 1)
        {
            if (bytex & mask)
                wordData+=pow(2,step);
        }
        }
    }
        else if (wordType == 2)
    {
        for (int y = 0; y <= 3; y++)
        {
            bytex = (data.at(th_byteNum+y));
        for (int i = 0, mask = 1; i <= 7; i++, step++, mask = mask << 1)
        {
            if (bytex & mask)
                wordData+=pow(2,step);
        }
        }
    }
    emit wordData2Mask(devNum, th_byteNum, wordData);
    }
}

void byteDefinition::param2FrontEndRX(int devNum, int byteNum, uint32_t wordData, int id, QString parameterName, int binRawValue, double endValue, bool viewInLogFlag, bool isNewData)
{
    emit param2FrontEndTX(devNum, byteNum, byteName, wordData, id, parameterName, binRawValue, endValue, viewInLogFlag, isNewData);
}
