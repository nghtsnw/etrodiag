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
    void incomingDataSlot(int devNum, QString devName, int byteNum, QString byteName, uint32_t wordData, int id, QString parameterName, int binRawValue, double endValue, bool viewInLogFlag, bool isNewData, bool drawGraphFlag, QString drawGraphColor);

private:
    Ui::liveGraph *ui;
    QTimer *timer = new QTimer(this); //таймер сек для сдвига ячеек и перерисовки графика
    int xShift = 0; //индекс сдвига ячеек разметки поля
    void shiftCells();
    void paintCurve(QVector<double> points, QString color);
    int xShiftPix;
    int pictWidth;
    int pictHeight;
    int verticalLineCount;
    int horizontalLineCount;
    int oneCellXpix;
    int oneCellYpix;
    QVector<double> findDeltaValue(QVector<double> &points);
    double findYScale(double delta_value);

    const int oneStepTime = 1000;//время для таймера сдвига на шаг и перерисовки (мсек)
    const int steps = 60; //ширина графика в шагах

protected:
    void paintEvent(QPaintEvent *event) override;

signals:
    void repaintCurves();
};

#endif // LIVEGRAPH_H
