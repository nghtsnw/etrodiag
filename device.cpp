#include "device.h"
#include "mainwindow.h"
#include <QDebug>
#include "bytedefinition.h"
#include <QBitArray>
#include <QDataStream>
#include <QTextStream>
#include "dynamicbaseprofile.h"


Device::Device(QWidget *parent) : QPushButton(parent)
{

}

Device::Device(int id, QVector<int> data) //инициализация нового устройства
{
    devNum = id;
    currState = data;
    oldState->reserve(data.size());
    //oldState = &data;
    oldState->fill(255,0);
    byteObjArr->reserve(data.size());
    byteObjectsInit(currState);
    byteObjArrOld->reserve(data.size());
    //byteObjArrOld->fill(zeroByteDef);
    //dynamicBaseProfile devPrf;// = new dynamicBaseProfile; //создаём профиль устройства
    //devPrf->addDataDev(devNum, data);
    //connect (this, &Device::setWordBitTX, devPrf, &dynamicBaseProfile::setWordBitRX);
    //connect (this, &Device::getWordTypeTX, devPrf, &dynamicBaseProfile::getWordTypeFromProfileRetranslator);
    //connect (devPrf, &dynamicBaseProfile::returnWordTypeTX, this, &Device::returnWordTypeRX);
    //connect (this, &Device::createNewMaskTX, devPrf, &dynamicBaseProfile::createNewMaskRX);
    //connect (devPrf, &dynamicBaseProfile::mask2FormTX, this, &Device::mask2FormRX);
    //connect (devPrf, &dynamicBaseProfile::maskData2FormTX, this, &Device::maskData2FormRX);
}

void Device::updateData(int id, QVector<int> devdata) //если устройство в списке уже есть, этой функцией оно обновляется
{

    if (id == devNum)
    {
        if (!(oldState->empty()))
        oldState->clear();
        for (int n = 0; n<devdata.size(); n++)
        {
        int currStateInt = byteObjArr->at(n)->th_data;
        oldState->append(currStateInt);
        }
        currState = devdata;
        byteObjectsUpd(devdata);

    }
    else qDebug() << "Dev " << devNum << " get devnum " << id << " in message, and skip this update";
}

void Device::byteObjectsInit(QVector<int> data) //инициализируем для каждого байта свой объект,
//с параметрами конкретно этого байта и значениями каждого бита, и загоняем объекты в массив
{

    int n = 0;
    while (n < data.size()) //набиваем массив ссылками на новые объекты байтов
    {
        byteDefinition *bytedef = new byteDefinition(devNum, n, data.at(n));

        connect (this, &Device::setWordBitTX, bytedef, &byteDefinition::setWordBitRX);// SIGNAL(setWordBitRX()), byteDef, SLOT()); //ВСЁ ПРОПАЛО
        connect (this, &Device::getWordTypeTX, bytedef, &byteDefinition::getWordType);
        connect (bytedef, &byteDefinition::returnWordType, this, &Device::returnWordTypeRX);
        connect (this, &Device::createNewMaskTX, bytedef, &byteDefinition::createNewMask);
        connect (bytedef, &byteDefinition::mask2FormTX, this, &Device::mask2FormRX);
        connect (bytedef, &byteDefinition::maskData2FormTX, this, &Device::maskData2FormRX);
        connect (this, &Device::requestMaskDataTX, bytedef, &byteDefinition::requestMaskDataRX);
        connect (bytedef, &byteDefinition::allMasksToListTX, this, &Device::allMasksToListRX);
        byteObjArr->append(bytedef);
        n++;
    }
}

void Device::byteObjectsUpd(QVector<int> data) //обновляем каждый объект, выявляем обновившиеся и передаём в лист изменений
{
    for (int n = 0; n < data.size(); n++)
    {
        byteObjArr->at(n)->th_data = data.at(n);
        byteObjArr->at(n)->calcWordData(devNum, data);
    }
//    uint8_t bitmask[] = {0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80};
//    int oneCurrentObj = 0;
//    int oneOldObj = 0;
//    int n = 0;
//    byteObjArr->clear();
//    while (n < data.size()) //набиваем массив ссылками на новые динамические объекты байтов
//    { //цикл перебора байтов
//        byteDefinition bytedef(devNum, n, data.at(n));
//        byteObjArr->append(&bytedef);
//        oneCurrentObj = bytedef.th_data;
//        oneOldObj = oldState->at(n);//byteObjArrOldIt.next();

//        if (oneCurrentObj != oneOldObj) //сравниваем текущую информацию байта и предыдущую
//        {
//            int nz = 0;
//            qDebug() << "current byte = " << oneCurrentObj << ", old byte = " << oneOldObj;
//            while (nz<=7) //цикл перебора битов
//            {

//                if ((oneCurrentObj & bitmask[nz]) != (oneOldObj & bitmask[nz])) //проверяю как работает метод определения изменившегося бита с помощью битовой маски (спойлер: прекрасно работает)
//                { //сравниваем побитово оба байта, ищем какие изменились
//                    QString strng;
//                    QTextStream (&strng) << "devNum = " << devNum << ", byte = " << n << ", bit = " <<  nz << ", new state = " << static_cast<bool>(oneCurrentObj & bitmask[nz]) << ", old state = " << static_cast<bool>(oneOldObj & bitmask[nz])<< '\n';   //тут будет emit сигнала в объект профиля, для демонстрации вывел в дебаг
//                    emit txtToGui(strng);
//                    qDebug() << strng;//"devNum = " << devNum << ", byte = " << n << ", bit = " << nz
//                        //<< "new state = " << static_cast<bool>(oneCurrentObj & bitmask[nz]) << ", old state = " << static_cast<bool>(oneOldObj & bitmask[nz]);
//                }
//                nz++;
//            }
//        }
//        n++;

//    }


}

void Device::clickedF()
{
    emit openDevSettSig(devNum, currState);
}

void Device::getDeviceName(int id)
{
    if (id == devNum)
    emit returnDeviceName(devNum, devPrf->getDeviceName());
}

void Device::setDeviceName(int id, QString name)
{
    if (id == devNum)
    devPrf->setDeviceName(name);
}

void Device::setWordTypeInByteProfile(int _devNum, int _byteNum, int _wordType)
{//по изменению битбокса в форме bytesettingsform, отправляем значение в bytedefinition
    emit setWordBitTX(_devNum, _byteNum, _wordType);
}

void Device::getWordTypeFromProfileRetranslator(int _devNum, int _byteNum)
{//при создании формы bytesettingsform, отправляем запрос на длину слова в bytedefinition
    emit getWordTypeTX(_devNum, _byteNum);
}

void Device::returnWordTypeRX(int _devNum, int _byteNum, int wordType)
{//возврат значения длины слова из bytedefinition в ответ на запрос из bytesettingsform
    emit returnWordTypeTX(_devNum, _byteNum, wordType);
}

void Device::createNewMaskRX(int _devNum, int _byteNum)
{//по нажатию кнопки добавления маски в bytesettingsform, отправляем сигнал в bytedefinition на создание маски
    emit createNewMaskTX(_devNum, _byteNum);
}

void Device::mask2FormRX(int _devNum, int _byteNum, int _id)
{//после создания новой маски сразу посылаем сигнал на открытие формы masksettingsdialog, сообщая ей параметры маски которую нужно редактировать
    emit mask2FormTX(_devNum, _byteNum, _id);
}

void Device::requestMaskDataRX(int _devNum, int _byteNum, int _id)
{//ответный сигнал от masksettingsdialog с запросом всех параметров маски bitmaskobject
    emit requestMaskDataTX(_devNum, _byteNum, _id);
}

void Device::maskData2FormRX(int _devNum, int _byteNum, int _id, QString _paramName, int _paramMask, int _paramType, int _valueShift, float _valueKoef, bool _viewInLogFlag, int wordType)
{//ответный сигнал со всеми данными маски bitmaskobj в masksettingsdialog
    emit maskData2FormTX(_devNum, _byteNum, _id, _paramName, _paramMask, _paramType, _valueShift, _valueKoef, _viewInLogFlag, wordType);
}

void Device::sendDataToProfileRX(int _devNum, int _byteNum, int _id, QString _paramName, int _paramMask, int _paramType, int _valueShift, float _valueKoef, bool _viewInLogFlag)
{//забор данных из формы masksettingsdialog и отправка в профиль bitmaskobj
    emit sendDataToProfileTX(_devNum, _byteNum, _id, _paramName, _paramMask, _paramType, _valueShift, _valueKoef, _viewInLogFlag);
}

void Device::allMasksToListRX(int devNum, int byteNum, int id, QString paramName, int paramMask, int paramType, int valueShift, float valueKoef, bool viewInLogFlag, int wordType)
{//сигнал от bitmaskobj предназначенный для bytesettingsform, для наполнения листа масок всеми имеющимися у этого байта
    emit allMasksToListTX(devNum, byteNum, id, paramName, paramMask, paramType, valueShift, valueKoef, viewInLogFlag, wordType);
}
