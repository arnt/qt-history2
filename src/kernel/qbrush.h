/****************************************************************************
**
** Definition of QBrush class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
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
#endif // QT_H


class Q_EXPORT QBrush: public Qt
{
public:
    QBrush();
    QBrush(BrushStyle bs);
    QBrush(const QColor &color, BrushStyle bs=SolidPattern);
    QBrush(Qt::GlobalColor color, BrushStyle bs=SolidPattern);
    QBrush(const QColor &color, const QPixmap &pixmap);
    QBrush(Qt::GlobalColor color, const QPixmap &pixmap);
    QBrush(const QPixmap &pixmap);
    QBrush(const QBrush &brush);
    ~QBrush();
    QBrush &operator=(const QBrush &brush);

    inline BrushStyle style() const { return d->style; }
    void setStyle( BrushStyle );
    inline const QColor &color() const { return d->color; }
    void setColor(const QColor &color);
    inline void setColor(Qt::GlobalColor color)
    { setColor(QColor(color)); }
    inline QPixmap *pixmap() const { return d->pixmap; }
    void setPixmap(const QPixmap &pixmap);

    inline operator const QColor&() const { return d->color; }
    inline operator const QPixmap*() const { return d->pixmap; }

    bool operator==(const QBrush &b) const;
    inline bool operator!=(const QBrush &b) const { return !(operator==(b)); }

private:
    friend class QPainter;
    inline void detach() { if (d->ref != 1) detach_helper(); }
    void detach_helper();
    void init(const QColor &color, BrushStyle bs);
    struct QBrushData {
	QAtomic ref;
	BrushStyle style;
	QColor color;
	QPixmap	 *pixmap;
    };
    QBrushData *d;
    void cleanUp(QBrushData *x);
    static QBrushData *shared_default;
};


/*****************************************************************************
  QBrush stream functions
 *****************************************************************************/

#ifndef QT_NO_DATASTREAM
Q_EXPORT QDataStream &operator<<( QDataStream &, const QBrush & );
Q_EXPORT QDataStream &operator>>( QDataStream &, QBrush & );
#endif

#endif // QBRUSH_H
