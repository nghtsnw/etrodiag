#include "livegraph.h"
#include "qdatetime.h"
#include "ui_livegraph.h"
#include <newgraph.h>
#include <QDebug>
#include <QMouseEvent>
#include <QScrollBar>
#include <QLabel>

liveGraph::liveGraph(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::liveGraphWidget)
{
    ui->setupUi(this);
    timeNavigationScrollbarNewMaxLevel(kSliderMax); //диапазон навигации по логу
    ui->timeScrollBar->setSingleStep(10);
    ui->timeScrollBar->setPageStep(1000); //ручка ~10% ширины, иначе её не ухватить
    connect (ui->timeScrollBar, &QScrollBar::valueChanged, this, &liveGraph::timeNavigationScrollbarPositionChanged);
    frameTimeLabel = new QLabel(this); //временная метка конца кадра над слайдером
    frameTimeLabel->setStyleSheet("QLabel{background:#FFFFAA;border:1px solid #808080;padding:2px;color:#000000;}");
    frameTimeLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
    frameTimeLabel->hide();
    ui->leftTimeLabel->setStyleSheet("QLabel{background:#FFFFAA;border:1px solid #808080;padding:2px;color:#000000;}");
    ui->rightTimeLabel->setStyleSheet("QLabel{background:#FFFFAA;border:1px solid #808080;padding:2px;color:#000000;}");
    connect (ui->timeScrollBar, &QScrollBar::sliderPressed, this, &liveGraph::showNavigationTimeLabel);
    connect (ui->timeScrollBar, &QScrollBar::sliderReleased, this, &liveGraph::hideNavigationTimeLabel);
    connect (this, &liveGraph::readFromFileSignal, [this](bool r) {
        readFromFile = r;
        if (r) {
            emit startOscillator();
        }
        else {
            emit stopOscillator();
        }
    });
    connect (this, &liveGraph::startOscillator, this, [this]() {
        timer->start(oneStepTime);
    });
    connect (this, &liveGraph::stopOscillator, this, [this]() {
        timer->stop();
    });
    connect (timer, &QTimer::timeout, this, &liveGraph::shiftCells);
    connect (timer, &QTimer::timeout, this, [this]() {
        if (!readFromFile) {
            realTime = QDateTime::currentDateTime(); //живой режим от порта - реальное время
        }
        else {
            realTime = lastTime; //чтение лога - время из лога (последняя принятая метка)
        }
        //betweenTime - длительность данных от первой метки до конца графика
        betweenTime = beginTime.msecsTo(realTime);
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
    pictHeight = this->size().height() - ui->timeScrollBar->size().height();//высота
    QPainter paint(this);
    if (paint.isActive())
    {
        paint.eraseRect(0, 0, pictWidth, pictHeight); // очищаем рисунок
        paint.setBrush(QBrush(Qt::white));
        paint.drawRect(0, 0, pictWidth, pictHeight);
        paint.setPen(Qt::lightGray);
        paint.setOpacity(0.5);
        verticalLineCount = steps / 25; //кол-во вертикальных линий рассчитывается по количеству шагов на кадр делённому на три, что-бы три шага соответствовало одной ячейке (для возможного масштабирования)
        horizontalLineCount = 10;
        onePixelTime = (timeFrames * 1000) / pictWidth;
        oneCellXpix = pictWidth / verticalLineCount; //определяем габариты ячеек
        oneCellYpix = pictHeight / horizontalLineCount;
        scaleErrorPix = pictHeight - (oneCellYpix * horizontalLineCount); //погрешность от деления высоты окна на количество ячеек, для коррекции масштаба графика
        vZeroLevel = oneCellYpix * horizontalLineCount; //вертикальный уровень нуля
        oneStepXpix = pictWidth / steps; //один шаг это ширина кадра делённая на количество шагов
        xShiftPix = oneStepXpix * xShift;//для текущего вызова функции определяем горизонтальный сдвиг в пикселях, с которым рисуем вертикальные линии
        /*if (xShift == 1) {
            xShiftPix = oneStepXpix; //xShiftPix = oneCellXpix/3;
        }
        else if (xShift == 2) {
            xShiftPix = oneStepXpix * 2; //(oneCellXpix/3)*2;
        }
        else {
            xShiftPix = 0;
        }*/
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
        paint.setPen(Qt::white);//рисуем рамки
        paint.drawLine(0, 0, 0, pictHeight);
        paint.drawLine(0, pictHeight - 1, pictWidth, pictHeight - 1);
        paint.drawLine(pictWidth - 1, pictHeight, pictWidth - 1, 0);
        paint.drawLine(pictWidth - 1, 0, 0, 0);
        paint.end();
    }
}

void liveGraph::shiftCells()
{ //переменная xShift используется для определения сдвига при отрисовке вертикальных линий
    /*if (xShift <= 25) {
        xShift++;
    }
    else {
        xShift = 0;
    }*/
    this->update();
}

void liveGraph::incomingDataSlot(QDateTime currentTimeForData, s_parameterMask data)
{
    if (waitFirstData)
    {
        waitFirstData = false;
        beginTime = currentTimeForData;
        ui->leftTimeLabel->setText(beginTime.toString("hh:mm:ss"));
    }
    lastTime = currentTimeForData;
    ui->rightTimeLabel->setText(lastTime.toString("hh:mm:ss"));

    newgraph *curve = findCurve(data);
    if (curve)
    {
        if (data.drawGraphFlag)
        { //в новых данных разрешено рисование - обновляем график
            emit data2graph(data.devNum, data.byteNum, data.id, data.endValue, steps, data.drawGraphColor, currentTimeForData);
        }
        else
        { //флаг снят - удаляем график вместе с подписью
            graphAnnotation.remove(data.drawGraphColor);
            graphAnnotationMinMax.remove(data.parameterName);
            delete curve;
        }
    }
    else if (data.drawGraphFlag)
    { //график не найден - создаём, инициализируем и сразу отправляем данные
        createCurve(data, currentTimeForData);
    }

    if (data.drawGraphFlag) {
        updateAnnotation(data);
    }
}

newgraph *liveGraph::findCurve(const s_parameterMask &data)
{
    const QList<newgraph*> graphList = this->findChildren<newgraph*>();
    for (newgraph *graph : graphList) {
        if (graph->devNum == data.devNum && graph->byteNum == data.byteNum && graph->id == data.id) {
            return graph;
        }
    }
    return nullptr;
}

newgraph *liveGraph::createCurve(const s_parameterMask &data, const QDateTime &time)
{
    newgraph *graph = new newgraph(this);
    connect (this, &liveGraph::repaintCurves, graph, &newgraph::repaintThis);
    connect (graph, &newgraph::graph2Painter, this, [this](QMap<QDateTime, double> points, QString color) {
        if (!navigationActive) { //при навигации по логу конец графика задаёт слайдер, а не реальное время
            calculatedEndTime = realTime;
        }
        //calculatedEndTime - конец графика: либо последняя принятая метка данных,
        //либо время по положению слайдера навигации. К нему привязывается выборка точек.
        paintCurve(points, calculatedEndTime, color);
    }); //graph2Painter отдаёт весь массив точек графика и цвет рисования
    connect (this, &liveGraph::data2graph, graph, &newgraph::dataPool);
    graph->devNum = data.devNum;
    graph->byteNum = data.byteNum;
    graph->id = data.id;
    graphAnnotationMinMax.insert(data.parameterName, {data.endValue, data.endValue});
    emit data2graph(data.devNum, data.byteNum, data.id, data.endValue, steps, data.drawGraphColor, time);
    return graph;
}

void liveGraph::updateAnnotation(const s_parameterMask &data)
{
    if (!graphAnnotationMinMax.contains(data.parameterName)) { //нет записи (например после смены профиля) - создаём, иначе .at() выйдет за границы
        graphAnnotationMinMax.insert(data.parameterName, {data.endValue, data.endValue});
    }
    if (minMaxOnOff)
    {
        QVector<double> minMax = graphAnnotationMinMax.value(data.parameterName);
        if (minMax.at(0) > data.endValue) {
            minMax[0] = data.endValue;
        }
        if (minMax.at(1) < data.endValue) {
            minMax[1] = data.endValue;
        }
        graphAnnotationMinMax.insert(data.parameterName, minMax);
        const QString annotationString = data.parameterName + '@' + data.devName + " - " + QString::number(data.endValue)
                                         + "| Min - " + QString::number(minMax.at(0)) + "| Max - " + QString::number(minMax.at(1));
        graphAnnotation.insert(data.drawGraphColor, annotationString);
    }
    else
    {
        graphAnnotationMinMax.insert(data.parameterName, {data.endValue, data.endValue});
        const QString annotationString = data.parameterName + '@' + data.devName + " - " + QString::number(data.endValue);
        graphAnnotation.insert(data.drawGraphColor, annotationString);
    }
}

void liveGraph::paintCurve(QMap<QDateTime, double> allPoints, QDateTime endTime, QString color) //приходит кривой endTime
{ //сюда каждый объект графика отдаёт массив данных и цвет на рисование
    QPainter paintcv(this);
    if (!paintcv.isActive()) {
        return;
    }
    QMap<QDateTime, double> points = pointsForTimeFrames(allPoints, endTime);
    if (points.isEmpty()) { //в выбранном временном окне нет точек
        return;
    }
    const QMap<qint64, double> pointsPixelMap = pointsToPixelMap(points);

    QColor paintColor;
    paintColor.setNamedColor(color);
    paintcv.setBrush(QBrush(paintColor));
    paintcv.setPen(QPen(paintColor, 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

    const QVector<double> minMaxDeltaValue = findDeltaValue(points); //минимальное, максимальное и дельта между ними
    const double yScale = findYScale(minMaxDeltaValue); //высота шкалы Y, кратная 10
    const double zeroShift = (minMaxDeltaValue.at(0) < 0) ? minMaxDeltaValue.at(0) * -1 : 0.0; //смещение нуля при отрицательных значениях
    const double oneUnitPix = vZeroLevel / yScale; //цена одного деления в пикселях
    const qint64 shiftMs = endTime.toMSecsSinceEpoch() - points.lastKey().toMSecsSinceEpoch();
    const int shiftPix = shiftMs / onePixelTime; //сдвиг так, чтобы конец кривой совпал с концом графика

    drawCurve(paintcv, pointsPixelMap, oneUnitPix, zeroShift, shiftPix);
    curvesCount++;
}

QMap<qint64, double> liveGraph::pointsToPixelMap(const QMap<QDateTime, double> &points) const
{ //карта позиций времени по шкале x кадра в пикселях
    QMap<qint64, double> pointsPixelMap;
    const qint64 base = points.firstKey().toMSecsSinceEpoch();
    for (const QDateTime &t : points.keys()) {
        pointsPixelMap.insert((t.toMSecsSinceEpoch() - base) / onePixelTime, points.value(t));
    }
    return pointsPixelMap;
}

double liveGraph::yPixel(double value, double oneUnitPix, double zeroShift) const
{ //вертикальная координата значения с учётом смещений и поправки масштаба
    return (((value + zeroShift) * oneUnitPix) - vZeroLevel - scaleErrorPix) * -1;
}

void liveGraph::drawCurve(QPainter &painter, const QMap<qint64, double> &pointsPixelMap, double oneUnitPix, double zeroShift, int shiftPix)
{ //отрезки кривой и точки по карте пикселей
    const int x0 = oneCellXpix * verticalLineCount; //начало координат
    qint64 prevPixels = 0;
    int x = 0, oldX = 0;
    for (const qint64 &pixels : pointsPixelMap.keys()) {
        oldX = x;
        x = x0 - pixels; //x - позиция точки по времени
        if (oldX > 0 && ((x - oldX) * onePixelTime) < kMaxGapMs) { //при паузе больше 3с линию не рисуем
            painter.drawLine(oldX - shiftPix, yPixel(pointsPixelMap.value(prevPixels), oneUnitPix, zeroShift),
                             x - shiftPix - oneStepXpix, yPixel(pointsPixelMap.value(pixels), oneUnitPix, zeroShift));
            painter.drawEllipse(x - shiftPix - 2, yPixel(pointsPixelMap.value(prevPixels), oneUnitPix, zeroShift) - 2, 4, 4);
        }
        prevPixels = pixels;
    }
}

void liveGraph::timeNavigationScrollbarPositionChanged(int pos) // Пропорционально положению слайдера, нужно выбрать временные рамки для отрисовки
{
    if (!readFromFile) { //навигация имеет смысл только при чтении заранее считанного лога
        return;
    }
    const double proportion_slider = pos / double(kSliderMax); //вещественное деление, иначе pos/10000 всегда 0
    const qint64 frameMs = qint64(timeFrames) * 1000; //ширина кадра в мс
    //Левая граница навигации - конец первого кадра, а не начало координат: в самом левом
    //положении показывается первый кадр [beginTime, beginTime + frameMs], поэтому метка
    //конца кадра на минуту (frameMs) больше начала координат.
    const qint64 navSpan = qMax(qint64(0), betweenTime - frameMs);
    calculatedEndTime = beginTime.addMSecs(frameMs + qint64(navSpan * proportion_slider));
    navigationActive = true;
    if (ui->timeScrollBar->isSliderDown()) { //при удержании слайдера показываем метку конца кадра
        showNavigationTimeLabel();
    }
    this->update();
}

void liveGraph::showNavigationTimeLabel()
{
    if (!readFromFile || !frameTimeLabel) {
        return;
    }
    const QDateTime frameEnd = calculatedEndTime.isValid() ? calculatedEndTime : lastTime;
    frameTimeLabel->setText(frameEnd.toString("hh:mm:ss.zzz"));
    frameTimeLabel->adjustSize();
    const QRect groove = ui->timeScrollBar->geometry();
    const double span = ui->timeScrollBar->maximum() - ui->timeScrollBar->minimum();
    const double frac = span > 0 ? (ui->timeScrollBar->value() - ui->timeScrollBar->minimum()) / span : 0.0;
    int x = groove.left() + int(frac * groove.width()) - frameTimeLabel->width() / 2;
    x = qBound(0, x, this->width() - frameTimeLabel->width());
    int y = groove.top() - frameTimeLabel->height() - 2;
    if (y < 0) { //если сверху нет места - показываем снизу
        y = groove.bottom() + 2;
    }
    frameTimeLabel->move(x, y);
    frameTimeLabel->raise();
    frameTimeLabel->show();
}

void liveGraph::hideNavigationTimeLabel()
{
    if (frameTimeLabel) {
        frameTimeLabel->hide();
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
        paintan.drawRect(1, 1, rectXSizePix.at(0) +15, oneStringYpix * graphAnnotation.size() + 3);
        paintan.setOpacity(1.0);
        for (int i = 0, y = 4; i < graphAnnotation.size(); ++i, y += oneStringYpix) {
            paintColor.setNamedColor(annotationKeys.at(i));
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
    QMap<QDateTime, double> splittedPoints;
    //QMapIterator<QDateTime, double> pointsIt(points);
    //Вычисляем время начала отрисовки, отнимая ширину фрейма в секундах от последнего времени в массиве точек
    QDateTime firstPointForDraw = QDateTime::fromMSecsSinceEpoch((timeMarker.toMSecsSinceEpoch()) - (timeFrames * 1000));
    //Теперь надо собрать массив точек для данного конкретного временного отрезка
    /*------------------------------------------------------*/
    QMap<QDateTime, double>::iterator it_lower = points.lowerBound(firstPointForDraw);
    QMap<QDateTime, double>::iterator it_upper = points.upperBound(timeMarker);
    for (auto it = it_lower; it != it_upper; ++it) {
        splittedPoints.insert(it.key(), it.value());
    }
    /*------------------------------------------------------*/
    /*pointsIt.toFront(); //переводим итератор в конец большого массива всех точек кривой
    while (pointsIt.peekPrevious().key() >= firstPointForDraw)
    { //двигаем итератор к точке ближайшей к началу требуемого временного промежутка
        pointsIt.previous();
    }
    while (pointsIt.hasNext())
    { //заполняем массив точками в временных рамках от firstPointForDraw до timeMarker
        auto item = pointsIt.next();
        splittedPoints.insert(item.key(), item.value());
    }*/
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
        delete graphListIt.next();
    }
    waitFirstData = true;
    ui->timeScrollBar->setValue(ui->timeScrollBar->maximum()); //возвращаемся к концу лога
    navigationActive = false;
}
