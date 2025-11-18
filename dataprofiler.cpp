#include "dataprofiler.h"
#include <QDebug>

dataprofiler::dataprofiler(QWidget *parent) : QObject(parent)
{

}

void dataprofiler::getByte(int byteFromBuf)
{    
    emit ready4read(false);
    frameMsg.enqueue(byteFromBuf);

    if ((frameMsg.size() >= markerPacketBeginSize) && (frameMsg[0] == markerPacketBeginByte1) && (frameMsg[1]==markerPacketBeginByte2))//если начало буффера соответствует началу пакета то продолжаем читать
    {
        if (frameMsg.size() == oneMsgLeight)
        {
            if (checkCRC()) emit deviceData(frameMsg.toVector());//если пакет сформирован, отправляем пакет в гуй и обнуляем буффер
            else emit badCRC(calculatedCRC, frameMsg.toVector());
            frameMsg.clear();
        }
    }
    else//а если начало пакета не сошлось то сдвигаем очередь
            if (frameMsg.size() >= markerPacketBeginSize) frameMsg.dequeue();
    emit ready4read(true);
    emit readNext();
}

bool dataprofiler::checkCRC(void)
{
    calculatedCRC = 0;
    for (int i = calcCRCFromPosition; i < frameMsg.size()-1; i++)
        calculatedCRC += frameMsg.at(i);
    return (calculatedCRC == frameMsg.at(frameMsg.size()-1)) ? true : false;
}

void dataprofiler::setPackerSize(int size)
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
    markerPacketBeginByte1 = (val>>8)&0xFF;
    markerPacketBeginByte1 = (val)&0xFF;
}
void dataprofiler::setTimeoutAfterLastByte(int timeout_ms)
{
    timeoutAfterLastByte = timeout_ms;
}
