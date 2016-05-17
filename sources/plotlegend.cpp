#include "headers/plotlegend.h"

PlotLegend::PlotLegend(QWidget *parent):
    QwtLegend(parent)
{
    QwtDynGridLayout *tl = qobject_cast<QwtDynGridLayout *>(
        contentsWidget()->layout());//d_data->view->contentsWidget->layout() );
    if ( tl ) {
        tl->setAlignment(Qt::AlignLeft);
        //tl->setContentsMargins(10,10,10,0);
        tl->setExpandingDirections(Qt::Horizontal);
    }
}

PlotLegend::~PlotLegend() {}
