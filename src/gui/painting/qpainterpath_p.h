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
};

/*!
 * Describes a subpath composed of multiple elements.
 */
struct QPainterSubpath
{
    QPainterSubpath(const QPointF &p)
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
    QPainterPath createPathOutlineFlat(int penWidth, const QMatrix &matrix);

    QList<QPainterSubpath> subpaths;
    QPainterPath::FillMode fillMode;
};

void qt_find_ellipse_coords(const QRectF &r, float angle, float length,
                            QPointF* startPoint, QPointF *endPoint);


#endif //QPAINTERPATH_P_H
