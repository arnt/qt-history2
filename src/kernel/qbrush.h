/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qbrush.h#36 $
**
** Definition of QBrush class
**
** Created : 940112
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees with valid Qt Professional Edition licenses may distribute and
** use this file in accordance with the Qt Professional Edition License
** provided at sale or upon request.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

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

Q_EXPORT QDataStream &operator<<( QDataStream &, const QBrush & );
Q_EXPORT QDataStream &operator>>( QDataStream &, QBrush & );


#endif // QBRUSH_H
