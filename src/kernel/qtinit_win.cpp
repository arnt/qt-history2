/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qtinit_win.cpp#5 $
**
** Implementation of Win32 startup routines.
**
** Created : 981021
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit Professional Edition.
**
** Licensees with valid Qt Professional Edition licenses may distribute and
** use this file in accordance with the Qt Professional Edition License
** provided at sale or upon request.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing.
**
*****************************************************************************/

#if !defined(QAPPLICATION_WIN_CPP)

#include "qapplication.h"
#include "qpixmap.h"
#include "qt_windows.h"

#endif // QAPPLICATION_WIN_CPP


/*
  Internal functions.
*/

Q_EXPORT
void qt_draw_tiled_pixmap( HDC, int, int, int, int,
			   const QPixmap *, int, int );

static
void qt_erase_background( HDC hdc, int x, int y, int w, int h,
			  const QColor &bg_color,
			  const QPixmap *bg_pixmap, int off_x, int off_y )
{
    if ( bg_pixmap && bg_pixmap->isNull() )	// empty background
	return;
    HPALETTE oldPal;
    if ( QColor::hPal() ) {
	oldPal = SelectPalette( hdc, QColor::hPal(), FALSE );
	RealizePalette( hdc );
    }
    if ( bg_pixmap ) {
	qt_draw_tiled_pixmap( hdc, x, y, w, h, bg_pixmap, off_x, off_y );
    } else {
	HBRUSH brush = CreateSolidBrush( bg_color.pixel() );
	HBRUSH oldBrush = (HBRUSH)SelectObject( hdc, brush );
	PatBlt( hdc, x, y, w, h, PATCOPY );
	SelectObject( hdc, oldBrush );
	DeleteObject( brush );
    }
    if ( QColor::hPal() ) {
	SelectPalette( hdc, oldPal, TRUE );
	RealizePalette( hdc );
    }
}


#if defined(QT_BASEAPP)

Q_EXPORT void qt_ebg( void * );

QApplication::QApplication( int &argc, char **argv )
    : QBaseApplication( argc, argv )
{
    qt_ebg( qt_erase_background );
}

#endif
