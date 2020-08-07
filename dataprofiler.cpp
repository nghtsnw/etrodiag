#include "dataprofiler.h"
#include <QDebug>
#include "mainwindow.h"

dataprofiler::dataprofiler(QWidget *parent) : QObject(parent)
{

}

void dataprofiler::getByte(int byteFromBuf)
{
    emit readyGetByte(false);
    frameMsg.enqueue(byteFromBuf);   
    if (frameMsg.size() == oneMsgLeight)
    {
        frameSnap();
        ffffchk();
        if (ffffbool)
        {
            emit deviceData(snapshot);//если маска начала пакета сошлась, отправляем пакет в гуй
        }
        snapshot.clear();
        frameMsg.dequeue();
    }
    emit readyGetByte(true);
}

void dataprofiler::frameSnap()
{
    int n = 0;
    while (n < oneMsgLeight)
    {
        snapshot.append(frameMsg.at(n));
        n++;
    }
}

void dataprofiler::ffffchk()
{
    if (countToNewFFFF < oneMsgLeight) countToNewFFFF++;
    if ((snapshot[0]==0xFF) && (snapshot[1]==0xFF)){
        ffffbool = true;
        countToNewFFFF = -2;
    }
    else ffffbool = false;
    if (countToNewFFFF == oneMsgLeight && !((snapshot[0]==0xFF) && (snapshot[1]==0xFF))){
                emit statusMessage (tr("Incorrect data frame"));
                countToNewFFFF = oneMsgLeight + 1;
    }
}
