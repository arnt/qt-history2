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
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
**
*****************************************************************************/

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
	qDebug("Freeing Colormap");
	if ( valid && widget && map )
	    XFreeColormap( widget->x11AppDisplay(), map );
    }
    
    bool valid;
    int size;
    QArray< QRgb > cells;
    Colormap       map;
    QWidget * widget;
};


QColormap::QColormap()
{
    d = new QColormapPrivate();
}

QColormap::QColormap( QWidget * w )
{
    d = new QColormapPrivate();
    if ( !w ) 
	return;
	
    d->map = 0;
    d->map = XCreateColormap( w->x11AppDisplay(), w->winId(),
			      (Visual *) w->x11AppVisual(), AllocAll );
    if ( d->map ) {
	d->valid  = TRUE;
	d->size   = ((Visual *) w->x11AppVisual())->map_entries;
	d->widget = w;
	d->cells.resize( d->size );
    }
}

QColormap::QColormap( const QColormap & map )
{
    d = map.d;
    d->ref();
}

QColormap::~QColormap()
{
    qDebug("QColormap::~(): Destroying a colormap");
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
	qDebug("QColormap::detach(): Making a copy of the colormap");
	QColormapPrivate * newd = new QColormapPrivate();
	// copy internal data of QColormapPrivate here
	newd->widget = d->widget;
	newd->size   = d->size;
	newd->valid  = d->valid;
	newd->cells  = d->cells;
	newd->cells.detach();
	newd->map = XCreateColormap( d->widget->x11AppDisplay(), 
				     d->widget->winId(),
				     (Visual *) d->widget->x11AppVisual(),
				     AllocAll );
	if ( d->deref() )
	    delete d;
	d = newd;
	
	// Re-set the color cells in the new colormap
	for ( int x = 0; x < d->size; x++ )
	    setEntry( x, d->cells[ x ] );
    }
}

void QColormap::setEntry( int idx, QRgb color )
{    
#if defined(QT_CHECK_RANGE)
    if ( !d->valid ) {
	qWarning( "QColormap::setEntry: Not a valid colormap" );
	return;
    }
    
    if ( idx < 0 || idx > d->size ) {
	qWarning( "QColormap::setEntry: Index out of range" );
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
    XStoreColor( d->widget->x11AppDisplay(), d->map, &c );
    d->cells[ idx ] = color;
}

void QColormap::setEntry( int idx, const QColor & color )
{
    setEntry( idx, color.rgb() );
}

bool QColormap::isValid() const
{
    return d->valid;
}

const Qt::HANDLE QColormap::colormap() const
{
    return d->map;
}
