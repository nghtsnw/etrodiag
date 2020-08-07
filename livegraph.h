#ifndef LIVEGRAPH_H
#define LIVEGRAPH_H

#include <QWidget>
#include <QPainter>
#include <QTimer>

namespace Ui {
class liveGraph;
}

class liveGraph : public QWidget
{
    Q_OBJECT

public:
    explicit liveGraph(QWidget *parent = nullptr);
    ~liveGraph();
    void initGraph();

private:
    Ui::liveGraph *ui;
    QTimer *timer = new QTimer(this); //таймер 1 сек для сдвига ячеек и перерисовки графика
    int xShift = 0; //индекс сдвига ячеек разметки поля
    void shiftCells();
    int xShiftPix;


protected:
    void paintEvent(QPaintEvent *event) override;
};

#endif // LIVEGRAPH_H
