#include "device.h"
#include "mainwindow.h"
#include <QDebug>
#include "bytedefinition.h"
#include <QBitArray>
#include <QDataStream>
#include <QTextStream>
#include <QTimer>
#include <QDateTime>


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
        emit byteObjUpdSig(devNum, devdata);
        if (devStatus == tr("offline"))
        {
            devStatus = tr("online");
            emit devStatusMessage(devName, devStatus);
        }
        if (devStatus == "init")
        {
            setDeviceName(id, QString("%1").arg(devdata.at(2),0,16).toUpper());
            devStatus = tr("offline");
        }
        changeButtonColor(devStatus);
        if (devStatus == tr("online"))
        devOnlineWatchdog(5000);
    }
}

void Device::byteObjectsInit(QVector<int> &data) //инициализируем для каждого байта свой объект,
//с параметрами конкретно этого байта и значениями каждого бита, и загоняем объекты в массив
{
    connect (timer, &QTimer::timeout, this, &Device::setOfflineStatus);
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
        connect (this, &Device::byteObjUpdSig, bytedef, &byteDefinition::updateSlot);
        connect (this, &Device::requestMaskCounting, bytedef, &byteDefinition::countMasks);
        connect (bytedef, &byteDefinition::returnMaskCountForThisByte, this, &Device::returnMaskCounting);
        n--;
    }
    byteObjReady = true;
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

void Device::setDeviceName(int id, QString name)
{
    if (id == devNum)
    {
        devName = name;
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

void Device::returnMaskCounting(int _devNum, int _byteNum, int _count)
{//возврат от каждого байта количества масок
    if (devNum == _devNum)
    {
        maskCountMap.insert(_byteNum, _count);
    }
}

int Device::calcMasksInDev()
{//считаем общее количество масок
    int result = 0;
    QList<int> values = maskCountMap.values();
    QListIterator<int> valuesIt(values);
    while (valuesIt.hasNext())
        result += valuesIt.next();
    return result;
}

int Device::countMasks()
{
    emit requestMaskCounting();
    return calcMasksInDev();
}

void Device::loadMaskRX(bitMaskDataStruct &bitMask)
{
    setDeviceName(bitMask.devNum, bitMask.devName);
    emit loadMaskTX(bitMask);
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

void Device::maskData2FormRX(bitMaskDataStruct &bitMask)
{//ответный сигнал со всеми данными маски bitmaskobj в masksettingsdialog
    emit maskData2FormTX(bitMask);
}

void Device::sendDataToProfileRX(bitMaskDataStruct &bitMask)
{//забор данных из формы masksettingsdialog и отправка в профиль bitmaskobj
    emit sendDataToProfileTX(bitMask);
}

void Device::allMasksToListRX(bitMaskDataStruct &bitMask)
{//сигнал от bitmaskobj предназначенный для bytesettingsform, для наполнения листа масок всеми имеющимися у этого байта
    emit allMasksToListTX(bitMask);
}

void Device::param2FrontEndRX(bitMaskDataStruct &bitMask)
{
    emit param2FrontEndTX(bitMask);
}

void Device::jsonMap(bitMaskDataStruct &bitMask)
{
    if (bitMask.devNum == devNum)
    {
        devParams->insert("DeviceName", bitMask.devName);
        devParams->insert("NumberBlock", QString::number(bitMask.devNum));
        devParams->insert(bitMask.paramName, QString::number(bitMask.endValue));
        devParamsCount++;
        if (devParamsCount == countMasks())
        {
            devParams->insert("DateTime", returnTimestamp().toString("yy-MM-ddThh:mm:ss.zzz"));
            emit devParamsToJson(*devParams);
            devParams->clear();//очищаем, так как может измениться набор параметров (например если поменяем имя параметра, чтоб не осталось старого поля в мапе)
            devParamsCount = 0;
        }
    }
}

QDateTime Device::returnTimestamp()
{
    quint64 timestamp = QDateTime::currentMSecsSinceEpoch();
    QDateTime dt3 = QDateTime::fromMSecsSinceEpoch(timestamp);
    return dt3;
}

void Device::hideDevButton(bool trueOrFalse, int _devNum)
{
    if (devNum != _devNum && trueOrFalse)
        this->hide();
    else this->show();
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
