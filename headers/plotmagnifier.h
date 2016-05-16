#ifndef PLOTMAGNIFIER_H
#define PLOTMAGNIFIER_H

#include <qwt_plot_magnifier.h>


class PlotMagnifier : public QwtPlotMagnifier
{
public:
    explicit PlotMagnifier(QwtPlotCanvas *canvas);

protected:
    virtual void rescale(double factor);
};

#endif // PLOTMAGNIFIER_H
