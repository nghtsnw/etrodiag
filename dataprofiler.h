
#ifndef DATAPROFILER_H
#define DATAPROFILER_H
#include <QQueue>
#include <QObject>
#include <QMainWindow>
#include <QTimer>

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
    void endOfPacket(void);
    uint8_t calculatedCRC;
    bool readFromFile = false;
    int blockIdentifycatorPosition = 2;
    int calcCRCFromPosition = 2;
    int markerPacketBeginSize = 2;
    uint8_t markerPacketBeginByte1 = 0xFF;
    uint8_t markerPacketBeginByte2 = 0xFF;
    int timeoutAfterLastByte = 3;
    QTimer timeout;

signals:
    void deviceData(QVector<int> snapshot);
    void badCRC(uint8_t calculatedCRC, QVector<int> snapshot);
    void corruptedData(QVector<int> snapshot);
    void readNext();
    void ready4read(bool);
    void s_readFromFile(bool val);

    void s_returnPacketSize();
    void s_returnBlockIdentifycatorPosition();
    void s_returnCalcCRCFromPosition();
    void s_returnMarkerPacketBeginSize();
    void s_returnMarkerPacketBeginText();
    void s_returnTimeoutAfterLastByte();
    void returnPacketSize(int size);
    void returnBlockIdentifycatorPosition(int pos);
    void returnCalcCRCFromPosition(int pos);
    void returnMarkerPacketBeginSize(int size);
    void returnMarkerPacketBeginText(QString text);
    void returnTimeoutAfterLastByte(int timeout_ms);


public slots:
    void getByte(int byteFromBuf);
    void setPacketSize(int size);
    void setBlockIdentifycatorPosition(int pos);
    void setCalcCRCFromPosition(int pos);
    void setMarkerPacketBeginSize(int size);
    void setMarkerPacketBeginText(QString text);
    void setTimeoutAfterLastByte(int timeout_ms);
};

#endif // DATAPROFILER_H
