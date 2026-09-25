#ifndef GLOBAL_H
#define GLOBAL_H
#include <stdint.h>
#include <QString>
#include <QSerialPort>

typedef struct s_protocolDescription
{
    int blockIdentifycatorPosition = 38;
    int packetSize = 40;
    int calcCRCFromPosition = 0;
    int markerPacketBeginSize = 1;
    uint8_t markerPacketBeginByte1 = 0xFF;
    uint8_t markerPacketBeginByte2 = 0x00;
    //int timeoutAfterLastByte;
    QString description;
    bool varControl = false;
} s_protocolDescription;

typedef struct s_Settings
{
    QString name;
    qint32 baudRate = QSerialPort::Baud115200;
    QString stringBaudRate = QStringLiteral("115200");
    QSerialPort::DataBits dataBits = QSerialPort::Data8;
    QString stringDataBits = QStringLiteral("8");
    QSerialPort::Parity parity = QSerialPort::NoParity;
    QString stringParity = QStringLiteral("None");
    QSerialPort::StopBits stopBits = QSerialPort::OneStop;
    QString stringStopBits = QStringLiteral("1");
    QSerialPort::FlowControl flowControl = QSerialPort::NoFlowControl;
    QString stringFlowControl = QStringLiteral("None");
    QString profilePath;
    bool readOnlyProfile = true;
    bool readFromFileFlag = false;
    QString pathToBinFile;
} s_Settings;

typedef struct s_parameterMask
{
    int devNum;
    QString devName;
    int byteNum;
    QString byteName;
    uint32_t wordData;
    int wordType;
    int id;
    QString parameterName;
    QString parameterMask;
    int parameterShift;
    int parameterLeight;
    double valueShift;
    double valueKoef;
    int binRawValue;
    double endValue;
    bool viewInLogFlag;
    bool isNewData;
    bool drawGraphFlag;
    QString drawGraphColor;
} s_parameterMask;

#endif // GLOBAL_H
