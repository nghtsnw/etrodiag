#ifndef LIVEGRAPH_H
#define LIVEGRAPH_H

#include <QWidget>
#include <QPainter>
#include <QTimer>
#include <QMap>

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
    QMap<QString, QString> graphAnnotation;
    void incomingDataSlot(int devNum, QString devName, int byteNum, QString byteName, uint32_t wordData, int id, QString parameterName, int binRawValue, double endValue, bool viewInLogFlag, bool isNewData, bool drawGraphFlag, QString drawGraphColor);

private:
    Ui::liveGraph *ui;
    QTimer *timer = new QTimer(this); //таймер сек для сдвига ячеек и перерисовки графика
    int xShift = 0; //индекс сдвига ячеек разметки поля
    void shiftCells();
    void paintCurve(QVector<double> points, QString color);
    void paintAnnotation();
    QVector<int> maxStringSizePix(QFont font, QList<QString> str);
    double xShiftPix = 0;
    int pictWidth = 0;
    int pictHeight = 0;
    int verticalLineCount = 0;
    int horizontalLineCount = 0;
    double oneCellXpix = 0;
    double oneCellYpix = 0;
    double oneStepXpix = 0;
    double vZeroLevel = 0;
    double scaleErrorPix = 0;
    QVector<double> findDeltaValue(QVector<double> &points);
    double findYScale(QVector<double> values);
    bool foundFlag = false;
    QList<QString> annotationKeys;

    const int oneStepTime = 1000;//время для таймера сдвига на шаг и перерисовки (мсек)
    const int steps = 60; //ширина графика в шагах

protected:
    void paintEvent(QPaintEvent *event) override;

signals:
    void repaintCurves();
    void data2graph(int devNum, int byteNum, int id, double endValue, int steps, QString drawGraphColor);
};

#endif // LIVEGRAPH_H
