/****************************************************************************
**
** Definition of the QPaintEngine class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QPAINTERPATH_P_H
#define QPAINTERPATH_P_H

/*!
 * Describes an element in a subpath
 */
struct QPainterPathElement
{
    enum ElementType { Line };

    QPoint firstPoint() const;

    ElementType type;
    union {
	struct { int x1, y1, x2, y2; } lineData;
    };
};

/*!
 * Describes a subpath composed of multiple elements.
 */
struct QPainterSubpath
{
    QPainterSubpath() : pointCount(0) { };    
    
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
    

    /*! Converts the path to a polygon */
    QPointArray toPolygon() const;

    void addLine(const QPoint &p1, const QPoint &p2);

    QList<QPainterPathElement> elements;
    QPoint lastPoint;
    int pointCount;
};

class QPainterPathPrivate
{
public:
    
    /* Moves current path to the list of subpaths and creates a new current */
    void beginSubpath_helper();

    QList<QPainterSubpath> subpaths;
};


#endif QPAINTERPATH_P_H
