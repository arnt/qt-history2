/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpen.h#24 $
**
** Definition of QPen class
**
** Created : 940112
**
** Copyright (C) 1994-1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QPEN_H
#define QPEN_H

#include "qcolor.h"
#include "qshared.h"


enum PenStyle { NoPen, SolidLine, DashLine,	// pen style
		DotLine, DashDotLine, DashDotDotLine };


class QPen
{
friend class QPainter;
public:
    QPen();
    QPen( PenStyle );
    QPen( const QColor &color, uint width=0, PenStyle style=SolidLine );
    QPen( const QPen & );
   ~QPen();
    QPen &operator=( const QPen & );

    PenStyle	style() const		{ return data->style; }
    void	setStyle( PenStyle );
    uint	width() const		{ return data->width; }
    void	setWidth( uint );
    const QColor &color() const		{ return data->color; }
    void	setColor( const QColor & );

    bool	operator==( const QPen &p ) const;
    bool	operator!=( const QPen &p ) const
					{ return !(operator==(p)); }

private:
    QPen	copy()	const;
    void	detach();
    void	init( const QColor &, uint, PenStyle );
    struct QPenData : public QShared {		// pen data
	PenStyle  style;
	uint	  width;
	QColor	  color;
    } *data;
};


/*****************************************************************************
  QPen stream functions
 *****************************************************************************/

QDataStream &operator<<( QDataStream &, const QPen & );
QDataStream &operator>>( QDataStream &, QPen & );


#endif // QPEN_H
