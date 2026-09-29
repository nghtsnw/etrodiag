#ifndef TESTDATAPROFILER_H
#define TESTDATAPROFILER_H

#include <QObject>

//Тесты разбора кадров: протокол и настройки модуль берёт из модели
class TestDataProfiler : public QObject
{
    Q_OBJECT

private slots:
    void acceptsValidFrame();
    void rejectsBadChecksum();
    void ignoresFrameWithWrongMarker();
    void resyncsAfterGarbageByte();
    void keepsLogTimestampWhenReadingFromFile();
    void doesNothingWithoutModel();
    void acceptsMarkerlessNineByteFrame();
};

#endif // TESTDATAPROFILER_H
