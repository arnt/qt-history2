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

#ifndef QPEN_H
#define QPEN_H

#ifndef QT_H
#include "qcolor.h"
#endif // QT_H


class Q_GUI_EXPORT QPen
{
public:
    QPen();
    QPen(Qt::PenStyle);
    QPen(const QColor &color, int width = 0, Qt::PenStyle style = Qt::SolidLine);
    QPen(const QColor &cl, int width, Qt::PenStyle s, Qt::PenCapStyle c, Qt::PenJoinStyle j);
    QPen(const QPen &pen);
   ~QPen();
    QPen &operator=(const QPen &pen);

    inline Qt::PenStyle style() const { return d->style; }
    void setStyle(Qt::PenStyle);
    inline int width() const { return d->width; }
    void setWidth(int width);
    inline QColor color() const { return d->color; }
    void setColor(const QColor &color);
    Qt::PenCapStyle        capStyle() const;
    void setCapStyle(Qt::PenCapStyle pcs);
    Qt::PenJoinStyle joinStyle() const;
    void setJoinStyle(Qt::PenJoinStyle pcs);

    bool operator==(const QPen &p) const;
    inline bool operator!=(const QPen &p) const { return !(operator==(p)); }

private:
    friend class QPainter;
    inline void detach() { if (d->ref != 1) detach_helper(); }
    void detach_helper();
    void init(const QColor &c, int width, uint linestyle);
    struct QPenData {
        QAtomic ref;
        Qt::PenStyle style;
        int width;
        QColor color;
        Q_UINT16 linest;
    };
    struct QPenData *d;
};


/*****************************************************************************
  QPen stream functions
 *****************************************************************************/
#ifndef QT_NO_DATASTREAM
Q_GUI_EXPORT QDataStream &operator<<(QDataStream &, const QPen &);
Q_GUI_EXPORT QDataStream &operator>>(QDataStream &, QPen &);
#endif

#endif // QPEN_H
