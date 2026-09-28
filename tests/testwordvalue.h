#ifndef TESTWORDVALUE_H
#define TESTWORDVALUE_H

#include <QObject>

//Тесты сборки слова (8/16/32 бита) из байтов пакета
class TestWordValue : public QObject
{
    Q_OBJECT

private slots:
    void byteCountsPerWordType();
    void eightBitWordIsSingleByte();
    void sixteenBitWordIsLittleEndian();
    void thirtyTwoBitWordIsLittleEndian();
    void missingDataGivesZero();
    void unknownWordTypeGivesZero();
    void matchesLegacyBitByBitAssembly();
};

#endif // TESTWORDVALUE_H
