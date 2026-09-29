#ifndef PACKETDIAGRAM_H
#define PACKETDIAGRAM_H

#include <QString>
#include <QVector>
#include "global.h"

//Схема формата пакета текстом (ASCII-картинка) для панели информации о профиле.
//Модуль чистый: на вход идут протокол и список масок, на выходе - готовый текст.
namespace packetdiagram {

//Собирает схему одного набора масок (одного устройства): линейка байтов, маркер,
//id устройства, контрольная сумма и прямоугольники параметров с расшифровкой
QString picture(const s_protocolDescription &protocol, const QVector<s_parameterMask> &masks);

//Выделение обозначения параметра в тексте схемы: где оно находится и каким цветом
struct ParamColor
{
    int position = 0;   //смещение обозначения в тексте схемы
    int length = 0;     //длина обозначения (P1, P10, ...)
    QString background; //цвет графика из настроек маски
    QString foreground; //цвет шрифта, читаемый на этом фоне
};

//Готовая схема: текст и обозначения параметров, которые нужно выделить цветом
struct DiagramPicture
{
    QString text;
    QVector<ParamColor> colors;
};

//Схема всех устройств профиля: общий заголовок протокола и отдельный раздел на каждое
//устройство, потому что состав параметров у устройств свой. Маски одного устройства
//считаются вместе, нумерация P1..Pn идёт внутри раздела
DiagramPicture pictureDevices(const s_protocolDescription &protocol, const QVector<s_parameterMask> &masks);

//Цвет шрифта для фона: светлый на тёмном фоне, тёмный на светлом.
//Пустая строка, если цвет фона не распознан.
QString contrastTextColor(const QString &background);

}

#endif // PACKETDIAGRAM_H
