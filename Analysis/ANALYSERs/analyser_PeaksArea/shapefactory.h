#ifndef _SHAPE_FACTORY_H_
#define _SHAPE_FACTORY_H_

#include <qpainterpath.h>

namespace ShapeFactory
{
    enum Shape
    {
        Rect,
        Triangle,
        Ellipse,
        Ring,
        Star,
        Hexagon,
        Polygon
    };

    QPainterPath path( Shape, const QPointF &, const QSizeF & , QVector<QPointF> &);
};

#endif
