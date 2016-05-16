#ifndef GRAPHICPLOT_H
#define GRAPHICPLOT_H

#include <QWidget>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_text_label.h>

#include "plotgrid.h"

class GraphicPlot : public QwtPlot
{
    Q_OBJECT
public:
    explicit    GraphicPlot(QString title, QWidget *parent = 0);
    virtual     ~GraphicPlot();

    void        setDisplayedPoints(int size, bool reset, qint8 mode);
    void        setDisplayStep(int step);

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

private slots:
    void        _plotPanned(int x, int y);

private:
    QwtPlotCurve*       _curveZero;
    QwtPlotCurve*       _curveOne;
//    QList<QPointF>      _points;

    int                 _count;
    int                 _displayedPoints;
    int                 _initializedPoints;
    int                 _displayStep;
    int                 _currentStep;
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
};

#endif // GRAPHICPLOT_H
