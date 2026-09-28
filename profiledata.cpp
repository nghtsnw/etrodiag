#include "profiledata.h"
#include <QSerialPort>
#include <QStringList>
#include <QTextStream>
#include <utility>

namespace profiledata {

//Разбор строки протокольной части профиля; false - строка не протокольная
static bool parseProtocolLine(const QStringList &strLst, s_protocolDescription &pt)
{
    const QString &key = strLst.at(0);
    const QString &val = strLst.at(1);
    if (key == "packetSize") {
        pt.packetSize = val.toInt(0, 10);
    }
    else if (key == "blockIdentifycatorPosition") {
        pt.blockIdentifycatorPosition = val.toInt(0, 10);
    }
    else if (key == "calcCRCFromPosition") {
        pt.calcCRCFromPosition = val.toInt(0, 10);
    }
    else if (key == "markerPacketBeginSize") {
        pt.markerPacketBeginSize = val.toInt(0, 10);
    }
    else if (key == "markerPacketBeginTextB0") {
        pt.markerPacketBeginByte1 = val.toInt(0, 16);
    }
    else if (key == "markerPacketBeginTextB1") {
        pt.markerPacketBeginByte2 = val.toInt(0, 16);
    }
    else if (key == "description") {
        pt.description = val;
    }
    else if (key == "varControl") {
        pt.varControl = (val == "true");
    }
    else {
        return false;
    }
    return true;
}

//Разбор строки настроек связи из профиля (portName старых профилей игнорируется - порт выбирается вручную)
static bool parseSettingsLine(const QStringList &strLst, s_Settings &st)
{
    const QString &key = strLst.at(0);
    const QString &val = strLst.at(1);
    if (key == "readFromFile") {
        st.readFromFileFlag = (val == "true");
    }
    else if (key == "logFilePath") {
        st.pathToBinFile = val;
    }
    else if (key == "baudRate") {
        st.baudRate = val.toInt(0, 10);
    }
    else if (key == "dataBits") {
        st.dataBits = static_cast<QSerialPort::DataBits>(val.toInt(0, 10));
    }
    else if (key == "parity") {
        st.parity = static_cast<QSerialPort::Parity>(val.toInt(0, 10));
    }
    else if (key == "stopBits") {
        st.stopBits = static_cast<QSerialPort::StopBits>(val.toInt(0, 10));
    }
    else if (key == "flowControl") {
        st.flowControl = static_cast<QSerialPort::FlowControl>(val.toInt(0, 10));
    }
    else {
        return false;
    }
    return true;
}

//Разбор строки маски (вызывается только если в строке есть все поля маски)
static s_parameterMask parseMaskLine(const QStringList &strLst)
{
    s_parameterMask mask{};
    mask.id = strLst.at(1).toInt(0, 10);
    mask.devNum = strLst.at(2).toInt(0, 10);
    mask.byteNum = strLst.at(3).toInt(0, 10);
    mask.devName = strLst.at(4);
    mask.byteName = strLst.at(5);
    mask.parameterName = strLst.at(6);
    mask.parameterMask = strLst.at(7);
    mask.valueShift = strLst.at(8).toDouble();
    mask.valueKoef = strLst.at(9).toDouble();
    mask.viewInLogFlag = (QString::compare(strLst.at(10), "true") == 0);
    mask.wordType = strLst.at(11).toInt(0, 10);
    mask.drawGraphFlag = (QString::compare(strLst.at(12), "true") == 0);
    mask.drawGraphColor = strLst.at(13);
    return mask;
}

ProfileText parse(const QString &text)
{
    ProfileText result;
    QStringList maskLines;
    //Читаем построчно: строки без разделителей и обрывки шапки пропускаем, чтобы не выйти за границы
    const QStringList lines = text.split('\n');
    for (QString line : lines) {
        if (line.endsWith('\r')) { //профиль мог быть отредактирован с CRLF
            line.chop(1);
        }
        const QStringList strLst = line.split('\t');
        if (strLst.size() < 2) {
            continue;
        }
        if (strLst.at(0) == maskTag()) {
            maskLines.append(line);
            continue;
        }
        if (!parseProtocolLine(strLst, result.protocol)) { //не протокольная - пробуем как настройку связи
            result.hasSettings = parseSettingsLine(strLst, result.settings) || result.hasSettings;
        }
    }
    //Протокол применяется раньше масок, поэтому маски разбираются во втором проходе
    for (const QString &maskLine : std::as_const(maskLines)) {
        const QStringList strLst = maskLine.split('\t');
        if (strLst.at(0) == maskTag() && strLst.size() >= kMaskFieldCount) {
            result.masks.append(parseMaskLine(strLst));
        }
    }
    return result;
}

QString serializeHeader(const QString &profileFileName, const s_protocolDescription &protocol,
                        const s_Settings &settings)
{
    QString content;
    QTextStream txtStream(&content);
    txtStream << profileFileName << "\n";
    txtStream << "packetSize" << "\t" << QString::number(protocol.packetSize, 10) << "\n";
    txtStream << "blockIdentifycatorPosition" << "\t" << QString::number(protocol.blockIdentifycatorPosition, 10) << "\n";
    txtStream << "calcCRCFromPosition" << "\t" << QString::number(protocol.calcCRCFromPosition, 10) << "\n";
    txtStream << "markerPacketBeginSize" << "\t" << QString::number(protocol.markerPacketBeginSize, 10) << "\n";
    txtStream << "markerPacketBeginTextB0" << "\t" << QString::number(protocol.markerPacketBeginByte1, 16) << "\n";
    txtStream << "markerPacketBeginTextB1" << "\t" << QString::number(protocol.markerPacketBeginByte2, 16) << "\n";
    txtStream << "description" << "\t" << protocol.description << "\n";
    txtStream << "varControl" << "\t" << (protocol.varControl ? "true" : "false") << "\n";
    txtStream << "readFromFile" << "\t" << (settings.readFromFileFlag ? "true" : "false") << "\n";
    txtStream << "logFilePath" << "\t" << settings.pathToBinFile << "\n";
    txtStream << "baudRate" << "\t" << QString::number(settings.baudRate, 10) << "\n";
    txtStream << "dataBits" << "\t" << QString::number(static_cast<int>(settings.dataBits), 10) << "\n";
    txtStream << "parity" << "\t" << QString::number(static_cast<int>(settings.parity), 10) << "\n";
    txtStream << "stopBits" << "\t" << QString::number(static_cast<int>(settings.stopBits), 10) << "\n";
    txtStream << "flowControl" << "\t" << QString::number(static_cast<int>(settings.flowControl), 10) << "\n";
    return content;
}

QString serializeMaskLine(const QList<QString> &maskFields)
{
    QString line;
    for (const QString &field : maskFields) {
        line += field + '\t';
    }
    return line + '\n';
}

}
