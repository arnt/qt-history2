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
    enum ElementType { Line, Bezier, Arc };

    QPoint firstPoint() const;

    ElementType type;

    union {
	struct { int x1, y1, x2, y2; } lineData;
        struct { int x1, y1, x2, y2, x3, y3, x4, y4; } bezierData;
        struct { int x, y, w, h, start, length, fpx, fpy, lpx, lpy; } arcData;
    };
};

/*!
 * Describes a subpath composed of multiple elements.
 */
struct QPainterSubpath
{
    QPainterSubpath() { };

    /*! Makes a straight line connection to the last point if \a p differs from
     * lastPoint. The addLine recursion is safe since we connect to lastPoint
     * so next call to connectLast will just do nothing..
     */
    void connectLast(const QPoint &p);

    /*! Closes the current path by connecting the last point
     * in the subpath path to the first one if they are different.
     */
    void close();

    /*! Returns true if the first and last point in the subpath are the
     * same
     */
    bool isClosed() const {
	return elements.size() > 0 && elements.at(0).firstPoint() == lastPoint;
    }

    QPoint firstPoint() const { return elements.at(0).firstPoint(); }

    /*! Converts the path to a polygon */
    QPointArray toPolygon(const QWMatrix &matrix) const;

    void addLine(const QPoint &p1, const QPoint &p2);
    void addBezier(const QPoint &p1, const QPoint &p2, const QPoint &p3, const QPoint &p4);
    void addArc(const QRect &rect, int startAngle, int arcLength);

    QList<QPainterPathElement> elements;
    QPoint lastPoint;
};

class QPainterPathPrivate
{
public:
    QPainterPathPrivate() :
        fillMode(QPainterPath::OddEven), bits(0)
    {
    }

    /* Flattens all the curves in the path to linear polygons */
    QList<QPointArray> flatten(const QWMatrix &matrix);

    /* Scanline converts the path to a bitmap */
    QBitmap scanToBitmap(const QRect &clip, const QWMatrix &xform, QRect *boundingRect);

    QList<QPainterSubpath> subpaths;
    QPainterPath::FillMode fillMode;

    uchar *bits;

};


#endif //QPAINTERPATH_P_H
