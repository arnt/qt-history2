/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qbrush.h#1 $
**
** Definition of QBrush class
**
** Author  : Haavard Nord
** Created : 940112
**
** Copyright (C) 1994 by Troll Tech as.  All rights reserved.
**
*****************************************************************************/

#ifndef QBRUSH_H
#define QBRUSH_H

#include "qcolor.h"
#include "qshared.h"


enum BrushStyle { NoBrush, SolidBrush,		// brush style
	Pix1Pattern, Pix2Pattern, Pix3Pattern, Pix4Pattern, Pix5Pattern,
	HorPattern, VerPattern, CrossPattern,
	BDiagPattern, FDiagPattern, DiagCrossPattern };


class QBrush
{
friend class QPainter;
public:
    QBrush();
    QBrush( const QColor &, BrushStyle=SolidBrush );
    QBrush( const QBrush & );
   ~QBrush();
    QBrush &operator=( const QBrush & );

    BrushStyle style()	const		{ return data->style; }
    QColor    color()	const		{ return data->color; }

    void      setStyle( BrushStyle );
    void      setColor( const QColor & );

private:
#if defined(_WS_WIN_) || defined(_WS_WIN32_)
    bool      update( HDC );
#elif defined(_WS_PM_)
    bool      update( HPS );
#endif
    struct QBrushData : QShared {		// brush data
	BrushStyle style;
	QColor	  color;
#if defined(_WS_WIN_) || defined(_WS_WIN32_)
	HANDLE	  hbrush;
	HANDLE	  hbmp;
	int	  invalid    : 1;
	bool	  stockBrush : 1;
#elif defined(_WS_PM_)
	int	  invalid    : 2;
#elif defined(_WS_X11_)
	Display	 *dpy;
	Pixmap	  pixmap;
#endif
    } *data;
};


#endif // QBRUSH_H
