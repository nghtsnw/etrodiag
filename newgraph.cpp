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
        watchDogFlag = false;
        watchDogTimer.start(3000);
        if (graphColor!=_drawGraphColor) graphColor = _drawGraphColor;
        if (pointsWithValues.size() != _pointsOnGraph+4) pointsWithValues.resize(_pointsOnGraph+4);//+4 потому что по другому почему-то график рисуется обрезаным, так что небольшой запас
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
        lastValue = value,
        value = 0;
    }
    else if (bufferForMidValue.isEmpty())
    {
        pointsWithValues.pop_back();
        if (!watchDogFlag) pointsWithValues.push_front(lastValue);
        else if (watchDogFlag) pointsWithValues.push_front(0);
        emit graph2Painter(pointsWithValues, graphColor);

    }
}

void newgraph::repaintThis()
{
     emit graph2Painter(pointsWithValues, graphColor);
}

void newgraph::watchDog()
{//если данных нет в течении времени таймера (например нет связи с устройством), шлём на отрисовку нули
    watchDogFlag = true;
}
