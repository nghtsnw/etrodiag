#include "testdataprofiler.h"
#include <QtTest>
#include "dataprofiler.h"
#include "framecheck.h"
#include "protocolsettings.h"

namespace {

//Кадр по умолчанию: 40 байт, маркер FF первым байтом, id устройства на позиции 38
QVector<int> makeFrame(const s_protocolDescription &protocol, int deviceId)
{
    QVector<int> frame(protocol.packetSize, 0);
    frame[0] = protocol.markerPacketBeginByte1;
    frame[protocol.blockIdentifycatorPosition] = deviceId;
    frame.last() = framecheck::checksum(frame, protocol.calcCRCFromPosition);
    return frame;
}

void feed(dataprofiler &profiler, const QVector<int> &bytes)
{
    for (int byte : bytes) {
        profiler.getByte(byte);
    }
}

}

void TestDataProfiler::acceptsValidFrame()
{
    ProtocolSettings model;
    dataprofiler profiler;
    profiler.setModel(&model);
    QSignalSpy spy(&profiler, &dataprofiler::deviceData);

    const QVector<int> frame = makeFrame(model.protocol(), 0x2A);
    feed(profiler, frame);

    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(1).value<QVector<int>>(), frame);
}

void TestDataProfiler::rejectsBadChecksum()
{
    ProtocolSettings model;
    dataprofiler profiler;
    profiler.setModel(&model);
    QSignalSpy good(&profiler, &dataprofiler::deviceData);
    QSignalSpy bad(&profiler, &dataprofiler::badCRC);

    QVector<int> frame = makeFrame(model.protocol(), 0x01);
    frame[5] = 0x7F; //данные испорчены, а сумма осталась прежней
    feed(profiler, frame);

    QCOMPARE(good.count(), 0);
    QCOMPARE(bad.count(), 1);
}

void TestDataProfiler::ignoresFrameWithWrongMarker()
{
    ProtocolSettings model;
    dataprofiler profiler;
    profiler.setModel(&model);
    QSignalSpy spy(&profiler, &dataprofiler::deviceData);

    QVector<int> frame = makeFrame(model.protocol(), 0x01);
    frame[0] = 0x00; //маркер не тот - сумма сходится, но кадр не принят
    frame.last() = framecheck::checksum(frame, model.protocol().calcCRCFromPosition);
    feed(profiler, frame);

    QCOMPARE(spy.count(), 0);
}

void TestDataProfiler::resyncsAfterGarbageByte()
{
    ProtocolSettings model;
    dataprofiler profiler;
    profiler.setModel(&model);
    QSignalSpy spy(&profiler, &dataprofiler::deviceData);

    const QVector<int> frame = makeFrame(model.protocol(), 0x2A);
    profiler.getByte(0x00); //мусор перед кадром
    feed(profiler, frame);

    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(1).value<QVector<int>>(), frame);
}

void TestDataProfiler::keepsLogTimestampWhenReadingFromFile()
{
    ProtocolSettings model;
    s_Settings settings;
    settings.readFromFileFlag = true; //при чтении лога время берётся из лога, а не из системных часов
    model.setSettings(settings);

    dataprofiler profiler;
    profiler.setModel(&model);
    profiler.setTime(QDateTime(QDate(2026, 2, 6), QTime(10, 21, 34, 567)));
    QSignalSpy spy(&profiler, &dataprofiler::deviceData);

    feed(profiler, makeFrame(model.protocol(), 0x05));

    QCOMPARE(spy.count(), 1);
    const QDateTime frameTime = spy.at(0).at(0).toDateTime();
    QCOMPARE(frameTime, QDateTime(QDate(2026, 2, 6), QTime(10, 21, 34, 567)));
}

void TestDataProfiler::doesNothingWithoutModel()
{
    dataprofiler profiler; //модель не подставлена
    QSignalSpy spy(&profiler, &dataprofiler::deviceData);

    s_protocolDescription protocol;
    feed(profiler, makeFrame(protocol, 0x01));

    QCOMPARE(spy.count(), 0);
}

void TestDataProfiler::acceptsMarkerlessNineByteFrame()
{
    //Протокол 2470 (профиль 2470.eag): кадр 9 байт без маркера - адрес устройства в байте 0,
    //байты данных 1..7, контрольная сумма в байте 8 как сумма байтов 0..7 (в прошивках
    //адрес плюс семь байт данных)
    ProtocolSettings model;
    s_protocolDescription protocol;
    protocol.packetSize = 9;
    protocol.blockIdentifycatorPosition = 0;
    protocol.calcCRCFromPosition = 0;
    protocol.markerPacketBeginSize = 0;
    model.setProtocol(protocol);

    dataprofiler profiler;
    profiler.setModel(&model);
    QSignalSpy spy(&profiler, &dataprofiler::deviceData);

    QVector<int> frame(9, 0);
    frame[0] = 2;    //адрес БУД2
    frame[2] = 0x80; //температура ОГ
    frame[5] = 0x05; //обороты, старший байт
    frame[6] = 0x3C; //обороты, младший байт
    frame.last() = framecheck::checksum(frame, protocol.calcCRCFromPosition);
    feed(profiler, frame);

    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(1).value<QVector<int>>(), frame);

    //тот же кадр с испорченной суммой не принимается, а разбор сдвигается на байт
    QVector<int> broken = frame;
    broken[3] = 0x77;
    QSignalSpy bad(&profiler, &dataprofiler::badCRC);
    feed(profiler, broken);
    QCOMPARE(bad.count() > 0, true);
    QCOMPARE(spy.count(), 1); //нового кадра нет
}
