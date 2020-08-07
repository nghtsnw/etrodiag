#ifndef LIVEGRAPH_H
#define LIVEGRAPH_H

#include <QWidget>
#include <QPainter>

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
    //QPixmap *graph = new QPixmap();

protected:
    void paintEvent(QPaintEvent *event) override;
};

#endif // LIVEGRAPH_H
