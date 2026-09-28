#include "wordvalue.h"

namespace wordvalue {

int byteCountForWordType(int wordType)
{
    switch (wordType) {
    case 0:
        return 1; //8 бит
    case 1:
        return 2; //16 бит
    case 2:
        return 4; //32 бита
    default:
        return 0;
    }
}

uint32_t assemble(const QVector<int> &data, int byteNum, int wordType)
{
    const int byteCount = byteCountForWordType(wordType);
    if (byteCount == 0 || byteNum < 0 || byteNum + byteCount > data.size()) {
        //После смены профиля пакет может быть короче требуемого слова
        return 0;
    }
    uint32_t word = 0;
    for (int i = 0; i < byteCount; i++) {
        word |= static_cast<uint32_t>(static_cast<uint8_t>(data.at(byteNum + i))) << (8 * i);
    }
    return word;
}

}
