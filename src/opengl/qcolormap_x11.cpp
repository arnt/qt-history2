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
#include "qarray.h"
#include "qwidget.h"
#include "qt_x11.h"

class QColormapPrivate : public QShared
{
public:
    QColormapPrivate() { 
	valid  = FALSE; 
	size   = 0;
	widget = 0;
	cells  = 0;
	map    = 0;
    }
    
    ~QColormapPrivate() {
	if ( valid && widget && map )
	    XFreeColormap( widget->x11Display(), map );
    }
    
    bool valid;
    int size;
    QArray< QRgb > cells;
    Colormap       map;
    QWidget * widget;
};


QColormap::QColormap( QWidget * w, const char * name )
    : QObject( w, name )
{
    d = new QColormapPrivate();
    if ( !w ) 
	return;
	
    d->map = 0;
    d->map = XCreateColormap( w->x11Display(), w->topLevelWidget()->winId(),
			      (Visual *) w->x11Visual(), AllocAll );
    if ( d->map ) {
	XSetWindowColormap( w->x11Display(), w->topLevelWidget()->winId(), 
			    d->map );
	d->valid  = TRUE;
	d->size   = ((Visual *) w->x11Visual())->map_entries;
	d->widget = w;
	d->cells.resize( d->size );
    }
}

QColormap::QColormap( const QColormap & map )
    : QObject( map.d->widget, map.name() )
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
	// copy internal data of QColormapPrivate here
	newd->widget = d->widget;
	newd->size   = d->size;
	newd->valid  = d->valid;
	newd->cells  = d->cells;
	newd->cells.detach();
	newd->map = XCreateColormap( d->widget->x11Display(), 
				     d->widget->topLevelWidget()->winId(),
				     (Visual *) d->widget->x11Visual(),
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
#if defined(QT_CHECK_RANGE)
    if ( !d->valid ) {
	qWarning( "QColormap::setRgb: Not a valid colormap" );
	return;
    }
    
    if ( idx < 0 || idx > d->size ) {
	qWarning( "QColormap::setRgb: Index out of range" );
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
    XStoreColor( d->widget->x11Display(), d->map, &c );
    d->cells[ idx ] = color;
}

QRgb QColormap::rgb( int idx ) const
{
    return d->cells[ idx ];
}

void QColormap::setColor( int idx, const QColor & color )
{
    setRgb( idx, color.rgb() );
}

QColor QColormap::color( int idx ) const
{
    return QColor( d->cells[ idx ] );
}

bool QColormap::isValid() const
{
    return d->valid;
}

Qt::HANDLE QColormap::colormap() const
{
    return d->map;
}

int QColormap::size() const
{
    return d->size;
}
