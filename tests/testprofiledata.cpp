#include "testprofiledata.h"
#include <QSerialPort>
#include <QtTest>
#include "profiledata.h"

//Профиль в том виде, в каком его пишет newconnect::saveProfile
static QString sampleProfileText()
{
    return QStringLiteral(
        "test.eag\n"
        "packetSize\t40\n"
        "blockIdentifycatorPosition\t38\n"
        "calcCRCFromPosition\t2\n"
        "markerPacketBeginSize\t1\n"
        "markerPacketBeginTextB0\tff\n"
        "markerPacketBeginTextB1\t0\n"
        "description\tTest profile\n"
        "varControl\ttrue\n"
        "readFromFile\tfalse\n"
        "logFilePath\tC:/Logs/test.csv\n"
        "baudRate\t115200\n"
        "dataBits\t8\n"
        "parity\t0\n"
        "stopBits\t1\n"
        "flowControl\t0\n");
}

//Строка маски: 14 полей, как их хранит txtmaskobj
static QList<QString> sampleMaskFields()
{
    return QList<QString>{"thisIsMask", "3", "1", "7", "Dev", "Byte", "Param",
                          "10101010", "0", "1", "true", "1", "true", "#00ff00"};
}

void TestProfileData::parsesProtocolFields()
{
    const profiledata::ProfileText parsed = profiledata::parse(sampleProfileText());

    QCOMPARE(parsed.protocol.packetSize, 40);
    QCOMPARE(parsed.protocol.blockIdentifycatorPosition, 38);
    QCOMPARE(parsed.protocol.calcCRCFromPosition, 2);
    QCOMPARE(parsed.protocol.markerPacketBeginSize, 1);
    QCOMPARE(int(parsed.protocol.markerPacketBeginByte1), 0xFF); //байты маркера пишутся в 16-ричном виде
    QCOMPARE(int(parsed.protocol.markerPacketBeginByte2), 0x00);
    QCOMPARE(parsed.protocol.description, QStringLiteral("Test profile"));
    QVERIFY(parsed.protocol.varControl);
    QVERIFY(parsed.masks.isEmpty());
}

void TestProfileData::parsesConnectionSettings()
{
    const profiledata::ProfileText parsed = profiledata::parse(sampleProfileText());

    QVERIFY(parsed.hasSettings);
    QCOMPARE(parsed.settings.readFromFileFlag, false);
    QCOMPARE(parsed.settings.pathToBinFile, QStringLiteral("C:/Logs/test.csv"));
    QCOMPARE(parsed.settings.baudRate, 115200);
    QCOMPARE(static_cast<int>(parsed.settings.dataBits), static_cast<int>(QSerialPort::Data8));
    QCOMPARE(static_cast<int>(parsed.settings.parity), static_cast<int>(QSerialPort::NoParity));
    QCOMPARE(static_cast<int>(parsed.settings.stopBits), static_cast<int>(QSerialPort::OneStop));
    QCOMPARE(static_cast<int>(parsed.settings.flowControl), static_cast<int>(QSerialPort::NoFlowControl));
}

void TestProfileData::missingSettingsIsReported()
{
    const QString text = QStringLiteral("test.eag\n"
                                        "packetSize\t12\n"
                                        "varControl\tfalse\n");
    const profiledata::ProfileText parsed = profiledata::parse(text);

    QCOMPARE(parsed.protocol.packetSize, 12);
    QVERIFY(!parsed.protocol.varControl);
    QVERIFY(!parsed.hasSettings);
}

void TestProfileData::emptyTextGivesProtocolDefaults()
{
    const profiledata::ProfileText parsed = profiledata::parse(QString());

    QCOMPARE(parsed.protocol.packetSize, 40);
    QCOMPARE(parsed.protocol.blockIdentifycatorPosition, 38);
    QCOMPARE(parsed.protocol.calcCRCFromPosition, 0);
    QCOMPARE(parsed.protocol.markerPacketBeginSize, 1);
    QCOMPARE(int(parsed.protocol.markerPacketBeginByte1), 0xFF);
    QVERIFY(!parsed.hasSettings);
    QVERIFY(parsed.masks.isEmpty());
}

void TestProfileData::ignoresGarbageLines()
{
    const QString text = QStringLiteral("test.eag\n"
                                        "\n"
                                        "packetSize\n"          //строка без значения
                                        "noTabsHere\n"          //строка без разделителей
                                        "unknownKey\tvalue\n"   //неизвестный ключ
                                        "packetSize\t64\n");
    const profiledata::ProfileText parsed = profiledata::parse(text);

    QCOMPARE(parsed.protocol.packetSize, 64);
    QVERIFY(!parsed.hasSettings);
    QVERIFY(parsed.masks.isEmpty());
}

void TestProfileData::parsesMaskLine()
{
    const profiledata::ProfileText parsed = profiledata::parse(sampleProfileText()
                                                              + profiledata::serializeMaskLine(sampleMaskFields()));

    QCOMPARE(parsed.masks.size(), 1);
    const s_parameterMask mask = parsed.masks.first();
    QCOMPARE(mask.id, 3);
    QCOMPARE(mask.devNum, 1);
    QCOMPARE(mask.byteNum, 7);
    QCOMPARE(mask.devName, QStringLiteral("Dev"));
    QCOMPARE(mask.byteName, QStringLiteral("Byte"));
    QCOMPARE(mask.parameterName, QStringLiteral("Param"));
    QCOMPARE(mask.parameterMask, QStringLiteral("10101010"));
    QCOMPARE(mask.valueShift, 0.0);
    QCOMPARE(mask.valueKoef, 1.0);
    QVERIFY(mask.viewInLogFlag);
    QCOMPARE(mask.wordType, 1);
    QVERIFY(mask.drawGraphFlag);
    QCOMPARE(mask.drawGraphColor, QStringLiteral("#00ff00"));
}

void TestProfileData::parsesMaskLineWithoutTrailingTab()
{
    //Профиль мог быть отредактирован руками: без завершающей табуляции в конце строки
    const QString maskLine = QStringLiteral("thisIsMask\t4\t1\t2\tDev\tByte\tParam\t1111\t0\t1\tfalse\t2\ttrue\t#ff0000\n");
    QCOMPARE(maskLine.split('\t').size(), profiledata::kMaskFieldCount);

    const profiledata::ProfileText parsed = profiledata::parse(maskLine);

    QCOMPARE(parsed.masks.size(), 1);
    QCOMPARE(parsed.masks.first().id, 4);
    QCOMPARE(parsed.masks.first().wordType, 2);
    QVERIFY(!parsed.masks.first().viewInLogFlag);
}

void TestProfileData::skipsIncompleteMaskLine()
{
    //Обрывок строки маски: перечислены не все поля - маска не разбирается
    const QString incompleteMask = QStringLiteral("thisIsMask\t1\t2\t3\tDev\tByte\tParam\t1010\t0\t1\ttrue\t1\ttrue\n");
    QCOMPARE(incompleteMask.split('\t').size(), profiledata::kMaskFieldCount - 1);

    const profiledata::ProfileText parsed = profiledata::parse(incompleteMask);

    QVERIFY(parsed.masks.isEmpty());
}

void TestProfileData::handlesCrlfLineEndings()
{
    QString text = sampleProfileText();
    text.replace(QStringLiteral("\n"), QStringLiteral("\r\n"));

    const profiledata::ProfileText parsed = profiledata::parse(text);

    QCOMPARE(parsed.protocol.description, QStringLiteral("Test profile")); //без '\r' в конце поля
    QCOMPARE(parsed.protocol.packetSize, 40);
    QVERIFY(parsed.hasSettings);
}

void TestProfileData::serializedHeaderParsesBack()
{
    s_protocolDescription protocol;
    protocol.packetSize = 64;
    protocol.blockIdentifycatorPosition = 10;
    protocol.calcCRCFromPosition = 4;
    protocol.markerPacketBeginSize = 2;
    protocol.markerPacketBeginByte1 = 0xFF;
    protocol.markerPacketBeginByte2 = 0xAB;
    protocol.description = QStringLiteral("Профиль №1");
    protocol.varControl = true;

    s_Settings settings;
    settings.readFromFileFlag = true;
    settings.pathToBinFile = QStringLiteral("C:/Logs/read.csv");
    settings.baudRate = 9600;
    settings.dataBits = QSerialPort::Data7;
    settings.parity = QSerialPort::EvenParity;
    settings.stopBits = QSerialPort::TwoStop;
    settings.flowControl = QSerialPort::HardwareControl;

    const QString text = profiledata::serializeHeader(QStringLiteral("my.eag"), protocol, settings);
    QCOMPARE(text.split('\n').first(), QStringLiteral("my.eag"));

    const profiledata::ProfileText parsed = profiledata::parse(text);
    QCOMPARE(parsed.protocol.packetSize, 64);
    QCOMPARE(parsed.protocol.blockIdentifycatorPosition, 10);
    QCOMPARE(parsed.protocol.calcCRCFromPosition, 4);
    QCOMPARE(parsed.protocol.markerPacketBeginSize, 2);
    QCOMPARE(int(parsed.protocol.markerPacketBeginByte1), 0xFF);
    QCOMPARE(int(parsed.protocol.markerPacketBeginByte2), 0xAB);
    QCOMPARE(parsed.protocol.description, QStringLiteral("Профиль №1"));
    QVERIFY(parsed.protocol.varControl);

    QVERIFY(parsed.hasSettings);
    QCOMPARE(parsed.settings.readFromFileFlag, true);
    QCOMPARE(parsed.settings.pathToBinFile, QStringLiteral("C:/Logs/read.csv"));
    QCOMPARE(parsed.settings.baudRate, 9600);
    QCOMPARE(static_cast<int>(parsed.settings.dataBits), static_cast<int>(QSerialPort::Data7));
    QCOMPARE(static_cast<int>(parsed.settings.parity), static_cast<int>(QSerialPort::EvenParity));
    QCOMPARE(static_cast<int>(parsed.settings.stopBits), static_cast<int>(QSerialPort::TwoStop));
    QCOMPARE(static_cast<int>(parsed.settings.flowControl), static_cast<int>(QSerialPort::HardwareControl));
}

void TestProfileData::serializedMaskLineRoundTrip()
{
    const QList<QString> fields = sampleMaskFields();
    const QString line = profiledata::serializeMaskLine(fields);

    QVERIFY(line.endsWith(QStringLiteral("\t\n"))); //как писал прежний saveProfile
    QCOMPARE(line.split('\t').size(), profiledata::kMaskFieldCount + 1);

    const profiledata::ProfileText parsed = profiledata::parse(line);
    QCOMPARE(parsed.masks.size(), 1);
    const s_parameterMask mask = parsed.masks.first();
    QCOMPARE(mask.id, 3);
    QCOMPARE(mask.devNum, 1);
    QCOMPARE(mask.byteNum, 7);
    QCOMPARE(mask.parameterMask, QStringLiteral("10101010"));
    QCOMPARE(mask.wordType, 1);
    QCOMPARE(mask.drawGraphColor, QStringLiteral("#00ff00"));
}
