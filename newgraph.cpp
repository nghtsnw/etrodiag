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
        if (pointsWithValues.size() != _pointsOnGraph+1) pointsWithValues.resize(_pointsOnGraph+1);
    }
}

void newgraph::oscillatorInput()//формирование массива чисел для отрисовки графика по тактовому сигналу
{
    if (!pointsWithValues.isEmpty() && !bufferForMidValue.isEmpty())
    {
        pointsWithValues.pop_back();
        for (double num : bufferForMidValue) //если за время паузы успело прийти несколько значений, вычисляется среднее
            value += num;
        value = value/bufferForMidValue.size();
        pointsWithValues.push_front(value);        
        bufferForMidValue.clear();
        lastValue = value,
        value = 0;
    }
    else if (bufferForMidValue.isEmpty())
    {
        pointsWithValues.pop_back();
        if (!watchDogFlag) pointsWithValues.push_front(lastValue);
        else if (watchDogFlag) pointsWithValues.push_front(0);
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
