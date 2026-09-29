#ifndef TESTPACKETDIAGRAM_H
#define TESTPACKETDIAGRAM_H

#include <QObject>

//Тесты схемы формата пакета (ASCII-картинка для панели информации о профиле)
class TestPacketDiagram : public QObject
{
    Q_OBJECT

private slots:
    void headerDescribesPacketAndStructuralBytes();
    void rowLayoutKeepsByteColumns();
    void emptyBytesAreMarked();
    void thirtyTwoBitWordSpansFourBytes();
    void twoByteMarkerAndIdAreShown();
    void noMarkerIsReported();
    void packetSizeZeroFallsBackToMasks();
    void maskBeyondPacketIsReported();
    void overlappingMasksAreReportedAndMarked();
    void wordSizeMismatchIsReported();
    void byteZeroMaskIsReported();
    void legendShowsMaskBits();
    void graphedParamGetsGraphColor();
    void ungraphedParamHasNoColor();
    void maskWithoutBitsIsShown();
    void deviceWithEmptyMasksGetsItsSection();
    void devicesGetTheirOwnSection();
    void bitFlagsOnOneByteAreNotOverlaps();
    void paramColorsFollowPacketOrder();
    void textColorContrastsWithBackground();
};

#endif // TESTPACKETDIAGRAM_H
