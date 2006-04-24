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

#include <QtGui/qmatrix.h>
#include <QtCore/qglobal.h>
#include <QtCore/qrect.h>
#include <QtCore/qline.h>
#include <QtCore/qvector.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

class QFont;
class QPainterPathPrivate;
class QPainterPathData;
class QPainterPathStrokerPrivate;
class QPolygonF;
class QRegion;

class Q_GUI_EXPORT QPainterPath
{
public:
    enum ElementType {
        MoveToElement,
        LineToElement,
        CurveToElement,
        CurveToDataElement
    };

    class Element {
    public:
        qreal x;
        qreal y;
        ElementType type;

        bool isMoveTo() const { return type == MoveToElement; }
        bool isLineTo() const { return type == LineToElement; }
        bool isCurveTo() const { return type == CurveToElement; }

        operator QPointF () const { return QPointF(x, y); }

        bool operator==(const Element &e) const { return qFuzzyCompare(x, e.x)
            && qFuzzyCompare(y, e.y) && type == e.type; }
        inline bool operator!=(const Element &e) const { return !operator==(e); }
    };

    QPainterPath();
    explicit QPainterPath(const QPointF &startPoint);
    QPainterPath(const QPainterPath &other);
    QPainterPath &operator=(const QPainterPath &other);
    ~QPainterPath();

    void closeSubpath();

    void moveTo(const QPointF &p);
    inline void moveTo(qreal x, qreal y);

    void lineTo(const QPointF &p);
    inline void lineTo(qreal x, qreal y);

    void arcMoveTo(const QRectF &rect, qreal angle);
    inline void arcMoveTo(qreal x, qreal y, qreal w, qreal h, qreal angle);

    void arcTo(const QRectF &rect, qreal startAngle, qreal arcLength);
    inline void arcTo(qreal x, qreal y, qreal w, qreal h, qreal startAngle, qreal arcLength);

    void cubicTo(const QPointF &ctrlPt1, const QPointF &ctrlPt2, const QPointF &endPt);
    inline void cubicTo(qreal ctrlPt1x, qreal ctrlPt1y, qreal ctrlPt2x, qreal ctrlPt2y,
                        qreal endPtx, qreal endPty);
    void quadTo(const QPointF &ctrlPt, const QPointF &endPt);
    inline void quadTo(qreal ctrlPtx, qreal ctrlPty, qreal endPtx, qreal endPty);

    QPointF currentPosition() const;

    void addRect(const QRectF &rect);
    inline void addRect(qreal x, qreal y, qreal w, qreal h);
    void addEllipse(const QRectF &rect);
    inline void addEllipse(qreal x, qreal y, qreal w, qreal h);
    void addPolygon(const QPolygonF &polygon);
    void addText(const QPointF &point, const QFont &f, const QString &text);
    inline void addText(qreal x, qreal y, const QFont &f, const QString &text);
    void addPath(const QPainterPath &path);
    void addRegion(const QRegion &region);

    void connectPath(const QPainterPath &path);

    bool contains(const QPointF &pt) const;
    bool contains(const QRectF &rect) const;
    bool intersects(const QRectF &rect) const;

    QRectF boundingRect() const;
    QRectF controlPointRect() const;

    Qt::FillRule fillRule() const;
    void setFillRule(Qt::FillRule fillRule);

    inline bool isEmpty() const;

    QPainterPath toReversed() const;
    QList<QPolygonF> toSubpathPolygons(const QMatrix &matrix = QMatrix()) const;
    QList<QPolygonF> toFillPolygons(const QMatrix &matrix = QMatrix()) const;
    QPolygonF toFillPolygon(const QMatrix &matrix = QMatrix()) const;

    inline int elementCount() const;
    inline const QPainterPath::Element &elementAt(int i) const;
    inline void setElementPositionAt(int i, qreal x, qreal y);

    bool operator==(const QPainterPath &other) const;
    bool operator!=(const QPainterPath &other) const;

private:
    QPainterPathPrivate *d_ptr;

    inline void ensureData() { if (!d_ptr) ensureData_helper(); }
    void ensureData_helper();
    inline void detach();
    void detach_helper();

    QPainterPathData *d_func() const { return reinterpret_cast<QPainterPathData *>(d_ptr); }

    friend class QPainterPathData;
    friend class QPainterPathStroker;
    friend class QPainterPathStrokerPrivate;
    friend class QMatrix;

#ifndef QT_NO_DATASTREAM
    friend Q_GUI_EXPORT QDataStream &operator<<(QDataStream &, const QPainterPath &);
    friend Q_GUI_EXPORT QDataStream &operator>>(QDataStream &, QPainterPath &);
#endif
};

class QPainterPathPrivate
{
    friend class QPainterPath;
    friend class QPainterPathData;
    friend class QPainterPathStroker;
    friend class QPainterPathStrokerPrivate;
    friend class QMatrix;
#ifndef QT_NO_DATASTREAM
    friend Q_GUI_EXPORT QDataStream &operator<<(QDataStream &, const QPainterPath &);
    friend Q_GUI_EXPORT QDataStream &operator>>(QDataStream &, QPainterPath &);
#endif
private:
    QAtomic ref;
    QVector<QPainterPath::Element> elements;
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

    void setWidth(qreal width);
    qreal width() const;

    void setCapStyle(Qt::PenCapStyle style);
    Qt::PenCapStyle capStyle() const;

    void setJoinStyle(Qt::PenJoinStyle style);
    Qt::PenJoinStyle joinStyle() const;

    void setMiterLimit(qreal length);
    qreal miterLimit() const;

    void setCurveThreshold(qreal threshold);
    qreal curveThreshold() const;

    void setDashPattern(Qt::PenStyle);
    void setDashPattern(const QVector<qreal> &dashPattern);
    QVector<qreal> dashPattern() const;

    QPainterPath createStroke(const QPainterPath &path) const;

private:
    QPainterPathStrokerPrivate *d_ptr;
};

inline void QPainterPath::moveTo(qreal x, qreal y)
{
    moveTo(QPointF(x, y));
}

inline void QPainterPath::lineTo(qreal x, qreal y)
{
    lineTo(QPointF(x, y));
}

inline void QPainterPath::arcTo(qreal x, qreal y, qreal w, qreal h, qreal startAngle, qreal arcLenght)
{
    arcTo(QRectF(x, y, w, h), startAngle, arcLenght);
}

inline void QPainterPath::arcMoveTo(qreal x, qreal y, qreal w, qreal h, qreal angle)
{
    arcMoveTo(QRectF(x, y, w, h), angle);
}

inline void QPainterPath::cubicTo(qreal ctrlPt1x, qreal ctrlPt1y, qreal ctrlPt2x, qreal ctrlPt2y,
                                   qreal endPtx, qreal endPty)
{
    cubicTo(QPointF(ctrlPt1x, ctrlPt1y), QPointF(ctrlPt2x, ctrlPt2y),
            QPointF(endPtx, endPty));
}

inline void QPainterPath::quadTo(qreal ctrlPtx, qreal ctrlPty, qreal endPtx, qreal endPty)
{
    quadTo(QPointF(ctrlPtx, ctrlPty), QPointF(endPtx, endPty));
}

inline void QPainterPath::addEllipse(qreal x, qreal y, qreal w, qreal h)
{
    addEllipse(QRectF(x, y, w, h));
}

inline void QPainterPath::addRect(qreal x, qreal y, qreal w, qreal h)
{
    addRect(QRectF(x, y, w, h));
}

inline void QPainterPath::addText(qreal x, qreal y, const QFont &f, const QString &text)
{
    addText(QPointF(x, y), f, text);
}

inline bool QPainterPath::isEmpty() const
{
    return !d_ptr || (d_ptr->elements.size() == 1 && d_ptr->elements.first().type == MoveToElement);
}

inline int QPainterPath::elementCount() const
{
    return d_ptr ? d_ptr->elements.size() : 0;
}

inline const QPainterPath::Element &QPainterPath::elementAt(int i) const
{
    Q_ASSERT(d_ptr);
    Q_ASSERT(i >= 0 && i < elementCount());
    return d_ptr->elements.at(i);
}

inline void QPainterPath::setElementPositionAt(int i, qreal x, qreal y)
{
    Q_ASSERT(d_ptr);
    Q_ASSERT(i >= 0 && i < elementCount());
    QPainterPath::Element &e = d_ptr->elements[i];
    e.x = x;
    e.y = y;
}


inline void QPainterPath::detach()
{
    if (d_ptr->ref != 1)
        detach_helper();
}

QT_END_HEADER

#endif // QPAINTERPATH_H
