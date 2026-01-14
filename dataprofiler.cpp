#include "dataprofiler.h"
#include <QDebug>

dataprofiler::dataprofiler(QWidget *parent) : QObject(parent)
{
    timeout.setSingleShot(true);
    connect(&timeout, &QChronoTimer::timeout, this, &dataprofiler::endOfPacket);
    connect(this, &dataprofiler::s_readFromFile, this, [ = ](bool val) {
        readFromFile = val;
    });
    connect(this, &dataprofiler::setProtocol, this, [ = ](s_protocolDescription p) { //Установка протокола
        protocol = p;
    });
    connect(this, &dataprofiler::setSettings, this, [ = ](s_Settings s) { //Установка протокола
        settings = s;
        float pause = ((1 / (settings.baudRate / 8))*protocol.timeoutAfterLastByte)*1000000;
        timeout.setInterval(std::chrono::nanoseconds(static_cast<int>(pause)));
    });
}

void dataprofiler::getByte(int byteFromBuf)
{
    bool marker = false;
    emit ready4read(false);
    frameMsg.enqueue(byteFromBuf);
    if ((frameMsg.size() >= protocol.markerPacketBeginSize)) //если начало буффера соответствует началу пакета то продолжаем читать
    {
        switch (protocol.markerPacketBeginSize) {
            {
            case 0:
            {
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
        if (frameMsg.size() == protocol.packetSize && marker)
        {
            if (checkCRC()) {
                emit deviceData(frameMsg.toVector()); //если пакет сформирован, отправляем пакет в гуй и обнуляем буффер
                frameMsg.clear();
            }
            else {
                emit badCRC(calculatedCRC, frameMsg.toVector());
                if (readFromFile) {
                    frameMsg.dequeue();
                }
            }
        }
    }
    if (!readFromFile) {
        timeout.start();
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

bool dataprofiler::checkCRC(void)
{
    calculatedCRC = 0;
    for (int i = protocol.calcCRCFromPosition; i < frameMsg.size() - 1; i++) {
        calculatedCRC += frameMsg.at(i);
    }
    return (calculatedCRC == frameMsg.at(frameMsg.size() - 1)) ? true : false;
}
