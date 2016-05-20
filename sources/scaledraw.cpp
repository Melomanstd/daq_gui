#include "headers/scaledraw.h"

#include "qwt_scale_draw.h"
#include "qwt_scale_div.h"
#include "qwt_scale_map.h"
#include "qwt_math.h"
#include "qwt_painter.h"
#include <qpen.h>
#include <qpainter.h>
#include <qmath.h>


ScaleDraw::ScaleDraw()
    :   QwtScaleDraw()
{
    //
}

void ScaleDraw::drawLabel(QPainter *painter, double val) const
{
    return;
    QwtScaleDraw::drawLabel(painter, val);
}

void ScaleDraw::drawTick(QPainter *painter, double val, double len) const
{
    return;
    QwtScaleDraw::drawTick(painter, val, len);
}

void ScaleDraw::drawBackbone(QPainter *painter) const
{
    return;
    QwtScaleDraw::drawBackbone(painter);
}
