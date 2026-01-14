
#ifndef DATAPROFILER_H
#define DATAPROFILER_H
#include <QQueue>
#include <QObject>
#include <QMainWindow>
#include <QChronoTimer>
#include "global.h"

class dataprofiler : public QObject
{
    Q_OBJECT
public:
    explicit dataprofiler(QWidget *parent = nullptr);

private:
    QQueue<int> frameMsg;
    QVector<int> snapshot;
    bool checkCRC(void);
    void endOfPacket(void);
    uint8_t calculatedCRC;
    bool readFromFile = false;
    QChronoTimer timeout;
    s_protocolDescription protocol;
    s_Settings settings;

signals:
    void deviceData(QVector<int> snapshot);
    void badCRC(uint8_t calculatedCRC, QVector<int> snapshot);
    void corruptedData(QVector<int> snapshot);
    void readNext();
    void ready4read(bool);
    void s_readFromFile(bool val);
    void setProtocol(s_protocolDescription);

public slots:
    void getByte(int byteFromBuf);
    void setPacketSize(int size);
    void setBlockIdentifycatorPosition(int pos);
    void setCalcCRCFromPosition(int pos);
    void setMarkerPacketBeginSize(int size);
    void setMarkerPacketBeginText(QString text);
    void setTimeoutAfterLastByte(int timeout_ms);
    void setSettings(s_Settings);
};

#endif // DATAPROFILER_H
