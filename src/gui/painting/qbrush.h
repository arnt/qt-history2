/****************************************************************************
**
** Definition of QBrush class.
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

#ifndef QBRUSH_H
#define QBRUSH_H

#ifndef QT_H
#include "qcolor.h"
#include "qpoint.h"
#endif // QT_H

struct QBrushData;
struct QTexturedBrushData;
struct QLinGradBrushData;

class Q_GUI_EXPORT QBrush: public Qt
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

    QBrush(const QPoint &p1, const QColor &col1, const QPoint &p2, const QColor &col2);

    ~QBrush();
    QBrush &operator=(const QBrush &brush);

    inline Qt::BrushStyle style() const;
    void setStyle(Qt::BrushStyle);

    inline const QColor &color() const;
    void setColor(const QColor &color);
    inline void setColor(Qt::GlobalColor color) { setColor(QColor(color)); }

    inline QPixmap *pixmap() const;
    void setPixmap(const QPixmap &pixmap);

    inline QColor gradientColor() const;
    inline QPoint gradientStart() const;
    inline QPoint gradientStop() const;

    inline operator const QColor&() const;
    inline operator const QPixmap*() const;

    bool operator==(const QBrush &b) const;
    inline bool operator!=(const QBrush &b) const { return !(operator==(b)); }

private:
#if defined(Q_WS_X11)
    friend class QX11PaintEngine;
#endif
#if defined(Q_WS_QWS)
    friend class QWSPaintEngine;
#endif
    friend class QPainter;
    inline void detach(Qt::BrushStyle newStyle);
    void detach_helper(Qt::BrushStyle newStyle);
    void init(const QColor &color, Qt::BrushStyle bs);
    QBrushData *d;
    QBrushData *d_func() { return d; }
    void cleanUp(QBrushData *x);
    static QBrushData *shared_default;
};


/*****************************************************************************
  QBrush stream functions
 *****************************************************************************/

#ifndef QT_NO_DATASTREAM
Q_GUI_EXPORT QDataStream &operator<<(QDataStream &, const QBrush &);
Q_GUI_EXPORT QDataStream &operator>>(QDataStream &, QBrush &);
#endif

#ifndef QT_NO_DEBUG
Q_GUI_EXPORT QDebug operator<<(QDebug, const QBrush &);
#endif

struct QBrushData
{
    QAtomic     ref;
    Qt::BrushStyle  style;
    QColor      color;
};

struct QTexturedBrushData : public QBrushData
{
    QPixmap     *pixmap;
};

struct QLinGradBrushData : public QBrushData
{
    QColor         color2;
    QPoint      p1;
    QPoint      p2;
};

inline Qt::BrushStyle QBrush::style() const { return d->style; }
inline const QColor &QBrush::color() const { return d->color; }

inline QBrush::operator const QColor&() const { return d->color; }
inline QBrush::operator const QPixmap*() const { return pixmap(); }

inline void QBrush::detach(Qt::BrushStyle newStyle) { if (newStyle != d->style || d->ref != 1) detach_helper(newStyle); }

inline QPixmap *QBrush::pixmap() const
{
    return d->style == Qt::CustomPattern
                     ? static_cast<const QTexturedBrushData*>(d)->pixmap : 0;
}

inline QPoint QBrush::gradientStart() const
{
    return d->style == Qt::LinearGradientPattern
                     ? static_cast<const QLinGradBrushData*>(d)->p1
                     : QPoint();
}

inline QPoint QBrush::gradientStop() const
{
    return d->style == Qt::LinearGradientPattern
                     ? static_cast<const QLinGradBrushData*>(d)->p2
                     : QPoint();
}

inline QColor QBrush::gradientColor() const
{
    return d->style == Qt::LinearGradientPattern
                     ? static_cast<const QLinGradBrushData*>(d)->color2
                     : QColor();
}

#endif // QBRUSH_H
