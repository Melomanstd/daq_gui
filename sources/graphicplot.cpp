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
        _scaleMinimum(-0.5),
        _scaleMaximum(0.5),
        _channelZeroEnabled(false),
        _channelOneEnabled(false),
        _channelZeroSamplesBuffer(0),
        _channelOneSamplesBuffer(0)
{
    setAxisScale(QwtPlot::yLeft, _scaleMinimum, _scaleMaximum);
    setAxisScale(QwtPlot::yRight, _scaleMinimum, _scaleMaximum);
    setAxisAutoScale(QwtPlot::yLeft, true);
    setAxisAutoScale(QwtPlot::yRight, true);
    setCanvasBackground(Qt::white);

//    insertLegend(new QwtLegend);
    QwtPlotGrid *g = new QwtPlotGrid();
    g->setMajPen(QPen(Qt::gray, 2));
    g->attach(this);

    _curveZero = new QwtPlotCurve();
    _curveZero->setTitle(tr("Channel 0"));
    _curveZero->setPen(QPen(Qt::blue, 6));
    _curveZero->setRenderHint(QwtPlotItem::RenderAntialiased, true);
    QwtSymbol *simba = new QwtSymbol(QwtSymbol::Ellipse,
                                     QBrush(Qt::yellow),
                                     QPen(Qt::red),
                                     QSize(8,8));
    _curveZero->setSymbol(simba);
    _curveZero->attach(this);

    _curveOne = new QwtPlotCurve();
    _curveOne->setTitle(tr("Channel 1"));
    _curveOne->setPen(QPen(Qt::darkCyan, 6));
    _curveOne->setRenderHint(QwtPlotItem::RenderAntialiased, true);
    simba = new QwtSymbol(QwtSymbol::Ellipse,
                          QBrush(Qt::yellow),
                          QPen(Qt::red),
                          QSize(8,8));
    _curveOne->setSymbol(simba);
    _curveOne->setYAxis(QwtPlot::yRight);
    _curveOne->attach(this);



//    _curve->setXAxis(QwtPlot::xTop);

//    QwtPlotZoomer *zoomer = new QwtPlotZoomer(canvas());
//    zoomer->setTrackerMode(QwtPlotZoomer::AlwaysOff);

    QwtPlotMagnifier *magnifier = new QwtPlotMagnifier(canvas());
    magnifier->setMouseButton(Qt::RightButton);

    QwtPlotPanner *panner = new QwtPlotPanner(canvas());
    panner->setMouseButton(Qt::MidButton);

//    setAutoReplot(true);
//    setAxisMaxMajor(QwtPlot::xBottom, 1);
//    setAxisMaxMinor(QwtPlot::xBottom, 6);
}

GraphicPlot::~GraphicPlot()
{
    if (_channelZeroSamplesBuffer != 0)
    {
        delete [] _channelZeroSamplesBuffer;
    }
    if (_channelOneSamplesBuffer != 0)
    {
        delete [] _channelOneSamplesBuffer;
    }
}

void GraphicPlot::setPoint(const double &voltage_0,
                           const double &voltage_1)
{
    if (_channelZeroEnabled == true)
    {
        ch0Point.setX(_count);
        ch0Point.setY(voltage_0);
        _pointsZero.append(ch0Point);
        _curveZero->setSamples(_pointsZero);
    }
    if (_channelOneEnabled == true)
    {
        ch1Point.setX(_count);
        ch1Point.setY(voltage_1);
        _pointsOne.append(ch1Point);
        _curveOne->setSamples(_pointsOne);
    }

    ++_count;

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
        if (_channelZeroEnabled)
        {
            _pointsZero.pop_front();
        }
        if (_channelOneEnabled)
        {
            _pointsOne.pop_front();
        }

        setAxisScale(QwtPlot::xBottom,
                     _count - _displayedPoints,
                     _count - 1,
                     _displayStep);

//        setAxisScale(QwtPlot::yLeft,
//                     _points.first().y(),
//                     voltage);
    }

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
        _pointsZero.clear();
        _pointsOne.clear();
    }

//    _points.resize(size);
    _pointsZero.reserve(size);
    _pointsOne.reserve(size);
}

void GraphicPlot::setDisplayStep(int step)
{
    _displayStep = step;
    _currentStep = step;
}

void GraphicPlot::setChannels(bool ch1, bool ch2)
{
    _channelOneEnabled = ch2;
    _channelZeroEnabled = ch1;
}

unsigned short *GraphicPlot::initializeChannelZeroBuffer(
        unsigned int size)
{
    if (_channelZeroSamplesBuffer != 0)
    {
        delete [] _channelZeroSamplesBuffer;
    }

    _channelZeroSamplesBuffer = new unsigned short[size];
    return _channelZeroSamplesBuffer;
}

unsigned short *GraphicPlot::initializeChannelOneBuffer(
        unsigned int size)
{
    if (_channelOneSamplesBuffer != 0)
    {
        delete [] _channelOneSamplesBuffer;
    }

    _channelOneSamplesBuffer = new unsigned short[size];
    return _channelOneSamplesBuffer;
}
