#include "headers/graphicplot.h"

#include <qwt_plot_grid.h>
#include <qwt_symbol.h>
#include <qwt_legend.h>
#include <qwt_plot_magnifier.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_zoomer.h>

GraphicPlot::GraphicPlot(QWidget *parent)
    :   QwtPlot(parent),
        _count(0),
        _displayedPoints(1),
        _initializedPoints(0),
        _displayStep(1),
        _currentStep(0),
        _scaleMinimum(-1.0),
        _scaleMaximum(1.0)
{
    setAxisTitle(QwtPlot::yLeft, "Y");
    setAxisTitle(QwtPlot::xBottom, "X");
//    setAxisScale(QwtPlot::yLeft, -1, 1);
    setCanvasBackground(Qt::white);

    insertLegend(new QwtLegend);

    QwtPlotGrid *g = new QwtPlotGrid();
    g->setMajPen(QPen(Qt::gray, 2));
    g->attach(this);

    _curve = new QwtPlotCurve();
    _curve->setTitle(tr("Channel 0"));
    _curve->setPen(QPen(Qt::blue, 6));
    _curve->setRenderHint(QwtPlotItem::RenderAntialiased, true);

    QwtSymbol *simba = new QwtSymbol(QwtSymbol::Ellipse,
                                     QBrush(Qt::yellow),
                                     QPen(Qt::red),
                                     QSize(8,8));
    _curve->setSymbol(simba);

//    _curve->setXAxis(QwtPlot::xTop);

    _curve->attach(this);

//    QwtPlotZoomer *zoomer = new QwtPlotZoomer(canvas());
//    zoomer->setTrackerMode(QwtPlotZoomer::AlwaysOff);

    QwtPlotMagnifier *magnifier = new QwtPlotMagnifier(canvas());
    magnifier->setMouseButton(Qt::RightButton);

    QwtPlotPanner *panner = new QwtPlotPanner(canvas());
    panner->setMouseButton(Qt::LeftButton);
}

GraphicPlot::~GraphicPlot()
{
    //
}

void GraphicPlot::setPoint(double voltage)
{
    QPointF point;

    point.setX(_count++);
    point.setY(voltage);

//    if (--_currentStep <= 0)
//    {
//        _currentStep = _displayStep;
//        _scaleMaximum = _count / _displayStep;
//        if ((_scaleMaximum - _scaleMinimum) > 10)
//        {
//            ++_scaleMinimum;
//            setAxisScale(QwtPlot::xBottom,
//                                 _scaleMinimum,
//                                 _scaleMaximum,
//                                 1);
//        }
//    }
    /*if (voltage < _scaleMinimum)
    {
        while (voltage < _scaleMinimum)
        {
            _scaleMinimum -= 0.25;
        }
        setAxisScale(QwtPlot::yLeft, _scaleMinimum, _scaleMaximum);
    }

    if (voltage > _scaleMaximum)
    {
        while (voltage > _scaleMaximum)
        {
            _scaleMaximum += 0.25;
        }
        setAxisScale(QwtPlot::yLeft, _scaleMinimum, _scaleMaximum);
    }*/

    if (_initializedPoints < _displayedPoints)
    {
        _initializedPoints++;
        setAxisScale(QwtPlot::xBottom,
                     0,
                     _initializedPoints - 1,
                     _displayStep);
    }
    else
    {
        _points.pop_front();

        setAxisScale(QwtPlot::xBottom,
                     _count - _displayedPoints,
                     _count - 1,
                     _displayStep);

//        setAxisScale(QwtPlot::yLeft,
//                     _points.first().y(),
//                     voltage);
    }
    _points.append(point);

    _curve->setSamples(_points);
    /*
    _points.append(QPointF(++_count, voltage));
    _curve->setSamples(_points.toVector());
    */



    replot();
}

void GraphicPlot::setPoint(unsigned short samples)
{

}

void GraphicPlot::setBlock(double *voltageBuffer, int size)
{

}

void GraphicPlot::setBlock(unsigned short *samples, int size)
{

}

void GraphicPlot::setDisplayedPoints(int size, bool reset)
{
    _displayedPoints = size;

    if (reset == true)
    {
        _count = 0;
        _initializedPoints = 0;
    }

//    _points.resize(size);
    _points.reserve(size);
}

void GraphicPlot::setDisplayStep(int step)
{
    _displayStep = step;
    _currentStep = step;
}
