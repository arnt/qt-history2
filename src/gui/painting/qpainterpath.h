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

#ifndef QPAINTERPATH_H
#define QPAINTERPATH_H

#include "qrect.h"
#include "qlinefloat.h"
#include "qsizefloat.h"

#define QRectFloat QRect

class QPainterPathPrivate;

class Q_GUI_EXPORT QPainterPath
{
    Q_DECLARE_PRIVATE(QPainterPath)
public:
    enum FillMode { OddEven, Winding };

    QPainterPath();
    QPainterPath(const QPainterPath &other);
    QPainterPath &operator=(const QPainterPath &other);
    ~QPainterPath();

    void beginSubpath();
    void closeSubpath();

    void addLine(const QLineFloat &l);
    void addLine(const QPointFloat &p);
    inline void addLine(const QPointFloat &p1, const QPointFloat &p2);
    inline void addLine(float x1, float y1, float x2, float y2);
    inline void addLine(float x, float y);

    void addRect(const QRectFloat &rect);
    inline void addRect(float x, float y, float w, float h);
    inline void addRect(const QPointFloat &topLeft, const QPointFloat &bottomRight);
    inline void addRect(const QPointFloat &topLeft, const QSizeFloat &dimension);

    void addBezier(const QPointFloat &p1, const QPointFloat &p2,
                   const QPointFloat &p3, const QPointFloat &p4);
    inline void addBezier(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4);
    void addArc(const QRectFloat &rect, float startAngle, float arcLength);
    inline void addArc(float x, float y, float w, float h, float startAngle, float arcLength);

    QRectFloat boundingRect() const;

    FillMode fillMode() const;
    void setFillMode(FillMode fillMode);

    bool isEmpty() const;
private:
    QPainterPathPrivate *d_ptr;

    friend class QPainter;
    friend class QWin32PaintEngine;
    friend class QCoreGraphicsPaintEngine;
    friend class QGdiplusPaintEngine;
    friend class QPSPrintEngine;
};

inline void QPainterPath::addLine(float x1, float y1, float x2, float y2)
{
    addLine(QLineFloat(x1, y1, x2, y2));
}

inline void QPainterPath::addLine(float x, float y)
{
    addLine(QPointFloat(x, y));
}

inline void QPainterPath::addLine(const QPointFloat &p1, const QPointFloat &p2)
{
    addLine(QLineFloat(p1, p2));
}

inline void QPainterPath::addRect(float x, float y, float w, float h)
{
    addRect(QRectFloat(x, y, w, h));
}

inline void QPainterPath::addRect(const QPointFloat &topLeft, const QPointFloat &bottomRight)
{
    addRect(QRectFloat(topLeft.toPoint(), bottomRight.toPoint()));
}

inline void QPainterPath::addRect(const QPointFloat &topLeft, const QSizeFloat &dim)
{
    addRect(QRectFloat(topLeft.toPoint(), dim.toSize()));
}

inline void QPainterPath::addBezier(float x1, float y1, float x2, float y2,
                                    float x3, float y3, float x4, float y4)
{
    addBezier(QPointFloat(x1, y1), QPointFloat(x2, y2), QPointFloat(x3, y3), QPointFloat(x4, y4));
}

inline void QPainterPath::addArc(float x, float y, float w, float h, float startAngle, float arcLength)
{
    addArc(QRectFloat(x, y, w, h), startAngle, arcLength);
}

#endif // QPAINTERPATH_H
