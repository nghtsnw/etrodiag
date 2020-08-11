#include "newgraph.h"

newgraph::newgraph(QObject *parent) : QObject(parent)
{
    connect (&watchDogTimer, &QTimer::timeout, this, &newgraph::watchDog);
}

newgraph::~newgraph()
{

}

void newgraph::dataPool(int _devNum, int _byteNum, int _id, double _endValue, int _pointsOnGraph, QString _drawGraphColor)
{
    if (devNum == _devNum && byteNum == _byteNum && id == _id)
    {
        bufferForMidValue.push_back(_endValue);
        watchDogTimer.start(3000);
        if (graphColor!=_drawGraphColor) graphColor = _drawGraphColor;
        if (pointsWithValues.size() != _pointsOnGraph) pointsWithValues.resize(_pointsOnGraph);
    }
}

void newgraph::oscillatorInput()
{
    if (!pointsWithValues.isEmpty() && !bufferForMidValue.isEmpty())
    {
        pointsWithValues.pop_back();
        for (double num : bufferForMidValue) //если за время паузы успело прийти несколько значений, вычисляется среднее
            value += num;
        value = value/bufferForMidValue.size();
        pointsWithValues.push_front(value);        
        emit graph2Painter(pointsWithValues, graphColor);
        bufferForMidValue.clear();
        value = 0;
    }
}

void newgraph::repaintThis()
{
     emit graph2Painter(pointsWithValues, graphColor);
}

void newgraph::watchDog()
{//если данных нет на протяжении размера окна, освобождаем память от объекта графика
    value = 0;
}
