#include "bytedefinition.h"
#include "wordvalue.h"
#include <QDebug>
#include "bitmaskobj.h"
#include <QByteArray>
#include <QDataStream>
//Создаётся для каждого байта при инициализации устройства.
//Данные отсюда будут подтягиваться в гуй параметров байта, и сюда же сохраняться.
//Вместе с параметрами устройства (dynamicbaseprofile) данные будут сохраняться в файл.
byteDefinition::byteDefinition()
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

void byteDefinition::updateSlot(int _devNum, QVector<int> _data)
{
    if (devNum == _devNum && th_byteNum >= 0 && th_byteNum < _data.size())
    {
        th_data = _data.at(th_byteNum);
        calcWordData(devNum, _data);
    }
}

void byteDefinition::setWordBitRX(int _devNum, int _byteNum, int _argBit)
{ //по изменению битбокса в форме bytesettingsform, отправляем значение в byteDefinition
    if (devNum == _devNum && th_byteNum == _byteNum)
    {
        wordType = _argBit;
    }
}

void byteDefinition::getWordType(int _devNum, int _byteNum)
{ //при создании формы bytesettingsform, отправляем запрос на длину слова в bytedefinition
    if (devNum == _devNum && th_byteNum == _byteNum) {
        emit returnWordType(_devNum, _byteNum, wordType);
    }
}

void byteDefinition::createNewMask(int _devNum, int _byteNum)
{ //по нажатию кнопки добавления маски в bytesettingsform, отправляем сигнал в bytedefinition на создание маски
    //найти всех детей типа bitMaskObj, что-бы присвоить маске айди
    if (_devNum == devNum && _byteNum == th_byteNum)
    {
        bitMaskObj *mask = new bitMaskObj;
        mask->currentMask.wordType = wordType;
        mask->currentMask.devNum = devNum;
        mask->currentMask.byteNum = _byteNum;
        mask->currentMask.id = calcMaskID();
        mask->setParent(this);
        connect (this, &byteDefinition::requestMaskDataRX, this, [this](int r_devNum, int r_byteNum, int r_id) { //ответный сигнал от masksettingsdialog с запросом всех параметров маски bitmaskobject
            if (devNum == r_devNum && th_byteNum == r_byteNum) {
                emit requestMaskDataTX(r_devNum, r_byteNum, r_id);
            }
        });
        connect (this, &byteDefinition::requestMaskDataTX, mask, &bitMaskObj::maskToForm);//запрос от формы
        connect (mask, &bitMaskObj::maskToFormSIG, this, [this](s_parameterMask answer) { //ответный сигнал со всеми данными маски bitmaskobj в masksettingsdialog
            if (devNum == answer.devNum && th_byteNum == answer.byteNum) {
                emit maskData2FormTX(answer);
            }
        });
        connect (this, &byteDefinition::sendDataToProfileRX, this, [this](s_parameterMask mask) { //забор данных из формы masksettingsdialog и отправка в профиль bitmaskobj
            if (devNum == mask.devNum && th_byteNum == mask.byteNum) {
                emit sendDataToProfileTX(mask);
            }
        });
        connect (this, &byteDefinition::sendDataToProfileTX, mask, &bitMaskObj::sendMaskToProfile);
        connect (this, &byteDefinition::wordData2Mask, mask, &bitMaskObj::calculateValue);
        connect (mask, &bitMaskObj::maskToListSIG, this, &byteDefinition::allMasksToListTX);
        connect (this, &byteDefinition::deleteMaskObjTX, mask, &bitMaskObj::deleteMaskObjectTX);
        connect (mask, &bitMaskObj::param2FrontEnd, this, &byteDefinition::param2FrontEndTX);
        connect (this, &byteDefinition::loadMaskTX, mask, &bitMaskObj::loadMaskRX);
        mask->newMaskObj(mask->currentMask);
        emit mask2FormTX(mask->currentMask.devNum, mask->currentMask.byteNum, mask->currentMask.id);//mask2FormRX(mask->currentMask);
    }
}

void byteDefinition::countMasks()
{ //сообщаем количество масок байта, в том числе нулевое:
  //устройство кэширует сумму, поэтому устаревшее значение исказит счёт
    emit returnMaskCountForThisByte(devNum, th_byteNum, this->findChildren<bitMaskObj*>().count());
}

void byteDefinition::loadMaskRX(s_parameterMask mask)
{
    byteName = mask.byteName;
    setWordBitRX(mask.devNum, mask.byteNum, mask.wordType);
    createNewMask(mask.devNum, mask.byteNum);
    emit loadMaskTX(mask);
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
            { //прогоняем число n по всем id
                if (n == bytedefChildListIt.next()->currentMask.id) {
                    notFoundFlag = 0; //если маска с таким id хоть раз попалась, то скидываем флаг
                }
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

void byteDefinition::calcWordData(int _devNum, QVector<int> data)
{ //формируем слово из полных данных устройства и заданной длины, и рассылаем слово маскам
    if (_devNum != devNum) {
        return;
    }
    wordData = wordvalue::assemble(data, th_byteNum, wordType);
    emit wordData2Mask(devNum, th_byteNum, wordData);
}
