/****************************************************************************
** $Id: //depot/qt/main/src/opengl/qcolormap_win.cpp#0 $
**
** Implementation of QColormap class
**
** Created : 20010326
**
** Copyright (C) 1992-2001 Trolltech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition licenses may use this
** file in accordance with the Qt Commercial License Agreement provided
** with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qcolormap.h"
#include "qshared.h"
#include "qarray.h"
#include "qwidget.h"
#include "qt_windows.h"

class QColormapPrivate : public QShared
{
public:
    QColormapPrivate( QWidget * w ) { 
	valid  = FALSE;
	widget = w;
	size   = 0;

        LOGPALETTE * lpal = (LOGPALETTE *) malloc( sizeof(LOGPALETTE)
			    + 256 * sizeof(PALETTEENTRY) );

	lpal->palVersion    = 0x300;
	lpal->palNumEntries = 256;
	hpal = CreatePalette( lpal );

	if ( hpal != 0 ) {
	    size   = 256;
	    widget = w;
	    valid  = TRUE;
	    cells.resize( size );
	}
	free( lpal );
    }
    
    ~QColormapPrivate() {
	if ( widget && hpal ) {
	    HDC hdc = GetDC( widget->topLevelWidget()->winId() ); 
	    SelectPalette( hdc, (HPALETTE) GetStockObject( DEFAULT_PALETTE ),
			   FALSE );
	    DeleteObject( hpal );
	    ReleaseDC( widget->topLevelWidget()->winId(), hdc );
	}
    }
    
    bool valid;
    int size;
    HPALETTE   hpal;
    QWidget *  widget;
    QArray< QRgb > cells;
};


QColormap::QColormap( QWidget * w, const char * name )
    : QObject( w, name )
{
    d = new QColormapPrivate( w );
}

QColormap::QColormap( const QColormap & map )
    : QObject( map.d->widget, map.name() )
{
}

QColormap::~QColormap()
{
}

QColormap & QColormap::operator=( const QColormap & map )
{
    QColormap dummy( d->widget );
    return dummy;
}

void QColormap::detach()
{
}

void QColormap::setRgb( int idx, QRgb color )
{    
    if ( !d->valid ) {
	return;
    }

    PALETTEENTRY pe;
    pe.peRed   = qRed( color );
    pe.peGreen = qGreen( color );
    pe.peBlue  = qBlue( color );
    pe.peFlags = 0;

    SetPaletteEntries( d->hpal, idx, 1, &pe );
    d->cells[ idx ] = color;
    HDC hdc = GetDC( d->widget->topLevelWidget()->winId() );
    SelectPalette( hdc, d->hpal, FALSE );
    //RealizePalette( hdc );
    ReleaseDC( d->widget->topLevelWidget()->winId(), hdc );
}

void QColormap::setColor( int idx, const QColor & color )
{
    setRgb( idx, color.rgb() );
}

QRgb QColormap::rgb( int idx ) const
{
    return 0;
}

QColor QColormap::color( int idx ) const
{
    return QColor();
}

bool QColormap::isValid() const
{
    return d->valid;
}

Qt::HANDLE QColormap::colormap() const
{
    return 0;
}

int QColormap::size() const
{
    return d->size;
}
