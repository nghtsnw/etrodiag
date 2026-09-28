#ifndef DATAPROFILER_H
#define DATAPROFILER_H
#include <QVector>
#include <QObject>
#include <QMainWindow>
#include "global.h"
#include "qdatetime.h"

class ProtocolSettings;

class dataprofiler : public QObject
{
    Q_OBJECT
public:
    explicit dataprofiler(QWidget *parent = nullptr);
    void setModel(const ProtocolSettings *model); //протокол и настройки читаем из модели, своих копий не держим

private:
    QVector<int> frameMsg;
    void handleFullFrame(bool marker);  //обработка собранного кадра
    uint8_t calculatedCRC;
    const ProtocolSettings *m_model = nullptr;
    QDateTime currentTime;

signals:
    void deviceData(QDateTime currentTime, QVector<int> snapshot);
    void badCRC(uint8_t calculatedCRC, QVector<int> snapshot);
    void readNext();
    void ready4read(bool);
    void setTime(QDateTime t);

public slots:
    void getByte(int byteFromBuf);
};

#endif // DATAPROFILER_H
