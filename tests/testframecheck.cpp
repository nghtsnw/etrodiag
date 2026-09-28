#include "testframecheck.h"
#include <QtTest>
#include "framecheck.h"

void TestFrameCheck::checksumSkipsLastByte()
{
    const QVector<int> frame{0x10, 0x20, 0x30, 0x99};
    QCOMPARE(int(framecheck::checksum(frame, 0)), 0x60);
}

void TestFrameCheck::checksumStartsAtConfiguredPosition()
{
    const QVector<int> frame{0xFF, 0xAB, 0x01, 0x02, 0x03};
    QCOMPARE(int(framecheck::checksum(frame, 2)), 0x01 + 0x02);
}

void TestFrameCheck::checksumWrapsAroundOneByte()
{
    const QVector<int> frame{200, 100, 0};
    QCOMPARE(int(framecheck::checksum(frame, 0)), (200 + 100) % 256);
}

void TestFrameCheck::singleByteFrameHasZeroChecksum()
{
    const QVector<int> frame{0x77};
    QCOMPARE(int(framecheck::checksum(frame, 0)), 0);
}

void TestFrameCheck::checksumMatchesLastByte()
{
    QVector<int> frame{0x01, 0x02, 0x03, 0};
    frame.last() = framecheck::checksum(frame, 0);
    QVERIFY(framecheck::isChecksumValid(frame, 0));

    frame.last() = 0xFF;
    QVERIFY(!framecheck::isChecksumValid(frame, 0));
}

void TestFrameCheck::emptyFrameIsNeverValid()
{
    const QVector<int> frame;
    QVERIFY(!framecheck::isChecksumValid(frame, 0));
    QVERIFY(!framecheck::isChecksumValid(uint8_t(0), frame));
}

void TestFrameCheck::markerIsOptional()
{
    s_protocolDescription protocol;
    protocol.markerPacketBeginSize = 0;
    QVERIFY(framecheck::isMarkerValid(QVector<int>{0x00, 0x01}, protocol));
    QVERIFY(framecheck::isMarkerValid(QVector<int>{}, protocol));
}

void TestFrameCheck::markerOfOneByte()
{
    s_protocolDescription protocol;
    protocol.markerPacketBeginSize = 1;
    protocol.markerPacketBeginByte1 = 0xFF;
    QVERIFY(framecheck::isMarkerValid(QVector<int>{0xFF, 0x01}, protocol));
    QVERIFY(!framecheck::isMarkerValid(QVector<int>{0xFE, 0x01}, protocol));
    QVERIFY(!framecheck::isMarkerValid(QVector<int>{}, protocol));
}

void TestFrameCheck::markerOfTwoBytes()
{
    s_protocolDescription protocol;
    protocol.markerPacketBeginSize = 2;
    protocol.markerPacketBeginByte1 = 0xFF;
    protocol.markerPacketBeginByte2 = 0xAB;
    QVERIFY(framecheck::isMarkerValid(QVector<int>{0xFF, 0xAB, 0x00}, protocol));
    QVERIFY(!framecheck::isMarkerValid(QVector<int>{0xFF, 0xAA, 0x00}, protocol));
    QVERIFY(!framecheck::isMarkerValid(QVector<int>{0xFF}, protocol));
}

void TestFrameCheck::unknownMarkerSizeIsInvalid()
{
    s_protocolDescription protocol;
    protocol.markerPacketBeginSize = 3;
    QVERIFY(!framecheck::isMarkerValid(QVector<int>{0xFF, 0xAB, 0x00, 0x01}, protocol));
}

void TestFrameCheck::defaultProtocolFrameIsAccepted()
{
    s_protocolDescription protocol; //по умолчанию: 40 байт, маркер FF первым байтом, id на позиции 38
    QVector<int> frame(protocol.packetSize, 0);
    frame[0] = protocol.markerPacketBeginByte1;
    frame[protocol.blockIdentifycatorPosition] = 0x2A;
    frame.last() = framecheck::checksum(frame, protocol.calcCRCFromPosition);

    QVERIFY(framecheck::isMarkerValid(frame, protocol));
    QVERIFY(framecheck::isChecksumValid(frame, protocol.calcCRCFromPosition));

    frame[0] = 0x00; //испорченный маркер
    QVERIFY(!framecheck::isMarkerValid(frame, protocol));
}
