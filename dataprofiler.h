
#ifndef DATAPROFILER_H
#define DATAPROFILER_H
#include <QQueue>
#include <QObject>
#include <QMainWindow>

class dataprofiler : public QObject
{
    Q_OBJECT
public:
    explicit dataprofiler(QWidget *parent = nullptr);
    int oneMsgLeight = 41;

private:

    QQueue<int> frameMsg;
    QVector<int> snapshot;
    bool checkCRC(void);
    uint8_t calculatedCRC;

    //int blockIdentifycatorPosition = 2;
    int calcCRCFromPosition = 2;
    int markerPacketBeginSize = 2;
    uint8_t markerPacketBeginByte1 = 0xFF;
    uint8_t markerPacketBeginByte2 = 0xFF;
    int timeoutAfterLastByte = 3;

signals:
    void deviceData(QVector<int> snapshot);
    void badCRC(uint8_t calculatedCRC, QVector<int> snapshot);
    void readNext();
    void ready4read(bool);

public slots:
    void getByte(int byteFromBuf);
    void setPackerSize(int size);
    //void setBlockIdentifycatorPosition(int pos);
    void setCalcCRCFromPosition(int pos);
    void setMarkerPacketBeginSize(int size);
    void setMarkerPacketBeginText(QString text);
    void setTimeoutAfterLastByte(int timeout_ms);
};

#endif // DATAPROFILER_H
