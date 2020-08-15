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
    paint.setPen(Qt::lightGray);
    paint.setOpacity(0.5);

    verticalLineCount = steps/3;//кол-во вертикальных линий рассчитывается по количеству шагов на кадр делённому на три, что-бы три шага соответствовало одной ячейке (для возможного масштабирования)
    horizontalLineCount = 10;
    oneCellXpix = pictWidth/verticalLineCount; //определяем габариты ячеек
    oneCellYpix = pictHeight/horizontalLineCount;
    scaleErrorPix = pictHeight-(oneCellYpix*horizontalLineCount);//погрешность от деления высоты окна на количество ячеек, для коррекции масштаба графика
    oneStepXpix = pictWidth/steps; //один шаг это ширина кадра делённая на количество шагов
    vZeroLevel = oneCellYpix*horizontalLineCount;//вертикальный уровень нуля
    if (xShift == 1) xShiftPix = oneStepXpix;//xShiftPix = oneCellXpix/3; //для текущего вызова функции определяем горизонтальный сдвиг в пикселях, с которым рисуем вертикальные линии
    else if (xShift == 2) xShiftPix = oneStepXpix*2;//(oneCellXpix/3)*2;
    else xShiftPix = 0;
    for (int i = horizontalLineCount+1, vCoord = scaleErrorPix; i > 0; --i) //рисуем горизонтальные линии
    {
        paint.drawLine(0,vCoord,pictWidth,vCoord);
        vCoord+=oneCellYpix;
    }
    for (int i = verticalLineCount+1, hCoord = xShiftPix*-1; i > 0; --i) //рисуем вертикальные линии
    {
        paint.drawLine(hCoord,0,hCoord,pictHeight);
        hCoord+=oneCellXpix;
    }
    paint.setOpacity(1.0);
    paint.setPen(Qt::white);//рисуем рамки белым цветом, создавая безрамочный эффект
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
        QList<newgraph*> graphList = this->findChildren<newgraph*>();
        QListIterator<newgraph*> graphListIt(graphList);
        foundFlag = false;
        if (!graphList.empty())
        {
            for (int i = 0; i < graphList.size(); ++i)
            {
                if (graphListIt.peekNext()->devNum == devNum && graphListIt.peekNext()->byteNum == byteNum && graphListIt.peekNext()->id == id)
                {//если нашёлся график
                    if (drawGraphFlag)
                    {//и в новых данных флаг на разрешение рисования, то обновляем график
                        foundFlag = true;
                        emit data2graph(devNum, byteNum, id, endValue, steps, drawGraphColor);
                        break;
                    }
                    else
                    {
                        graphAnnotation.remove(drawGraphColor);
                        graphListIt.next()->~newgraph(); //если флаг снят - удаляем объект графика
                        break;
                    }
                }
                graphListIt.next();
            }
        }
        if (!foundFlag && drawGraphFlag)
        {//если график не найден то создаём, инициализируем и сразу отправляем данные
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
        if (drawGraphFlag)
        {
            QString tmp = parameterName+'@'+devName;
            graphAnnotation.insert(drawGraphColor, tmp);
        }
}

void liveGraph::paintCurve(QVector<double> points, QString color)
{//сюда каждый объект графика отдаёт массив данных и цвет на рисование
    QPainter paintcv(this);
    if (!paintcv.isActive()) paintcv.begin(this);
    QColor paintColor;
    paintColor.setNamedColor(color);
    QPen pen(paintColor, 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    paintcv.setBrush(QBrush(paintColor));
    paintcv.setPen(pen);
    QVector<double> minMaxDeltaValue = findDeltaValue(points); //мнимальное, максимальное и дельта между ними
    double yScale = findYScale(minMaxDeltaValue); //вычисляем по дельте высоту шкалы Y, кратную 10
    double zeroShift = 0;
    if (minMaxDeltaValue.at(0) < 0)
    zeroShift = minMaxDeltaValue.at(0)*-1; //смещение нуля если минимальное значение меньше нуля
    double oneUnitPix = vZeroLevel/yScale; //цена одного деления в пикселях
    //рисуем линии с учётом всех смещений и поправок на масштабирование
    for (double i = 0, x = oneCellXpix*verticalLineCount; i < points.size()-1; ++i, x = x - oneStepXpix) {
        paintcv.drawLine(x, (((points.at(i)+zeroShift)*oneUnitPix)-vZeroLevel-scaleErrorPix)*-1,
                         x - oneStepXpix, (((points.at(i+1)+zeroShift)*oneUnitPix)-vZeroLevel-scaleErrorPix)*-1);
        paintcv.drawEllipse(x-2,(((points.at(i)+zeroShift)*oneUnitPix)-vZeroLevel-scaleErrorPix+2)*-1,4,4);
    }
    curvesCount++;
    if (curvesCount == graphAnnotation.size()) paintAnnotation();
}

void liveGraph::paintAnnotation()
{
    QPainter paintan(this);
    if (!paintan.isActive()) paintan.begin(this);
    QColor paintColor;
    QFont font("Times", 8);
    paintan.setFont(font);
    annotationKeys = graphAnnotation.keys();
    rectXSizePix = maxStringSizePix(font, graphAnnotation.values());
    const int oneStringYpix = 18;
    paintan.setPen(Qt::white);
    paintan.setBrush(QBrush(Qt::white));
    paintan.setOpacity(0.7);
    paintan.drawRect(0, 0, rectXSizePix.at(0)+15, oneStringYpix*graphAnnotation.size()+3);
    paintan.setOpacity(1.0);
    for (int i = 0, y = 4; i < graphAnnotation.size(); ++i, y+=oneStringYpix) {
        paintColor.setNamedColor(annotationKeys.at(i));
        paintan.setPen(paintColor);
        paintan.setBrush(QBrush(paintColor));
        paintan.drawEllipse(2,y,8,8);
        paintan.setPen(Qt::black);
        paintan.setBrush(QBrush(Qt::black));
        paintan.drawText(12, y+10, graphAnnotation.value(annotationKeys.at(i)));
    }
    paintan.end();
    curvesCount = 0;
}

QVector<int> liveGraph::maxStringSizePix(QFont font, QList<QString> str)
{
    QFontMetrics fm(font);
    int pixelsWideMax = 0;
    for (QString string : str)
    {
        int pixelsWide = fm.horizontalAdvance(string);
        if (pixelsWide > pixelsWideMax) pixelsWideMax = pixelsWide;
    }
    int pixelsHigh = fm.height();
    QVector<int> maxSizePix = {pixelsWideMax, pixelsHigh};
    return maxSizePix;
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
    if (lastMinValue >= 0) deltaValue = lastMaxValue;
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
