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

#include "qrectfloat.h"
#include "qlinefloat.h"

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

    void closeSubpath();

    void moveTo(const QPointFloat &p);
    inline void moveTo(float x, float y);

    void lineTo(const QPointFloat &p);
    inline void lineTo(float x, float y);

    void arcTo(const QRectFloat &rect, float startAngle, float arcLength);
    inline void arcTo(float x, float y, float w, float h, float startAngle, float arcLength);

    void curveTo(const QPointFloat &ctrlPt1, const QPointFloat &ctrlPt2, const QPointFloat &endPt);
    inline void curveTo(float ctrlPt1x, float ctrlPt1y, float ctrlPt2x, float ctrlPt2y,
                        float endPtx, float endPty);

    void addRect(const QRectFloat &rect);
    inline void addRect(float x, float y, float w, float h);

    QPainterPath createPathOutline(int width);

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

inline void QPainterPath::moveTo(float x, float y)
{
    moveTo(QPointFloat(x, y));
}

inline void QPainterPath::lineTo(float x, float y)
{
    lineTo(QPointFloat(x, y));
}

inline void QPainterPath::arcTo(float x, float y, float w, float h, float startAngle, float arcLenght)
{
    arcTo(QRectFloat(x, y, w, h), startAngle, arcLenght);
}

inline void QPainterPath::curveTo(float ctrlPt1x, float ctrlPt1y, float ctrlPt2x, float ctrlPt2y,
                                   float endPtx, float endPty)
{
    curveTo(QPointFloat(ctrlPt1x, ctrlPt1y), QPointFloat(ctrlPt2x, ctrlPt2y),
            QPointFloat(endPtx, endPty));
}

inline void QPainterPath::addRect(float x, float y, float w, float h)
{
    addRect(QRectFloat(x, y, w, h));
}

#endif // QPAINTERPATH_H
