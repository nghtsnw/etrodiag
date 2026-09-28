#ifndef PACKETDIAGRAM_H
#define PACKETDIAGRAM_H

#include <QString>
#include <QVector>
#include "global.h"

//Схема формата пакета текстом (ASCII-картинка) для панели информации о профиле.
//Модуль чистый: на вход идут протокол и список масок, на выходе - готовый текст.
namespace packetdiagram {

//Собирает схему: линейка байтов, маркер, id устройства, контрольная сумма и
//прямоугольники параметров с расшифровкой (байты, размер слова, биты маски, цвет графика)
QString picture(const s_protocolDescription &protocol, const QVector<s_parameterMask> &masks);

}

#endif // PACKETDIAGRAM_H
