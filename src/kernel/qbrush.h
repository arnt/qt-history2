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
#include "qshared.h"
#endif // QT_H


class Q_EXPORT QBrush: public Qt
{
friend class QPainter;
public:
    QBrush();
    QBrush( BrushStyle );
    QBrush( const QColor &, BrushStyle=SolidPattern );
    QBrush( const QColor &, const QPixmap & );
    QBrush( const QBrush & );
   ~QBrush();
    QBrush &operator=( const QBrush & );

    BrushStyle	style()	 const		{ return data->style; }
    void	setStyle( BrushStyle );
    const QColor &color()const		{ return data->color; }
    void	setColor( const QColor & );
    QPixmap    *pixmap() const		{ return data->pixmap; }
    void	setPixmap( const QPixmap & );

    bool	operator==( const QBrush &p ) const;
    bool	operator!=( const QBrush &b ) const
					{ return !(operator==(b)); }

private:
    QBrush	copy()	const;
    void	detach();
    void	init( const QColor &, BrushStyle );
    struct QBrushData : public QShared {	// brush data
	BrushStyle style;
	QColor	  color;
	QPixmap	 *pixmap;
    } *data;
};


/*****************************************************************************
  QBrush stream functions
 *****************************************************************************/

#ifndef QT_NO_DATASTREAM
Q_EXPORT QDataStream &operator<<( QDataStream &, const QBrush & );
Q_EXPORT QDataStream &operator>>( QDataStream &, QBrush & );
#endif

#endif // QBRUSH_H
