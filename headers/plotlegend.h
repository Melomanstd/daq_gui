#ifndef PLOTLEGEND_H
#define PLOTLEGEND_H
#include <qwt_legend.h>
#include <QLayout>
#include <qwt_dyngrid_layout.h>

class PlotLegend : public QwtLegend
{
public:
    explicit PlotLegend( QWidget *parent = NULL );
    virtual ~PlotLegend();

};

#endif // PLOTLEGEND_H
