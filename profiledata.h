#ifndef PROFILEDATA_H
#define PROFILEDATA_H

#include <QList>
#include <QString>
#include <QVector>
#include "global.h"

//Разбор и запись профиля .eag: текстовый формат, поля через табуляцию.
//Функции чистые - не зависят от интерфейса и файловой системы, поэтому их можно тестировать отдельно.
namespace profiledata {

//Полей в строке маски (id, номер устройства, номер байта, имена, маска, коэффициенты, флаги)
constexpr int kMaskFieldCount = 14;

//Первое поле строки маски: по нему строка маски отличается от шапки профиля
inline QString maskTag() { return QStringLiteral("thisIsMask"); }

//Разобранное содержимое профиля
struct ProfileText
{
    s_protocolDescription protocol;
    s_Settings settings;
    bool hasSettings = false;   //в профиле были настройки связи
    QVector<s_parameterMask> masks;
};

//Разбирает текст профиля. Пустые, неполные и неизвестные строки пропускаются.
ProfileText parse(const QString &text);

//Собирает шапку профиля: имя файла, протокол и настройки связи.
//COM-порт не сохраняется: он всегда выбирается вручную из найденных устройств
QString serializeHeader(const QString &profileFileName, const s_protocolDescription &protocol,
                        const s_Settings &settings);

//Собирает строку маски из её полей (в таком виде их хранит txtmaskobj)
QString serializeMaskLine(const QList<QString> &maskFields);

}

#endif // PROFILEDATA_H
