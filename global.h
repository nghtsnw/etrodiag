#ifndef GLOBAL_H
#define GLOBAL_H
#include <stdint.h>
#include <QString>

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

#endif // GLOBAL_H
