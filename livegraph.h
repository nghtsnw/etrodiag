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

class newgraph;

class liveGraph : public QWidget
{
    Q_OBJECT

public:
    explicit liveGraph(QWidget *parent = nullptr);
    ~liveGraph();
    void initGraph();
    void incomingDataSlot(QDateTime currentTime, s_parameterMask data);
    void chngMinMaxVisible();
    void cleanGraph();
    void clearAnnotations(); //сбросить подписи кривых

private:
    Ui::liveGraphWidget *ui;
    QTimer *timer = new QTimer(this); //таймер сек для сдвига ячеек и перерисовки графика
    QTimer *rowClickTimer = new QTimer(this); //отложенный одиночный клик по строке подписи (чтобы не мешать двойному)
    QString pendingKey; //ключ строки подписи, ожидающей одиночного клика
    int xShift = 0; //индекс сдвига ячеек разметки поля
    void shiftCells();
    void paintCurve(QMap<QDateTime, double> points, QDateTime endTime, QString color);
    QMap<QDateTime, double> smoothedPoints(const QMap<QDateTime, double> &points) const; //скользящее среднее для отображения
    double xPixel(const QDateTime &time, const QDateTime &windowStart) const; //координата X времени, с дробной точностью
    double yPixel(double value, double oneUnitPix, double zeroShift) const; //координата Y значения
    void drawCurve(QPainter &painter, const QMap<QDateTime, double> &points, const QDateTime &windowStart,
                   double oneUnitPix, double zeroShift); //отрисовка кривой
    void paintAnnotation();
    void paintHover(); //вертикальная линия и окно значений под курсором
    bool curveValueAt(const QMap<QDateTime, double> &points, const QDateTime &time, double &value) const; //значение кривой в точке времени
    QVector<int> maxStringSizePix(QFont font, QList<QString> str);
    double xShiftPix = 0;
    int pictWidth = 0;
    int pictHeight = 0;
    int verticalLineCount = 0;
    int horizontalLineCount = 0;
    double onePixelTime = 0.0; //мс в одном пикселе по X (дробное - время не квантуется по пикселям)
    int oneCellXpix = 0;
    int oneCellYpix = 0;
    int oneStepXpix = 0;
    double vZeroLevel = 0;
    double scaleErrorPix = 0.0;
    QVector<double> findDeltaValue(QMap<QDateTime, double> &points);
    double findYScale(const QVector<double> &values);
    newgraph *findCurve(const s_parameterMask &data); // график для этой маски
    newgraph *findCurveByKey(const QString &key) const; // график по ключу строки подписи
    static QString curveKey(int devNum, int byteNum, int id); // ключ кривой: devNum:byteNum:id
    int annotationRowAt(const QPoint &pos) const; // строка подписи под точкой или -1
    void applyRowClick(); // одиночный клик: скрыть/показать кривую
    void resetMinMax(const QString &key); // двойной клик: сброс Min/Max кривой
    void removeAnnotation(const QString &key); // убрать строку подписи удалённой кривой
    newgraph *createCurve(const s_parameterMask &data, const QDateTime &time);// создать/инициализировать график
    void updateAnnotation(const s_parameterMask &data); // обновить Min/Max и подпись
    QList<QString> annotationKeys; //ключи кривых в порядке появления
    QVector<QRect> annotationRows; //прямоугольники строк подписи: по ним клик скрывает/показывает кривую
    QMap<QString, QString> annotationText;  //ключ кривой -> строка подписи
    QMap<QString, QString> annotationColor; //ключ кривой -> цвет
    QMap<QString, QVector<double>> annotationMinMax; //ключ кривой -> {min, max}
    QVector<int> rectXSizePix;
    int curvesCount = 0;
    bool minMaxOnOff = true;
    bool smoothEnabled = false; //сглаживание отображения; по умолчанию выключено и не сохраняется в профиль
    bool navigationActive = false; //true когда конец графика задан слайдером навигации по логу
    QPoint hoverPos; //положение курсора над графиком
    bool hoverActive = false; //курсор сейчас над графиком - рисуем линию и подсказку

private slots:
    void timeNavigationScrollbarPositionChanged(int pos);
    void timeNavigationScrollbarNewMaxLevel(int max);
    void showNavigationTimeLabel();
    void hideNavigationTimeLabel();

private:

    QMap<QDateTime, double> pointsForTimeFrames(QMap<QDateTime, double>& points, QDateTime timeMarker); //буфер для точек в отрезке времени размере кадра
    static constexpr int kSliderMax = 10000; //диапазон слайдера навигации по логу
    static constexpr int kMaxGapMs = 3000; //разрыв линии графика при паузе данных, мс
    static constexpr int kCurveLineWidth = 2; //толщина линий графика, точек
    static constexpr qreal kPointRadius = 1.0; //радиус точки на кривой, точек
    static constexpr int kPointMinSpacing = 4; //точки рисуем, только если соседние разнесены по x, точек
    static constexpr int kSmoothWindow = 5; //окно скользящего среднего при включённом сглаживании, отсчётов
    int timeFrames = 60; // ширина графика в секундах (менять для увеличения и уменьшения общего масштаба)
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
    void mouseMoveEvent(QMouseEvent *event) override; //наведение: чёрная полоса и окно значений
    void leaveEvent(QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override; // клик по строке подписи скрывает/показывает кривую
    void mouseDoubleClickEvent(QMouseEvent *event) override; // двойной клик по строке подписи сбрасывает Min/Max кривой
    void paintEvent(QPaintEvent *event) override;

signals:
    void repaintCurves();
    void data2graph(int devNum, int byteNum, int id, double endValue, int steps, QString drawGraphColor, QDateTime currentTime);
    void startOscillator();
    void stopOscillator();
    void readFromFileSignal(bool);
};

#endif // LIVEGRAPH_H
