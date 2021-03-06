/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef PLOT_GRID_H
#define PLOT_GRID_H

#include "qwt_plot_grid.h"
#include <QTimer>

/*!
  \brief A class which draws a coordinate grid

  The QwtPlotGrid class can be used to draw a coordinate grid.
  A coordinate grid consists of major and minor vertical
  and horizontal gridlines. The locations of the gridlines
  are determined by the X and Y scale divisions which can
  be assigned with setXDiv() and setYDiv().
  The draw() member draws the grid within a bounding
  rectangle.
*/
class Plot;

class PlotGrid: public QwtPlotGrid
{
public:
    explicit     PlotGrid();
    virtual      ~PlotGrid();

    void         setRightScaleDivider(QwtScaleDiv &div);
    void         drawLeftScale(bool draw);
    void         drawRightScale(bool draw);

    virtual void draw( QPainter *p,
        const    QwtScaleMap &xMap, const QwtScaleMap &yMap,
        const    QRectF &rect ) const;
    void         cleanTime();
    void         updateTime(double time);
    void         usingTimeValues(bool state);
    void         trannsformScaleValue(bool state, int divider);

private:
    void drawLines( QPainter *painter, const QRectF &,
        Qt::Orientation orientation, const QwtScaleMap &,
        const QList<double> & ) const;

private:
    QwtScaleDiv*    _yRightScale;
    bool            _drawLeftScale;
    bool            _drawRightScale;

    bool            _useTimeValues;

    QList<QString>  _stringTime;
    bool            _transformScaleValue;
    int             _timescaleDivider;
};

#endif
