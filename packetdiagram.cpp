#include "packetdiagram.h"
#include <QColor>
#include <QCoreApplication>
#include <QHash>
#include <QMap>
#include <QPair>
#include <QSet>
#include <QStringList>
#include <algorithm>
#include <cmath>
#include <utility>
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
    bool drawGraph = false;
    QString graphColor;
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
    QVector<int> tokenParam; //номер параметра-хозяина байта (0 - байт занят маркером, id или CRC)
    QVector<Param> params;
    QStringList notes;
    QSet<int> conflicts;  //байты, где параметры делят одни и те же биты
    QSet<int> moreParams; //байты, в которых есть и другие параметры
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
    for (const s_parameterMask &mask : std::as_const(ordered)) {
        Param param;
        param.number = d.params.size() + 1;
        param.fromByte = mask.byteNum;
        param.toByte = mask.byteNum + wordvalue::byteCountForWordType(mask.wordType) - 1;
        param.wordType = mask.wordType;
        param.name = mask.parameterName;
        param.mask = mask.parameterMask;
        param.drawGraph = mask.drawGraphFlag;
        param.graphColor = mask.drawGraphColor;
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
    d.tokenParam = QVector<int>(d.byteCount, 0);
    for (int i = 0; i < d.markerSize && i < d.byteCount; i++) {
        d.token[i] = (d.markerSize == 1) ? QStringLiteral("Mk") : QString("M%1").arg(i + 1);
    }
    if (d.idPos >= 0 && d.idPos < d.byteCount) {
        d.token[d.idPos] = QStringLiteral("ID");
    }
    if (d.crcByte >= 0) {
        d.token[d.crcByte] = QStringLiteral("CR");
    }

    //Байт принадлежит параметру целиком (слово читается из нескольких байтов), а пересечение
    //считается по битам: у параметров-флагов на одном байте разные биты - это не конфликт
    QVector<quint32> usedBits(d.byteCount, 0); //занятые биты байта, бит 0 - младший
    QVector<int> covered(d.byteCount, 0);      //сколько параметров покрывает байт
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
            covered[b]++;
            if (d.tokenParam.at(b) != 0) {
                continue; //байт уже занят другим параметром
            }
            if (d.token.at(b).isEmpty()) {
                d.token[b] = QString("P%1").arg(param.number);
                d.tokenParam[b] = param.number;
            }
            else { //байт занят маркером, id или контрольной суммой
                d.conflicts.insert(b);
                d.notes << DiagramText::tr("byte %1: P%2 (%3) overlaps with %4")
                               .arg(b).arg(param.number).arg(param.name).arg(d.token.at(b));
            }
        }
        const int wordBits = maskmath::wordBits(param.wordType);
        for (int bit = param.bitFrom; bit >= 0 && bit < wordBits && bit < param.mask.size(); bit++) {
            if (param.mask.at(param.mask.size() - 1 - bit) != '1') {
                continue; //бит не входит в маску
            }
            const int b = param.fromByte + bit / 8;
            if (b < 0 || b >= d.byteCount) {
                continue; //байт за пределами пакета
            }
            const quint32 bitInByte = quint32(1) << (bit % 8);
            if (usedBits.at(b) & bitInByte) { //бит уже занят - настоящее пересечение
                d.conflicts.insert(b);
                d.notes << DiagramText::tr("byte %1: P%2 (%3) overlaps with %4")
                               .arg(b).arg(param.number).arg(param.name).arg(d.token.at(b));
            }
            usedBits[b] |= bitInByte;
        }
    }
    for (int b = 0; b < d.byteCount; b++) {
        if (!d.conflicts.contains(b) && covered.at(b) > 1) {
            d.moreParams.insert(b); //биты разные, но байт делят несколько параметров
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

//Ширина колонки имён в легенде - по самому длинному имени, чтобы столбцы не съезжали
int namesWidth(const QVector<Param> &params)
{
    int width = 8;
    for (const Param &param : params) {
        width = qMax(width, param.name.size());
    }
    return width;
}

//Перевод канала sRGB в линейный вид - по нему считается относительная яркость
double linearChannel(double value)
{
    return (value <= 0.03928) ? value / 12.92 : std::pow((value + 0.055) / 1.055, 2.4);
}

//Цвет выделения обозначения параметра в тексте схемы
struct HighlightColor
{
    QString background;
    QString foreground;
};

//Цвета выделения обозначений: номер параметра -> цвет графика из настроек маски и цвет шрифта
QHash<int, HighlightColor> highlightColors(const Diagram &d)
{
    QHash<int, HighlightColor> colors;
    for (const Param &param : d.params) {
        if (!param.drawGraph || param.graphColor.isEmpty()) {
            continue; //график у параметра выключен - выделять нечем
        }
        const QString foreground = packetdiagram::contrastTextColor(param.graphColor);
        if (foreground.isEmpty()) {
            continue; //цвет из настроек маски не распознан
        }
        colors.insert(param.number, HighlightColor{param.graphColor, foreground});
    }
    return colors;
}

//Сборка текста схемы: попутно запоминаются места обозначений параметров для подсветки
class PictureBuilder
{
public:
    void add(const QString &text) { m_text += text; }
    const QString &text() const { return m_text; }
    const QVector<packetdiagram::ParamColor> &colors() const { return m_colors; }

    //Обозначение параметра - цветом выделяем только его, а не весь байт схемы
    void addParamToken(int number, const QString &token, const QHash<int, HighlightColor> &colors)
    {
        const int position = m_text.size();
        m_text += token;
        if (number == 0 || !colors.contains(number)) {
            return;
        }
        const HighlightColor color = colors.value(number);
        packetdiagram::ParamColor highlight;
        highlight.position = position;
        highlight.length = token.size();
        highlight.background = color.background;
        highlight.foreground = color.foreground;
        m_colors.append(highlight);
    }

private:
    QString m_text;
    QVector<packetdiagram::ParamColor> m_colors;
};

constexpr int kRowBytes = 16; //байтов в одной строке схемы
constexpr int kCellWidth = 5; //ширина колонки байта вместе с разделителем

//Строки байтов: линейка индексов, граница, данные, граница
void appendRows(PictureBuilder &out, const Diagram &d, const QHash<int, HighlightColor> &colors)
{
    for (int rowStart = 0; rowStart < d.byteCount; rowStart += kRowBytes) {
        const int rowEnd = qMin(rowStart + kRowBytes, d.byteCount) - 1;
        QString indexes;
        QString border;
        for (int b = rowStart; b <= rowEnd; b++) {
            indexes += QString::number(b).rightJustified(kCellWidth - 1) + " ";
            border += "+----";
        }
        out.add(indexes + " \n" + border + "+\n");
        for (int b = rowStart; b <= rowEnd; b++) {
            const QString token = d.token.at(b);
            QString suffix;
            if (d.conflicts.contains(b)) {
                suffix += '*'; //параметры делят одни и те же биты этого байта
            }
            else if (d.moreParams.contains(b)) {
                suffix += '+'; //биты разные, но байт делят несколько параметров
            }
            const QString field = (token.isEmpty() ? QStringLiteral("··") : token) + suffix;
            out.add("|");
            if (token.isEmpty()) {
                out.add(QStringLiteral("··"));
            }
            else {
                out.addParamToken(d.tokenParam.at(b), token, colors);
            }
            out.add(suffix + QString(qMax(0, kCellWidth - 1 - field.size()), ' '));
        }
        out.add("|\n" + border + "+\n");
    }
}

//Легенда: что означают обозначения байтов и параметров, плюс примечания по профилю
void appendLegend(PictureBuilder &out, const Diagram &d, const QHash<int, HighlightColor> &colors)
{
    out.add("\n" + DiagramText::tr("Legend:") + "\n");
    if (d.markerSize > 0) {
        QString note;
        if (d.markerSize > 1) {
            note = QStringLiteral(": ") + DiagramText::tr("M1 is the first byte, M2 is the second");
        }
        out.add("  " + DiagramText::tr("Mk/M1,M2") + QString(2, ' ')
                + DiagramText::tr("start-of-frame marker (%1)%2").arg(d.markerText).arg(note) + "\n");
    }
    if (d.idPos >= 0) {
        out.add("  ID   " + DiagramText::tr("device id") + "\n");
    }
    if (d.crcByte >= 0) {
        out.add("  CR   " + DiagramText::tr("checksum (last byte, sum of bytes %1..%2)")
                            .arg(d.crcFrom).arg(d.crcByte - 1) + "\n");
    }
    if (!d.moreParams.isEmpty()) {
        out.add("  Pn+  " + DiagramText::tr("the byte is shared with other parameters of the device") + "\n");
    }
    if (!d.conflicts.isEmpty()) {
        out.add("  Pn*  " + DiagramText::tr("parameters claim the same bits of the byte") + "\n");
    }
    const int nameWidth = namesWidth(d.params);
    for (const Param &param : d.params) {
        out.add("  ");
        out.addParamToken(param.number, QString("P%1").arg(param.number), colors);
        out.add("  " + QString("%1").arg(param.name, -nameWidth)
                + " " + QString("%1").arg(bytesText(param), -14)
                + " " + QString("%1").arg(wordText(param.wordType), -8)
                + " " + DiagramText::tr("mask %1").arg(param.mask)
                + "  (" + bitsText(param) + ")\n");
    }
    if (!d.notes.isEmpty()) {
        out.add("\n" + DiagramText::tr("Notes:") + "\n");
        for (const QString &note : d.notes) {
            out.add("  - " + note + "\n");
        }
    }
}

//Схема одного набора масок (одного устройства): строки байтов и легенда
void appendDiagram(PictureBuilder &out, const Diagram &d)
{
    const QHash<int, HighlightColor> colors = highlightColors(d);
    appendRows(out, d, colors);
    appendLegend(out, d, colors);
}

//Имя устройства из его масок (в маски попадает имя кнопки устройства)
QString deviceName(const QVector<s_parameterMask> &masks)
{
    for (const s_parameterMask &mask : masks) {
        if (!mask.devName.isEmpty()) {
            return mask.devName;
        }
    }
    return QString();
}
} // namespace

QString packetdiagram::picture(const s_protocolDescription &protocol, const QVector<s_parameterMask> &masks)
{ //схема одного устройства: заголовок протокола, строки байтов и легенда
    const Diagram d = build(protocol, masks);
    PictureBuilder out;
    out.add(header(d));
    appendDiagram(out, d);
    return out.text();
}

packetdiagram::DiagramPicture packetdiagram::pictureDevices(const s_protocolDescription &protocol, const QVector<s_parameterMask> &masks)
{
    //Маски группируются по устройствам: у каждого устройства свой состав параметров, поэтому
    //в одной картинке параметры разных устройств наезжали бы друг на друга
    QMap<int, QVector<s_parameterMask>> byDevice;
    for (const s_parameterMask &mask : masks) {
        byDevice[mask.devNum].append(mask);
    }
    const Diagram whole = build(protocol, masks); //заголовок протокола общий для всех устройств
    PictureBuilder out;
    out.add(header(whole));
    if (byDevice.isEmpty()) {
        appendDiagram(out, whole); //масок нет - показываем только протокол
    }
    for (auto it = byDevice.constBegin(); it != byDevice.constEnd(); ++it) {
        const QString name = deviceName(it.value());
        out.add("\n" + DiagramText::tr("Device %1").arg(it.key())
                + (name.isEmpty() ? QString() : QStringLiteral(" (%1)").arg(name)) + "\n");
        appendDiagram(out, build(protocol, it.value())); //нумерация P1..Pn идёт внутри устройства
    }
    DiagramPicture result;
    result.text = out.text();
    result.colors = out.colors();
    return result;
}

QString packetdiagram::contrastTextColor(const QString &background)
{
    const QColor color(background);
    if (!color.isValid()) {
        return QString(); //цвет не распознан - выделять нечем
    }
    //Относительная яркость фона (0 - чёрный, 1 - белый)
    const double luminance = 0.2126 * linearChannel(color.redF())
                             + 0.7152 * linearChannel(color.greenF())
                             + 0.0722 * linearChannel(color.blueF());
    //Контраст с белым и с чёрным: берём тот вариант, где контраст выше
    const double contrastWithWhite = 1.05 / (luminance + 0.05);
    const double contrastWithBlack = (luminance + 0.05) / 0.05;
    return (contrastWithBlack >= contrastWithWhite) ? QStringLiteral("#000000") : QStringLiteral("#FFFFFF");
}
