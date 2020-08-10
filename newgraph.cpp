#include "newgraph.h"

newgraph::newgraph(QObject *parent) : QObject(parent)
{

}

newgraph::~newgraph()
{

}

void newgraph::dataPool(int _devNum, int _byteNum, int _id, double _endValue, int _pointsOnGraph, QString _drawGraphColor)
{
    if (devNum == _devNum && byteNum == _byteNum && id == _id)
    {
        value = _endValue;
        if (graphColor!=_drawGraphColor) graphColor = _drawGraphColor;
        if (pointsWithValues.size() != _pointsOnGraph) pointsWithValues.resize(_pointsOnGraph);
    }
}

void newgraph::oscillatorInput()
{
    if (!pointsWithValues.isEmpty())
    {
        pointsWithValues.pop_back();
        pointsWithValues.push_front(value);
        //watchDog();
        value = 0; //если не будет новых данных, следующий шаг будет нарисован через 0
        emit graph2Painter(pointsWithValues, graphColor);
    }
}

void newgraph::repaintThis()
{
     emit graph2Painter(pointsWithValues, graphColor);
}

void newgraph::watchDog()
{//если данных нет на протяжении размера окна, освобождаем память от объекта графика
    if (pointsWithValues.at(0) == -1) watchDogCount++;
    if (watchDogCount > pointsWithValues.size()) this->~newgraph();
}
