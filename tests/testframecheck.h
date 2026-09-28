#ifndef TESTFRAMECHECK_H
#define TESTFRAMECHECK_H

#include <QObject>

//Тесты проверок кадра: контрольная сумма и маркер протокола
class TestFrameCheck : public QObject
{
    Q_OBJECT

private slots:
    void checksumSkipsLastByte();
    void checksumStartsAtConfiguredPosition();
    void checksumWrapsAroundOneByte();
    void singleByteFrameHasZeroChecksum();
    void checksumMatchesLastByte();
    void emptyFrameIsNeverValid();
    void markerIsOptional();
    void markerOfOneByte();
    void markerOfTwoBytes();
    void unknownMarkerSizeIsInvalid();
    void defaultProtocolFrameIsAccepted();
};

#endif // TESTFRAMECHECK_H
