#include "dataprofiler.h"
#include "qdatetime.h"
#include <QDebug>

dataprofiler::dataprofiler(QWidget *parent) : QObject(parent)
{
    timeout.setSingleShot(true);
    connect(&timeout, &QChronoTimer::timeout, this, &dataprofiler::endOfPacket);
    connect(this, &dataprofiler::setProtocol, this, [ = ](s_protocolDescription p) { //Установка протокола
        protocol = p;
    });
    connect(this, &dataprofiler::setSettings, this, [ = ](s_Settings s) { //Установка настроек
        settings = s;
        float pause_byte_nsec = (((float)1 / (settings.baudRate / 8)) * protocol.timeoutAfterLastByte) * 1000000;
        timeout.setInterval(std::chrono::nanoseconds(static_cast<int>(pause_byte_nsec)));
    });
}

void dataprofiler::getByte(int byteFromBuf)
{
    bool marker = false;
    emit ready4read(false);
    frameMsg.enqueue(byteFromBuf);
    if ((frameMsg.size() >= protocol.markerPacketBeginSize)) //если принятые данные уже можно проверять на наличие маркера - начинаем обработку
    {
        switch (protocol.markerPacketBeginSize) { //Выявляем что маркер совпал (если он должен быть)
            {
            case 0:
            { //Если маркера нет в протоколе, то определить целостность пакета можно будет только по контрольной сумме в конце
                marker = true;
                break;
            }
            case 1:
            {
                if (frameMsg[0] == protocol.markerPacketBeginByte1) {
                    marker = true;
                }
                break;
            }
            case 2:
            {
                if ((frameMsg[0] == protocol.markerPacketBeginByte1) && (frameMsg[1] == protocol.markerPacketBeginByte2)) {
                    marker = true;
                }
                break;
            }
            default:
                break;
            }
        }
        if ((frameMsg.size() == protocol.packetSize) && marker) //Когда набрался весь пакет
        {
            if (checkCRC()) {
                if (!settings.readFromFileFlag) {
                    currentTime = QDateTime::currentDateTime();
                }
                emit deviceData(currentTime, frameMsg.toVector()); //Если пакет сформирован, отправляем пакет в гуй и обнуляем буффер
            }
            else {
                emit badCRC(calculatedCRC, frameMsg.toVector()); //Если контрольная сумма не сошлась
            }
            frameMsg.clear();
        }
    }
    if (!settings.readFromFileFlag) {
        timeout.start(); //После приёма каждого байта таймер перезапускается до срабатывания его прерывания детектирования прекращения приёма
    }
    else {
        quint64 differenceTime = currentTime.msecsTo(nextTime);
        //В QDateTime нет возможности оперировать наносекундами, да и вроде нет надобности. Константной паузы 1 мсек должно хватить.
        if (differenceTime > 1) {
            endOfPacket();
        }
    }
    emit ready4read(true);
    emit readNext();
}

void dataprofiler::endOfPacket(void)
{
    if (!frameMsg.isEmpty()) {
        emit corruptedData(frameMsg.toVector());
    }
    frameMsg.clear();
}

void dataprofiler::readFromFile(QMap<QDateTime, QVector<uint8_t >> &dataWithTime)
{
    QList<QDateTime> timeKeys = dataWithTime.keys();
    QListIterator<QDateTime> i(timeKeys);
    while (i.hasNext()) {
        QDateTime time = i.next();
        QVector<uint8_t> data = dataWithTime.value(time);
        previousTime = currentTime;
        currentTime = time;
        nextTime = i.peekNext();
        for (const uint8_t byte : data) {
            getByte(byte);
        }
    }
}

bool dataprofiler::checkCRC(void)
{
    calculatedCRC = 0;
    for (int i = protocol.calcCRCFromPosition; i < frameMsg.size() - 1; i++) {
        calculatedCRC += frameMsg.at(i);
    }
    return (calculatedCRC == frameMsg.at(frameMsg.size() - 1)) ? true : false;
}
