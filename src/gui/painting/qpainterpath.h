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

#include "qglobal.h"
#include "qrect.h"
#include "qline.h"
#include "qvector.h"
#include "qmatrix.h"


class QFont;
class QPainterPathPrivate;
class QPainterPathStrokerPrivate;
class QPolygonF;
class QRegion;

class Q_GUI_EXPORT QPainterPath
{
    Q_DECLARE_PRIVATE(QPainterPath)
public:
    enum ElementType {
        MoveToElement,
        LineToElement,
        CurveToElement,
        CurveToDataElement
    };

    class Element {
    public:
        qReal x;
        qReal y;
        ElementType type;

        bool operator==(const Element &e) const { return x == e.x && y == e.y && type == e.type; }
    };


    QPainterPath();
    QPainterPath(const QPointF &startPoint);
    QPainterPath(const QPainterPath &other);
    QPainterPath &operator=(const QPainterPath &other);
    ~QPainterPath();

    void closeSubpath();

    void moveTo(const QPointF &p);
    inline void moveTo(qReal x, qReal y);

    void lineTo(const QPointF &p);
    inline void lineTo(qReal x, qReal y);

    void arcTo(const QRectF &rect, qReal startAngle, qReal arcLength);
    inline void arcTo(qReal x, qReal y, qReal w, qReal h, qReal startAngle, qReal arcLength);

    void curveTo(const QPointF &ctrlPt1, const QPointF &ctrlPt2, const QPointF &endPt);
    inline void curveTo(qReal ctrlPt1x, qReal ctrlPt1y, qReal ctrlPt2x, qReal ctrlPt2y,
                        qReal endPtx, qReal endPty);

    void addRect(const QRectF &rect);
    inline void addRect(qReal x, qReal y, qReal w, qReal h);
    void addEllipse(const QRectF &rect);
    inline void addEllipse(qReal x, qReal y, qReal w, qReal h);
    void addPolygon(const QPolygonF &polygon);
    void addText(const QPointF &point, const QFont &f, const QString &text);
    inline void addText(qReal x, qReal y, const QFont &f, const QString &text);
    void addPath(const QPainterPath &path);
    void addRegion(const QRegion &region);

    bool contains(const QPointF &pt) const;
    bool contains(const QRectF &rect) const;

    QRectF boundingRect() const;

    Qt::FillRule fillRule() const;
    void setFillRule(Qt::FillRule fillRule);

    inline bool isEmpty() const;

    QPainterPath toReversed() const;
    QList<QPolygonF> toSubpathPolygons(const QMatrix &matrix = QMatrix()) const;
    QList<QPolygonF> toFillPolygons(const QMatrix &matrix = QMatrix()) const;
    QPolygonF toFillPolygon(const QMatrix &matrix = QMatrix()) const;

    int elementCount() const { return elements.size(); }
    const QPainterPath::Element &elementAt(int i) const { return elements.at(i); }

    inline QPainterPath &operator +=(const QPainterPath &other) { addPath(other); return *this; }

    bool operator==(const QPainterPath &other) const;
    bool operator!=(const QPainterPath &other) const;

private:
    QPainterPathPrivate *d_ptr;
    QVector<Element> elements;

    friend class QPainterPathStroker;
    friend class QPainterPathStrokerPrivate;
    friend class QMatrix;

#ifndef QT_NO_DATASTREAM
    friend Q_GUI_EXPORT QDataStream &operator<<(QDataStream &, const QPainterPath &);
    friend Q_GUI_EXPORT QDataStream &operator>>(QDataStream &, QPainterPath &);
#endif

};

Q_DECLARE_TYPEINFO(QPainterPath::Element, Q_PRIMITIVE_TYPE);

#ifndef QT_NO_DATASTREAM
Q_GUI_EXPORT QDataStream &operator<<(QDataStream &, const QPainterPath &);
Q_GUI_EXPORT QDataStream &operator>>(QDataStream &, QPainterPath &);
#endif

class Q_GUI_EXPORT QPainterPathStroker
{
    Q_DECLARE_PRIVATE(QPainterPathStroker)
public:
    QPainterPathStroker();
    ~QPainterPathStroker();

    void setWidth(qReal width);
    qReal width() const;

    void setStyle(Qt::PenStyle style);
    Qt::PenStyle style() const;

    void setCapStyle(Qt::PenCapStyle style);
    Qt::PenCapStyle capStyle() const;

    void setJoinStyle(Qt::PenJoinStyle style);
    Qt::PenJoinStyle joinStyle() const;

    void setMiterLimit(qReal length);
    qReal miterLimit() const;

    QPainterPath createStroke(const QPainterPath &path) const;

private:
    QPainterPathStrokerPrivate *d_ptr;
};

inline void QPainterPath::moveTo(qReal x, qReal y)
{
    moveTo(QPointF(x, y));
}

inline void QPainterPath::lineTo(qReal x, qReal y)
{
    lineTo(QPointF(x, y));
}

inline void QPainterPath::arcTo(qReal x, qReal y, qReal w, qReal h, qReal startAngle, qReal arcLenght)
{
    arcTo(QRectF(x, y, w, h), startAngle, arcLenght);
}

inline void QPainterPath::curveTo(qReal ctrlPt1x, qReal ctrlPt1y, qReal ctrlPt2x, qReal ctrlPt2y,
                                   qReal endPtx, qReal endPty)
{
    curveTo(QPointF(ctrlPt1x, ctrlPt1y), QPointF(ctrlPt2x, ctrlPt2y),
            QPointF(endPtx, endPty));
}

inline void QPainterPath::addEllipse(qReal x, qReal y, qReal w, qReal h)
{
    addEllipse(QRectF(x, y, w, h));
}

inline void QPainterPath::addRect(qReal x, qReal y, qReal w, qReal h)
{
    addRect(QRectF(x, y, w, h));
}

inline void QPainterPath::addText(qReal x, qReal y, const QFont &f, const QString &text)
{
    addText(QPointF(x, y), f, text);
}

inline bool QPainterPath::isEmpty() const
{
    return elements.size() == 1 && elements.first().type == MoveToElement;
}

#endif // QPAINTERPATH_H

