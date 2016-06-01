/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "headers/plotgrid.h"

#include "qwt_painter.h"
#include "qwt_text.h"
#include "qwt_scale_map.h"
#include "qwt_scale_div.h"
#include "qwt_math.h"
#include <qpainter.h>
#include <qpen.h>

#define BUFFER_SIZE 11

//! Enables major grid, disables minor grid
PlotGrid::PlotGrid()
    :   QwtPlotGrid(  ),
        _yRightScale(0),
        _drawLeftScale(0),
        _drawRightScale(0),
        _useTimeValues(false),
        _transformScaleValue(false),
        _timescaleDivider(1)
{
    cleanTime();
}

//! Destructor
PlotGrid::~PlotGrid()
{
    if (_yRightScale != 0)
    {
        delete _yRightScale;
        _yRightScale = 0;
    }
}

/*!
  \brief Draw the grid

  The grid is drawn into the bounding rectangle such that
  gridlines begin and end at the rectangle's borders. The X and Y
  maps are used to map the scale divisions into the drawing region
  screen.
  \param painter  Painter
  \param xMap X axis map
  \param yMap Y axis
  \param canvasRect Contents rect of the plot canvas
*/
void PlotGrid::draw( QPainter *painter,
    const QwtScaleMap &xMap, const QwtScaleMap &yMap,
    const QRectF &canvasRect ) const
{
    bool xEnabled=this->xEnabled();
    bool yEnabled=this->yEnabled();
    bool xMinEnabled=this->xMinEnabled();
    bool yMinEnabled=this->yMinEnabled();

    QwtScaleDiv xScaleDiv=this->xScaleDiv();
    QwtScaleDiv yScaleDiv=this->yScaleDiv();

    QPen majPen=this->majPen();
    QPen minPen=this->minPen();

    minPen.setCapStyle( Qt::FlatCap );

    painter->setPen( minPen );

    if ( xEnabled && xMinEnabled )
    {
        drawLines( painter, canvasRect, Qt::Vertical, xMap,
            xScaleDiv.ticks( QwtScaleDiv::MinorTick ) );
        drawLines( painter, canvasRect, Qt::Vertical, xMap,
            xScaleDiv.ticks( QwtScaleDiv::MediumTick ) );
    }

    if ( yEnabled && yMinEnabled )
    {
        drawLines( painter, canvasRect, Qt::Horizontal, yMap,
            yScaleDiv.ticks( QwtScaleDiv::MinorTick ) );
        drawLines( painter, canvasRect, Qt::Horizontal, yMap,
            yScaleDiv.ticks( QwtScaleDiv::MediumTick ) );
    }

    majPen.setCapStyle( Qt::FlatCap );

    painter->setPen( majPen );

    if ( xEnabled )
    {
        drawLines( painter, canvasRect, Qt::Vertical, xMap,
            xScaleDiv.ticks( QwtScaleDiv::MajorTick ) );
    }

    if ( yEnabled )
    {
        drawLines( painter, canvasRect, Qt::Horizontal, yMap,
            yScaleDiv.ticks( QwtScaleDiv::MajorTick ) );
    }
}

void PlotGrid::drawLines( QPainter *painter, const QRectF &canvasRect,
    Qt::Orientation orientation, const QwtScaleMap &scaleMap,
    const QList<double> &values ) const
{
    const double x1 = canvasRect.left();
    const double x2 = canvasRect.right() - 1.0;
    const double y1 = canvasRect.top();
    const double y2 = canvasRect.bottom() - 1.0;

    const bool doAlign = QwtPainter::roundingAlignment( painter );

    for ( int i = 0; i < values.count(); i++ )
    {
        double scaleValue = 0.0;

        double value = scaleMap.transform( values[i] );
        if ( doAlign )
            value = qRound( value );

        if ( orientation == Qt::Horizontal )
        {
            scaleValue = values[i];
            if ( qwtFuzzyGreaterOrEqual( value, y1 ) &&
                qwtFuzzyLessOrEqual( value, y2 ) )
            {
                QwtPainter::drawLine( painter, x1, value, x2, value );
                painter->save();
                painter->setBackground(QBrush(Qt::black));
                painter->setBackgroundMode(Qt::OpaqueMode);
                painter->setPen(Qt::white);
                painter->setOpacity(0.5);// прозрачность
                QFont font = painter->font();
                font.setPointSize(12);
                font.setBold(true);
                painter->setFont(font);
                if ((i != 0) && (i != (values.count() - 1)) &&
                        (_drawLeftScale == true))
                {
                    QwtPainter::drawText(painter,
                                         x1+5,
                                         value - 10,
                                         100,
                                         20,
                                         Qt::AlignLeft,
                                         QString::number(scaleValue));
                }

                if ((i != 0) && (i != (values.count() - 1)) &&
                        (_yRightScale != 0) && (_drawRightScale == true))
                {
                    QList<double> yRValues = _yRightScale->ticks(
                                QwtScaleDiv::MajorTick);

                    QwtPainter::drawText(painter,
                                         (x2)-100,//*90,
                                         value - 10,
                                         100,
                                         20,
                                         Qt::AlignRight,
                                         QString::number(yRValues[i]));
                }
                painter->restore();
            }
        }
        else
        {
            scaleValue = /*qRound*/(values[i] * 4.0 / _timescaleDivider);

            if ( qwtFuzzyGreaterOrEqual( value, x1 ) &&
                qwtFuzzyLessOrEqual( value, x2 ) )
            {
                QwtPainter::drawLine( painter, value, y1, value, y2 );
                painter->save();
                painter->setBackground(QBrush(Qt::black));
                painter->setBackgroundMode(Qt::OpaqueMode);
                painter->setPen(Qt::white);
                painter->setOpacity(0.5);// прозрачность
                QFont font = painter->font();
                font.setPointSize(12);
                font.setBold(true);
                painter->setFont(font);

                if (_useTimeValues == false)
                {
                    if (i != 0 && i != values.count() - 1)
                    {
                        QwtPainter::drawText( painter,
                                              value-50,
                                              y2-20,
                                              100,
                                              20,
                                              Qt::AlignHCenter,
                                              QString::number(scaleValue));
                    }
                    else if (i == 0)
                    {
                        QwtPainter::drawText( painter,
                                              value-30,
                                              y2-20,
                                              100,
                                              20,
                                              Qt::AlignHCenter,
                                              QString::number(scaleValue));
                    }
                }
                else
                {
                    if (i != 0 && i != values.count() - 1)
                    {
                        QwtPainter::drawText( painter,
                                              value-50,
                                              y2-20,
                                              100,
                                              20,
                                              Qt::AlignHCenter,
                                              _stringTime[i]);
    //                                          QString::number(scaleValue));
                    }
                    else if (i == 0)
                    {
                        QwtPainter::drawText( painter,
                                              value-30,
                                              y2-20,
                                              100,
                                              20,
                                              Qt::AlignHCenter,
                                              _stringTime[i]);
    //                                          QString::number(scaleValue));
                    }
                }
                painter->restore();
            }
        }

    }

}

void PlotGrid::setRightScaleDivider(QwtScaleDiv &div)
{
    if (_yRightScale == 0)
    {
        _yRightScale = new QwtScaleDiv();
    }

    *_yRightScale = div;
}

void PlotGrid::drawLeftScale(bool draw)
{
    _drawLeftScale = draw;
}

void PlotGrid::drawRightScale(bool draw)
{
    _drawRightScale = draw;
}

void PlotGrid::cleanTime()
{
    _stringTime.clear();
    _stringTime.reserve(BUFFER_SIZE);

    for (int i = 0; i < BUFFER_SIZE; i++)
    {
        _stringTime.append("");
    }
    _useTimeValues = false;
    _transformScaleValue = false;
    _timescaleDivider = 1;
}

void PlotGrid::updateTime(double time)
{
    _stringTime.pop_back();
    _stringTime.push_front(QString::number(time));
}

void PlotGrid::usingTimeValues(bool state)
{
    _useTimeValues = state;
}

void PlotGrid::trannsformScaleValue(bool state, int divider)
{
    _transformScaleValue = state;
    _timescaleDivider = divider;
}
