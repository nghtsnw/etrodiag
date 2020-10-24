#include "dataprofiler.h"
#include <QDebug>
#include "mainwindow.h"

dataprofiler::dataprofiler(QWidget *parent) : QObject(parent)
{

}

void dataprofiler::getByte(int byteFromBuf)
{    
    frameMsg.enqueue(byteFromBuf);
    if ((frameMsg.size() >= 2) && (frameMsg[0] == 0xFF) && (frameMsg[1]==0xFF))//если начало буффера соответствует началу пакета то продолжаем читать
    {
        if (frameMsg.size() == oneMsgLeight)
        {
            emit deviceData(frameMsg.toVector());//если пакет сформирован, отправляем пакет в гуй и обнуляем буффер
            frameMsg.clear();
        }
    }
    else//а если начало пакета не сошлось то сдвигаем очередь
            if (frameMsg.size() >= 2) frameMsg.dequeue();
}
