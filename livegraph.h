#ifndef LIVEGRAPH_H
#define LIVEGRAPH_H

#include <QWidget>
#include <QPainter>
#include <QTimer>
#include <QMap>
#include <QLabel>
#include "global.h"
#include <QDateTime>

namespace Ui {
    class liveGraphWidget;
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
    Ui::liveGraphWidget *ui;
    QTimer *timer = new QTimer(this); //таймер сек для сдвига ячеек и перерисовки графика
    int xShift = 0; //индекс сдвига ячеек разметки поля
    void shiftCells();
    void paintCurve(QMap<QDateTime, double> points, QDateTime endTime, QString color);
    void paintAnnotation();
    QVector<int> maxStringSizePix(QFont font, QList<QString> str);
    double xShiftPix = 0;
    int pictWidth = 0;
    int pictHeight = 0;
    int verticalLineCount = 0;
    int horizontalLineCount = 0;
    qint64 onePixelTime = 0;
    int oneCellXpix = 0;
    int oneCellYpix = 0;
    int oneStepXpix = 0;
    double vZeroLevel = 0;
    double scaleErrorPix = 0.0;
    QVector<double> findDeltaValue(QMap<QDateTime, double> &points);
    double findYScale(const QVector<double> &values);
    bool foundFlag = false;
    QList<QString> annotationKeys;
    QVector<int> rectXSizePix;
    int curvesCount = 0;
    bool minMaxOnOff = true;
    bool navigationActive = false; //true когда конец графика задан слайдером навигации по логу

private slots:
    void timeNavigationScrollbarPositionChanged(int pos);
    void timeNavigationScrollbarNewMaxLevel(int max);
    void showNavigationTimeLabel();
    void hideNavigationTimeLabel();

private:

    QMap<QDateTime, double> pointsForTimeFrames(QMap<QDateTime, double>& points, QDateTime timeMarker); //буфер для точек в отрезке времени размере кадра
    int timeFrames = 60; // ширина графика в секундах (менять для увеличения и уменьшения общего масштаба)
    QDateTime frameFront; // передний край графика (либо сдвигается таймером по времени в live режиме, либо последняя запись из файла лога)
    QDateTime calculatedEndTime; // время конца нарисованного графика пропорционально положению слайдера навигации
    QDateTime beginTime; // начало отсчёта для нового соединения или начальная метка из файла
    QDateTime realTime;
    QDateTime lastTime; // последняя принятая временная метка
    qint64 betweenTime = 0; // разница между begin и real

    bool waitFirstData = false;
    bool readFromFile = false;
    QLabel *frameTimeLabel = nullptr; //метка конца кадра, показывается над слайдером при перетаскивании

    const int oneStepTime = 100;//время для таймера сдвига на шаг и перерисовки (мсек)
    const int steps = 300; //ширина графика в шагах

protected:
    void mouseEvent(QMouseEvent *event);
    void paintEvent(QPaintEvent *event) override;

signals:
    void repaintCurves();
    void data2graph(int devNum, int byteNum, int id, double endValue, int steps, QString drawGraphColor, QDateTime currentTime);
    void startOscillator();
    void stopOscillator();
    void readFromFileSignal(bool);
};

#endif // LIVEGRAPH_H
