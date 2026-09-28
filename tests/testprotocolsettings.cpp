#include "testprotocolsettings.h"
#include <QSerialPort>
#include <QtTest>
#include "protocolsettings.h"

void TestProtocolSettings::protocolDefaults()
{
    const ProtocolSettings model;
    QCOMPARE(model.protocol().packetSize, 40);
    QCOMPARE(model.protocol().blockIdentifycatorPosition, 38);
    QCOMPARE(model.protocol().markerPacketBeginSize, 1);
    QCOMPARE(int(model.protocol().markerPacketBeginByte1), 0xFF);
}

void TestProtocolSettings::settingsDefaults()
{
    const ProtocolSettings model;
    QCOMPARE(model.settings().baudRate, 115200);
    QCOMPARE(static_cast<int>(model.settings().dataBits), static_cast<int>(QSerialPort::Data8));
    QVERIFY(model.settings().profilePath.isEmpty());
    QVERIFY(!model.settings().readFromFileFlag);
}

void TestProtocolSettings::storesProtocol()
{
    ProtocolSettings model;
    s_protocolDescription protocol;
    protocol.packetSize = 64;
    protocol.markerPacketBeginSize = 2;
    protocol.markerPacketBeginByte2 = 0xAB;
    protocol.description = QStringLiteral("Профиль");

    model.setProtocol(protocol);

    QCOMPARE(model.protocol().packetSize, 64);
    QCOMPARE(model.protocol().markerPacketBeginSize, 2);
    QCOMPARE(int(model.protocol().markerPacketBeginByte2), 0xAB);
    QCOMPARE(model.protocol().description, QStringLiteral("Профиль"));
}

void TestProtocolSettings::storesSettings()
{
    ProtocolSettings model;
    s_Settings settings;
    settings.name = QStringLiteral("COM7");
    settings.baudRate = 9600;
    settings.pathToBinFile = QStringLiteral("C:/Logs/a.csv");
    settings.readFromFileFlag = true;

    model.setSettings(settings);

    QCOMPARE(model.settings().name, QStringLiteral("COM7"));
    QCOMPARE(model.settings().baudRate, 9600);
    QCOMPARE(model.settings().pathToBinFile, QStringLiteral("C:/Logs/a.csv"));
    QVERIFY(model.settings().readFromFileFlag);
}

void TestProtocolSettings::mutableSettingsWritesThrough()
{
    ProtocolSettings model;
    model.mutableSettings().baudRate = 38400;
    model.mutableSettings().parity = QSerialPort::OddParity;

    QCOMPARE(model.settings().baudRate, 38400);
    QCOMPARE(static_cast<int>(model.settings().parity), static_cast<int>(QSerialPort::OddParity));
}

void TestProtocolSettings::referencesStayStable()
{
    //Читатели держат ссылку, поэтому её адрес не должен меняться
    ProtocolSettings model;
    const s_protocolDescription *protocolAddress = &model.protocol();
    const s_Settings *settingsAddress = &model.settings();

    s_protocolDescription protocol;
    protocol.packetSize = 12;
    model.setProtocol(protocol);

    QCOMPARE(&model.protocol(), protocolAddress);
    QCOMPARE(&model.settings(), settingsAddress);
    QCOMPARE(model.protocol().packetSize, 12);
}
