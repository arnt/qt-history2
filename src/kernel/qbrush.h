/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qbrush.h#11 $
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
    QBrush( const QColor &, const QBitMap & );
    QBrush( const QBrush & );
   ~QBrush();
    QBrush &operator=( const QBrush & );

    QBrush	copy() const;

    BrushStyle	style()	 const		{ return data->style; }
    QColor	color()	 const		{ return data->color; }
    QBitMap    *bitmap() const		{ return data->bitmap; }

    void	setStyle( BrushStyle );
    void	setColor( const QColor & );
    void	setBitMap( const QBitMap & );

    bool	operator==( const QBrush &p ) const;
    bool	operator!=( const QBrush &b ) const
					{ return !(operator==(b)); }

private:
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
	QBitMap	 *bitmap;
#if defined(_WS_WIN_)
	HANDLE	  hbrush;
	HANDLE	  hbmp;
	uint	  invalid    : 1;
	uint	  stockBrush : 1;
#elif defined(_WS_PM_)
	uint	  invalid    : 2;
#elif defined(_WS_X11_)
	Display	 *dpy;
	Pixmap	  pixmap;
#endif
    } *data;
};


// --------------------------------------------------------------------------
// QBrush stream functions
//

QDataStream &operator<<( QDataStream &, const QBrush & );
QDataStream &operator>>( QDataStream &, QBrush & );


#endif // QBRUSH_H
