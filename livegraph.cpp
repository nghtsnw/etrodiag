#include "livegraph.h"
#include "ui_livegraph.h"

liveGraph::liveGraph(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::liveGraph)
{
    ui->setupUi(this);
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
    paint.begin(this); // запускаем отрисовку
    paint.eraseRect(0,0,pictH,pictV); // очищаем рисунок

    paint.setBrush(QBrush(Qt::white));
    paint.drawRect(0,0,pictH,pictV);
    paint.setPen(Qt::gray);

    int vLineCount = 20;
    int hLineCount = 10;
    //int frameSizeSecond = 60;
    int oneCellXpix = pictH/vLineCount; //рисуем сетку 10Х20 клеток
    int oneCellYpix = pictV/hLineCount;
    for (int i = hLineCount, vCoord = 0; i > 0; --i) //рисуем горизонтальные линии
    {
        paint.drawLine(0,vCoord,pictH,vCoord);
        vCoord+=oneCellYpix;
    }
    for (int i = vLineCount, hCoord = 0; i > 0; --i) //рисуем вертикальные линии
    {
        paint.drawLine(hCoord,0,hCoord,pictV);
        hCoord+=oneCellXpix;
    }

    paint.setPen(Qt::red);//рисуем рамки
    paint.drawLine(0,0,0,pictV);
    paint.drawLine(0,pictV-1,pictH,pictV-1);
    paint.drawLine(pictH-1, pictV, pictH-1, 0);
    paint.drawLine(pictH-1,0,0,0);
}
