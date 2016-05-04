#ifndef GRAPHICPLOT_H
#define GRAPHICPLOT_H

#include <QWidget>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>

class GraphicPlot : public QwtPlot
{
public:
    explicit    GraphicPlot(QWidget *parent = 0);
    virtual     ~GraphicPlot();

    void        setDisplayedPoints(int size);

    void        setPoint(double/*F64*/ voltage);
    void        setPoint(unsigned short/*U16*/ samples);

    void        setBlock(double/*F64*/ *voltageBuffer, int size);
    void        setBlock(unsigned short/*U16*/ *samples, int size);

private:
    QwtPlotCurve*       _curve;
//    QList<QPointF>      _points;

    int                 _count;
    int                 _displayedPoints;
    int                 _initializedPoints;
    QVector<QPointF>    _points;
};

#endif // GRAPHICPLOT_H
