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

#ifndef QPAINTERPATH_H
#define QPAINTERPATH_H

#include "qpoint.h"
#include "qrect.h"

class QPainterPathPrivate;

class Q_GUI_EXPORT QPainterPath
{
    Q_DECLARE_PRIVATE(QPainterPath)
public:
    enum FillMode { OddEven, Winding };

    QPainterPath();
    ~QPainterPath();

    void beginSubpath();
    void closeSubpath();

    void addLine(const QPoint &p1, const QPoint &p2);
    inline void addLine(int x1, int y1, int x2, int y2);
    void addLine(const QPoint &p);
    inline void addLine(int x, int y);


    void addRect(const QRect &rect);
    inline void addRect(int x, int y, int w, int h);
    inline void addRect(const QPoint &topLeft, const QPoint &bottomRight);
    inline void addRect(const QPoint &topLeft, const QSize &dimension);

    void addBezier(const QPoint &p1, const QPoint &p2, const QPoint &p3, const QPoint &p4);
    void addBezier(const QPointArray &pa);
    inline void addBezier(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4);

    void addArc(const QRect &rect, int startAngle, int arcLength);
    inline void addArc(int x, int y, int w, int h, int startAngle, int arcLength);

    FillMode fillMode() const;
    void setFillMode(FillMode fillMode);
private:
    QPainterPathPrivate *d_ptr;

    friend class QPainter;
    friend class QWin32PaintEngine;
    friend class QCoreGraphicsPaintEngine;
    friend class QGdiplusPaintEngine;
};

inline void QPainterPath::addLine(int x1, int y1, int x2, int y2)
{
    addLine(QPoint(x1, y1), QPoint(x2, y2));
}

inline void QPainterPath::addLine(int x, int y)
{
    addLine(QPoint(x, y));
}

inline void QPainterPath::addRect(int x, int y, int w, int h)
{
    addRect(QRect(x, y, w, h));
}

inline void QPainterPath::addRect(const QPoint &topLeft, const QPoint &bottomRight)
{
    addRect(QRect(topLeft, bottomRight));
}

inline void QPainterPath::addRect(const QPoint &topLeft, const QSize &dim)
{
    addRect(QRect(topLeft, dim));
}

inline void QPainterPath::addBezier(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4)
{
    addBezier(QPoint(x1, y1), QPoint(x2, y2), QPoint(x3, y3), QPoint(x4, y4));
}

inline void QPainterPath::addArc(int x, int y, int w, int h, int startAngle, int arcLength)
{
    addArc(QRect(x, y, w, h), startAngle, arcLength);
}

#endif // QPAINTERPATH_H
