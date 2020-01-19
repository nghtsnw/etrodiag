#include "getstream.h"
#include <QString>
#include <QDebug>
#include <QChar>
#include <QByteArray>
#include <QQueue>


getStream::getStream(QWidget *parent) : QObject(parent)
{

}

void getStream::getRawData(QByteArray r_data) //пишем все данные в буфер
{
    QByteArray toQue;
    int intToQue;
    QString tmpstr;
    n=0;
    bool ok;
    while (n < (r_data.size()))
    {
        toQue.insert(0, r_data.at(n));
        intToQue = toQue.toHex().toInt(&ok, 16);
        tmpstr = QString("%1").arg(intToQue,0,16).toUpper(); //проверка преобразования int в hex
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



