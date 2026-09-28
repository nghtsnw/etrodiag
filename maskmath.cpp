#include "maskmath.h"

namespace maskmath {

int wordBits(int wordType)
{
    switch (wordType) {
    case 0:
        return 8;
    case 1:
        return 16;
    case 2:
        return 32;
    default:
        return 32; //неизвестный тип - берём максимум, значение влияет только на защиту от зацикливания
    }
}

uint32_t maskToInt(const QString &mask)
{
    uint32_t value = 0;
    for (int i = mask.size() - 1, bit = 0; i > -1; i--, bit++) {
        if (bit >= 32) { //в слове больше 32 битов не бывает
            break;
        }
        if (mask.at(i) == '1') {
            value |= (uint32_t(1) << bit);
        }
    }
    return value;
}

namespace {

//Индекс младшего единичного бита значения (0 - если единиц нет)
int lowestSetBitIndex(uint32_t value)
{
    int index = 0;
    while (value != 0 && (value & 0x01) == 0) {
        value = value >> 1;
        index++;
    }
    return index;
}

}

int parameterShift(const QString &mask, int wordType)
{
    const uint32_t value = maskToInt(mask);
    if (value == 0) {
        return 0; //в маске нет единиц - сдвиг не определён
    }
    const int lowestBit = lowestSetBitIndex(value);
    //Единица за пределами слова - маска не соответствует типу, сдвиг не применяем
    return (lowestBit <= wordBits(wordType)) ? lowestBit : 0;
}

}
