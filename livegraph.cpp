#include "livegraph.h"
#include "qdatetime.h"
#include "ui_livegraph.h"
#include <newgraph.h>
#include <QDebug>
#include <QMouseEvent>

liveGraph::liveGraph(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::liveGraphWidget)
{
    ui->setupUi(this);
    connect (timer, &QTimer::timeout, this, &liveGraph::shiftCells);
    connect (timer, &QTimer::timeout, this, [ = ]() {
        if (!readFromFile) {
            realTime = QDateTime::currentDateTime(); //При чтении в реальном времени
        }
        else
        {
            realTime = QDateTime::fromMSecsSinceEpoch(beginTime->toMSecsSinceEpoch() + (startTime.toMSecsSinceEpoch() - QDateTime::currentDateTime().toMSecsSinceEpoch()));
        }
        betweenTime = beginTime->msecsTo(realTime);
        /*
         realTime - "реальное" время для текущего режима работы. Если читаем данные с порта, то подставляем системное время на момент чтения.
            Если читаем из файла, то высчитываем "реальное" время, беря за ноль первую временную метку из файла, прибавляя к ней время от начала чтения.
         startTime - системное время на начало чтения из файла.
         betweenTime - время между начальной временной меткой и "реальным" временем
        */
        //В режиме чтения из лога отключить / переделать
    });
    connect (this, &liveGraph::startOscillator, this, [this]() {
        timer->start(oneStepTime);
    });
    connect (this, &liveGraph::stopOscillator, this, [this]() {
        timer->stop();
    });
    waitFirstData = true;
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
    paintAnnotation();
}

void liveGraph::initGraph()
{
    pictWidth = this->size().width();//ширина
    pictHeight = this->size().height();//высота
    QPainter paint(this);
    if (paint.isActive())
    {
        paint.eraseRect(0, 0, pictWidth, pictHeight); // очищаем рисунок
        paint.setBrush(QBrush(Qt::white));
        paint.drawRect(0, 0, pictWidth, pictHeight);
        paint.setPen(Qt::lightGray);
        paint.setOpacity(0.5);
        verticalLineCount = steps / 15; //кол-во вертикальных линий рассчитывается по количеству шагов на кадр делённому на три, что-бы три шага соответствовало одной ячейке (для возможного масштабирования)
        horizontalLineCount = 10;
        oneCellXpix = pictWidth / verticalLineCount; //определяем габариты ячеек
        oneCellYpix = pictHeight / horizontalLineCount;
        scaleErrorPix = pictHeight - (oneCellYpix * horizontalLineCount); //погрешность от деления высоты окна на количество ячеек, для коррекции масштаба графика
        oneStepXpix = pictWidth / steps; //один шаг это ширина кадра делённая на количество шагов
        vZeroLevel = oneCellYpix * horizontalLineCount; //вертикальный уровень нуля
        //TODO Переделать на 15 шагов
        if (xShift == 1) {
            xShiftPix = oneStepXpix; //xShiftPix = oneCellXpix/3; //для текущего вызова функции определяем горизонтальный сдвиг в пикселях, с которым рисуем вертикальные линии
        }
        else if (xShift == 2) {
            xShiftPix = oneStepXpix * 2; //(oneCellXpix/3)*2;
        }
        else {
            xShiftPix = 0;
        }
        /*---------------------------*/
        for (int i = horizontalLineCount + 1, vCoord = pictHeight; i > 0; --i) //рисуем горизонтальные линии
        {
            paint.drawLine(0, vCoord, pictWidth, vCoord);
            vCoord -= oneCellYpix;
        }
        for (int i = verticalLineCount + 1, hCoord = xShiftPix * -1; i > 0; --i) //рисуем вертикальные линии
        {
            paint.drawLine(hCoord, 0, hCoord, pictHeight);
            hCoord += oneCellXpix;
        }
        paint.setOpacity(1.0);
        paint.setPen(Qt::white);//рисуем рамки белым цветом, создавая безрамочный эффект
        paint.drawLine(0, 0, 0, pictHeight);
        paint.drawLine(0, pictHeight - 1, pictWidth, pictHeight - 1);
        paint.drawLine(pictWidth - 1, pictHeight, pictWidth - 1, 0);
        paint.drawLine(pictWidth - 1, 0, 0, 0);
        paint.end();
    }
}

void liveGraph::shiftCells()
{ //так как условно одна ячейка это три шага, переменная xShift используется для определения сдвига при отрисовке вертикальных линий
    if (xShift != 14) {
        xShift++;
    }
    else {
        xShift = 0;
    }
    this->update();
}

void liveGraph::incomingDataSlot(QDateTime currentTime, s_parameterMask data)
{
    if (waitFirstData)
    {
        waitFirstData = false;
        beginTime = &currentTime;
        startTime = QDateTime::currentDateTime();
    }
    QList<newgraph*> graphList = this->findChildren<newgraph*>();
    QListIterator<newgraph*> graphListIt(graphList);
    foundFlag = false;
    if (!graphList.empty())
    {
        for (int i = 0; i < graphList.size(); ++i)
        {
            if (graphListIt.peekNext()->devNum == data.devNum && graphListIt.peekNext()->byteNum == data.byteNum && graphListIt.peekNext()->id == data.id)
            { //если нашёлся график
                if (data.drawGraphFlag)
                { //и в новых данных флаг на разрешение рисования, то обновляем график
                    foundFlag = true;
                    emit data2graph(data.devNum, data.byteNum, data.id, data.endValue, steps, data.drawGraphColor, currentTime);
                    break;
                }
                else
                {
                    graphAnnotation.remove(data.drawGraphColor);
                    graphAnnotationMinMax.remove(data.parameterName);
                    graphListIt.next()->~newgraph(); //если флаг снят - удаляем объект графика
                    break;
                }
            }
            graphListIt.next();
        }
    }
    if (!foundFlag && data.drawGraphFlag)
    { //если график не найден то создаём, инициализируем и сразу отправляем данные
        newgraph *graph = new newgraph(this);
        connect (this, &liveGraph::repaintCurves, graph, &newgraph::repaintThis);
        connect (graph, &newgraph::graph2Painter, this, [ = ](QMap<QDateTime, double> points, QString color) {
            //Тут надо врезать подстановку calculatedEndTime
            paintCurve(points, calculatedEndTime, color); //Подаётся невалидное время
        }); //calculatedEndTime либо реальное время - и до него ищется ближайшая временная метка в графике
        //либо вычисленное по положению слайдера, и так же ищется ближайшая метка в графике
        //graph2Painter отдаёт указатель на весь массив графика, и цвет рисования
        //на paintCurve нужно выдать уже время конца, к котрому привязывается график
        connect (this, &liveGraph::data2graph, graph, &newgraph::dataPool);
        graph->devNum = data.devNum;
        graph->byteNum = data.byteNum;
        graph->id = data.id;
        emit data2graph(data.devNum, data.byteNum, data.id, data.endValue, steps, data.drawGraphColor, currentTime);
        //connect (timer, &QTimer::timeout, graph, &newgraph::oscillatorInput);
        graphAnnotationMinMax.insert(data.parameterName, {data.endValue, data.endValue});
    }
    if (data.drawGraphFlag)
    {
        if (graphAnnotationMinMax.value(data.parameterName).at(0) > data.endValue) //Тут сыпется при попытке рисования
        {
            QVector<double> minMax = {data.endValue, graphAnnotationMinMax.value(data.parameterName).at(1)};
            graphAnnotationMinMax.insert(data.parameterName, minMax);
        }
        if (graphAnnotationMinMax.value(data.parameterName).at(1) < data.endValue)
        {
            QVector<double> minMax = {graphAnnotationMinMax.value(data.parameterName).at(0), data.endValue};
            graphAnnotationMinMax.insert(data.parameterName, minMax);
        }
        if (minMaxOnOff)
        {
            QString annotationString = data.parameterName + '@' + data.devName + " - " + QString::number(data.endValue)
                                       + "| Min - " + QString::number(graphAnnotationMinMax.value(data.parameterName).at(0)) + "| Max - " +
                                       QString::number(graphAnnotationMinMax.value(data.parameterName).at(1));
            graphAnnotation.insert(data.drawGraphColor, annotationString);
        }
        else
        {
            graphAnnotationMinMax.insert(data.parameterName, {data.endValue, data.endValue});
            QString annotationString = data.parameterName + '@' + data.devName + " - " + QString::number(data.endValue);
            graphAnnotation.insert(data.drawGraphColor, annotationString);
        }
    }
}

void liveGraph::paintCurve(QMap<QDateTime, double> allPoints, QDateTime endTime, QString color)
{ //сюда каждый объект графика отдаёт массив данных и цвет на рисование
    QPainter paintcv(this);
    if (paintcv.isActive())
    {
        QMap<QDateTime, double> points = pointsForTimeFrames(allPoints, endTime); //TODO Добавить маркер времени начала для возможности навигации по графику
        QColor paintColor;
        paintColor.fromString(color);
        QPen pen(paintColor, 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        paintcv.setBrush(QBrush(paintColor));
        paintcv.setPen(pen);
        QVector<double> minMaxDeltaValue = findDeltaValue(points); //мнимальное, максимальное и дельта между ними
        double yScale = findYScale(minMaxDeltaValue); //вычисляем по дельте высоту шкалы Y, кратную 10
        double zeroShift = 0;
        if (minMaxDeltaValue.at(0) < 0) {
            zeroShift = minMaxDeltaValue.at(0) * -1; //смещение нуля если минимальное значение меньше нуля
        }
        double oneUnitPix = vZeroLevel / yScale; //цена одного деления в пикселях
        //рисуем линии с учётом всех смещений и поправок на масштабирование
        for (double i = 0, x = oneCellXpix * verticalLineCount; i < points.size() - 1; ++i, x = x - oneStepXpix) {
            paintcv.drawLine(x, (((points.values().at(i) + zeroShift)*oneUnitPix) - vZeroLevel - scaleErrorPix) * -1,
                             x - oneStepXpix, (((points.values().at(i + 1) + zeroShift)*oneUnitPix) - vZeroLevel - scaleErrorPix) * -1);
            paintcv.drawEllipse(x - 2, (((points.values().at(i) + zeroShift)*oneUnitPix) - vZeroLevel - scaleErrorPix + 2) * -1, 4, 4);
        }
        curvesCount++;
    }
}

void liveGraph::timeNavigationScrollbarPositionChanged(int pos) // Пропорционально положению слайдера, нужно выбрать временные рамки для отрисовки
{
    double proportion_slider = (pos / 10000/*slider maximum*/);
    qint64 proportion_time = betweenTime * proportion_slider;
    QDateTime markerTimePosition = beginTime->addMSecs(proportion_time);
    if (proportion_slider < 1.0) {
        calculatedEndTime = markerTimePosition;
    }
}

void liveGraph::timeNavigationScrollbarNewMaxLevel(int max)
{
    ui->timeScrollBar->setMaximum(max);
}


void liveGraph::paintAnnotation()
{
    QPainter paintan(this);
    if (paintan.isActive())
    {
        QColor paintColor;
        QFont font("Times", 9);
        paintan.setFont(font);
        annotationKeys = graphAnnotation.keys();
        rectXSizePix = maxStringSizePix(font, graphAnnotation.values());//[0] - длина строки, [1] - высота
        const int oneStringYpix = rectXSizePix.at(1) +2;
        paintan.setPen(Qt::white);
        paintan.setBrush(QBrush(Qt::white));
        paintan.setOpacity(0.7);
        paintan.drawRect(0, 0, rectXSizePix.at(0) +15, oneStringYpix * graphAnnotation.size() + 3);
        paintan.setOpacity(1.0);
        for (int i = 0, y = 4; i < graphAnnotation.size(); ++i, y += oneStringYpix) {
            paintColor.fromString(annotationKeys.at(i));
            paintan.setPen(paintColor);
            paintan.setBrush(QBrush(paintColor));
            paintan.drawEllipse(2, y, 8, 8);
            paintan.setPen(Qt::black);
            paintan.setBrush(QBrush(Qt::black));
            paintan.drawText(12, y + 10, graphAnnotation.value(annotationKeys.at(i)));
        }
    }
    curvesCount = 0;
}

QVector<int> liveGraph::maxStringSizePix(QFont font, QList<QString> str)//считаем максимальный размер строки для рисования аннотации к графику
{
    QFontMetrics fm(font);
    int pixelsWideMax = 0;
    for (const QString &string : str)
    {
        int pixelsWide = fm.horizontalAdvance(string);
        if (pixelsWide > pixelsWideMax) {
            pixelsWideMax = pixelsWide;
        }
    }
    QVector<int> maxSizePix = {pixelsWideMax, fm.height()};
    return maxSizePix;
}

QMap<QDateTime, double> liveGraph::pointsForTimeFrames(QMap<QDateTime, double>& points, QDateTime timeMarker)
{
    QList<QDateTime> keysForPoints = points.keys();
    QListIterator<QDateTime> keysForPointsIt(keysForPoints);
    //Вычисляем время начала отрисовки, отнимая ширину фрейма в секундах от последнего времени в массиве точек
    QDateTime firstPointForDraw = QDateTime::fromMSecsSinceEpoch((timeMarker.toMSecsSinceEpoch()) - (timeFrames * 1000));
    //Теперь надо собрать массив точек для данного конкретного временного отрезка
    /*QList<QDateTime> splittedForFramesKeys;
    keysForPointsIt.toFront();
    do {
        splittedForFramesKeys.push_back(keysForPointsIt.previous());
    }
    while (keysForPointsIt.peekPrevious() >= firstPointForDraw); // Тут сыпется*/
    //По собранным ключам добавляются значения из большого массива
    QMap<QDateTime, double> splittedPoints;
    for (auto key : keysForPoints) {
        splittedPoints.insert(key, points.value(key));
    }
    return splittedPoints;
}

QVector<double> liveGraph::findDeltaValue(QMap<QDateTime, double>& _points)
{
    QVector<double> points = _points.values();
    double lastMinValue = points.at(0);
    double lastMaxValue = points.at(0);
    for (int num : std::as_const(points)) {
        if (num > lastMaxValue) {
            lastMaxValue = num;
        }
        else if (num < lastMinValue) {
            lastMinValue = num;
        }
    }
    double deltaValue = 0.0;
    if (lastMinValue >= 0) {
        deltaValue = lastMaxValue;
    }
    else if (lastMinValue < 0) {
        deltaValue = lastMaxValue + (lastMinValue * -1);
    }
    QVector<double> values = {lastMinValue, lastMaxValue, deltaValue};
    return values;
}

double liveGraph::findYScale(const QVector<double>& values)
{ //вычисляем цену шкалы делений кратную 10
    double val4CalcYScale;
    if (values.at(2) != 0) {
        val4CalcYScale = values.at(2);
    }
    else {
        val4CalcYScale = values.at(1); //если дельта 0, то вычисляем по мин(макс) размеру
    }
    double result = 1;
    if (val4CalcYScale >= 1)
    {
        for (int var = 0, x = 10; var < 10; ++var) {
            if (val4CalcYScale > x) {
                x = x * 10;
            }
            else
            {
                result = x;
                continue;
            }
        }
    }
    else if (val4CalcYScale < 1 && val4CalcYScale > 0)
    {
        result = 1;
    }
    else {
        result = 1;
    }
    return result;
}

void liveGraph::chngMinMaxVisible()
{
    minMaxOnOff = !minMaxOnOff;
}

void liveGraph::cleanGraph()
{
    QList<newgraph*> graphList = this->findChildren<newgraph*>();
    QListIterator<newgraph*> graphListIt(graphList);
    graphAnnotation.clear();
    graphAnnotationMinMax.clear();
    while (graphListIt.hasNext())
    {
        graphListIt.next()->~newgraph();
    }
    waitFirstData = true;
}
