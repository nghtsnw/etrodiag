#ifndef DATAPROFILER_H
#define DATAPROFILER_H
#include <QQueue>
#include <QObject>
#include <QMainWindow>
#include <QChronoTimer>
#include "global.h"
#include "qdatetime.h"

class dataprofiler : public QObject
{
    Q_OBJECT
public:
    explicit dataprofiler(QWidget *parent = nullptr);

private:
    QQueue<int> frameMsg;
    QVector<int> snapshot;
    bool checkCRC(void);
    //void endOfPacket(void);
    uint8_t calculatedCRC;
    //QChronoTimer timeout;
    s_protocolDescription protocol;
    s_Settings settings;
    //QDateTime nextTime;
    QDateTime currentTime;
    //QDateTime previousTime;

signals:
    void deviceData(QDateTime currentTime, QVector<int> snapshot);
    void badCRC(uint8_t calculatedCRC, QVector<int> snapshot);
    //void corruptedData(QVector<int> snapshot);
    void readNext();
    void ready4read(bool);
    void setProtocol(s_protocolDescription);
    void setSettings(s_Settings);
    void setTime(QDateTime t);

public slots:
    void getByte(int byteFromBuf);

    //void readFromFile(QMap<QDateTime, QVector<uint8_t >> );
};

#endif // DATAPROFILER_H
