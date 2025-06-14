
#ifndef DATAPROFILER_H
#define DATAPROFILER_H
#include <QQueue>
#include <QObject>
#include <QMainWindow>

class dataprofiler : public QObject
{
    Q_OBJECT
public:
    explicit dataprofiler(QWidget *parent = nullptr);

private:
    const int oneMsgLeight = 41;
    QQueue<int> frameMsg;
    QVector<int> snapshot;
    bool checkCRC(void);
    uint8_t calculatedCRC;

signals:
    void deviceData(QVector<int> snapshot);
    void badCRC(uint8_t calculatedCRC, QVector<int> snapshot);
    void readNext();
    void ready4read(bool);

public slots:
    void getByte(int byteFromBuf);
};

#endif // DATAPROFILER_H
