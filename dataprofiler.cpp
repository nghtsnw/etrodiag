#include "dataprofiler.h"
#include <QDebug>

dataprofiler::dataprofiler(QWidget *parent) : QObject(parent)
{

}

void dataprofiler::getByte(int byteFromBuf)
{    
    emit ready4read(false);
    frameMsg.enqueue(byteFromBuf);
    if ((frameMsg.size() >= 2) && (frameMsg[0] == 0xFF) && (frameMsg[1]==0xFF))//если начало буффера соответствует началу пакета то продолжаем читать
    {
        if (frameMsg.size() == oneMsgLeight)
        {
            if (checkCRC()) emit deviceData(frameMsg.toVector());//если пакет сформирован, отправляем пакет в гуй и обнуляем буффер
            else emit badCRC(calculatedCRC, frameMsg.toVector());
            frameMsg.clear();
        }
    }
    else//а если начало пакета не сошлось то сдвигаем очередь
            if (frameMsg.size() >= 2) frameMsg.dequeue();
    emit ready4read(true);
    emit readNext();
}

bool dataprofiler::checkCRC(void)
{
    calculatedCRC = 0;
    for (int i = 2; i < frameMsg.size()-1; i++) //Предположительно ff:ff не считаем
        calculatedCRC += frameMsg.at(i);
    return (calculatedCRC == frameMsg.at(frameMsg.size()-1)) ? true : false;
}
