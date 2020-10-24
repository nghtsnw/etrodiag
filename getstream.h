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

private:
    int n;
    bool ok;
    QByteArray toQue;
    int intToQue = 0;

};

#endif // GETSTREAM_H
