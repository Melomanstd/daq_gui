#ifndef SCALEDRAW_H
#define SCALEDRAW_H

#include <qwt_scale_draw.h>

class ScaleDraw : public QwtScaleDraw
{
public:
    ScaleDraw();

protected:
    virtual void drawLabel(QPainter *painter, double val) const;
    virtual void drawTick(QPainter *painter, double val, double len) const;
    virtual void drawBackbone(QPainter *painter) const;
};

#endif // SCALEDRAW_H
