/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QPAINTERPATH_P_H
#define QPAINTERPATH_P_H

#include <qlist.h>
#include <qpainterpath.h>

/*!
 * Describes an element in a subpath
 */
struct QPainterPathElement
{
    enum ElementType { Line, Curve, Arc };

    ElementType type;

    union {
	struct { float x, y; } lineData;
        struct { float c1x, c1y, c2x, c2y, ex, ey; } curveData;
        struct { float x, y, w, h, start, length; } arcData;
    };

    /*! Some factory functions */
    static inline QPainterPathElement line(float x, float y);
    static inline QPainterPathElement curve(float c1x, float c1y, float c2x, float c2y,
                                            float ex, float ey);
    static inline QPainterPathElement arc(float x, float y, float w, float h,
                                          float start, float length);
};

/*!
 * Describes a subpath composed of multiple elements.
 */
struct QPainterSubpath
{
    QPainterSubpath(const QPointF &p = QPoint(0, 0))
    {
        startPoint = p;
    };

    /*! Makes a straight line connection to the last point if \a p differs from
     * lastPoint. The addLine recursion is safe since we connect to lastPoint
     * so next call to connectLast will just do nothing..
     */
    void connectLast(const QPointF &p);

    /*! Closes the current path by connecting the last point
     * in the subpath path to the first one if they are different.
     */
    void close();

    /*! Returns true if the first and last point in the subpath are the
     * same
     */
    bool isClosed() const {
	return elements.size() > 0 && currentPoint == startPoint;
    }

    /*! Converts the path to a polygon */
    QPointArray toPolygon(const QMatrix &matrix) const;

    void lineTo(const QPointF &p);
    void curveTo(const QPointF &p2, const QPointF &p3, const QPointF &p4);
    void arcTo(const QRectF &rect, float startAngle, float arcLength);

    QList<QPainterPathElement> elements;
    QPointF currentPoint;
    QPointF startPoint;
};

class QPainterPathPrivate
{
public:
    enum FlattenInclusion { ClosedSubpaths   = 0x0001,
                            UnclosedSubpaths = 0x0002,
                            AllSubpaths      = 0x0003
    };

    QPainterPathPrivate() :
        fillMode(QPainterPath::OddEven)
    {
    }

    /* Flattens all the curves in the path to linear polygons */
    QList<QPointArray> flatten(const QMatrix &matrix, FlattenInclusion include = AllSubpaths);

    /* Scanline converts the path to a bitmap */
    QBitmap scanToBitmap(const QRect &clip, const QMatrix &xform, QRect *boundingRect);

    /* Creates a path containing the outline of this path of width \a penwidth */
    QPainterPath createStroke(const QPen &pen);

    QList<QPainterSubpath> subpaths;
    QPainterPath::FillMode fillMode;
};

void qt_find_ellipse_coords(const QRectF &r, float angle, float length,
                            QPointF* startPoint, QPointF *endPoint);


inline QPainterPathElement QPainterPathElement::line(float x, float y)
{
    QPainterPathElement e;
    e.type = Line;
    e.lineData.x = x;
    e.lineData.y = y;
    return e;
}

inline QPainterPathElement QPainterPathElement::curve(float c1x, float c1y,
                                                      float c2x, float c2y,
                                                      float ex, float ey)
{
    QPainterPathElement e;
    e.type = QPainterPathElement::Curve;
    e.curveData.c1x = c1x;
    e.curveData.c1y = c1y;
    e.curveData.c2x = c2x;
    e.curveData.c2y = c2y;
    e.curveData.ex = ex;
    e.curveData.ey = ey;
    return e;
}

inline QPainterPathElement QPainterPathElement::arc(float x, float y, float w, float h,
                                                    float start, float length)
{
    QPainterPathElement e;
    e.arcData.x = x;
    e.arcData.y = y;
    e.arcData.w = w;
    e.arcData.h = h;
    e.arcData.start = start;
    e.arcData.length = length;
    return e;
}



#endif //QPAINTERPATH_P_H
