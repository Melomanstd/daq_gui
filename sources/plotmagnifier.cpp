#include "headers/plotmagnifier.h"
#include "headers/graphicplot.h"

PlotMagnifier::PlotMagnifier(QwtPlotCanvas *canvas)
    :   QwtPlotMagnifier(canvas)
{

}

void PlotMagnifier::rescale(double factor)
{
    factor = qAbs( factor );
    if ( factor == 1.0 || factor == 0.0 )
        return;

    bool doReplot = false;
    GraphicPlot* plt = dynamic_cast<GraphicPlot*> (plot());

    const bool autoReplot = plt->autoReplot();
    plt->setAutoReplot( false );

    for ( int axisId = 0; axisId < QwtPlot::axisCnt; axisId++ )
    {
        if (axisId == QwtPlot::xBottom)
        {
            continue;
        }


        const QwtScaleDiv *scaleDiv = plt->axisScaleDiv( axisId );
        if ( isAxisEnabled( axisId ) && scaleDiv->isValid() )
        {
            const double center =
                scaleDiv->lowerBound() + scaleDiv->range() / 2;
            const double width_2 = scaleDiv->range() / 2 * factor;

//            plt->setAxisScale( axisId, center - width_2, center + width_2 );
            plt->rescaleAxis(static_cast<QwtPlot::Axis> (axisId),
                             center - width_2,
                             center + width_2);
            doReplot = true;
        }
    }

    plt->setAutoReplot( autoReplot );

    if ( doReplot )
        plt->replot();
}
