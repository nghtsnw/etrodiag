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
    for (int i = 0; i < r_data.size(); ++i)
    {
        const int byteValue = static_cast<unsigned char>(r_data.at(i));
        if (profilerReadyToReceive && buffer.isEmpty()) {
            emit giveMyByte(byteValue);
        }
        else {
            buffer.enqueue(byteValue);
        }
    }
}
void getStream::readPermission(bool p)
{
    profilerReadyToReceive = p;
}

void getStream::readIntByte()
{
    if (!buffer.isEmpty()) {
        emit giveMyByte(buffer.dequeue());
    }
}

