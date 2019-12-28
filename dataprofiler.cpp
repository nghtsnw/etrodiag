#include "dataprofiler.h"
#include <QDebug>
#include "mainwindow.h"

dataprofiler::dataprofiler(QWidget *parent) : QObject(parent)
{
    //connect(this, SIGNAL(deviceData()), &w, SLOT(addDeviceToList(QVector<QString> ddata)));
}

void dataprofiler::getByte(int byteFromBuf)
{
    rdyGB = false; //readyGetByte
    readyGetByteF();
    frameMsg.enqueue(byteFromBuf);
   // qDebug() << "frameMsg.size() = " << frameMsg.size();
    if (frameMsg.size() == oneMsgLeight)
    {
        frameSnap();
        ffffchk();
        if (ffffbool) deviceManager(snapshot);
        snapshot.clear();
        frameMsg.dequeue();
    }

    rdyGB = true;
    readyGetByteF();
}

void dataprofiler::frameSnap()
{
    int n = 0;
    //snapshot.reserve(oneMsgLeight);
    //snapshot.fill(0);
    while (n < oneMsgLeight)
    {
        //qDebug() << "n = " << n;
        snapshot.append(frameMsg.at(n));
        //qDebug() << "snapshot.size() = " << snapshot.size() << ", snapshot = " << snapshot;
           n++;
    }
    //qDebug() << "Frame: " << snapshot;
}

void dataprofiler::ffffchk()
{
    if ((snapshot[0]==0xFF) && (snapshot[1]==0xFF))
        ffffbool = true;
    else ffffbool = false;
}

void dataprofiler::deviceManager(QVector<int> snapshot)
{
    //qDebug() << "Snapshot of msg " << snapshot; //пока что так
    emit deviceData(snapshot); //чтобы передать этот сигнал главному окну в gui, приходится передавать через посредника newconnect
}

void dataprofiler::readyGetByteF()
{
    emit readyGetByte(rdyGB);
}

