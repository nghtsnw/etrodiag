#include "dataprofiler.h"
#include <QDebug>

dataprofiler::dataprofiler(QWidget *parent) : QObject(parent)
{
    connect(&timeout, &QTimer::timeout, this, &dataprofiler::endOfPacket);
    connect(this, &dataprofiler::s_readFromFile, this, [ = ](bool val) {
        readFromFile = val;
    });
    /*connect(this, &dataprofiler::s_returnPacketSize, this, [ = ]() {
        emit returnPacketSize(packetSize)
    });*/
    connect(this, &dataprofiler::s_returnBlockIdentifycatorPosition, this, [ = ]() {
        emit returnBlockIdentifycatorPosition(blockIdentifycatorPosition);
    });
    connect(this, &dataprofiler::s_returnCalcCRCFromPosition, this, [ = ]() {
        emit returnCalcCRCFromPosition(calcCRCFromPosition);
    });
    connect(this, &dataprofiler::s_returnMarkerPacketBeginSize, this, [ = ]() {
        emit returnMarkerPacketBeginSize(markerPacketBeginSize);
    });
    connect(this, &dataprofiler::s_returnMarkerPacketBeginText, this, [ = ]() {
        QString markerPacketBeginText = QString::number(markerPacketBeginByte1, 16).toUpper()
                                        + QString::number(markerPacketBeginByte2, 16).toUpper();
        emit returnMarkerPacketBeginText(markerPacketBeginText);
    });
    connect(this, &dataprofiler::s_returnTimeoutAfterLastByte, this, [ = ]() {
        emit returnTimeoutAfterLastByte(timeoutAfterLastByte);
    });
}

void dataprofiler::getByte(int byteFromBuf)
{
    bool marker = false;
    emit ready4read(false);
    frameMsg.enqueue(byteFromBuf);
    if ((frameMsg.size() >= markerPacketBeginSize)) //если начало буффера соответствует началу пакета то продолжаем читать
    {
        switch (markerPacketBeginSize) {
            {
            case 0:
            {
                marker = true;
                break;
            }
            case 1:
            {
                if (frameMsg[0] == markerPacketBeginByte1) {
                    marker = true;
                }
                break;
            }
            case 2:
            {
                if ((frameMsg[0] == markerPacketBeginByte1) && (frameMsg[1] == markerPacketBeginByte2)) {
                    marker = true;
                }
                break;
            }
            default:
                break;
            }
        }
        if (frameMsg.size() == oneMsgLeight && marker)
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
        timeout.start(timeoutAfterLastByte);
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
    for (int i = calcCRCFromPosition; i < frameMsg.size() - 1; i++) {
        calculatedCRC += frameMsg.at(i);
    }
    return (calculatedCRC == frameMsg.at(frameMsg.size() - 1)) ? true : false;
}

void dataprofiler::setPacketSize(int size)
{
    oneMsgLeight = size;
}
void dataprofiler::setBlockIdentifycatorPosition(int pos)
{
    blockIdentifycatorPosition = pos;
}
void dataprofiler::setCalcCRCFromPosition(int pos)
{
    calcCRCFromPosition = pos;
}
void dataprofiler::setMarkerPacketBeginSize(int size)
{
    markerPacketBeginSize = size;
}
void dataprofiler::setMarkerPacketBeginText(QString text)
{
    int val = text.toInt(0, 16);
    markerPacketBeginByte1 = (val >> 8) & 0xFF;
    markerPacketBeginByte1 = (val) & 0xFF;
}
void dataprofiler::setTimeoutAfterLastByte(int timeout_ms)
{
    timeoutAfterLastByte = timeout_ms;
}
