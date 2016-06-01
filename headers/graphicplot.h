#ifndef GRAPHICPLOT_H
#define GRAPHICPLOT_H

#include <QWidget>
#include <QTimer>
#include <QTime>

#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_text_label.h>

#include "plotgrid.h"

class GraphicPlot : public QwtPlot
{
    Q_OBJECT
public:
    explicit    GraphicPlot(QString title,
                            int channelsCount,
                            QWidget *parent = 0);
    virtual     ~GraphicPlot();

    void        setDisplayedPoints(int displayedSize,
                                   qint8 mode,
                                   int realSize);

    void        setPoint(const double/*F64*/ &voltage_0,
                         const double &voltage_1);
    void        setPoint(unsigned short/*U16*/ samples);

    void        setBlock(double/*F64*/ *voltageBuffer, int size);
    void        setBlock(unsigned short/*U16*/ *samples, int size);
    void        setChannels(bool ch1, bool ch2);

    double      *initializeChannelZeroBuffer(unsigned int size);
    double      *initializeChannelOneBuffer(unsigned int size);

    void        displayBlock();

    void        zoomAxis(Axis ax, int value);
    void        setCurveProperties(qint8 channel, QPen pen);
    void        setColor_0(QColor color);
    void        setColor_1(QColor color);
    void        setLineStyle_0(Qt::PenStyle style);
    void        setLineStyle_1(Qt::PenStyle style);
    void        setLineWidth_0(int width);
    void        setLineWidth_1(int width);

    void        rescaleAxis(Axis axis, double minimum, double maximum);
    void        measuringStopped();

private:
    void        _initialize(int channelsCount);
    QString     _getVoltagePrefix(double value);

private slots:
    void        _plotPanned(int x, int y);
    void        _scaleTimerTimeout();

private:
    QwtPlotCurve*       _curveZero;
    QwtPlotCurve*       _curveOne;

    int                 _count;
    int                 _displayedPoints;
    int                 _initializedPoints;
    double              _scaleMinimum;
    double              _scaleMaximum;

    QVector<QPointF>    _pointsZero;
    QVector<QPointF>    _pointsOne;

    QPointF             ch0Point;
    QPointF             ch1Point;

    bool                _channelZeroEnabled;
    bool                _channelOneEnabled;

    double*             _channelZeroVoltageBuffer;
    double*             _channelOneVoltageBuffer;

    int                 _lastZoom_0;
    int                 _lastZoom_1;

    PlotGrid*           _grid;
    QColor              _channelColor_0;
    QColor              _channelColor_1;

    QwtTextLabel*       _channelOutput_0;
    QwtTextLabel*       _channelOutput_1;

    QTimer*             _scaleTimer;
    QTime               _scaleTime;
};

#endif // GRAPHICPLOT_H
