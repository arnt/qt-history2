/****************************************************************************
**
** Definition of QPen class.
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

#ifndef QPEN_H
#define QPEN_H

#ifndef QT_H
#include "qcolor.h"
#endif // QT_H


class Q_GUI_EXPORT QPen: public Qt
{
public:
    QPen();
    QPen(PenStyle);
    QPen(const QColor &color, int width = 0, PenStyle style = SolidLine);
    QPen(const QColor &cl, int width, PenStyle s, PenCapStyle c, PenJoinStyle j);
    QPen(const QPen &pen);
   ~QPen();
    QPen &operator=(const QPen &pen);

    inline PenStyle style() const { return d->style; }
    void setStyle( PenStyle );
    inline int width() const { return d->width; }
    void setWidth(int width);
    inline QColor color() const { return d->color; }
    void setColor(const QColor &color);
    PenCapStyle	capStyle() const;
    void setCapStyle(PenCapStyle pcs);
    PenJoinStyle joinStyle() const;
    void setJoinStyle(PenJoinStyle pcs);

    bool operator==(const QPen &p) const;
    inline bool operator!=(const QPen &p) const { return !(operator==(p)); }

private:
    friend class QPainter;
    inline void detach() { if (d->ref != 1) detach_helper(); }
    void detach_helper();
    void init(const QColor &c, int width, uint linestyle);
    struct QPenData {
	QAtomic ref;
	PenStyle style;
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
Q_GUI_EXPORT QDataStream &operator<<( QDataStream &, const QPen & );
Q_GUI_EXPORT QDataStream &operator>>( QDataStream &, QPen & );
#endif

#endif // QPEN_H
