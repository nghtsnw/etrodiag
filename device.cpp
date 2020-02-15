#include "device.h"
#include "mainwindow.h"
#include <QDebug>
#include "bytedefinition.h"
#include <QBitArray>
#include <QDataStream>
#include <QTextStream>
#include <QTimer>


Device::Device(QWidget *parent) : QPushButton(parent)
{

}

Device::Device(int id) //инициализация нового устройства
{
    devNum = id;
}

void Device::updateData(int id, QVector<int> devdata) //если устройство в списке уже есть, этой функцией оно обновляется
{    
    if (id == devNum)
    {
        currState = devdata;
        if (!byteObjReady)
            byteObjectsInit(currState);
        byteObjectsUpd(devdata);
        if (devStatus == "offline") devStatus = "online";
        changeButtonColor(devStatus);
        devOnlineWatchdog(5000);
    }
//    else qDebug() << "Dev " << devNum << " get devnum " << id << " in message, and skip this update";
}

void Device::byteObjectsInit(QVector<int> data) //инициализируем для каждого байта свой объект,
//с параметрами конкретно этого байта и значениями каждого бита, и загоняем объекты в массив
{

    int n = data.size()-1;
    while (n != 0)//набиваем массив ссылками на новые объекты байтов
    {
        byteDefinition *bytedef = new byteDefinition(devNum, n, data.at(n));

        connect (this, &Device::setWordBitTX, bytedef, &byteDefinition::setWordBitRX);
        connect (this, &Device::getWordTypeTX, bytedef, &byteDefinition::getWordType);
        connect (bytedef, &byteDefinition::returnWordType, this, &Device::returnWordTypeRX);
        connect (this, &Device::createNewMaskTX, bytedef, &byteDefinition::createNewMask);
        connect (bytedef, &byteDefinition::mask2FormTX, this, &Device::mask2FormRX);
        connect (bytedef, &byteDefinition::maskData2FormTX, this, &Device::maskData2FormRX);
        connect (this, &Device::requestMaskDataTX, bytedef, &byteDefinition::requestMaskDataRX);
        connect (bytedef, &byteDefinition::allMasksToListTX, this, &Device::allMasksToListRX);
        connect (this, &Device::sendDataToProfileTX, bytedef, &byteDefinition::sendDataToProfileRX);
        connect (this, &Device::deleteMaskObjTX, bytedef, &byteDefinition::deleteMaskObjTX);
        connect (bytedef, &byteDefinition::param2FrontEndTX, this, &Device::param2FrontEndRX);
        connect (this, &Device::loadMaskTX, bytedef, &byteDefinition::loadMaskRX);
        connect (this, &Device::getByteNameTX, bytedef, &byteDefinition::getByteNameRX);
        connect (bytedef, &byteDefinition::returnByteName, this, &Device::returnByteNameTX);
        connect (this, &Device::saveByteNameTX, bytedef, &byteDefinition::saveByteNameRX);
        connect (this, &Device::byteObjUpdSig, bytedef, &byteDefinition::updateSlot);
        //byteObjArr->append(bytedef);
        n--;
    }
    byteObjReady = true;
}

void Device::byteObjectsUpd(QVector<int> data) //обновляем каждый объект, выявляем обновившиеся и передаём в лист изменений
{
    for (int n = data.size()-1; n > -1; n--)
    {
        emit byteObjUpdSig(devNum, n, data);
    }
}

void Device::clickedF()
{
    emit openDevSettSig(devNum, currState);
}

void Device::getDeviceName(int id)
{//при открытии формы devsettingsform, она запрашивает имя устройства, тут устройство отвечает на запрос
    if (id == devNum)
    emit returnDeviceName(devNum, devName);
}

void Device::getByteNameRX(int _devNum, int byteNum)
{
    if (devNum == _devNum)
        emit getByteNameTX(_devNum, byteNum);
}

void Device::saveByteNameRX(int _devNum, int _byteNum, QString _byteName)
{
    if (devNum == _devNum)
        emit saveByteNameTX(_devNum, _byteNum, _byteName);
}

void Device::setDeviceName(int id, QString name)
{
    if (id == devNum)
    {
        devName = name; //найти почему вызывается два раза
        this->setText(devName);
    }
}

void Device::requestMasks4Saving()
{
    //каждому байту устройства отправляем сигнал на выдачу всех масок
    for (int i = 0; i <= currState.size(); i++) {
        requestMaskDataRX(devNum, i, 999);
    }
}

void Device::loadMaskRX(int devNum, QString devName, int byteNum, QString byteName, int id, QString paramName, QString paramMask, int paramType, double valueShift, double valueKoef, bool viewInLogFlag, int wordType)
{
    setDeviceName(devNum,devName);
    emit loadMaskTX(devNum, devName, byteNum, byteName, id, paramName, paramMask, paramType, valueShift, valueKoef, viewInLogFlag, wordType);
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

void Device::maskData2FormRX(int _devNum, int _byteNum, int _id, QString _paramName, QString _paramMask, int _paramType, double _valueShift, double _valueKoef, bool _viewInLogFlag, int wordType)
{//ответный сигнал со всеми данными маски bitmaskobj в masksettingsdialog
    emit maskData2FormTX(_devNum, _byteNum, _id, _paramName, _paramMask, _paramType, _valueShift, _valueKoef, _viewInLogFlag, wordType);
}

void Device::sendDataToProfileRX(int _devNum, QString, int _byteNum, QString, int _id, QString _paramName, QString _paramMask, int _paramType, double _valueShift, double _valueKoef, bool _viewInLogFlag, int _wordType)
{//забор данных из формы masksettingsdialog и отправка в профиль bitmaskobj
    emit sendDataToProfileTX(_devNum, _byteNum, _id, _paramName, _paramMask, _paramType, _valueShift, _valueKoef, _viewInLogFlag);
}

void Device::allMasksToListRX(int devNum, int byteNum, QString byteName, int id, QString paramName, QString paramMask, int paramType, double valueShift, double valueKoef, bool viewInLogFlag, int wordType)
{//сигнал от bitmaskobj предназначенный для bytesettingsform, для наполнения листа масок всеми имеющимися у этого байта
    emit allMasksToListTX(devNum, devName, byteNum, byteName, id, paramName, paramMask, paramType, valueShift, valueKoef, viewInLogFlag, wordType);
}

void Device::param2FrontEndRX(int devNum, int byteNum, QString byteName, uint32_t wordData, int id, QString parameterName, int binRawValue, double endValue, bool viewInLogFlag)
{
    emit param2FrontEndTX(devNum, devName, byteNum, byteName, wordData, id, parameterName, binRawValue, endValue, viewInLogFlag);
}

void Device::hideDevButton(bool trueOrFalse, int _devNum)
{
    if (devNum != _devNum && trueOrFalse)
        this->hide();
    else this->show();
}

void Device::devOnlineWatchdog(int msec)
{
        connect (timer, &QTimer::timeout, this, &Device::setOfflineStatus);
//        timer->setInterval(msec);
        timer->start(msec);
        qDebug() << "timer start " << msec;
}

void Device::setOfflineStatus()
{
    qDebug() << "timer timeout!";
    devStatus = "offline";
    timer->stop();
    changeButtonColor(devStatus);
}

void Device::changeButtonColor(QString _status)
{
    qDebug() << "set dev status " << _status;
    if (_status == "offline")
    {
        this->setStyleSheet("QPushButton{background:#add8e6;}");
    }
    if (_status == "online")
    {
        this->setStyleSheet("QPushButton{background:#90ee90;}");
    }
}
