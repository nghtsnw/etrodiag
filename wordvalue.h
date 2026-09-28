#ifndef WORDVALUE_H
#define WORDVALUE_H

#include <QVector>
#include <QtGlobal>

//Сборка значения слова (8/16/32 бита) из байтов пакета.
namespace wordvalue {

//Сколько байтов занимает слово: 0 - 8 бит, 1 - 16 бит, 2 - 32 бита, другое - 0 байт
int byteCountForWordType(int wordType);

//Собирает слово младшим байтом вперёд, начиная с байта byteNum.
//Если данных не хватает или wordType неизвестен - возвращает 0
uint32_t assemble(const QVector<int> &data, int byteNum, int wordType);

}

#endif // WORDVALUE_H
