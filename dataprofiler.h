
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
    QQueue<int> frameMsg;
    void frameSnap();
    void ffffchk();
    QVector<int> snapshot;

private:
    const int oneMsgLeight = 40;
    bool ffffbool;
    int countToNewFFFF;


signals:
    void readyGetByte(bool rdyGB);
    void deviceData(QVector<int> snapshot);
    void statusMessage(QString);

public slots:
    void getByte(int byteFromBuf);
};

#endif // DATAPROFILER_H
