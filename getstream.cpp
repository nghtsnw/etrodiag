#include "getstream.h"
#include <QString>
#include <QDebug>
#include <QChar>
#include <QByteArray>
#include <QQueue>


getStream::getStream(QWidget *parent) : QObject(parent)
{

}

void getStream::getRawData(QByteArray r_data) //побайтово читаем из буфера и отсылаем на обработку
{
    QByteArray toQue;
    int intToQue;
    n=0;
    bool ok;
    while (n < (r_data.size()))
    {
        toQue.insert(0, r_data.at(n));
        intToQue = toQue.toHex().toInt(&ok, 16);
        generalBuffer.enqueue(intToQue);
        toQue.clear();
        emit giveMyByte(generalBuffer.dequeue());
        n++;
    }
}

void getStream::profilerReadyToReceive(bool getByteFlag) //читаем из буфера побайтово
{
   if (getByteFlag){
    //emit giveMeByte(generalBuffer.dequeue());
}
}



