#include "livegraph.h"
#include "qdatetime.h"
#include "ui_livegraph.h"
#include <newgraph.h>
#include <QApplication>
#include <QDebug>
#include <QMouseEvent>
#include <QScrollBar>
#include <QLabel>
#include <QCheckBox>

liveGraph::liveGraph(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::liveGraphWidget)
{
    ui->setupUi(this);
    timeNavigationScrollbarNewMaxLevel(kSliderMax); //диапазон навигации по логу
    ui->timeScrollBar->setSingleStep(10);
    ui->timeScrollBar->setPageStep(1000); //ручка ~10% ширины, иначе её не ухватить
    ui->timeScrollBar->setEnabled(false); //навигация по логу доступна только в режиме чтения файла
    ui->smoothCheckBox->setChecked(false); //сглаживание выключено при каждом запуске программы
    connect(ui->smoothCheckBox, &QCheckBox::toggled, this, [this](bool on) {
        smoothEnabled = on; //сглаживание только для отображения, данные и профиль не меняются
        this->update();
    });
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
        ui->timeScrollBar->setEnabled(r); //слайдер навигации активен только при чтении лога
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
    rowClickTimer->setSingleShot(true);
    rowClickTimer->setInterval(QApplication::doubleClickInterval());
    connect (rowClickTimer, &QTimer::timeout, this, &liveGraph::applyRowClick);
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
    setMouseTracking(true); //наведение мышью: вертикальная линия и подсказка значений
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
    paintHover(); //линия и окно значений рисуются поверх кривых и подписи
}

void liveGraph::initGraph()
{
    pictWidth = this->size().width();//ширина
    pictHeight = this->size().height() - ui->timeScrollBar->size().height();//высота
    QPainter paint(this);
    if (paint.isActive())
    {
        paint.setRenderHint(QPainter::Antialiasing, true); //сглаживание линий сетки
        paint.eraseRect(0, 0, pictWidth, pictHeight); // очищаем рисунок
        paint.setBrush(QBrush(Qt::white));
        paint.drawRect(0, 0, pictWidth, pictHeight);
        paint.setPen(Qt::lightGray);
        paint.setOpacity(0.5);
        verticalLineCount = steps / 25; //кол-во вертикальных линий рассчитывается по количеству шагов на кадр делённому на три, что-бы три шага соответствовало одной ячейке (для возможного масштабирования)
        horizontalLineCount = 10;
        onePixelTime = pictWidth > 0 ? double(timeFrames * 1000) / pictWidth : 0.0; //мс на пиксель, дробное
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
            removeAnnotation(curveKey(data.devNum, data.byteNum, data.id));
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

newgraph *liveGraph::findCurveByKey(const QString &key) const
{
    const QList<newgraph*> graphList = this->findChildren<newgraph*>();
    for (newgraph *graph : graphList) {
        if (curveKey(graph->devNum, graph->byteNum, graph->id) == key) {
            return graph;
        }
    }
    return nullptr;
}

QString liveGraph::curveKey(int devNum, int byteNum, int id)
{ //уникальный ключ кривой - именно он различает кривые, а не цвет (цвета могут повторяться)
    return QString("%1:%2:%3").arg(devNum).arg(byteNum).arg(id);
}

newgraph *liveGraph::createCurve(const s_parameterMask &data, const QDateTime &time)
{
    newgraph *graph = new newgraph(this);
    connect (this, &liveGraph::repaintCurves, graph, &newgraph::repaintThis);
    connect (graph, &newgraph::graph2Painter, this, [this, graph](QMap<QDateTime, double> points, QString color) {
        if (!graph->curveVisible) {
            return; //кривая скрыта кликом по её строке в подписи
        }
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
    graph->parameterName = data.parameterName; //для подсказки при наведении
    graph->devName = data.devName;
    annotationMinMax.insert(curveKey(data.devNum, data.byteNum, data.id), {data.endValue, data.endValue});
    emit data2graph(data.devNum, data.byteNum, data.id, data.endValue, steps, data.drawGraphColor, time);
    return graph;
}

void liveGraph::updateAnnotation(const s_parameterMask &data)
{
    const QString key = curveKey(data.devNum, data.byteNum, data.id); //ключ кривой, а не цвет: у разных кривых цвет может совпадать
    if (!annotationKeys.contains(key)) {
        annotationKeys.append(key);
    }
    annotationColor.insert(key, data.drawGraphColor);
    if (!annotationMinMax.contains(key)) { //нет записи (например после смены профиля) - создаём, иначе .at() выйдет за границы
        annotationMinMax.insert(key, {data.endValue, data.endValue});
    }
    if (minMaxOnOff)
    {
        QVector<double> minMax = annotationMinMax.value(key);
        if (minMax.at(0) > data.endValue) {
            minMax[0] = data.endValue;
        }
        if (minMax.at(1) < data.endValue) {
            minMax[1] = data.endValue;
        }
        annotationMinMax.insert(key, minMax);
        annotationText.insert(key, data.parameterName + '@' + data.devName + " - " + QString::number(data.endValue)
                              + "| Min - " + QString::number(minMax.at(0)) + "| Max - " + QString::number(minMax.at(1)));
    }
    else
    {
        annotationMinMax.insert(key, {data.endValue, data.endValue});
        annotationText.insert(key, data.parameterName + '@' + data.devName + " - " + QString::number(data.endValue));
    }
}

void liveGraph::removeAnnotation(const QString &key)
{
    annotationKeys.removeAll(key);
    annotationText.remove(key);
    annotationColor.remove(key);
    annotationMinMax.remove(key);
}

void liveGraph::clearAnnotations()
{
    annotationKeys.clear();
    annotationText.clear();
    annotationColor.clear();
    annotationMinMax.clear();
    annotationRows.clear();
}

void liveGraph::paintCurve(QMap<QDateTime, double> allPoints, QDateTime endTime, QString color) //приходит кривой endTime
{ //сюда каждый объект графика отдаёт массив данных и цвет на рисование
    QPainter paintcv(this);
    if (!paintcv.isActive()) {
        return;
    }
    paintcv.setRenderHint(QPainter::Antialiasing, true); //сглаживаем сами кривые
    QMap<QDateTime, double> points = pointsForTimeFrames(allPoints, endTime);
    if (points.isEmpty()) { //в выбранном временном окне нет точек
        return;
    }
    QMap<QDateTime, double> drawPoints = smoothEnabled ? smoothedPoints(points) : points; //сглаживание только для отображения
    QColor paintColor = QColor::fromString(color);
    paintcv.setBrush(QBrush(paintColor));
    paintcv.setPen(QPen(paintColor, kCurveLineWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

    const QVector<double> minMaxDeltaValue = findDeltaValue(drawPoints); //минимальное, максимальное и дельта между ними
    const double yScale = findYScale(minMaxDeltaValue); //высота шкалы Y, кратная 10
    const double zeroShift = (minMaxDeltaValue.at(0) < 0) ? minMaxDeltaValue.at(0) * -1 : 0.0; //смещение нуля при отрицательных значениях
    const double oneUnitPix = vZeroLevel / yScale; //цена одного деления в пикселях
    //Окно [endTime - timeFrames, endTime]: x считаем от его начала, поэтому кривая не дёргается,
    //когда из окна уходит самая старая точка.
    const QDateTime windowStart = endTime.addMSecs(-qint64(timeFrames) * 1000);

    drawCurve(paintcv, drawPoints, windowStart, oneUnitPix, zeroShift);
    curvesCount++;
}

QMap<QDateTime, double> liveGraph::smoothedPoints(const QMap<QDateTime, double> &points) const
{ //скользящее среднее по значениям (окно kSmoothWindow, крайние точки без заворота) - только для отображения
    const QVector<double> values = points.values();
    const QList<QDateTime> keys = points.keys();
    const int count = values.size();
    const int half = kSmoothWindow / 2;
    QMap<QDateTime, double> smoothed;
    for (int i = 0; i < count; ++i) {
        const int from = qMax(0, i - half);
        const int to = qMin(count - 1, i + half);
        double sum = 0.0;
        for (int j = from; j <= to; ++j) {
            sum += values.at(j);
        }
        smoothed.insert(keys.at(i), sum / double(to - from + 1));
    }
    return smoothed;
}

double liveGraph::xPixel(const QDateTime &time, const QDateTime &windowStart) const
{ //позиция точки по X: время от начала окна, в пикселях, с дробной точностью (без квантования по столбцам)
    if (onePixelTime <= 0.0) { //защита от деления на ноль при нулевой ширине виджета
        return 0.0;
    }
    return double(windowStart.msecsTo(time)) / onePixelTime;
}

double liveGraph::yPixel(double value, double oneUnitPix, double zeroShift) const
{ //вертикальная координата значения с учётом смещений и поправки масштаба
    return (((value + zeroShift) * oneUnitPix) - vZeroLevel - scaleErrorPix) * -1;
}

void liveGraph::drawCurve(QPainter &painter, const QMap<QDateTime, double> &points, const QDateTime &windowStart,
                          double oneUnitPix, double zeroShift)
{ //отрезки кривой и точки, x и y считаются с дробной точностью
    bool havePrevious = false;
    double previousX = 0;
    double previousY = 0;
    for (const QDateTime &t : points.keys()) {
        //время идёт слева направо: старые точки слева, новые справа, поэтому x растёт со временем
        const double x = xPixel(t, windowStart); //x - позиция точки по времени
        const double y = yPixel(points.value(t), oneUnitPix, zeroShift);
        if (havePrevious && (x - previousX) * onePixelTime < kMaxGapMs) { //при паузе больше 3с линию не рисуем
            //Соседние точки соединяем напрямую: сдвиг по x у прошлой версии оставлял разрывы у точек
            //QPointF сохраняет дробные координаты, иначе сглаженная линия "рвётся" на округлении
            painter.drawLine(QPointF(previousX, previousY), QPointF(x, y));
        }
        //Точки у плотного лога сливаются в толстую полосу, поэтому рисуем их только при заметном разносе
        if (!havePrevious || qAbs(x - previousX) >= kPointMinSpacing) {
            painter.drawEllipse(QPointF(x, y), kPointRadius, kPointRadius);
        }
        previousX = x;
        previousY = y;
        havePrevious = true;
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
    //Подписи по краям слайдера показывают границы видимого окна, а не весь лог целиком
    ui->leftTimeLabel->setText(calculatedEndTime.addMSecs(-frameMs).toString("hh:mm:ss"));
    ui->rightTimeLabel->setText(qMin(calculatedEndTime, lastTime).toString("hh:mm:ss"));
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


int liveGraph::annotationRowAt(const QPoint &pos) const
{ //индекс строки подписи под точкой, иначе -1
    for (int i = 0; i < annotationRows.size() && i < annotationKeys.size(); ++i) {
        if (annotationRows.at(i).contains(pos)) {
            return i;
        }
    }
    return -1;
}

void liveGraph::applyRowClick()
{ //одиночный клик по строке подписи скрывает/показывает кривую
    if (pendingKey.isEmpty()) {
        return;
    }
    newgraph *curve = findCurveByKey(pendingKey);
    pendingKey.clear();
    if (curve) {
        curve->curveVisible = !curve->curveVisible; //скрытая кривая не рисуется в graph2Painter
        this->update();
    }
}

void liveGraph::resetMinMax(const QString &key)
{ //двойной клик по строке подписи: Min/Max начинают накапливаться заново с текущего значения
    annotationMinMax.remove(key); //следующая точка снова инициализирует пару Min/Max
}

void liveGraph::mousePressEvent(QMouseEvent *event)
{ //клик по строке подписи скрывает/показывает кривую; ждём интервал двойного клика, чтобы не сработать раньше сброса
    if (event->button() != Qt::LeftButton) {
        QWidget::mousePressEvent(event);
        return;
    }
    const int row = annotationRowAt(event->pos());
    if (row < 0) {
        QWidget::mousePressEvent(event);
        return;
    }
    pendingKey = annotationKeys.at(row);
    rowClickTimer->start();
}

void liveGraph::mouseDoubleClickEvent(QMouseEvent *event)
{ //двойной клик по строке подписи сбрасывает Min/Max этой кривой
    if (event->button() != Qt::LeftButton) {
        QWidget::mouseDoubleClickEvent(event);
        return;
    }
    const int row = annotationRowAt(event->pos());
    if (row < 0) {
        QWidget::mouseDoubleClickEvent(event); //не наша строка - отдаём событие дальше (показ Min/Max)
        return;
    }
    rowClickTimer->stop();
    pendingKey.clear();
    resetMinMax(annotationKeys.at(row));
    this->update();
    //событие принято - MainWindow не переключает общий показ Min/Max
}

void liveGraph::paintAnnotation()
{
    QPainter paintan(this);
    if (paintan.isActive())
    {
        paintan.setRenderHint(QPainter::Antialiasing, true); //сглаживание маркеров подписи
        paintan.setRenderHint(QPainter::TextAntialiasing, true); //сглаживание текста, как для шрифтов
        QColor paintColor;
        QFont font("Times", 9);
        paintan.setFont(font);
        rectXSizePix = maxStringSizePix(font, annotationText.values());//[0] - длина строки, [1] - высота
        const int oneStringYpix = rectXSizePix.at(1) +2;
        paintan.setPen(Qt::white);
        paintan.setBrush(QBrush(Qt::white));
        paintan.setOpacity(0.7);
        paintan.drawRect(1, 1, rectXSizePix.at(0) +15, oneStringYpix * annotationKeys.size() + 3);
        paintan.setOpacity(1.0);
        annotationRows.clear();
        for (int i = 0, y = 4; i < annotationKeys.size(); ++i, y += oneStringYpix) {
            const QString key = annotationKeys.at(i);
            annotationRows.append(QRect(1, y, rectXSizePix.at(0) +15, oneStringYpix)); //запоминаем строку подписи под клик
            const newgraph *curve = findCurveByKey(key);
            const bool hidden = curve && !curve->curveVisible; //скрытая кривая в подписи приглушена
            paintColor = QColor::fromString(annotationColor.value(key, QStringLiteral("#000000")));
            paintan.setPen(hidden ? QColor(Qt::gray) : paintColor);
            paintan.setBrush(QBrush(hidden ? QColor(Qt::gray) : paintColor));
            paintan.drawEllipse(2, y, 8, 8);
            paintan.setPen(hidden ? QColor(Qt::gray) : QColor(Qt::black));
            paintan.setBrush(QBrush(hidden ? QColor(Qt::gray) : QColor(Qt::black)));
            paintan.drawText(12, y + 10, annotationText.value(key));
        }
    }
    curvesCount = 0;
}

void liveGraph::mouseMoveEvent(QMouseEvent *event)
{ //курсор над графиком: запоминаем позицию и перерисовываем линию с подсказкой
    hoverPos = event->pos();
    hoverActive = true;
    this->update();
}

void liveGraph::leaveEvent(QEvent *event)
{
    hoverActive = false;
    this->update();
    QWidget::leaveEvent(event);
}

bool liveGraph::curveValueAt(const QMap<QDateTime, double> &points, const QDateTime &time, double &value) const
{ //значение кривой в точке времени: линейная интерполяция между соседними отсчётами
    if (points.isEmpty()) {
        return false;
    }
    auto upper = points.lowerBound(time); //первый отсчёт с меткой >= time
    if (upper == points.begin() || upper == points.end()) {
        return false; //точка вне диапазона данных этой кривой
    }
    auto lower = upper;
    --lower;
    const qint64 span = lower.key().msecsTo(upper.key());
    if (span <= 0 || span > kMaxGapMs) {
        return false; //в разрыве данных линии нет, значит и значения нет
    }
    const double fraction = double(lower.key().msecsTo(time)) / double(span);
    value = lower.value() + (upper.value() - lower.value()) * fraction;
    return true;
}

void liveGraph::paintHover()
{ //чёрная вертикальная полоса на всю высоту графика и окно со значениями кривых под курсором
    if (!hoverActive || !calculatedEndTime.isValid()) {
        return;
    }
    const int cursorX = hoverPos.x();
    const int cursorY = hoverPos.y();
    if (cursorX < 0 || cursorX > pictWidth || cursorY < 0 || cursorY >= pictHeight) {
        return; //курсор вне поля графика (например над полосой прокрутки)
    }
    QPainter paint(this);
    if (!paint.isActive()) {
        return;
    }
    paint.setRenderHint(QPainter::Antialiasing, true);
    paint.setPen(QPen(Qt::black, 1));
    paint.drawLine(cursorX, 0, cursorX, pictHeight); //полоса на весь график

    const QDateTime windowStart = calculatedEndTime.addMSecs(-qint64(timeFrames) * 1000);
    const QDateTime cursorTime = windowStart.addMSecs(qint64(cursorX * onePixelTime)); //время под курсором

    QFont font("Times", 9);
    paint.setFont(font);
    QList<QString> rows;
    QList<QString> rowColors;
    rows.append(cursorTime.toString("hh:mm:ss.zzz")); //метка времени положения курсора
    rowColors.append(QString());
    for (const QString &key : annotationKeys) {
        newgraph *curve = findCurveByKey(key);
        if (!curve || !curve->curveVisible) {
            continue; //скрытые кривые не попадают в подсказку, как и в подпись
        }
        double value = 0.0;
        if (!curveValueAt(curve->points(), cursorTime, value)) {
            continue; //в этой точке у кривой нет данных
        }
        rows.append(curve->parameterName + '@' + curve->devName + ": " + QString::number(value, 'g', 6));
        rowColors.append(annotationColor.value(key, QStringLiteral("#000000")));
    }

    QFontMetrics metrics(font);
    int textWidth = 0;
    for (const QString &row : std::as_const(rows)) {
        textWidth = qMax(textWidth, metrics.horizontalAdvance(row));
    }
    const int lineHeight = metrics.height() + 2;
    const int boxWidth = textWidth + 26;
    const int boxHeight = lineHeight * rows.size() + 6;
    int boxX = cursorX + 12;
    int boxY = cursorY + 12;
    if (boxX + boxWidth > this->width()) { //у правого края окно уходит влево от курсора
        boxX = cursorX - boxWidth - 12;
    }
    boxX = qBound(0, boxX, qMax(0, this->width() - boxWidth));
    if (boxY + boxHeight > pictHeight) { //у нижнего края окно поднимается вверх
        boxY = cursorY - boxHeight - 12;
    }
    boxY = qBound(0, boxY, qMax(0, pictHeight - boxHeight));

    paint.setPen(QPen(Qt::gray, 1));
    paint.setBrush(QColor(255, 255, 255, 235));
    paint.drawRect(boxX, boxY, boxWidth, boxHeight);
    for (int i = 0; i < rows.size(); ++i) {
        const int rowY = boxY + 4 + i * lineHeight;
        if (!rowColors.at(i).isEmpty()) {
            const QColor marker = QColor::fromString(rowColors.at(i));
            paint.setPen(marker);
            paint.setBrush(marker);
            paint.drawEllipse(boxX + 6, rowY + 2, 8, 8);
        }
        paint.setPen(Qt::black);
        paint.setBrush(Qt::black);
        paint.drawText(boxX + 18, rowY + lineHeight - 4, rows.at(i));
    }
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
    clearAnnotations();
    pendingKey.clear();
    while (graphListIt.hasNext())
    {
        delete graphListIt.next();
    }
    waitFirstData = true;
    ui->timeScrollBar->setValue(ui->timeScrollBar->maximum()); //возвращаемся к концу лога
    navigationActive = false;
}
