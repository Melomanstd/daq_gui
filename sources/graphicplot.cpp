#include "headers/graphicplot.h"
#include "headers/defines.h"
#include "headers/plotgrid.h"
#include "headers/plotlegend.h"
#include "headers/plotmagnifier.h"

#include <qwt_plot_grid.h>
#include <qwt_symbol.h>
#include <qwt_legend.h>
#include <qwt_plot_magnifier.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_zoomer.h>
#include <qwt_scale_div.h>
#include <qwt_plot_layout.h>



GraphicPlot::GraphicPlot(QWidget *parent)
    :   QwtPlot(parent),
        _count(0),
        _displayedPoints(1),
        _initializedPoints(0),
        _displayStep(1),
        _currentStep(0),
        _scaleMinimum(0.0),
        _scaleMaximum(0.0),
        _channelZeroEnabled(false),
        _channelOneEnabled(false),
        _channelZeroVoltageBuffer(0),
        _channelOneVoltageBuffer(0),
        _lastZoom_0(0),
        _lastZoom_1(0)
{
    setAxisScale(QwtPlot::yLeft, _scaleMinimum, _scaleMaximum);
    setAxisScale(QwtPlot::yRight, _scaleMinimum, _scaleMaximum);

//    setCanvasBackground(Qt::white);

    canvas()->setAttribute(Qt::WA_PaintOutsidePaintEvent, true);

    if (canvas()->testPaintAttribute(QwtPlotCanvas::BackingStore) == true)
    {
        canvas()->setAttribute(Qt::WA_PaintOnScreen, true);
        canvas()->setAttribute(Qt::WA_NoSystemBackground, true);
    }

    QPalette pal = canvas()->palette();
    QLinearGradient gradient(0.0, 0.0, 1.0, 1.0);
    gradient.setCoordinateMode(QLinearGradient::StretchToDeviceMode);
    gradient.setColorAt(0.0, QColor(0, 49, 110));
    gradient.setColorAt(1.0, QColor(0, 87, 174));
    pal.setBrush(QPalette::Window, QBrush(gradient));
    canvas()->setPalette(pal);

    enableAxis(yLeft, false);
    enableAxis(yRight, false);
    enableAxis(xBottom, false);

    _grid = new PlotGrid();
    _grid->setMajPen(QPen(Qt::gray, 1, Qt::DotLine));

    _grid->attach(this);

    _curveZero = new QwtPlotCurve();
    _curveZero->setTitle(tr("Channel 0"));
    _curveZero->setPen(QPen(Qt::blue, 6, Qt::DashLine));
    _curveZero->setRenderHint(QwtPlotItem::RenderAntialiased, true);
    _curveZero->attach(this);

    _curveOne = new QwtPlotCurve();
    _curveOne->setTitle(tr("Channel 1"));
    _curveOne->setPen(QPen(Qt::darkCyan, 6, Qt::DotLine));
    _curveOne->setRenderHint(QwtPlotItem::RenderAntialiased, true);
    _curveOne->setYAxis(QwtPlot::yRight);
    _curveOne->attach(this);

//    _curve->setXAxis(QwtPlot::xTop);

//    QwtPlotZoomer *zoomer = new QwtPlotZoomer(canvas());
//    zoomer->setTrackerMode(QwtPlotZoomer::AlwaysOff);

    PlotMagnifier *magnifier = new PlotMagnifier(canvas());
    magnifier->setMouseButton(Qt::NoButton);
    magnifier->setWheelFactor(0.8);

    QwtPlotPanner *panner = new QwtPlotPanner(canvas());
    panner->setMouseButton(Qt::LeftButton);
    connect(panner, SIGNAL(panned(int,int)),
            this, SLOT(_plotPanned(int,int)));

    _channelOutput_0 = new QwtTextLabel(QwtText(tr("Channel 1 Voltage: 0")));
    _channelOutput_1 = new QwtTextLabel(QwtText(tr("Channel 2 Voltage: 0")));

    QFont font = _channelOutput_0->font();
    font.setPointSize(14);
    font.setBold(true);
    _channelOutput_0->setFont(font);
    _channelOutput_1->setFont(font);

    PlotLegend *legend = new PlotLegend();
    legend->setAutoFillBackground(true);
//    plotLayout()->setSpacing(0);
//    legend->setPalette(canvas()->palette());
    legend->insert(_curveZero, _channelOutput_0);
//    legend->insert(_grid, new QSpacerItem(40,20,
//                                   QSizePolicy::Maximum,
//                                   QSizePolicy::Maximum));
    legend->insert(_curveOne, _channelOutput_1);
    insertLegend(legend,QwtPlot::BottomLegend);




    replot();

    rescaleAxis(xBottom, 0, 100);
}

GraphicPlot::~GraphicPlot()
{
    if (_channelZeroVoltageBuffer != 0)
    {
        delete [] _channelZeroVoltageBuffer;
    }
    if (_channelOneVoltageBuffer != 0)
    {
        delete [] _channelOneVoltageBuffer;
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
        _channelOutput_0->setText(tr("Channle 1 Voltage: ") +
                                     QString::number(voltage_0));
    }
    if (_channelOneEnabled == true)
    {
        ch1Point.setX(_count);
        ch1Point.setY(voltage_1);
        _pointsOne.append(ch1Point);
        _curveOne->setSamples(_pointsOne);
        _channelOutput_1->setText(tr("Channle 2 Voltage: ") +
                                     QString::number(voltage_1));
    }

    ++_count;

    if (_initializedPoints < _displayedPoints)
    {
        _initializedPoints++;
//        _rescaleAxis(xBottom, 0, _initializedPoints - 1);
        setAxisScale(xBottom,
                     0,
                     _initializedPoints - 1,
                     _displayStep);
    }
    else
    {
        if ((_channelZeroEnabled == true) &&
                (_pointsZero.size() >= _initializedPoints))
        {
            _pointsZero.pop_front();
        }
        if ((_channelOneEnabled == true) &&
                (_pointsOne.size() >= _initializedPoints))
        {
            _pointsOne.pop_front();
        }

        rescaleAxis(xBottom,
                     _count - _displayedPoints,
                     _count - 1);
//        setAxisScale(QwtPlot::xBottom,
//                     _count - _displayedPoints,
//                     _count - 1,
//                     _displayStep);
    }

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

void GraphicPlot::setDisplayedPoints(int size, bool reset, qint8 mode)
{
    _displayedPoints = size;
    _grid->restartTime();

    if (reset == true)
    {
        _count = 0;
        _initializedPoints = 0;
        _pointsZero.clear();
        _pointsOne.clear();
    }

    if (mode == MODE_SINGLESHOT_MEASURING)
    {
        QwtSymbol *simba = new QwtSymbol(QwtSymbol::Ellipse,
                                         QBrush(Qt::yellow),
                                         QPen(Qt::red),
                                         QSize(10,10));
        _curveZero->setSymbol(simba);

        simba = new QwtSymbol(QwtSymbol::Rect,
                              QBrush(Qt::red),
                              QPen(Qt::darkCyan),
                              QSize(10,10));
        _curveOne->setSymbol(simba);

        setLineStyle_0(Qt::DashLine);
        setLineStyle_1(Qt::DotLine);
        setLineWidth_0(3);
        setLineWidth_1(3);

    } else if ((mode == MODE_BLOCK_MEASURING) ||
               (mode == MODE_HF_MEASURING))
    {
        _curveZero->setSymbol(0);
        _curveOne->setSymbol(0);

        setLineStyle_0(Qt::SolidLine);
        setLineStyle_1(Qt::SolidLine);
        setLineWidth_0(2);
        setLineWidth_1(2);
    }

//    _points.resize(size);
    _pointsZero.reserve(size);
    _pointsOne.reserve(size);
}

void GraphicPlot::setDisplayStep(int step)
{
    _grid->setStep(step);
    _displayStep = step;
    _currentStep = step;
}

void GraphicPlot::setChannels(bool ch1, bool ch2)
{
    _channelOneEnabled = ch2;
    _channelZeroEnabled = ch1;

    if (_channelZeroEnabled == true)
    {
        _curveZero->show();
    }
    else
    {
        _curveZero->hide();
        _pointsZero.clear();
        _curveZero->setSamples(_pointsZero);
    }

    if (_channelOneEnabled == true)
    {
        _curveOne->show();
    }
    else
    {
        _curveOne->hide();
        _pointsOne.clear();
        _curveOne->setSamples(_pointsOne);
    }

    _grid->drawLeftScale(ch1);
    _grid->drawRightScale(ch2);

    replot();
}

double *GraphicPlot::initializeChannelZeroBuffer(
        unsigned int size)
{
    if (_channelZeroVoltageBuffer != 0)
    {
        delete [] _channelZeroVoltageBuffer;
    }

    _channelZeroVoltageBuffer = new double[size];
    return _channelZeroVoltageBuffer;
}

double *GraphicPlot::initializeChannelOneBuffer(
        unsigned int size)
{
    if (_channelOneVoltageBuffer != 0)
    {
        delete [] _channelOneVoltageBuffer;
    }

    _channelOneVoltageBuffer = new double[size];
    return _channelOneVoltageBuffer;
}

void GraphicPlot::displayBlock()
{
    _pointsZero.clear();
    _pointsOne.clear();
    _count = 1;

    rescaleAxis(QwtPlot::xBottom,
                 _count-1,
                 _displayedPoints);

    for (int i = 0; i < _displayedPoints; i++)
    {
        if (_channelZeroEnabled == true)
        {
            ch0Point.setX(_count);
            ch0Point.setY(_channelZeroVoltageBuffer[i]);
            _pointsZero.append(ch0Point);
        }
        if (_channelOneEnabled == true)
        {
            ch1Point.setX(_count);
            ch1Point.setY(_channelOneVoltageBuffer[i]);
            _pointsOne.append(ch1Point);
        }

        ++_count;
    }

    _curveZero->setSamples(_pointsZero);
    _curveOne->setSamples(_pointsOne);
    replot();
}

void GraphicPlot::zoomAxis(Axis ax, int value)
{
    double zoomDiff = 0.0;

    if (ax == yLeft)
    {
        zoomDiff = (static_cast<double> (value) -
                    static_cast<double> (_lastZoom_0)) * 0.1;
        _lastZoom_0 = value;
    }
    else if (ax == yRight)
    {
        zoomDiff = (static_cast<double> (value) -
                    static_cast<double> (_lastZoom_1)) * 0.1;
        _lastZoom_1 = value;
    }

    _scaleMinimum = axisInterval(ax).minValue();
    _scaleMaximum = axisInterval(ax).maxValue();

    _scaleMinimum = _scaleMinimum - zoomDiff;
    _scaleMaximum = _scaleMaximum + zoomDiff;

    rescaleAxis(ax, _scaleMinimum, _scaleMaximum);
    replot();
}

void GraphicPlot::setCurveProperties(qint8 channel, QPen pen)
{
    if (channel == 0)
    {
        _curveZero->setPen(pen);
    }
    else if (channel == 1)
    {
        _curveOne->setPen(pen);
    }
}

void GraphicPlot::_plotPanned(int x, int y)
{
    _scaleMinimum = axisInterval(yLeft).minValue();
    _scaleMaximum = axisInterval(yLeft).maxValue();
    rescaleAxis(yLeft, _scaleMinimum, _scaleMaximum);
    _scaleMinimum = axisInterval(yRight).minValue();
    _scaleMaximum = axisInterval(yRight).maxValue();
    rescaleAxis(yRight, _scaleMinimum, _scaleMaximum);
    _scaleMinimum = axisInterval(xBottom).minValue();
    _scaleMaximum = axisInterval(xBottom).maxValue();
    rescaleAxis(xBottom, _scaleMinimum, _scaleMaximum);

//    setAxisScale(ax,_scaleMinimum, _scaleMaximum);
    replot();
}

void GraphicPlot::rescaleAxis(Axis axis,
                               double minimum,
                               double maximum)
{
    QwtScaleDiv *division = 0;
    double zoomDiff = maximum - minimum;
    zoomDiff = zoomDiff / 10.0;

    QList<double> ticks[QwtScaleDiv::NTickTypes];
    ticks[2].append(minimum);
    for (int i = 1; i < 10; i++)
    {
        ticks[2].append(minimum + (zoomDiff * i));
    }
    ticks[2].append(maximum);
    if (qAbs(minimum) == qAbs(maximum))
    {
        ticks[2][5] = 0.0;
    }
    division = new QwtScaleDiv(minimum, maximum, ticks);

    if (axis == yRight)
    {
        _grid->setRightScaleDivider(*division);
    }

    /*if (axis == xBottom)
    {
        for (int i = 0; i < ticks[2].size(); i++)
        {
            ticks[2][i] = qRound(ticks[2][i]);
        }
    }*/

    setAxisScaleDiv(axis, *division);
    delete division;
}

void GraphicPlot::setColor_0(QColor color)
{
    _channelColor_0 = color;
    QPen pen = _curveZero->pen();
    pen.setColor(color);
    _curveZero->setPen(pen);
}

void GraphicPlot::setColor_1(QColor color)
{
    _channelColor_1 = color;
    QPen pen = _curveOne->pen();
    pen.setColor(color);
    _curveOne->setPen(pen);
}

void GraphicPlot::setLineStyle_0(Qt::PenStyle style)
{
    QPen pen = _curveZero->pen();
    pen.setStyle(style);
    _curveZero->setPen(pen);
}

void GraphicPlot::setLineStyle_1(Qt::PenStyle style)
{
    QPen pen = _curveOne->pen();
    pen.setStyle(style);
    _curveOne->setPen(pen);
}

void GraphicPlot::setLineWidth_0(int width)
{
    QPen pen = _curveZero->pen();
    pen.setWidth(width);
    _curveZero->setPen(pen);
}

void GraphicPlot::setLineWidth_1(int width)
{
    QPen pen = _curveOne->pen();
    pen.setWidth(width);
    _curveOne->setPen(pen);
}
