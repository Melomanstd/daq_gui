#ifndef GRAPHICPLOT_H
#define GRAPHICPLOT_H

#include <QWidget>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>

class GraphicPlot : public QwtPlot
{
    Q_OBJECT
public:
    explicit    GraphicPlot(QWidget *parent = 0);
    virtual     ~GraphicPlot();

    void        setDisplayedPoints(int size, bool reset);
    void        setDisplayStep(int step);

    void        setPoint(const double/*F64*/ &voltage_0,
                         const double &voltage_1);
    void        setPoint(unsigned short/*U16*/ samples);

    void        setBlock(double/*F64*/ *voltageBuffer, int size);
    void        setBlock(unsigned short/*U16*/ *samples, int size);
    void        setChannels(bool ch1, bool ch2);

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
};

#endif // GRAPHICPLOT_H
