/****************************************************************************
** $Id: //depot/qt/main/src/opengl/qcolormap_x11.cpp#0 $
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
#include "qmemarray.h"
#include "qwidget.h"
#include "qt_x11.h"

class QColormapPrivate : public QShared
{
public:
    QColormapPrivate() {
	valid  = FALSE;
	size   = 0;
	cells  = 0;
	map    = 0;
	topLevelWidget = 0;
    }

    ~QColormapPrivate() {
	if ( valid && topLevelWidget && map )
	    XFreeColormap( topLevelWidget->x11Display(), map );
    }

    bool valid;
    int size;
    QMemArray< QRgb > cells;
    Colormap  map;
    QWidget * topLevelWidget;
};

QColormap::QColormap( QWidget * w )
    : QObject( w->topLevelWidget() )
{
    d = new QColormapPrivate();
    create( w );
}

QColormap::QColormap( const QColormap & map )
    : QObject( map.d->topLevelWidget )
{
    d = map.d;
    d->ref();
}

QColormap::~QColormap()
{
    if ( d && d->deref() ) {
	delete d;
	d = 0;
    }
}

void QColormap::create( QWidget * widget )
{
    if ( !widget )
	return;

    // colormaps can only be installed in toplevel widgets
    QWidget * w = widget->topLevelWidget();     

    bool validVisual = FALSE;
    int  numVisuals;
    long mask;
    XVisualInfo templ;
    XVisualInfo *visuals;
    VisualID id = XVisualIDFromVisual( (Visual *) w->x11Visual() );

    mask = VisualScreenMask;
    templ.screen = w->x11Screen();
    visuals = XGetVisualInfo( w->x11Display(), mask, &templ, &numVisuals );

    for ( int i = 0; i < numVisuals; i++ ) {
	if ( visuals[i].visualid == id ) {
	    switch ( visuals[i].c_class ) {
		case TrueColor:
		case StaticColor:
		case StaticGray:
		    validVisual = FALSE;
		    break;
		case DirectColor:
		case PseudoColor:
		case GrayScale:
		    validVisual = TRUE;
		    break;
	    }
	    break;
	}
    }
    XFree( visuals );

    if ( !validVisual ) {
	qWarning( "QColormap: Cannot create a read/write "
		  "colormap for this visual (ID = 0x%x).", (uint) id );
	return;
    }

    d->map = XCreateColormap( w->x11Display(), w->winId(),
			      (Visual *) w->x11Visual(), AllocAll );
    if ( d->map ) {
	XSetWindowColormap( w->x11Display(), w->winId(),
			    d->map );
	d->valid  = TRUE;
	d->size   = ((Visual *) w->x11Visual())->map_entries;
	d->topLevelWidget = w;
	d->cells.resize( d->size );
    }
}

void QColormap::install( QWidget * w )
{    
    if ( w && d->valid && d->map ) { 
	XSetWindowColormap( w->topLevelWidget()->x11Display(), 
			    w->topLevelWidget()->winId(), d->map );
    }
}

QColormap & QColormap::operator=( const QColormap & map )
{
    map.d->ref();
    if ( d->deref() )
	delete d;
    d = map.d;

    return *this;
}

void QColormap::detach()
{
    if ( d->count != 1 ) {
	QColormapPrivate * newd = new QColormapPrivate();
	newd->topLevelWidget = d->topLevelWidget;
	newd->size   = d->size;
	newd->valid  = d->valid;
	newd->cells  = d->cells;
	newd->cells.detach();
	newd->map = XCreateColormap( d->topLevelWidget->x11Display(),
				     d->topLevelWidget->winId(),
				     (Visual *) d->topLevelWidget->x11Visual(),
				     AllocAll );
	if ( d->deref() )
	    delete d;
	d = newd;

	// Re-set the color cells in the new colormap
	for ( int x = 0; x < d->size; x++ )
	    setRgb( x, d->cells[ x ] );
    }
}

void QColormap::setRgb( int idx, QRgb color )
{
    if ( !d->valid ) {
	return;
    }

#if defined(QT_CHECK_RANGE)
    if ( idx < 0 || idx > d->size ) {
	qWarning( "QColormap::setRgb: Index out of range." );
	return;
    }
#endif

    XColor c;

    detach();
    c.pixel = idx;
    c.red   = (ushort)( (qRed(color) / 255.0) * 65535.0 + 0.5 );
    c.green = (ushort)( (qGreen(color) / 255.0) * 65535.0 + 0.5 );
    c.blue  = (ushort)( (qBlue(color) / 255.0) * 65535.0 + 0.5 );
    c.flags = DoRed | DoGreen | DoBlue;
    XStoreColor( d->topLevelWidget->x11Display(), d->map, &c );
    d->cells[ idx ] = color;
}

void QColormap::setRgb( int base, int count, const QRgb * colors )
{
    if ( !colors || base < 0 || base >= d->size )
	return;
    
    for( int i = base; i < base + count; i++ ) {
	if ( i < d->size )
	    setRgb( i, colors[i] );
	else
	    break;
    }
}

QRgb QColormap::rgb( int idx ) const
{
    if ( !d->valid || idx < 0 || idx > d->size )
	return 0;
    else
	return d->cells[ idx ];
}

void QColormap::setColor( int idx, const QColor & color )
{
    setRgb( idx, color.rgb() );
}

QColor QColormap::color( int idx ) const
{
    if ( !d->valid || idx < 0 || idx > d->size )
	return QColor();
    else
	return QColor( d->cells[ idx ] );
}

bool QColormap::isValid() const
{
    return d->valid;
}

int QColormap::size() const
{
    return d->size;
}
