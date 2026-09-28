#include "testpacketdiagram.h"
#include <QtTest>
#include "packetdiagram.h"

namespace {

//Строка с данными строки схемы: заголовок занимает 3 строки, дальше тройки
//"линейка индексов / граница / данные / граница"
QString dataRow(const QString &text, int row)
{
    const QStringList lines = text.split('\n');
    return lines.value(3 + row * 4 + 2);
}

QString indexRow(const QString &text, int row)
{
    const QStringList lines = text.split('\n');
    return lines.value(3 + row * 4);
}

s_parameterMask makeMask(int byteNum, int wordType, const QString &mask, const QString &name = QStringLiteral("Param"))
{
    s_parameterMask result{};
    result.byteNum = byteNum;
    result.wordType = wordType;
    result.parameterMask = mask;
    result.parameterName = name;
    return result;
}

}

void TestPacketDiagram::headerDescribesPacketAndStructuralBytes()
{
    s_protocolDescription protocol; //40 байт, маркер FF, id 38, CRC с 0
    const QString text = packetdiagram::picture(protocol, QVector<s_parameterMask>());

    const QStringList lines = text.split('\n');
    QCOMPARE(lines.at(0), QStringLiteral("Packet format"));
    QVERIFY(lines.at(1).startsWith(QStringLiteral("Packet 40 bytes")));
    QVERIFY(lines.at(1).contains(QStringLiteral("marker FF (byte 0)")));
    QVERIFY(lines.at(1).contains(QStringLiteral("device id: byte 38")));
    QVERIFY(lines.at(1).contains(QStringLiteral("checksum: byte 39 (sum of bytes 0..38)")));
    QVERIFY(lines.at(2).startsWith(QStringLiteral("Words are little-endian")));
}

void TestPacketDiagram::rowLayoutKeepsByteColumns()
{
    s_protocolDescription protocol;
    protocol.blockIdentifycatorPosition = 2; //как в реальных профилях
    QVector<s_parameterMask> masks;
    masks.append(makeMask(13, 0, QStringLiteral("11111111"), QStringLiteral("Parameter")));

    const QString text = packetdiagram::picture(protocol, masks);
    const QString row = dataRow(text, 0);

    QCOMPARE(row.at(0), QLatin1Char('|'));
    QCOMPARE(row.mid(1, 2), QStringLiteral("Mk"));           //байт 0 - маркер
    QCOMPARE(row.mid(1 + 2 * 5, 2), QStringLiteral("ID"));   //байт 2 - id устройства
    QCOMPARE(row.mid(1 + 13 * 5, 2), QStringLiteral("P1"));  //байт 13 - параметр
    QCOMPARE(row.size(), 16 * 5 + 1);                        //16 байтов в строке
    QCOMPARE(dataRow(text, 2).mid(1 + 7 * 5, 2), QStringLiteral("CR")); //последний байт строки 32..39

    //Индексы выровнены по тем же колонкам (номер по правому краю поля)
    QCOMPARE(indexRow(text, 0).mid(13 * 5, 4).trimmed(), QStringLiteral("13"));
    QCOMPARE(indexRow(text, 1).mid(0, 4).trimmed(), QStringLiteral("16"));
}

void TestPacketDiagram::emptyBytesAreMarked()
{
    s_protocolDescription protocol;
    const QString text = packetdiagram::picture(protocol, QVector<s_parameterMask>());

    QVERIFY(dataRow(text, 0).contains(QStringLiteral("··")));
    QVERIFY(!dataRow(text, 0).contains(QStringLiteral("P1"))); //масок нет - параметров нет
}

void TestPacketDiagram::thirtyTwoBitWordSpansFourBytes()
{
    s_protocolDescription protocol;
    QVector<s_parameterMask> masks;
    masks.append(makeMask(6, 2, QStringLiteral("11111111111111111111111111111111"), QStringLiteral("Counter")));

    const QString text = packetdiagram::picture(protocol, masks);
    const QString row = dataRow(text, 0);

    for (int byte = 6; byte <= 9; byte++) {
        QCOMPARE(row.mid(1 + byte * 5, 2), QStringLiteral("P1"));
    }
    QVERIFY(text.contains(QStringLiteral("bytes 6-9")));
    QVERIFY(text.contains(QStringLiteral("32 bit")));
}

void TestPacketDiagram::twoByteMarkerAndIdAreShown()
{
    s_protocolDescription protocol;
    protocol.markerPacketBeginSize = 2;
    protocol.markerPacketBeginByte1 = 0xFF;
    protocol.markerPacketBeginByte2 = 0xAB;

    const QString text = packetdiagram::picture(protocol, QVector<s_parameterMask>());
    const QString row = dataRow(text, 0);

    QCOMPARE(row.mid(1, 2), QStringLiteral("M1"));
    QCOMPARE(row.mid(1 + 5, 2), QStringLiteral("M2"));
    QVERIFY(text.contains(QStringLiteral("marker FF AB (bytes 0-1)")));
    QVERIFY(text.contains(QStringLiteral("M1 is the first byte, M2 is the second")));
}

void TestPacketDiagram::noMarkerIsReported()
{
    s_protocolDescription protocol;
    protocol.markerPacketBeginSize = 0;

    const QString text = packetdiagram::picture(protocol, QVector<s_parameterMask>());

    QVERIFY(text.split('\n').at(1).contains(QStringLiteral("no marker (checksum only)")));
}

void TestPacketDiagram::packetSizeZeroFallsBackToMasks()
{
    s_protocolDescription protocol;
    protocol.packetSize = 0;
    protocol.markerPacketBeginSize = 2;
    protocol.blockIdentifycatorPosition = 2;

    QVector<s_parameterMask> masks;
    masks.append(makeMask(3, 0, QStringLiteral("11111111"), QStringLiteral("A")));
    masks.append(makeMask(15, 1, QStringLiteral("1111111111111111"), QStringLiteral("B")));

    const QString text = packetdiagram::picture(protocol, masks);

    QVERIFY(text.split('\n').at(1).startsWith(QStringLiteral("Packet: size is not set, 17 bytes shown")));
    QVERIFY(text.contains(QStringLiteral("packet size is not set in the profile (0)")));
    QCOMPARE(dataRow(text, 0).size(), 16 * 5 + 1);
    QCOMPARE(dataRow(text, 1).size(), 1 * 5 + 1); //остался один байт - 16-й
}

void TestPacketDiagram::maskBeyondPacketIsReported()
{
    s_protocolDescription protocol;
    protocol.packetSize = 8;

    QVector<s_parameterMask> masks;
    masks.append(makeMask(7, 1, QStringLiteral("1111111111111111"), QStringLiteral("Tail")));

    const QString text = packetdiagram::picture(protocol, masks);

    QVERIFY(text.contains(QStringLiteral("P1 (Tail) occupies bytes 7-8 while the packet is 8 bytes")));
}

void TestPacketDiagram::overlappingMasksAreReportedAndMarked()
{
    s_protocolDescription protocol;
    QVector<s_parameterMask> masks;
    masks.append(makeMask(4, 1, QStringLiteral("1111111111111111"), QStringLiteral("First")));
    masks.append(makeMask(5, 1, QStringLiteral("0000000000000001"), QStringLiteral("Second")));

    const QString text = packetdiagram::picture(protocol, masks);
    const QString row = dataRow(text, 0);

    //Первый параметр занимает байт, второй на него наезжает - байт помечен звёздочкой,
    //а перекрытие попадает в примечания
    QCOMPARE(row.mid(1 + 5 * 5, 2), QStringLiteral("P1"));
    QCOMPARE(row.mid(1 + 5 * 5 + 2, 1), QStringLiteral("*"));
    QVERIFY(text.contains(QStringLiteral("byte 5: P2 (Second) overlaps with P1")));
}

void TestPacketDiagram::wordSizeMismatchIsReported()
{
    s_protocolDescription protocol;
    QVector<s_parameterMask> masks;
    //маска на 32 символа, а слово объявлено 16-битным
    masks.append(makeMask(6, 1, QStringLiteral("11111111111111111111111111111111"), QStringLiteral("Bad")));

    const QString text = packetdiagram::picture(protocol, masks);

    QVERIFY(text.contains(QStringLiteral("P1 (Bad): the mask has 32 characters while the word is 16 bit")));
}

void TestPacketDiagram::byteZeroMaskIsReported()
{
    s_protocolDescription protocol;
    protocol.markerPacketBeginSize = 0;

    QVector<s_parameterMask> masks;
    masks.append(makeMask(0, 0, QStringLiteral("11111111"), QStringLiteral("Zero")));

    const QString text = packetdiagram::picture(protocol, masks);

    QVERIFY(text.contains(QStringLiteral("P1 (Zero) is on byte 0, which the app does not process")));
}

void TestPacketDiagram::legendShowsMaskBits()
{
    s_protocolDescription protocol;
    s_parameterMask mask = makeMask(3, 0, QStringLiteral("00001111"), QStringLiteral("Режим"));
    mask.drawGraphFlag = true; //флаг графика и цвет в схему не попадают
    mask.drawGraphColor = QStringLiteral("#ff0000");

    QVector<s_parameterMask> masks;
    masks.append(mask);

    const QString text = packetdiagram::picture(protocol, masks);

    QVERIFY(text.contains(QStringLiteral("Legend:")));
    QVERIFY(text.contains(QStringLiteral("P1  Режим")));
    QVERIFY(text.contains(QStringLiteral("byte 3")));
    QVERIFY(text.contains(QStringLiteral("mask 00001111")));
    QVERIFY(text.contains(QStringLiteral("(bits 0-3)")));
    QVERIFY(!text.contains(QStringLiteral("graph")));
    QVERIFY(!text.contains(QStringLiteral("#ff0000")));
}
