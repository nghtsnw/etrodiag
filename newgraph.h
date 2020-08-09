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
    int pointsOnGraph;
    void setNextValue(int value);

public slots:
    void dataPool(int _devNum, QString _devName, int _byteNum, QString _byteName, uint32_t _wordData, int _id, QString parameterName, int _binRawValue, double _endValue, bool viewInLogFlag, bool isNewData, bool _drawGraphFlag, QString _drawGraphColor);//приём живых данных
    void oscillatorInput();//по внешнему таймеру сдвиг массива с точками на один шаг и отправка на рисование
    void repaintThis();
private:
    QVector<double> pointsWithValues;
    QString graphColor;
    int value;
    int watchDogCount = 0;
    void watchDog();

signals:
    void graph2Painter(QVector<double> data, QString color);

};

#endif // NEWGRAPH_H
