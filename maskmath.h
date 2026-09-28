#ifndef MASKMATH_H
#define MASKMATH_H

#include <QString>
#include <QtGlobal>

//Преобразования маски параметра. Маска хранится строкой из '0' и '1'.
namespace maskmath {

//Сколько битов в слове: 0 - 8, 1 - 16, 2 - 32, другое - 32 (защита от зацикливания)
int wordBits(int wordType);

//Число из строки маски: правый символ строки - младший бит
uint32_t maskToInt(const QString &mask);

//Сдвиг параметра - индекс младшего единичного бита маски, 0 если единиц нет.
//Маска-строка читается как двоичное число; единица за пределами слова (испорченная
//маска) сдвига не даёт, чтобы не портить значение
int parameterShift(const QString &mask, int wordType);

}

#endif // MASKMATH_H
