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
#include "qline.h"

class QPolygon;
class QPainterPathPrivate;

class Q_GUI_EXPORT QPainterPath
{
    Q_DECLARE_PRIVATE(QPainterPath)
public:
    enum FillMode { OddEven, Winding };

    QPainterPath();
    QPainterPath(const QPointF &startPoint);
    QPainterPath(const QPainterPath &other);
    QPainterPath &operator=(const QPainterPath &other);
    ~QPainterPath();

    void closeSubpath();

    void moveTo(const QPointF &p);
    inline void moveTo(float x, float y);

    void lineTo(const QPointF &p);
    inline void lineTo(float x, float y);

    void arcTo(const QRectF &rect, float startAngle, float arcLength);
    inline void arcTo(float x, float y, float w, float h, float startAngle, float arcLength);

    void curveTo(const QPointF &ctrlPt1, const QPointF &ctrlPt2, const QPointF &endPt);
    inline void curveTo(float ctrlPt1x, float ctrlPt1y, float ctrlPt2x, float ctrlPt2y,
                        float endPtx, float endPty);

    void addPolygon(const QPolygon &polygon);

    void addEllipse(const QRectF &rect);
    inline void addEllipse(float x, float y, float w, float h);

    void addRect(const QRectF &rect);
    inline void addRect(float x, float y, float w, float h);

    void addText(const QPointF &point, const QFont &f, const QString &text);
    inline void addText(float x, float y, const QFont &f, const QString &text);

    QPainterPath createPathOutline(int width,
                                   Qt::PenStyle penStyle = Qt::SolidLine,
                                   Qt::PenCapStyle capStyle = Qt::FlatCap,
                                   Qt::PenJoinStyle joinStyle = Qt::BevelJoin);

    QRectF boundingRect() const;

    FillMode fillMode() const;
    void setFillMode(FillMode fillMode);

    bool isEmpty() const;

    QPolygon toPolygon() const;

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
    moveTo(QPointF(x, y));
}

inline void QPainterPath::lineTo(float x, float y)
{
    lineTo(QPointF(x, y));
}

inline void QPainterPath::arcTo(float x, float y, float w, float h, float startAngle, float arcLenght)
{
    arcTo(QRectF(x, y, w, h), startAngle, arcLenght);
}

inline void QPainterPath::curveTo(float ctrlPt1x, float ctrlPt1y, float ctrlPt2x, float ctrlPt2y,
                                   float endPtx, float endPty)
{
    curveTo(QPointF(ctrlPt1x, ctrlPt1y), QPointF(ctrlPt2x, ctrlPt2y),
            QPointF(endPtx, endPty));
}

inline void QPainterPath::addEllipse(float x, float y, float w, float h)
{
    addEllipse(QRectF(x, y, w, h));
}

inline void QPainterPath::addRect(float x, float y, float w, float h)
{
    addRect(QRectF(x, y, w, h));
}

inline void QPainterPath::addText(float x, float y, const QFont &f, const QString &text)
{
    addText(QPointF(x, y), f, text);
}

#endif // QPAINTERPATH_H
