#include "testmaskmath.h"
#include <QtTest>
#include <cmath>
#include "maskmath.h"

//Прежняя реализация (биты через pow) - эталон для сверки
static uint32_t legacyMaskToInt(const QString &mask)
{
    uint32_t value = 0;
    for (int i = mask.size() - 1, y = 0; i > -1; i--, y++) {
        if (mask.at(i) == '1') {
            value += static_cast<uint32_t>(pow(2, y));
        }
    }
    return value;
}

//Прежняя реализация поиска сдвига (цикл с остановкой по границе слова)
static int legacyParameterShift(const QString &mask, int wordType)
{
    int wordTypeInt = 32;
    switch (wordType) {
    case 0:
        wordTypeInt = 8;
        break;
    case 1:
        wordTypeInt = 16;
        break;
    case 2:
        wordTypeInt = 32;
        break;
    default:
        break;
    }
    int value = mask.toInt(0, 10);
    int shift = 0;
    int n = 0;
    bool stopFlag = false;
    while (!stopFlag) {
        if (!(value & 0x01)) {
            value = value >> 1;
            n++;
        }
        else if (value & 0x01) {
            shift = n;
            stopFlag = true;
        }
        if (n > wordTypeInt) {
            shift = 0;
            stopFlag = true;
        }
    }
    return shift;
}

void TestMaskMath::wordBitsPerWordType()
{
    QCOMPARE(maskmath::wordBits(0), 8);
    QCOMPARE(maskmath::wordBits(1), 16);
    QCOMPARE(maskmath::wordBits(2), 32);
    QCOMPARE(maskmath::wordBits(3), 32); //неизвестный тип - защита от зацикливания
}

void TestMaskMath::maskToIntReadsRightmostCharAsLowBit()
{
    QCOMPARE(maskmath::maskToInt(QStringLiteral("00000001")), 1u);
    QCOMPARE(maskmath::maskToInt(QStringLiteral("10000000")), 128u);
    QCOMPARE(maskmath::maskToInt(QStringLiteral("00001111")), 15u);
    QCOMPARE(maskmath::maskToInt(QStringLiteral("0000000011111111")), 0xFFu);
    QCOMPARE(maskmath::maskToInt(QStringLiteral("1111111111111111")), 0xFFFFu);
    QCOMPARE(maskmath::maskToInt(QStringLiteral("0000")), 0u);
    QCOMPARE(maskmath::maskToInt(QString()), 0u);
}

void TestMaskMath::maskToIntIgnoresBitsAboveWord()
{
    const QString mask(40, QLatin1Char('1')); //в слове больше 32 битов не бывает
    QCOMPARE(maskmath::maskToInt(mask), 0xFFFFFFFFu);
}

void TestMaskMath::maskToIntMatchesLegacyLoop()
{
    const QStringList masks{
        QStringLiteral("0"), QStringLiteral("1"), QStringLiteral("00000000"),
        QStringLiteral("11111111"), QStringLiteral("10101010"), QStringLiteral("10000000"),
        QStringLiteral("1111"), QStringLiteral("1111111111111111"),
        QStringLiteral("0000000000000001"), QStringLiteral("11111111111111110000000000000000"),
        QStringLiteral("10101010101010101010101010101010"),
        QStringLiteral("11111111111111111111111111111111")
    };
    for (const QString &mask : masks) {
        QCOMPARE(maskmath::maskToInt(mask), legacyMaskToInt(mask));
    }
}

void TestMaskMath::parameterShiftIsIndexOfFirstSetBit()
{
    QCOMPARE(maskmath::parameterShift(QStringLiteral("11111111"), 0), 0);
    QCOMPARE(maskmath::parameterShift(QStringLiteral("11110000"), 0), 4);
    QCOMPARE(maskmath::parameterShift(QStringLiteral("10000000"), 0), 7);
    QCOMPARE(maskmath::parameterShift(QStringLiteral("0000000000000001"), 1), 0);
    QCOMPARE(maskmath::parameterShift(QStringLiteral("1111111111110000"), 1), 4);
    QCOMPARE(maskmath::parameterShift(QStringLiteral("1111000000000000"), 1), 12);
    QCOMPARE(maskmath::parameterShift(QStringLiteral("11111111111111110000000000000000"), 2), 16);
    QCOMPARE(maskmath::parameterShift(QStringLiteral("10000000000000000000000000000000"), 2), 31);
}

void TestMaskMath::parameterShiftIsZeroWhenMaskHasNoBits()
{
    QCOMPARE(maskmath::parameterShift(QStringLiteral("00000000"), 0), 0);
    QCOMPARE(maskmath::parameterShift(QString(), 0), 0);
    QCOMPARE(maskmath::parameterShift(QStringLiteral("0000"), 2), 0);
}

void TestMaskMath::parameterShiftIgnoresBitsOutsideWordType()
{
    //единица за пределами слова даёт сдвиг 0, как в прежнем коде
    QCOMPARE(maskmath::parameterShift(QStringLiteral("1000000000000"), 0), 0);
    QCOMPARE(maskmath::parameterShift(QStringLiteral("1000000"), 0), 6); //в пределах 8 бит
}

void TestMaskMath::parameterShiftWorksForMasksWiderThanInt()
{
    //16- и 32-разрядные маски в int не влезают: раньше сдвиг для них всегда
    //получался нулевым, и значения снимались без сдвига
    QCOMPARE(QStringLiteral("1111111111110000").toInt(0, 10), 0); //toInt переполняется
    QCOMPARE(maskmath::parameterShift(QStringLiteral("1111111111110000"), 1), 4);
    QCOMPARE(maskmath::parameterShift(QStringLiteral("11111111111111110000000000000000"), 2), 16);
}

void TestMaskMath::parameterShiftKeepsLegacyResultForNarrowMasks()
{
    //Узкие маски должны считаться так же, как прежним алгоритмом
    const QStringList masks{
        QStringLiteral("0"), QStringLiteral("1"), QStringLiteral("0000"),
        QStringLiteral("00000000"), QStringLiteral("11111111"), QStringLiteral("11110000"),
        QStringLiteral("10000000"), QStringLiteral("00000001"), QStringLiteral("10101010"),
        QStringLiteral("1000000"), QStringLiteral("111111111")
    };
    for (const QString &mask : masks) {
        bool ok = false;
        mask.toInt(&ok, 10);
        QVERIFY(ok); //предусловие: прежний алгоритм на этой маске не переполнялся
        for (int wordType = 0; wordType <= 3; wordType++) {
            QCOMPARE(maskmath::parameterShift(mask, wordType),
                     legacyParameterShift(mask, wordType));
        }
    }
}
