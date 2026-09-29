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
    QVERIFY(text.contains(QStringLiteral("Pn*  parameters claim the same bits of the byte")));
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

void TestPacketDiagram::maskWithoutBitsIsShown()
{
    //Пустая маска, реально записанная в профиль, - это параметр профиля: показываем её как есть
    s_protocolDescription protocol;
    s_parameterMask withoutBits = makeMask(3, 1, QStringLiteral("00000000"), QStringLiteral("Пустая"));
    withoutBits.devNum = 1;
    s_parameterMask real = makeMask(5, 0, QStringLiteral("11111111"), QStringLiteral("Первая"));
    real.devNum = 1;

    QVector<s_parameterMask> masks;
    masks.append(withoutBits);
    masks.append(real);

    const QString text = packetdiagram::picture(protocol, masks);
    const QString row = dataRow(text, 0);

    QVERIFY(text.contains(QStringLiteral("Пустая")));
    QVERIFY(text.contains(QStringLiteral("(no bits)")));
    QVERIFY(text.contains(QStringLiteral("the mask has 8 characters")));
    QCOMPARE(row.mid(1 + 3 * 5, 2), QStringLiteral("P1")); //пустая маска занимает своё слово
    QCOMPARE(row.mid(1 + 4 * 5, 2), QStringLiteral("P1"));
    QCOMPARE(row.mid(1 + 5 * 5, 2), QStringLiteral("P2")); //нумерация по порядку байтов
}

void TestPacketDiagram::deviceWithEmptyMasksGetsItsSection()
{
    s_protocolDescription protocol;
    s_parameterMask emptyDev = makeMask(3, 0, QStringLiteral("00000000"), QStringLiteral("Пустая"));
    emptyDev.devNum = 1;
    emptyDev.devName = QStringLiteral("БЗА");
    s_parameterMask realDev = makeMask(3, 0, QStringLiteral("11111111"), QStringLiteral("A"));
    realDev.devNum = 2;
    realDev.devName = QStringLiteral("КДГ");

    QVector<s_parameterMask> masks;
    masks.append(emptyDev);
    masks.append(realDev);

    const QString text = packetdiagram::pictureDevices(protocol, masks).text;

    QVERIFY(text.contains(QStringLiteral("Device 1 (БЗА)")));
    QVERIFY(text.contains(QStringLiteral("Device 2 (КДГ)")));
    QVERIFY(!text.contains(QStringLiteral("overlaps"))); //маски разных устройств не смешиваются
}

void TestPacketDiagram::graphedParamGetsGraphColor()
{
    s_protocolDescription protocol;
    s_parameterMask mask = makeMask(3, 0, QStringLiteral("00001111"), QStringLiteral("Режим"));
    mask.drawGraphFlag = true;
    mask.drawGraphColor = QStringLiteral("#000080"); //тёмно-синий цвет графика из настроек маски

    QVector<s_parameterMask> masks;
    masks.append(mask);

    const packetdiagram::DiagramPicture picture = packetdiagram::pictureDevices(protocol, masks);

    // Обозначение выделяется и в схеме, и в легенде
    QCOMPARE(picture.colors.size(), 2);
    const packetdiagram::ParamColor color = picture.colors.first();
    //Выделяется только обозначение параметра, а не весь байт
    QCOMPARE(picture.text.mid(color.position, color.length), QStringLiteral("P1"));
    QCOMPARE(picture.colors.at(1).length, color.length);
    QCOMPARE(picture.text.mid(picture.colors.at(1).position, picture.colors.at(1).length), QStringLiteral("P1"));
    QCOMPARE(color.background, QStringLiteral("#000080"));
    QCOMPARE(color.foreground, QStringLiteral("#FFFFFF")); //тёмный фон - шрифт светлый
}

void TestPacketDiagram::ungraphedParamHasNoColor()
{
    s_protocolDescription protocol;
    s_parameterMask withoutGraph = makeMask(3, 0, QStringLiteral("11111111"), QStringLiteral("A"));
    withoutGraph.drawGraphFlag = false;
    withoutGraph.drawGraphColor = QStringLiteral("#ff0000");
    s_parameterMask withoutColor = makeMask(4, 0, QStringLiteral("11111111"), QStringLiteral("B"));
    withoutColor.drawGraphFlag = true;
    withoutColor.drawGraphColor = QString();

    QVector<s_parameterMask> masks;
    masks.append(withoutGraph);
    masks.append(withoutColor);

    QVERIFY(packetdiagram::pictureDevices(protocol, masks).colors.isEmpty());
}

void TestPacketDiagram::devicesGetTheirOwnSection()
{
    //Профиль 2543.eag: у каждого устройства свой состав параметров на тех же байтах,
    //и параметры разных устройств не должны считаться пересекающимися
    s_protocolDescription protocol;
    protocol.packetSize = 8;
    protocol.markerPacketBeginSize = 0;
    protocol.blockIdentifycatorPosition = 0;

    s_parameterMask firstDev = makeMask(3, 1, QStringLiteral("1111111111111111"), QStringLiteral("A"));
    firstDev.devNum = 1;
    firstDev.devName = QStringLiteral("КДГ");
    s_parameterMask secondDev = makeMask(3, 0, QStringLiteral("10101010"), QStringLiteral("B"));
    secondDev.devNum = 2;
    secondDev.devName = QStringLiteral("ПДУ");
    s_parameterMask lateDev = makeMask(3, 0, QStringLiteral("10101010"), QStringLiteral("C"));
    lateDev.devNum = 5; //устройство без имени
    lateDev.devName.clear();

    QVector<s_parameterMask> masks;
    masks.append(secondDev);
    masks.append(lateDev); //порядок масок в списке не задаёт порядок разделов
    masks.append(firstDev);

    const QString text = packetdiagram::pictureDevices(protocol, masks).text;

    QVERIFY(text.contains(QStringLiteral("Device 1 (КДГ)")));
    QVERIFY(text.contains(QStringLiteral("Device 2 (ПДУ)")));
    QVERIFY(text.contains(QStringLiteral("Device 5\n"))); //имени нет - только номер
    QVERIFY(text.indexOf(QStringLiteral("Device 1")) < text.indexOf(QStringLiteral("Device 2")));
    QVERIFY(text.indexOf(QStringLiteral("Device 2")) < text.indexOf(QStringLiteral("Device 5")));
    //Нумерация параметров идёт внутри устройства: в каждом разделе есть свой P1
    QCOMPARE(text.count(QStringLiteral("mask 10101010")), 2);
    QVERIFY(!text.contains(QStringLiteral("overlaps"))); //пересечений между устройствами нет
}

void TestPacketDiagram::bitFlagsOnOneByteAreNotOverlaps()
{
    //Параметры-флаги на одном байте делят его, но их биты разные - это не пересечение
    s_protocolDescription protocol;
    protocol.packetSize = 8;
    protocol.markerPacketBeginSize = 0;
    protocol.blockIdentifycatorPosition = 0;

    s_parameterMask low = makeMask(3, 1, QStringLiteral("0000000000000001"), QStringLiteral("Low"));
    low.devNum = 1;
    s_parameterMask high = makeMask(3, 1, QStringLiteral("0000000000010000"), QStringLiteral("High"));
    high.devNum = 1;

    QVector<s_parameterMask> masks;
    masks.append(low);
    masks.append(high);

    const QString text = packetdiagram::picture(protocol, masks);
    const QString row = dataRow(text, 0);

    QCOMPARE(row.mid(1 + 3 * 5, 2), QStringLiteral("P1"));
    QCOMPARE(row.mid(1 + 3 * 5 + 2, 1), QStringLiteral("+")); //в байте есть и другие параметры
    QVERIFY(!text.contains(QStringLiteral("overlaps")));
    QVERIFY(text.contains(QStringLiteral("Pn+  the byte is shared with other parameters of the device")));
    //Первый байт слова у второго параметра пустой: его бит лежит во втором байте
    QCOMPARE(row.mid(1 + 4 * 5, 2), QStringLiteral("P1"));
    QCOMPARE(row.mid(1 + 4 * 5 + 2, 1), QStringLiteral("+"));

    //А совпадающие биты - настоящее пересечение
    masks[1] = makeMask(3, 1, QStringLiteral("0000000000000001"), QStringLiteral("Same"));
    masks[1].devNum = 1;
    const QString conflict = packetdiagram::picture(protocol, masks);
    QVERIFY(conflict.contains(QStringLiteral("byte 3: P2 (Same) overlaps with P1")));
    QCOMPARE(dataRow(conflict, 0).mid(1 + 3 * 5 + 2, 1), QStringLiteral("*"));
}

void TestPacketDiagram::paramColorsFollowPacketOrder()
{
    s_protocolDescription protocol;
    s_parameterMask late = makeMask(9, 0, QStringLiteral("11111111"), QStringLiteral("Late"));
    late.drawGraphFlag = true;
    late.drawGraphColor = QStringLiteral("#00ff00");
    s_parameterMask early = makeMask(2, 0, QStringLiteral("11111111"), QStringLiteral("Early"));
    early.drawGraphFlag = true;
    early.drawGraphColor = QStringLiteral("#000080");

    QVector<s_parameterMask> masks;
    masks.append(late);
    masks.append(early); //маски приходят в произвольном порядке - нумерация идёт по байтам пакета

    const packetdiagram::DiagramPicture picture = packetdiagram::pictureDevices(protocol, masks);

    //По два выделения на параметр: в схеме и в легенде
    QCOMPARE(picture.colors.size(), 4);
    //Первое по порядку выделение - обозначение параметра на байте 2 (P1), затем на байте 9 (P2)
    QCOMPARE(picture.text.mid(picture.colors.at(0).position, picture.colors.at(0).length), QStringLiteral("P1"));
    QCOMPARE(picture.colors.at(0).background, QStringLiteral("#000080"));
    QCOMPARE(picture.text.mid(picture.colors.at(1).position, picture.colors.at(1).length), QStringLiteral("P2"));
    QCOMPARE(picture.colors.at(1).background, QStringLiteral("#00ff00"));
    QCOMPARE(picture.text.mid(picture.colors.at(2).position, picture.colors.at(2).length), QStringLiteral("P1"));
    QCOMPARE(picture.text.mid(picture.colors.at(3).position, picture.colors.at(3).length), QStringLiteral("P2"));
}

void TestPacketDiagram::textColorContrastsWithBackground()
{
    //Из двух вариантов (чёрный/белый) берётся тот, у которого контраст с фоном выше
    QCOMPARE(packetdiagram::contrastTextColor(QStringLiteral("#FFFFFF")), QStringLiteral("#000000")); //светлый фон
    QCOMPARE(packetdiagram::contrastTextColor(QStringLiteral("#00FF00")), QStringLiteral("#000000"));
    QCOMPARE(packetdiagram::contrastTextColor(QStringLiteral("#FF0000")), QStringLiteral("#000000"));
    QCOMPARE(packetdiagram::contrastTextColor(QStringLiteral("#000000")), QStringLiteral("#FFFFFF")); //тёмный фон
    QCOMPARE(packetdiagram::contrastTextColor(QStringLiteral("#000080")), QStringLiteral("#FFFFFF"));
    QCOMPARE(packetdiagram::contrastTextColor(QString()), QString()); //нераспознанный цвет - выделять нечем
}
