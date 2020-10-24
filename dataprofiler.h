
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
    const int oneMsgLeight = 40;
    QQueue<int> frameMsg;
    QVector<int> snapshot;

signals:
    void deviceData(QVector<int> snapshot);

public slots:
    void getByte(int byteFromBuf);
};

#endif // DATAPROFILER_H
