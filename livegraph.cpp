#include "livegraph.h"
#include "ui_livegraph.h"

liveGraph::liveGraph(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::liveGraph)
{
    ui->setupUi(this);
    connect (timer, &QTimer::timeout, this, &liveGraph::shiftCells);
    timer->start(1000);
}

liveGraph::~liveGraph()
{
    delete ui;
}

void liveGraph::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    initGraph();
}

void liveGraph::initGraph()
{
    int pictH = this->size().width();//ширина
    int pictV = this->size().height();//высота
    QPainter paint(this); // и пэинтер
    if (!paint.isActive()) paint.begin(this); // запускаем отрисовку
    paint.eraseRect(0,0,pictH,pictV); // очищаем рисунок

    paint.setBrush(QBrush(Qt::white));
    paint.drawRect(0,0,pictH,pictV);
    paint.setPen(Qt::gray);

    int vLineCount = 20;//количество линий по количеству ячеек
    int hLineCount = 10;
    int oneCellXpix = pictH/vLineCount; //определяем габариты ячеек
    int oneCellYpix = pictV/hLineCount;
    if (xShift == 1) xShiftPix = oneCellXpix/3; //для текущего вызова функции определяем сдвиг с которым рисуем вертикальные линии
    else if (xShift == 2) xShiftPix = (oneCellXpix/3)*2;
    else xShiftPix = 0;
    for (int i = hLineCount, vCoord = 0; i > 0; --i) //рисуем горизонтальные линии
    {
        paint.drawLine(0,vCoord,pictH,vCoord);
        vCoord+=oneCellYpix;
    }
    for (int i = vLineCount+1, hCoord = xShiftPix*-1; i > 0; --i) //рисуем вертикальные линии
    {
        paint.drawLine(hCoord,0,hCoord,pictV);
        hCoord+=oneCellXpix;
    }

    paint.setPen(Qt::blue);//рисуем рамки
    paint.drawLine(0,0,0,pictV);
    paint.drawLine(0,pictV-1,pictH,pictV-1);
    paint.drawLine(pictH-1, pictV, pictH-1, 0);
    paint.drawLine(pictH-1,0,0,0);
}

void liveGraph::shiftCells()
{
    if (xShift !=2) xShift++;
    else xShift = 0;
    this->update();
}
