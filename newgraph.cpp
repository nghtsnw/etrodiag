#include "newgraph.h"

newgraph::newgraph(QObject *parent) : QObject(parent)
{

}

newgraph::~newgraph()
{

}

void newgraph::setNextValue(int _value)
{
    value = _value;
}

void newgraph::dataPool(int _devNum, QString _devName, int _byteNum, QString _byteName, uint32_t _wordData, int _id, QString parameterName, int _binRawValue, double _endValue, bool viewInLogFlag, bool isNewData, bool _drawGraphFlag, QString _drawGraphColor)
{
    if (devNum == _devNum && byteNum == _byteNum && id == _id)
    {
        value = _endValue;
        if (graphColor!=_drawGraphColor) graphColor = _drawGraphColor;
        if (pointsWithValues.size() != pointsOnGraph) pointsWithValues.resize(pointsOnGraph);
    }
}

void newgraph::oscillatorInput()
{
    pointsWithValues.pop_back();
    pointsWithValues.push_front(value);
    //watchDog();
    value = 0; //если не будет новых данных, следующий шаг будет нарисован через 0
    emit graph2Painter(pointsWithValues, graphColor);
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
