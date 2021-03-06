#include "headers/graphicplot.h"
#include "headers/defines.h"
#include "headers/plotgrid.h"
#include "headers/plotlegend.h"
#include "headers/plotmagnifier.h"
#include "headers/scaledraw.h"

#include <qwt_plot_grid.h>
#include <qwt_symbol.h>
#include <qwt_legend.h>
#include <qwt_plot_magnifier.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_zoomer.h>
#include <qwt_scale_div.h>
#include <qwt_plot_layout.h>

GraphicPlot::GraphicPlot(QString title , int channelsCount, QWidget *parent)
    :   QwtPlot(parent),
        _curveZero(0),
        _curveOne(0),
        _count(0),
        _displayedPoints(1),
        _initializedPoints(0),
        _scaleMinimum(-1.0),
        _scaleMaximum(1.0),
        _channelZeroEnabled(false),
        _channelOneEnabled(false),
        _channelZeroVoltageBuffer(0),
        _channelOneVoltageBuffer(0),
        _lastZoom_0(0),
        _lastZoom_1(0)
{
    _initialize(channelsCount);
}

GraphicPlot::~GraphicPlot()
{
    disconnect(_scaleTimer, SIGNAL(timeout()),
               this, SLOT(_scaleTimerTimeout()));
    _scaleTimer->stop();
    delete _scaleTimer;
    _scaleTimer = 0;

    if (_channelZeroVoltageBuffer != 0)
    {
        delete [] _channelZeroVoltageBuffer;
    }
    if (_channelOneVoltageBuffer != 0)
    {
        delete [] _channelOneVoltageBuffer;
    }
}

void GraphicPlot::_initialize(int channelsCount)
{
    setAutoReplot(false);
    _scaleTimer = new QTimer;
    _scaleTimer->setInterval(100);
    connect(_scaleTimer, SIGNAL(timeout()),
            this, SLOT(_scaleTimerTimeout()));

    setAxisScale(QwtPlot::yLeft, _scaleMinimum, _scaleMaximum);
    setAxisScale(QwtPlot::yRight, _scaleMinimum, _scaleMaximum);

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
    setAxisScaleDraw(xBottom, new ScaleDraw());
    setAxisTitle(xBottom, tr("Timescale"));

    _grid = new PlotGrid();
    _grid->setMajPen(QPen(Qt::gray, 1, Qt::DotLine));

    _grid->attach(this);

    QFont font = QwtTextLabel().font();
    font.setPointSize(14);
    font.setBold(true);

    if (channelsCount > 0)
    {
        _curveZero = new QwtPlotCurve();
        _curveZero->setTitle(tr("Signal 1"));
        _curveZero->setPen(QPen(Qt::blue, 6, Qt::DashLine));
        _curveZero->setRenderHint(QwtPlotItem::RenderAntialiased, true);
        _curveZero->attach(this);

        QString text = _curveZero->title().text() + tr(" Voltage: 0");
        _channelOutput_0 = new QwtTextLabel(text);
//        _channelOutput_0->setMinimumWidth(400);
//        _channelOutput_0->setMaximumWidth(400);
        _channelOutput_0->setFont(font);
    }

    if (channelsCount > 1)
    {
        _curveOne = new QwtPlotCurve();
        _curveOne->setTitle(tr("Signal 2"));
        _curveOne->setPen(QPen(Qt::darkCyan, 6, Qt::DotLine));
        _curveOne->setRenderHint(QwtPlotItem::RenderAntialiased, true);
        _curveOne->setYAxis(QwtPlot::yRight);
        _curveOne->attach(this);

        QString text = _curveOne->title().text() + tr(" Voltage: 0");
        _channelOutput_1 = new QwtTextLabel(text);
//        _channelOutput_1->setMinimumWidth(400);
//        _channelOutput_1->setMaximumWidth(400);
        _channelOutput_1->setFont(font);
    }
    else
    {
        _curveZero->setTitle(tr("Signal 3"));
        QString text = _curveZero->title().text() + tr(" Voltage: 0");
        _channelOutput_0->setText(text);
    }

    PlotLegend *legend = new PlotLegend();
    legend->setAutoFillBackground(true);

    if (_curveZero != 0)
        legend->insert(_curveZero, _channelOutput_0);
    if (_curveOne != 0)
        legend->insert(_curveOne, _channelOutput_1);
    insertLegend(legend,QwtPlot::TopLegend);

//    QwtPlotZoomer *zoomer = new QwtPlotZoomer(canvas());
//    zoomer->setTrackerMode(QwtPlotZoomer::AlwaysOff);

    PlotMagnifier *magnifier = new PlotMagnifier(canvas());
    magnifier->setMouseButton(Qt::NoButton);
    magnifier->setWheelFactor(0.8);

    QwtPlotPanner *panner = new QwtPlotPanner(canvas());
    panner->setMouseButton(Qt::LeftButton);
    connect(panner, SIGNAL(panned(int,int)),
            this, SLOT(_plotPanned(int,int)));

    replot();

    rescaleAxis(xBottom, 0, 100);
}

void GraphicPlot::setPoint(const double &voltage_0,
                           const double &voltage_1)
{
    if ((_channelZeroEnabled == true) &&
            (_curveZero != 0))
    {
        ch0Point.setX(_count);
        ch0Point.setY(voltage_0);
        _pointsZero.append(ch0Point);
        _curveZero->setSamples(_pointsZero);
        _channelOutput_0->setText(tr("Signal 1 Voltage: ") +
                                     QString::number(voltage_0) +
                                     _getVoltagePrefix(voltage_0));
    }
    if ((_channelOneEnabled == true) &&
            (_curveOne != 0))
    {
        ch1Point.setX(_count);
        ch1Point.setY(voltage_1);
        _pointsOne.append(ch1Point);
        _curveOne->setSamples(_pointsOne);
        _channelOutput_1->setText(tr("Signal 2 Voltage: ") +
                                     QString::number(voltage_1) +
                                     _getVoltagePrefix(voltage_1));
    }

    ++_count;

    if (_initializedPoints < _displayedPoints)
    {
        _initializedPoints++;
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
                     _count);
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

void GraphicPlot::setDisplayedPoints(int displayedSize,
                                     qint8 mode,
                                     int realSize)
{
    _displayedPoints = displayedSize;
    _grid->cleanTime();
    int time = realSize * 4;
    QSize pointSize(10, 10);

    //reset
    _count = 0;
    _initializedPoints = 0;
    _pointsZero.clear();
    _pointsOne.clear();

    if (mode == MODE_SINGLESHOT_MEASURING)
    {
        QwtSymbol *simba = new QwtSymbol(QwtSymbol::Ellipse,
                                         QBrush(Qt::yellow),
                                         QPen(Qt::red),
                                         pointSize);

//        if (_curveZero != 0)
//            _curveZero->setSymbol(simba);

        simba = new QwtSymbol(QwtSymbol::Rect,
                              QBrush(Qt::red),
                              QPen(Qt::darkCyan),
                              pointSize);
//        if (_curveOne != 0)
//            _curveOne->setSymbol(simba);

        setLineStyle_0(Qt::DashLine);
        setLineStyle_1(Qt::DotLine);
        setLineWidth_0(3);
        setLineWidth_1(3);

        _grid->usingTimeValues(true);
        _scaleTimer->start();
        _scaleTime.restart();
        rescaleAxis(xBottom, 0, displayedSize);
        setAxisTitle(xBottom, tr("Seconds"));
        _grid->trannsformScaleValue(false, 1);

    } else if ((mode == MODE_BLOCK_MEASURING) ||
               (mode == MODE_HF_MEASURING))
    {
        if (_curveZero != 0)
        {
            _curveZero->setSymbol(0);
            setLineStyle_0(Qt::SolidLine);
            setLineWidth_0(2);
        }
        if (_curveOne != 0)
        {
            _curveOne->setSymbol(0);
            setLineStyle_1(Qt::SolidLine);
            setLineWidth_1(2);
        }

        if (time < 1000)
        {
            setAxisTitle(xBottom, tr("Micro Seconds"));
            _grid->trannsformScaleValue(true, 1);
        }
        else if ((time >= 1000) && (time < 1000000))
        {
            setAxisTitle(xBottom, tr("Milli Seconds"));
            _grid->trannsformScaleValue(true, 1000);
        }
        else //size > 1 000 000
        {
            setAxisTitle(xBottom, tr("Seconds"));
            _grid->trannsformScaleValue(true, 1000000);
        }

        rescaleAxis(QwtPlot::xTop, displayedSize, 0);
        rescaleAxis(QwtPlot::xBottom, realSize, 0);
        _curveZero->setXAxis(QwtPlot::xTop);
    }

    _pointsZero.reserve(displayedSize);
    _pointsOne.reserve(displayedSize);
}

void GraphicPlot::setChannels(bool ch1, bool ch2)
{
    _channelOneEnabled = ch2;
    _channelZeroEnabled = ch1;

    if (_curveZero != 0)
    {
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
    }

    if (_curveOne != 0)
    {
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
    _count = 0;
    double value = 0.0;

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

    if ((_curveZero != 0) && (_channelZeroEnabled == true))
    {
        value = _channelZeroVoltageBuffer[_displayedPoints -1];

        _curveZero->setSamples(_pointsZero);
        QString text =  _curveZero->title().text() +
                        tr(" Voltage: ") +
                        QString::number(value) +
                        _getVoltagePrefix(value);
        _channelOutput_0->setText(text);
    }
    if ((_curveOne != 0) && (_channelOneEnabled == true))
    {
        value = _channelOneVoltageBuffer[_displayedPoints-1];

        _curveOne->setSamples(_pointsOne);
        QString text =  _curveOne->title().text() +
                        tr(" Voltage: ") +
                        QString::number(value) +
                        _getVoltagePrefix(value);
        _channelOutput_1->setText(text);
    }
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
    if ((channel == 0) && (_curveZero != 0))
    {
        _curveZero->setPen(pen);
    }
    else if ((channel == 1) && (_curveOne != 0))
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
    _scaleMinimum = axisInterval(xTop).minValue();
    _scaleMaximum = axisInterval(xTop).maxValue();
    rescaleAxis(xTop, _scaleMinimum, _scaleMaximum);

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

    setAxisScaleDiv(axis, *division);
    delete division;
}

void GraphicPlot::setColor_0(QColor color)
{
    if (_curveZero == 0)
    {
        return;
    }

    _channelColor_0 = color;
    QPen pen = _curveZero->pen();
    pen.setColor(color);
    _curveZero->setPen(pen);
}

void GraphicPlot::setColor_1(QColor color)
{
    if (_curveOne == 0)
    {
        return;
    }

    _channelColor_1 = color;
    QPen pen = _curveOne->pen();
    pen.setColor(color);
    _curveOne->setPen(pen);
}

void GraphicPlot::setLineStyle_0(Qt::PenStyle style)
{
    if (_curveZero == 0)
    {
        return;
    }

    QPen pen = _curveZero->pen();
    pen.setStyle(style);
    _curveZero->setPen(pen);
}

void GraphicPlot::setLineStyle_1(Qt::PenStyle style)
{
    if (_curveOne == 0)
    {
        return;
    }

    QPen pen = _curveOne->pen();
    pen.setStyle(style);
    _curveOne->setPen(pen);
}

void GraphicPlot::setLineWidth_0(int width)
{
    if (_curveZero == 0)
    {
        return;
    }

    QPen pen = _curveZero->pen();
    pen.setWidth(width);
    _curveZero->setPen(pen);
}

void GraphicPlot::setLineWidth_1(int width)
{
    if (_curveOne == 0)
    {
        return;
    }

    QPen pen = _curveOne->pen();
    pen.setWidth(width);
    _curveOne->setPen(pen);
}

void GraphicPlot::_scaleTimerTimeout()
{
    _grid->updateTime(static_cast<double> (_scaleTime.elapsed() / 1000.0));
}


void GraphicPlot::measuringStopped()
{
    _scaleTimer->stop();
    _grid->usingTimeValues(false);
}

QString GraphicPlot::_getVoltagePrefix(double value)
{
    if ((value >= 1) || (value <= -1))
        return tr(" V");
    if ((value >= 0.001) && ((value < 1)))
         return tr(" mV");
    if ((value <= -0.001) && (value > -1))
        return tr(" mV");
//    else if ((value < 0.001) && (value >))
    return tr(" mkV");
}
