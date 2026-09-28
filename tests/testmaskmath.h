#ifndef TESTMASKMATH_H
#define TESTMASKMATH_H

#include <QObject>

//Тесты преобразований маски параметра
class TestMaskMath : public QObject
{
    Q_OBJECT

private slots:
    void wordBitsPerWordType();
    void maskToIntReadsRightmostCharAsLowBit();
    void maskToIntIgnoresBitsAboveWord();
    void maskToIntMatchesLegacyLoop();
    void parameterShiftIsIndexOfFirstSetBit();
    void parameterShiftIsZeroWhenMaskHasNoBits();
    void parameterShiftIgnoresBitsOutsideWordType();
    void parameterShiftWorksForMasksWiderThanInt();
    void parameterShiftKeepsLegacyResultForNarrowMasks();
};

#endif // TESTMASKMATH_H
