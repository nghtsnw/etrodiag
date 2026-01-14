#ifndef GLOBAL_H
#define GLOBAL_H
#include <stdint.h>
#include <QString>
#include <QSerialPort>

typedef struct
{
    int blockIdentifycatorPosition ;
    int packetSize;
    int calcCRCFromPosition;
    int markerPacketBeginSize;
    uint8_t markerPacketBeginByte1;
    uint8_t markerPacketBeginByte2;
    int timeoutAfterLastByte;
    QString description;
    bool varControl;
} s_protocolDescription;

typedef struct s_Settings
{
    QString name;
    qint32 baudRate;
    QString stringBaudRate;
    QSerialPort::DataBits dataBits;
    QString stringDataBits;
    QSerialPort::Parity parity;
    QString stringParity;
    QSerialPort::StopBits stopBits;
    QString stringStopBits;
    QSerialPort::FlowControl flowControl;
    QString stringFlowControl;
    QString profilePath;
    bool readOnlyProfile = true;
    bool readFromFileFlag = false;
    QString pathToBinFile;
} s_Settings;

#endif // GLOBAL_H
