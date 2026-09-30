#ifndef NEWGRAPH_H
#define NEWGRAPH_H

#include <QObject>
#include <QTimer>
#include <QMap>
#include <QDateTime>

class newgraph : public QObject
{
    Q_OBJECT
public:
    explicit newgraph(QObject *parent = nullptr);
    ~newgraph();
    static constexpr int kWatchdogMs = 3000; //пауза без данных, после которой кривая помечается "мёртвой"
    int devNum;
    int byteNum;
    int id;
    QString parameterName; //имя параметра и устройства - для подсказки при наведении
    QString devName;
    bool curveVisible = true; //кривую можно скрыть кликом по её строке в подписи графика
    const QMap<QDateTime, double> &points() const { return *pointsWithValues; } //все точки кривой (время -> значение)

public slots:
    void dataPool(int _devNum, int _byteNum, int _id, double _endValue, int pointsOnGraph, QString _drawGraphColor, QDateTime currentTime);//приём живых данных
    //void oscillatorInput();//по внешнему таймеру сдвиг массива с точками на один шаг и отправка на рисование
    void repaintThis();
private:
    QMap<QDateTime, double> *pointsWithValues;
    //QVector<double> bufferForMidValue;
    QString graphColor;
    double value = 0.0;
    double lastValue = 0.0;
    int watchDogCount = 0;
    void watchDog();
    QTimer watchDogTimer;
    bool watchDogFlag = false;

signals:
    void graph2Painter(QMap<QDateTime, double> pointsWithValues, QString color);

};

#endif // NEWGRAPH_H
