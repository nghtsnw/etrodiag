#ifndef NEWGRAPH_H
#define NEWGRAPH_H

#include <QObject>

class newgraph : public QObject
{
    Q_OBJECT
public:
    explicit newgraph(QObject *parent = nullptr);
    ~newgraph();
    int devNum;
    int byteNum;
    int id;

public slots:
    void dataPool(int _devNum, int _byteNum, int _id, double _endValue, int pointsOnGraph, QString _drawGraphColor);//приём живых данных
    void oscillatorInput();//по внешнему таймеру сдвиг массива с точками на один шаг и отправка на рисование
    void repaintThis();
private:
    QVector<double> pointsWithValues;
    QString graphColor;
    double value = 0.0;
    int watchDogCount = 0;
    void watchDog();

signals:
    void graph2Painter(QVector<double> data, QString color);

};

#endif // NEWGRAPH_H
