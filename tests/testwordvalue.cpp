#include "testwordvalue.h"
#include <QtTest>
#include <cmath>
#include "wordvalue.h"

//Прежняя реализация (разбор битов через маску и pow) - эталон для сверки
static uint32_t legacyAssemble(const QVector<int> &data, int byteNum, int wordType)
{
    uint32_t word = 0;
    int step = 0;
    if (wordType == 0) {
        if (byteNum < data.size()) {
            word = static_cast<uint32_t>(data.at(byteNum));
        }
    }
    else if (wordType == 1) {
        if (byteNum + 1 < data.size()) {
            for (int y = 0; y <= 1; y++) {
                const int bytex = data.at(byteNum + y);
                for (int i = 0, mask = 1; i <= 7; i++, step++, mask = mask << 1) {
                    if (bytex & mask) {
                        word += static_cast<uint32_t>(pow(2, step));
                    }
                }
            }
        }
    }
    else if (wordType == 2) {
        if (byteNum + 3 < data.size()) {
            for (int y = 0; y <= 3; y++) {
                const int bytex = data.at(byteNum + y);
                for (int i = 0, mask = 1; i <= 7; i++, step++, mask = mask << 1) {
                    if (bytex & mask) {
                        word += static_cast<uint32_t>(pow(2, step));
                    }
                }
            }
        }
    }
    return word;
}

void TestWordValue::byteCountsPerWordType()
{
    QCOMPARE(wordvalue::byteCountForWordType(0), 1);
    QCOMPARE(wordvalue::byteCountForWordType(1), 2);
    QCOMPARE(wordvalue::byteCountForWordType(2), 4);
    QCOMPARE(wordvalue::byteCountForWordType(3), 0);
}

void TestWordValue::eightBitWordIsSingleByte()
{
    const QVector<int> data{0x12, 0x34};
    QCOMPARE(wordvalue::assemble(data, 1, 0), 0x34u);
    QCOMPARE(wordvalue::assemble(data, 0, 0), 0x12u);
}

void TestWordValue::sixteenBitWordIsLittleEndian()
{
    const QVector<int> data{0x34, 0x12, 0xFF};
    QCOMPARE(wordvalue::assemble(data, 0, 1), 0x1234u);
    QCOMPARE(wordvalue::assemble(data, 1, 1), 0xFF12u);
}

void TestWordValue::thirtyTwoBitWordIsLittleEndian()
{
    const QVector<int> data{0x78, 0x56, 0x34, 0x12};
    QCOMPARE(wordvalue::assemble(data, 0, 2), 0x12345678u);
}

void TestWordValue::missingDataGivesZero()
{
    const QVector<int> data{0x01, 0x02, 0x03};
    QCOMPARE(wordvalue::assemble(data, 0, 2), 0u); //для 32 бит нужно 4 байта
    QCOMPARE(wordvalue::assemble(data, 2, 1), 0u); //для 16 бит не хватает байта
    QCOMPARE(wordvalue::assemble(data, 3, 0), 0u); //такого байта нет
    QCOMPARE(wordvalue::assemble(data, -1, 0), 0u);
    QCOMPARE(wordvalue::assemble(QVector<int>{}, 0, 0), 0u);
}

void TestWordValue::unknownWordTypeGivesZero()
{
    const QVector<int> data{0x01, 0x02, 0x03, 0x04};
    QCOMPARE(wordvalue::assemble(data, 0, 3), 0u);
    QCOMPARE(wordvalue::assemble(data, 0, -1), 0u);
}

void TestWordValue::matchesLegacyBitByBitAssembly()
{
    const QVector<int> data{0x12, 0x34, 0x56, 0x78, 0x9A};
    for (int byteNum = 0; byteNum + 4 <= data.size(); byteNum++) {
        for (int wordType = 0; wordType <= 2; wordType++) {
            QCOMPARE(wordvalue::assemble(data, byteNum, wordType),
                     legacyAssemble(data, byteNum, wordType));
        }
    }
}
