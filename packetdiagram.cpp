#include "packetdiagram.h"
#include <QCoreApplication>
#include <QPair>
#include <QSet>
#include <QStringList>
#include <algorithm>
#include "maskmath.h"
#include "wordvalue.h"

namespace {

//Строки схемы переводимые, как и остальной интерфейс
class DiagramText
{
    Q_DECLARE_TR_FUNCTIONS(packetdiagram)
};

struct Param
{
    int number = 0;
    int fromByte = 0;
    int toByte = 0;
    int wordType = 0;
    QString name;
    QString mask;
    int bitFrom = -1;
    int bitTo = -1;
    bool outsidePacket = false;
};

struct Diagram
{
    int byteCount = 0;
    bool sizeFromProfile = true;
    int packetSize = 0;
    int markerSize = 1;
    QString markerText;
    int idPos = -1;
    int crcFrom = 0;
    int crcByte = -1;
    QVector<QString> token;
    QVector<Param> params;
    QStringList notes;
    QSet<int> conflicts;
};

QString hexByte(int value)
{
    return QString("%1").arg(value, 2, 16, QLatin1Char('0')).toUpper();
}

//Младший и старший единичные биты маски (правый символ строки - младший бит)
QPair<int, int> maskBitRange(const QString &mask)
{
    int from = -1;
    int to = -1;
    for (int i = mask.size() - 1, bit = 0; i > -1; i--, bit++) {
        if (mask.at(i) == '1') {
            if (from < 0) {
                from = bit;
            }
            to = bit;
        }
    }
    return qMakePair(from, to);
}

QString wordText(int wordType)
{
    return DiagramText::tr("%1 bit").arg(maskmath::wordBits(wordType));
}

QString bitsText(const Param &param)
{
    if (param.bitFrom < 0) {
        return DiagramText::tr("no bits");
    }
    if (param.bitFrom == 0 && param.bitTo == maskmath::wordBits(param.wordType) - 1) {
        return DiagramText::tr("all bits");
    }
    if (param.bitFrom == param.bitTo) {
        return DiagramText::tr("bit %1").arg(param.bitFrom);
    }
    return DiagramText::tr("bits %1-%2").arg(param.bitFrom).arg(param.bitTo);
}

QString bytesText(const Param &param)
{
    return (param.fromByte == param.toByte)
               ? DiagramText::tr("byte %1").arg(param.fromByte)
               : DiagramText::tr("bytes %1-%2").arg(param.fromByte).arg(param.toByte);
}

Diagram build(const s_protocolDescription &protocol, const QVector<s_parameterMask> &masks)
{
    Diagram d;
    d.packetSize = protocol.packetSize;
    d.markerSize = qMax(0, protocol.markerPacketBeginSize);
    d.idPos = protocol.blockIdentifycatorPosition;
    d.crcFrom = protocol.calcCRCFromPosition;
    d.sizeFromProfile = (d.packetSize > 0);

    if (d.markerSize > 0) {
        d.markerText = hexByte(protocol.markerPacketBeginByte1);
        if (d.markerSize > 1) {
            d.markerText += " " + hexByte(protocol.markerPacketBeginByte2);
        }
    }

    //Маски идут в порядке байтов, чтобы нумерация P1..Pn совпадала с порядком в пакете
    QVector<s_parameterMask> ordered = masks;
    std::stable_sort(ordered.begin(), ordered.end(), [](const s_parameterMask &left, const s_parameterMask &right) {
        if (left.byteNum != right.byteNum) {
            return left.byteNum < right.byteNum;
        }
        return left.parameterName < right.parameterName;
    });

    int lastUsed = qMax(0, d.markerSize - 1);
    lastUsed = qMax(lastUsed, d.idPos);
    for (int i = 0; i < ordered.size(); i++) {
        const s_parameterMask &mask = ordered.at(i);
        Param param;
        param.number = i + 1;
        param.fromByte = mask.byteNum;
        param.toByte = mask.byteNum + wordvalue::byteCountForWordType(mask.wordType) - 1;
        param.wordType = mask.wordType;
        param.name = mask.parameterName;
        param.mask = mask.parameterMask;
        const QPair<int, int> bits = maskBitRange(mask.parameterMask);
        param.bitFrom = bits.first;
        param.bitTo = bits.second;
        param.outsidePacket = d.sizeFromProfile && (param.toByte >= d.packetSize);
        d.params.append(param);
        if (mask.parameterMask.size() != maskmath::wordBits(mask.wordType)) {
            d.notes << DiagramText::tr("P%1 (%2): the mask has %3 characters while the word is %4 - check the word size")
                           .arg(param.number).arg(param.name)
                           .arg(mask.parameterMask.size()).arg(wordText(mask.wordType));
        }
        if (mask.byteNum == 0) {
            d.notes << DiagramText::tr("P%1 (%2) is on byte 0, which the app does not process")
                           .arg(param.number).arg(param.name);
        }
        lastUsed = qMax(lastUsed, param.toByte);
    }

    if (d.sizeFromProfile) {
        d.byteCount = d.packetSize;
        d.crcByte = d.packetSize - 1;
    }
    else {
        d.byteCount = lastUsed + 1;
        d.notes << DiagramText::tr("packet size is not set in the profile (0): the diagram bounds come from masks and marker/id positions");
    }

    d.token = QVector<QString>(d.byteCount);
    for (int i = 0; i < d.markerSize && i < d.byteCount; i++) {
        d.token[i] = (d.markerSize == 1) ? QStringLiteral("Mk") : QString("M%1").arg(i + 1);
    }
    if (d.idPos >= 0 && d.idPos < d.byteCount) {
        d.token[d.idPos] = QStringLiteral("ID");
    }
    if (d.crcByte >= 0) {
        d.token[d.crcByte] = QStringLiteral("CR");
    }

    for (const Param &param : d.params) {
        const int from = qMax(0, param.fromByte);
        const int to = qMin(d.byteCount - 1, param.toByte);
        if (param.fromByte < 0 || from > to) {
            d.notes << DiagramText::tr("P%1 (%2) is outside the diagram").arg(param.number).arg(param.name);
            continue;
        }
        if (param.outsidePacket) {
            d.notes << DiagramText::tr("P%1 (%2) occupies %3 while the packet is %4 bytes - part of the word is outside the packet")
                           .arg(param.number).arg(param.name).arg(bytesText(param)).arg(d.packetSize);
        }
        for (int b = from; b <= to; b++) {
            if (d.token[b].isEmpty()) {
                d.token[b] = QString("P%1").arg(param.number);
            }
            else {
                d.conflicts.insert(b);
                d.notes << DiagramText::tr("byte %1: P%2 (%3) overlaps with %4")
                               .arg(b).arg(param.number).arg(param.name).arg(d.token.at(b));
            }
        }
    }
    d.notes.removeDuplicates();
    return d;
}

QString header(const Diagram &d)
{
    QString s = DiagramText::tr("Packet format") + "\n";
    if (d.sizeFromProfile) {
        s += DiagramText::tr("Packet %1 bytes").arg(d.packetSize);
    }
    else {
        s += DiagramText::tr("Packet: size is not set, %1 bytes shown").arg(d.byteCount);
    }
    if (d.markerSize > 0) {
        const QString place = (d.markerSize == 1) ? DiagramText::tr("byte 0")
                                                  : DiagramText::tr("bytes 0-%1").arg(d.markerSize - 1);
        s += " · " + DiagramText::tr("marker %1 (%2)").arg(d.markerText).arg(place);
    }
    else {
        s += " · " + DiagramText::tr("no marker (checksum only)");
    }
    if (d.idPos >= 0) {
        s += " · " + DiagramText::tr("device id: byte %1").arg(d.idPos);
    }
    if (d.crcByte >= 0) {
        s += " · " + DiagramText::tr("checksum: byte %1 (sum of bytes %2..%3)")
                        .arg(d.crcByte).arg(d.crcFrom).arg(d.crcByte - 1);
    }
    s += "\n" + DiagramText::tr("Words are little-endian: the first byte of a word is the least significant") + "\n";
    return s;
}

QString legend(const Diagram &d)
{
    QString s = "\n" + DiagramText::tr("Legend:") + "\n";
    if (d.markerSize > 0) {
        QString note;
        if (d.markerSize > 1) {
            note = QStringLiteral(": ") + DiagramText::tr("M1 is the first byte, M2 is the second");
        }
        s += "  " + DiagramText::tr("Mk/M1,M2") + QString(2, ' ')
             + DiagramText::tr("start-of-frame marker (%1)%2").arg(d.markerText).arg(note) + "\n";
    }
    if (d.idPos >= 0) {
        s += "  ID   " + DiagramText::tr("device id") + "\n";
    }
    if (d.crcByte >= 0) {
        s += "  CR   " + DiagramText::tr("checksum (last byte, sum of bytes %1..%2)")
                            .arg(d.crcFrom).arg(d.crcByte - 1) + "\n";
    }
    for (const Param &param : d.params) {
        QString line = QString("  P%1 ").arg(param.number);
        line += QString(" %1").arg(param.name, -22);
        line += QString(" %1").arg(bytesText(param), -14);
        line += QString(" %1").arg(wordText(param.wordType), -8);
        line += QString(" %1").arg(DiagramText::tr("mask %1").arg(param.mask));
        line += QString("  (%1)").arg(bitsText(param));
        s += line + "\n";
    }
    if (!d.notes.isEmpty()) {
        s += "\n" + DiagramText::tr("Notes:") + "\n";
        for (const QString &note : d.notes) {
            s += "  - " + note + "\n";
        }
    }
    return s;
}

} // namespace

QString packetdiagram::picture(const s_protocolDescription &protocol, const QVector<s_parameterMask> &masks)
{
    const Diagram d = build(protocol, masks);
    const int perRow = 16;
    const int cell = 5;

    QString s = header(d);
    for (int rowStart = 0; rowStart < d.byteCount; rowStart += perRow) {
        const int rowEnd = qMin(rowStart + perRow, d.byteCount) - 1;
        QString indexes;
        QString border;
        QString line;
        for (int b = rowStart; b <= rowEnd; b++) {
            indexes += QString::number(b).rightJustified(cell - 1) + " ";
            border += "+----";
            const QString token = d.token.at(b);
            QString field = token.isEmpty() ? QStringLiteral("··") : token;
            if (d.conflicts.contains(b)) {
                field += "*";
            }
            line += "|" + field.leftJustified(cell - 1);
        }
        indexes += " ";
        border += "+";
        line += "|";
        s += indexes + "\n" + border + "\n" + line + "\n" + border + "\n";
    }
    return s + legend(d);
}
