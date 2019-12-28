#ifndef GETSTREAM_H
#define GETSTREAM_H

#include <QMainWindow>
#include <QObject>
#include <QQueue>

class getStream : public QObject
{
    Q_OBJECT
public:
    explicit getStream(QWidget *parent = nullptr);
    QQueue<int> generalBuffer;

    void ffffCheck();
    int n;
    bool ok;
    bool compareLeight;
    QByteArray toQue;
    int intToQue;
    bool getByteFlag;

signals:
    void giveMyByte(int byte);

public slots:
    void getRawData(QByteArray r_data);
    void profilerReadyToReceive(bool getByteFlag);

private:

};

#endif // GETSTREAM_H
