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
#include "qvector.h"

class QPolygon;
class QPainterPathPrivate;
class QPainterPathStrokerPrivate;

class Q_GUI_EXPORT QPainterPath
{
    Q_DECLARE_PRIVATE(QPainterPath)
public:
    enum FillMode { OddEven, Winding };

    enum ElementType {
        MoveToElement,
        LineToElement,
        CurveToElement,
        CurveToDataElement
    };

    class Element {
    public:
        float x;
        float y;
        ElementType type;

        bool operator==(const Element &e) { return x == e.x && y == e.y && type == e.type; }
    };


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

    QRectF boundingRect() const;

    FillMode fillMode() const;
    void setFillMode(FillMode fillMode);

    inline bool isEmpty() const;

    QList<QPolygon> toSubpathPolygons() const;
    QPolygon toFillPolygon() const;

    int elementCount() const { return elements.size(); }
    const QPainterPath::Element &elementAt(int i) const { return elements.at(i); }


private:
    QPainterPathPrivate *d_ptr;
    QVector<Element> elements;

    friend class QPainterPathStroker;
};

class Q_GUI_EXPORT QPainterPathStroker
{
    Q_DECLARE_PRIVATE(QPainterPathStroker)
public:
    QPainterPathStroker();

    void setWidth(float width);
    float width() const;

    void setStyle(Qt::PenStyle style);
    Qt::PenStyle style() const;

    void setCapStyle(Qt::PenCapStyle style);
    Qt::PenCapStyle capStyle() const;

    void setJoinStyle(Qt::PenJoinStyle style);
    Qt::PenJoinStyle joinStyle() const;

    QPainterPath createStroke(const QPainterPath &path) const;

private:
    QPainterPathStrokerPrivate *d_ptr;
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

inline bool QPainterPath::isEmpty() const
{
    return elements.size() == 1 && elements.first().type == MoveToElement;
}

#endif // QPAINTERPATH_H

