/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpen.h#12 $
**
** Definition of QPen class
**
** Author  : Haavard Nord
** Created : 940112
**
** Copyright (C) 1994,1995 by Troll Tech AS.  All rights reserved.
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
    uint	width() const		{ return data->width; }
    QColor	color() const		{ return data->color; }

    void	setStyle( PenStyle );
    void	setWidth( uint );
    void	setColor( const QColor & );

    bool	operator==( const QPen &p ) const;
    bool	operator!=( const QPen &p ) const
					{ return !(operator==(p)); }

private:
    void	detach();
#if defined(_WS_WIN_)
    void	update( HDC );
#elif defined(_WS_PM_)
    void	update( HPS );
#endif
    void	init( const QColor &, uint, PenStyle );
    struct QPenData : QShared {			// pen data
	PenStyle  style;
	uint	  width;
	QColor	  color;
#if defined(_WS_WIN_)
	HANDLE	  hpen;
	uint	  invalid  : 1;
	uint	  stockPen : 1;
#elif defined(_WS_PM_)
	uint	  invalid  : 3;
#endif
    } *data;
};


// --------------------------------------------------------------------------
// QPen stream functions
//

QDataStream &operator<<( QDataStream &, const QPen & );
QDataStream &operator>>( QDataStream &, QPen & );


#endif // QPEN_H
