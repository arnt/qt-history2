/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qbrush.h#19 $
**
** Definition of QBrush class
**
** Author  : Haavard Nord
** Created : 940112
**
** Copyright (C) 1994,1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QBRUSH_H
#define QBRUSH_H

#include "qcolor.h"
#include "qshared.h"


enum BrushStyle					// brush style
      { NoBrush, SolidPattern,
	Dense1Pattern, Dense2Pattern, Dense3Pattern, Dense4Pattern,
	Dense5Pattern, Dense6Pattern, Dense7Pattern,
	HorPattern, VerPattern, CrossPattern,
	BDiagPattern, FDiagPattern, DiagCrossPattern, CustomPattern=24 };


class QBrush
{
friend class QPainter;
public:
    QBrush();
    QBrush( BrushStyle );
    QBrush( const QColor &, BrushStyle=SolidPattern );
    QBrush( const QColor &, const QBitmap & );
    QBrush( const QBrush & );
   ~QBrush();
    QBrush &operator=( const QBrush & );

    BrushStyle	style()	 const		{ return data->style; }
    const QColor &color()const		{ return data->color; }
    QBitmap    *bitmap() const		{ return data->bitmap; }

    void	setStyle( BrushStyle );
    void	setColor( const QColor & );
    void	setBitmap( const QBitmap & );

    bool	operator==( const QBrush &p ) const;
    bool	operator!=( const QBrush &b ) const
					{ return !(operator==(b)); }

private:
    QBrush	copy()	const;
    void	detach();
#if defined(_WS_WIN_)
    bool	update( HDC );
#elif defined(_WS_PM_)
    bool	update( HPS );
#endif
    void	init( const QColor &, BrushStyle );
    void	reset();
    struct QBrushData : QShared {		// brush data
	BrushStyle style;
	QColor	  color;
	QBitmap	 *bitmap;
#if defined(_WS_WIN_)
	HANDLE	  hbrush;
	HANDLE	  hbm;
	uint	  invalid    : 1;
	uint	  stockBrush : 1;
#elif defined(_WS_PM_)
	uint	  invalid    : 2;
#elif defined(_WS_X11_)
	Display	 *dpy;
#endif
    } *data;
};


// --------------------------------------------------------------------------
// QBrush stream functions
//

QDataStream &operator<<( QDataStream &, const QBrush & );
QDataStream &operator>>( QDataStream &, QBrush & );


#endif // QBRUSH_H
