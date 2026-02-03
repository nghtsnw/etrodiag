#ifndef LIVEGRAPH_H
#define LIVEGRAPH_H

#include <QWidget>
#include <QPainter>
#include <QTimer>
#include <QMap>
#include "global.h"
#include "qdatetime.h"

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
    QMap<QString, QVector<double >> graphAnnotationMinMax;
    void incomingDataSlot(QDateTime currentTime, s_parameterMask data);
    void chngMinMaxVisible();
    void cleanGraph();

private:
    Ui::liveGraph *ui;
    QTimer *timer = new QTimer(this); //таймер сек для сдвига ячеек и перерисовки графика
    int xShift = 0; //индекс сдвига ячеек разметки поля
    void shiftCells();
    void paintCurve(QMap<QDateTime, double> points, QString color);
    void paintAnnotation();
    QVector<int> maxStringSizePix(QFont font, QList<QString> str);
    double xShiftPix = 0;
    double pictWidth = 0;
    double pictHeight = 0;
    int verticalLineCount = 0;
    int horizontalLineCount = 0;
    double oneCellXpix = 0;
    double oneCellYpix = 0.0;
    double oneStepXpix = 0;
    double vZeroLevel = 0;
    double scaleErrorPix = 0.0;
    QVector<double> findDeltaValue(QMap<QDateTime, double> &points);
    double findYScale(const QVector<double> &values);
    bool foundFlag = false;
    QList<QString> annotationKeys;
    QVector<int> rectXSizePix;
    int curvesCount = 0;
    bool minMaxOnOff = true;

    QMap<QDateTime, double> pointsForTimeFrames(QMap<QDateTime, double>& points); //буфер для точек в отрезке времени размере кадра
    int timeFrames = 60; // ширина графика в секундах (менять для увеличения и уменьшения общего масштаба)
    QDateTime frameFront; // передний край графика (либо сдвигается таймером по времени в live режиме, либо последняя запись из файла лога)

    const int oneStepTime = 500;//время для таймера сдвига на шаг и перерисовки (мсек)
    const int steps = 120; //ширина графика в шагах

protected:
    void mouseEvent(QMouseEvent *event);
    void paintEvent(QPaintEvent *event) override;

signals:
    void repaintCurves();
    void data2graph(int devNum, int byteNum, int id, double endValue, int steps, QString drawGraphColor, QDateTime currentTime);
};

#endif // LIVEGRAPH_H
