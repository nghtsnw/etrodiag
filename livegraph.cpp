#include "livegraph.h"
#include "ui_livegraph.h"
#include <newgraph.h>
#include <QDebug>

liveGraph::liveGraph(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::liveGraph)
{
    ui->setupUi(this);
    connect (timer, &QTimer::timeout, this, &liveGraph::shiftCells);
    initGraph();
    timer->start(oneStepTime);
}

liveGraph::~liveGraph()
{
    delete ui;
}

void liveGraph::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    initGraph(); //заново инициализируем переменные при изменении размера виджета
    emit repaintCurves(); //перерисовываем графики с учётом новых значений переменных
}

void liveGraph::initGraph()
{
    pictWidth = this->size().width();//ширина
    pictHeight = this->size().height();//высота
    QPainter paint(this);
    if (!paint.isActive()) paint.begin(this); // запускаем отрисовку
    paint.eraseRect(0,0,pictWidth,pictHeight); // очищаем рисунок

    paint.setBrush(QBrush(Qt::white));
    paint.drawRect(0,0,pictWidth,pictHeight);
    paint.setPen(Qt::gray);

    verticalLineCount = steps/3;//кол-во вертикальных линий рассчитывается по количеству шагов на кадр делённому на три, что-бы три шага соответствовало одной ячейке (для возможного масштабирования)
    horizontalLineCount = 10;
    oneCellXpix = pictWidth/verticalLineCount; //определяем габариты ячеек
    oneCellYpix = pictHeight/horizontalLineCount;
    oneStepXpix = pictWidth/steps; //один шаг это ширина кадра делённая на количество шагов
    if (xShift == 1) xShiftPix = oneCellXpix/3; //для текущего вызова функции определяем горизонтальный сдвиг в пикселях, с которым рисуем вертикальные линии
    else if (xShift == 2) xShiftPix = (oneCellXpix/3)*2;
    else xShiftPix = 0;
    for (int i = horizontalLineCount, vCoord = 0; i > 0; --i) //рисуем горизонтальные линии
    {
        paint.drawLine(0,vCoord,pictWidth,vCoord);
        vCoord+=oneCellYpix;
    }
    for (int i = verticalLineCount+1, hCoord = xShiftPix*-1; i > 0; --i) //рисуем вертикальные линии
    {
        paint.drawLine(hCoord,0,hCoord,pictHeight);
        hCoord+=oneCellXpix;
    }

    paint.setPen(Qt::blue);//рисуем рамки
    paint.drawLine(0,0,0,pictHeight);
    paint.drawLine(0,pictHeight-1,pictWidth,pictHeight-1);
    paint.drawLine(pictWidth-1, pictHeight, pictWidth-1, 0);
    paint.drawLine(pictWidth-1,0,0,0);
}

void liveGraph::shiftCells()
{//так как условно одна ячейка это три шага, переменная xShift используется для определения сдвига при отрисовке вертикальных линий
    if (xShift !=2) xShift++;
    else xShift = 0;
    this->update();
}

void liveGraph::incomingDataSlot(int devNum, QString devName, int byteNum, QString byteName, uint32_t wordData, int id, QString parameterName, int binRawValue, double endValue, bool viewInLogFlag, bool isNewData, bool drawGraphFlag, QString drawGraphColor)
{
    if (drawGraphFlag)
    {
        QList<newgraph*> graphList = this->findChildren<newgraph*>();
        qDebug() << "graphlist count " << graphList.size();
        QListIterator<newgraph*> graphListIt(graphList);

        if (!graphList.empty())
        {
            for (int i = 0; i < graphList.size(); ++i)
            {
                if (graphListIt.peekNext()->devNum == devNum && graphListIt.peekNext()->byteNum == byteNum && graphListIt.peekNext()->id == id)
                {
                    foundFlag = true;
                    emit data2graph(devNum, byteNum, id, endValue, steps, drawGraphColor);
                    break;
                }
            }
        }
        if (!foundFlag)
        {
            newgraph *graph = new newgraph(this);
            connect (this, &liveGraph::repaintCurves, graph, &newgraph::repaintThis);
            connect (graph, &newgraph::graph2Painter, this, &liveGraph::paintCurve);
            connect (this, &liveGraph::data2graph, graph, &newgraph::dataPool);
            graph->devNum = devNum;
            graph->byteNum = byteNum;
            graph->id = id;
            emit data2graph(devNum, byteNum, id, endValue, steps, drawGraphColor);
            connect (timer, &QTimer::timeout, graph, &newgraph::oscillatorInput);
        }
    }
}

void liveGraph::paintCurve(QVector<double> points, QString color)
{
    qDebug() << "paint curve, color " << color;
    const int Yzero = pictHeight; //так как нули координат находятся в левом верхнем углу, сделаю условные координаты для удобства
    QPainter paintcv(this);
    if (!paintcv.isActive()) paintcv.begin(this);
    QColor paintColor;
    paintColor.setNamedColor(color);
    QPen pen(paintColor, 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    paintcv.setPen(pen);
    QVector<double> minMaxDeltaValue = findDeltaValue(points); //мнимальное, максимальное и дельта между ними
    double yScale = findYScale(minMaxDeltaValue); //вычисляем по дельте высоту шкалы Y, кратную 10
    double zeroShift = 0;
    if (minMaxDeltaValue.at(0) < 0)
    zeroShift = minMaxDeltaValue.at(0)*-1; //смещение нуля если минимальное значение меньше нуля
    double oneUnitPix = pictHeight/yScale; //цена одного деления в пикселях

    for (int i = 0, x = pictWidth; i < points.size()-1; ++i, x = x - oneStepXpix) {
        paintcv.drawLine(x, (((points.at(i)+zeroShift)*oneUnitPix)-pictHeight)*-1, x - oneStepXpix, (((points.at(i+1)+zeroShift)*oneUnitPix)-pictHeight)*-1);
    }
}

QVector<double> liveGraph::findDeltaValue(QVector<double>& points)
{
    double lastMinValue = points.at(0);
    double lastMaxValue = points.at(0);
    for (int num : points) {
        if (num > lastMaxValue)
            lastMaxValue = num;
        else if (num < lastMinValue)
            lastMinValue = num;
    }
    double deltaValue = 0.0;
    if (lastMinValue >= 0) deltaValue = lastMaxValue + lastMinValue;
    else if (lastMinValue < 0) deltaValue = lastMaxValue + (lastMinValue * -1);
    QVector<double> values = {lastMinValue, lastMaxValue, deltaValue};
    return values;
}

double liveGraph::findYScale(QVector<double> values)
{
    if (values.at(2) >= 1)
    {
        for (int var = 0, x = 10; var < 10; ++var) {
            if (values.at(2)>x) x = x * 10;
            else return x;
        }
    }
    else if (values.at(2) < 1 && values.at(2) > 0)
    {
        for (double var = 0, x = 1; var < 10; ++var) {
            if (values.at(2)>x) x = x / 10;
            else return x*10;
        }
    }
    else if (values.at(2) == 0)
    { //если дельта между размерами равна нулю, то вычисляем разрешение вертикальной шкалы по мин или макс размеру
        if (values.at(1) >= 1)
        {
            for (int var = 0, x = 10; var < 10; ++var) {
                if (values.at(1)>x) x = x * 10;
                else return x;
            }
        }
        else if (values.at(1) < 1 && values.at(1) > 0)
        {
            for (double var = 0, x = 1; var < 10; ++var) {
                if (values.at(1)>x) x = x / 10;
                else return x*10;
            }
        }
    }
}
