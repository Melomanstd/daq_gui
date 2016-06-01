#include "headers/plotlegend.h"

PlotLegend::PlotLegend(QWidget *parent):
    QwtLegend(parent)
{
    QwtDynGridLayout *tl = qobject_cast<QwtDynGridLayout *>(
        contentsWidget()->layout());
    if ( tl ) {
        tl->setAlignment(Qt::AlignLeft);
        tl->setExpandingDirections(Qt::Horizontal);
    }
}

PlotLegend::~PlotLegend() {}
