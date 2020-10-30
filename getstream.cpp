#include "getstream.h"
#include <QString>
#include <QDebug>
#include <QChar>
#include <QByteArray>
#include <QQueue>


getStream::getStream(QWidget *parent) : QObject(parent)
{

}

void getStream::getRawData(QByteArray r_data) //побайтово читаем из буфера, конвертируем в int и отсылаем на обработку
{
    n=0;
    while (n < (r_data.size()))
    {
        toQue.insert(0, r_data.at(n));
        intToQue = toQue.toHex().toInt(&ok, 16);
        if (profilerReadyToReceive && buffer.isEmpty()) giveMyByte(intToQue);
        else buffer.enqueue(intToQue);
        toQue.clear();
        n++;
    }
}
void getStream::readPermission(bool p)
{
    profilerReadyToReceive = p;
}

void getStream::readIntByte()
{
    if (!buffer.isEmpty())
        giveMyByte(buffer.dequeue());
}

