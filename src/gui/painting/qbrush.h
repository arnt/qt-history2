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

#ifndef QBRUSH_H
#define QBRUSH_H

#include "QtCore/qpair.h"
#include "QtCore/qpoint.h"
#include "QtCore/qvector.h"
#include "QtGui/qcolor.h"

struct QBrushData;
class QPixmap;
class QGradient;
class QVariant;

class Q_GUI_EXPORT QBrush
{
public:
    QBrush();
    QBrush(Qt::BrushStyle bs);
    QBrush(const QColor &color, Qt::BrushStyle bs=Qt::SolidPattern);
    QBrush(Qt::GlobalColor color, Qt::BrushStyle bs=Qt::SolidPattern);

    QBrush(const QColor &color, const QPixmap &pixmap);
    QBrush(Qt::GlobalColor color, const QPixmap &pixmap);
    QBrush(const QPixmap &pixmap);

    QBrush(const QBrush &brush);

    QBrush(const QGradient &gradient);

    ~QBrush();
    QBrush &operator=(const QBrush &brush);
    operator QVariant() const;

    inline Qt::BrushStyle style() const;
    void setStyle(Qt::BrushStyle);

    QPixmap texture() const;
    void setTexture(const QPixmap &pixmap);

    inline const QColor &color() const;
    void setColor(const QColor &color);
    inline void setColor(Qt::GlobalColor color) { setColor(QColor(color)); }

    const QGradient *gradient() const;

    bool isOpaque() const;

    bool operator==(const QBrush &b) const;
    inline bool operator!=(const QBrush &b) const { return !(operator==(b)); }

#ifdef QT3_SUPPORT
    inline QT3_SUPPORT operator const QColor&() const;
    QT3_SUPPORT QPixmap *pixmap() const;
    inline QT3_SUPPORT void setPixmap(const QPixmap &pixmap) { setTexture(pixmap); }
#endif

private:
#if defined(Q_WS_X11)
    friend class QX11PaintEngine;
#endif
#if defined(Q_WS_QWS)
    friend class QWSPaintEngine;
#endif
    friend class QPainter;
    void detach(Qt::BrushStyle newStyle);
    void init(const QColor &color, Qt::BrushStyle bs);
    QBrushData *d;
    void cleanUp(QBrushData *x);
    static QBrushData *shared_default;
};

Q_DECLARE_TYPEINFO(QBrush, Q_MOVABLE_TYPE);

/*****************************************************************************
  QBrush stream functions
 *****************************************************************************/

#ifndef QT_NO_DATASTREAM
Q_GUI_EXPORT QDataStream &operator<<(QDataStream &, const QBrush &);
Q_GUI_EXPORT QDataStream &operator>>(QDataStream &, QBrush &);
#endif

#ifndef QT_NO_DEBUG_STREAM
Q_GUI_EXPORT QDebug operator<<(QDebug, const QBrush &);
#endif

struct QBrushData
{
    QAtomic ref;
    Qt::BrushStyle style;
    QColor color;
};

inline Qt::BrushStyle QBrush::style() const { return d->style; }
inline const QColor &QBrush::color() const { return d->color; }

#ifdef QT3_SUPPORT
inline QBrush::operator const QColor&() const { return d->color; }
#endif


/*******************************************************************************
 * QGradients
 */
class QGradientPrivate;

typedef QPair<qreal, QColor> QGradientStop;
typedef QVector<QGradientStop> QGradientStops;

class Q_GUI_EXPORT QGradient
{
public:
    enum Type {
        LinearGradient,
        RadialGradient,
        ConicalGradient
    };

    enum Spread {
        PadSpread,
        ReflectSpread,
        RepeatSpread
    };

    QGradient();

    Type type() const { return m_type; }

    void setSpread(Spread spread) { m_spread = spread; }
    Spread spread() const { return m_spread; }

    void appendStop(qreal pos, const QColor &color);
    void appendStop(const QGradientStop &stop);

    void setStops(const QGradientStops &stops);
    QGradientStops stops() const;

    bool operator==(const QGradient &gradient);

private:
    friend class QLinearGradient;
    friend class QRadialGradient;
    friend class QConicalGradient;
    friend Q_GUI_EXPORT QDataStream &operator<<(QDataStream &, const QBrush &);

    Type m_type;
    Spread m_spread;
    QGradientStops m_stops;
    union {
        struct {
            qreal x1, y1, x2, y2;
        } linear;
        struct {
            qreal cx, cy, fx, fy, radius;
        } radial;
        struct {
            qreal cx, cy, angle;
        } conical;
    } m_data;
    void *dummy;
};


class Q_GUI_EXPORT QLinearGradient : public QGradient
{
public:
    QLinearGradient(const QPointF &start, const QPointF &finalStop);
    QLinearGradient(qreal xStart, qreal yStart, qreal xFinalStop, qreal yFinalStop);

    QPointF start() const;
    QPointF finalStop() const;
};


class Q_GUI_EXPORT QRadialGradient : public QGradient
{
public:
    QRadialGradient(const QPointF &center, qreal radius, const QPointF &focalPoint = QPointF());
    QRadialGradient(qreal cx, qreal cy, qreal radius, qreal fx=0, qreal fy=0);

    QPointF center() const;
    QPointF focalPoint() const;
    qreal radius() const;
};


class Q_GUI_EXPORT QConicalGradient : public QGradient
{
public:
    QConicalGradient(const QPointF &center, qreal startAngle);
    QConicalGradient(qreal cx, qreal cy, qreal startAngle);

    QPointF center() const;
    qreal angle() const;
};

#endif // QBRUSH_H
