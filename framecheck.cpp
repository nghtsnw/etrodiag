#include "framecheck.h"

namespace framecheck {

uint8_t checksum(const QVector<int> &frame, int fromPosition)
{
    uint8_t sum = 0;
    for (int i = fromPosition; i < frame.size() - 1; i++) {
        sum += frame.at(i);
    }
    return sum;
}

bool isChecksumValid(uint8_t calculated, const QVector<int> &frame)
{
    if (frame.isEmpty()) { //нечего сверять
        return false;
    }
    return (calculated == frame.at(frame.size() - 1));
}

bool isChecksumValid(const QVector<int> &frame, int fromPosition)
{
    return isChecksumValid(checksum(frame, fromPosition), frame);
}

bool isMarkerValid(const QVector<int> &frame, const s_protocolDescription &protocol)
{
    switch (protocol.markerPacketBeginSize) {
    case 0:
        //Маркера нет - целостность пакета определяется только контрольной суммой в конце
        return true;
    case 1:
        return !frame.isEmpty() && frame.at(0) == protocol.markerPacketBeginByte1;
    case 2:
        return frame.size() >= 2 && frame.at(0) == protocol.markerPacketBeginByte1
               && frame.at(1) == protocol.markerPacketBeginByte2;
    default:
        return false;
    }
}

}
