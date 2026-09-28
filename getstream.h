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

signals:
    void giveMyByte(int byte);

public slots:
    void getRawData(QByteArray r_data);
    void readIntByte();
    void readPermission(bool p);

private:
    QQueue<int> buffer;
    bool profilerReadyToReceive = true;
};

#endif // GETSTREAM_H
