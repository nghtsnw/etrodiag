#include "device.h"
#include <QDebug>
#include "bytedefinition.h"
#include <QBitArray>
#include <QDataStream>
#include <QTextStream>
#include <QTimer>
#include <QDateTime>
#include "global.h"


Device::Device(QWidget *parent) : QPushButton(parent)
{
}

Device::Device(int id) //инициализация нового устройства
{
    devNum = id;
}

void Device::updateData(QDateTime currTime, int id, QVector<int> devdata) //если устройство в списке уже есть, этой функцией оно обновляется
{
    if (id == devNum)
    {
        currentState = devdata;
        currentTime = currTime;
        if (!byteObjReady) {
            byteObjectsInit(currentState);
        }
        emit byteObjUpdSig(devNum, devdata);
        if (devStatus == tr("offline"))
        {
            devStatus = tr("online");
            emit devStatusMessage(devName, devStatus);
        }
        if (devStatus == "init")
        {
            setDeviceName(id, QString("%1").arg(devdata.at(2), 0, 16).toUpper());
            devStatus = tr("offline");
        }
        changeButtonColor(devStatus);
        if (devStatus == tr("online")) {
            devOnlineWatchdog(5000);
        }
    }
}

void Device::byteObjectsInit(QVector<int> &data) //инициализируем для каждого байта свой объект,
//с параметрами конкретно этого байта и значениями каждого бита, и загоняем объекты в массив
{
    connect (timer, &QTimer::timeout, this, &Device::setOfflineStatus);
    int n = data.size() - 1;
    while (n != 0)//набиваем массив ссылками на новые объекты байтов
    {
        byteDefinition *bytedef = new byteDefinition(devNum, n, data.at(n));
        connect (this, &Device::setWordBitTX, bytedef, &byteDefinition::setWordBitRX);
        connect (this, &Device::getWordTypeTX, bytedef, &byteDefinition::getWordType);
        connect (bytedef, &byteDefinition::returnWordType, this, &Device::returnWordTypeTX);
        connect (this, &Device::createNewMaskTX, bytedef, &byteDefinition::createNewMask);
        connect (bytedef, &byteDefinition::mask2FormTX, this, &Device::mask2FormTX);
        connect (bytedef, &byteDefinition::maskData2FormTX, this, &Device::maskData2FormTX);
        connect (this, &Device::requestMaskDataTX, bytedef, &byteDefinition::requestMaskDataRX);
        connect (bytedef, &byteDefinition::allMasksToListTX, this, &Device::allMasksToListTX);
        connect (this, &Device::sendDataToProfileTX, bytedef, &byteDefinition::sendDataToProfileRX);
        connect (this, &Device::deleteMaskObjTX, bytedef, &byteDefinition::deleteMaskObjTX);
        connect (bytedef, &byteDefinition::param2FrontEndTX, this, [ = ](s_parameterMask mask) {
            emit param2FrontEndTX(currentTime, mask);
        });
        connect (this, &Device::loadMaskTX, bytedef, &byteDefinition::loadMaskRX);
        connect (this, &Device::byteObjUpdSig, bytedef, &byteDefinition::updateSlot);
        connect (this, &Device::requestMaskCounting, bytedef, &byteDefinition::countMasks);
        connect (bytedef, &byteDefinition::returnMaskCountForThisByte, this, &Device::returnMaskCounting);
        n--;
    }
    byteObjReady = true;
}

void Device::clickedF()
{
    emit openDevSettSig(devNum, currentState);
}

void Device::getDeviceName(int id)
{ //при открытии формы devsettingsform, она запрашивает имя устройства, тут устройство отвечает на запрос
    if (id == devNum) {
        emit returnDeviceName(devNum, devName);
    }
}

void Device::setDeviceName(int id, QString name)
{
    if (id == devNum)
    {
        devName = name;
        this->setText(devName);
    }
}

void Device::requestMasks4Saving()
{ //каждому байту устройства отправляем сигнал на выдачу всех масок
    for (int i = 0; i <= currentState.size(); i++) {
        emit requestMaskDataTX(devNum, i, 999);
    }
}

void Device::returnMaskCounting(int _devNum, int _byteNum, int _count)
{ //возврат от каждого байта количества масок
    if (devNum == _devNum)
    {
        maskCountMap.insert(_byteNum, _count);
    }
}

int Device::calcMasksInDev()
{ //считаем общее количество масок
    int result = 0;
    QList<int> values = maskCountMap.values();
    QListIterator<int> valuesIt(values);
    while (valuesIt.hasNext()) {
        result += valuesIt.next();
    }
    return result;
}

int Device::countMasks()
{
    emit requestMaskCounting();
    return calcMasksInDev();
}

void Device::loadMaskRX(s_parameterMask mask)
{
    setDeviceName(devNum, devName);
    s_parameterMask _mask = mask;
    _mask.devNum = devNum;
    _mask.devName = devName;
    emit loadMaskTX(_mask);
}

void Device::jsonMap(s_parameterMask mask)
{
    if (mask.devNum == devNum)
    {
        devParams->insert("DeviceName", mask.devName);
        devParams->insert("NumberBlock", QString::number(mask.devNum));
        devParams->insert(mask.parameterName, QString::number(mask.endValue));
        devParamsCount++;
        if (devParamsCount == countMasks())
        {
            devParams->insert("DateTime", currentTime.toString("yy-MM-ddThh:mm:ss.zzz"));
            emit devParamsToJson(*devParams);
            devParams->clear();//очищаем, так как может измениться набор параметров (например если поменяем имя параметра, чтоб не осталось старого поля в мапе)
            devParamsCount = 0;
        }
    }
}

void Device::hideDevButton(bool trueOrFalse, int _devNum)
{
    if (devNum != _devNum && trueOrFalse) {
        this->hide();
    }
    else {
        this->show();
    }
}

void Device::devOnlineWatchdog(int msec)
{
    timer->start(msec);
}

void Device::setOfflineStatus()
{
    devStatus = tr("offline");
    emit devStatusMessage(devName, devStatus);
    timer->stop();
    changeButtonColor(devStatus);
}

void Device::changeButtonColor(QString _status)
{
    if (_status == tr("offline"))
    {
        this->setStyleSheet("QPushButton{background:#808080;}");
    }
    if (_status == tr("online"))
    {
        this->setStyleSheet("QPushButton{background:#00FF00;}");
    }
}
