/****************************************************************************
** $Id$
**
** Implementation of QDesktopWidget class.
**
** Created : 
**
** Copyright (C) 1992-2001 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
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
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Unix/X11 may use this file in accordance with the Qt Commercial
** License Agreement provided with the Software.
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

#include "qdesktopwidget.h"
#include "qt_x11.h"

extern int qt_x11_create_desktop_on_screen; // defined in qwidget_x11.cpp

class QDesktopWidgetPrivate
{
public:
    QDesktopWidgetPrivate();
    ~QDesktopWidgetPrivate();

    bool use_xinerama;
    int defaultScreen;
    int screenCount;

    QWidget **screens;
    QMemArray<QRect> rects;
};

QDesktopWidgetPrivate::QDesktopWidgetPrivate()
    : use_xinerama(FALSE)
{

#ifndef QT_NO_XINERAMA
    XineramaScreenInfo *xinerama_screeninfo = 0;
    int unused;
    use_xinerama = (XineramaQueryExtension(QPaintDevice::x11AppDisplay(),
					   &unused, &unused) &&
		    XineramaIsActive(QPaintDevice::x11AppDisplay()));

    if (use_xinerama) {
	xinerama_screeninfo =
	    XineramaQueryScreens(QPaintDevice::x11AppDisplay(), &screenCount);
	defaultScreen = 0;
    } else
#endif // QT_NO_XINERAMA
    {
	defaultScreen = DefaultScreen(QPaintDevice::x11AppDisplay());
	screenCount = ScreenCount(QPaintDevice::x11AppDisplay());
    }

    // get the geometry of each screen
    rects.resize( screenCount );
    int i, x, y, w, h;
    for (i = 0; i < screenCount; i++) {

#ifndef QT_NO_XINERAMA
	if (use_xinerama) {
	    x = xinerama_screeninfo[i].x_org;
	    y = xinerama_screeninfo[i].y_org;
	    w = xinerama_screeninfo[i].width;
	    h = xinerama_screeninfo[i].height;
	} else
#endif // QT_NO_XINERAMA

	    {
		x = 0;
		y = 0;
		w = WidthOfScreen(ScreenOfDisplay(QPaintDevice::x11AppDisplay(), i));
		h = HeightOfScreen(ScreenOfDisplay(QPaintDevice::x11AppDisplay(), i));
	    }

	rects[i].setRect(x, y, w, h);
    }

    screens = 0;

#ifndef QT_NO_XINERAMA
    if (xinerama_screeninfo)
	XFree(xinerama_screeninfo);
#endif // QT_NO_XINERAMA

}

QDesktopWidgetPrivate::~QDesktopWidgetPrivate()
{
    if (! screens)
	return;

    for ( int i = 0; i < screenCount; ++i ) {
	if (i == defaultScreen) continue;

	delete screens[ i ];
	screens[i] = 0;
    }

    delete [] screens;
}

// the QDesktopWidget itself will be created on the default screen
// as qt_x11_create_desktop_on_screen defaults to -1
QDesktopWidget::QDesktopWidget()
: QWidget( 0, "desktop", WType_Desktop )
{
    d = new QDesktopWidgetPrivate;
}

QDesktopWidget::~QDesktopWidget()
{
    delete d;
}

bool QDesktopWidget::isVirtualDesktop() const
{
    return d->use_xinerama;
}

int QDesktopWidget::primaryScreen() const
{
    return d->defaultScreen;
}

int QDesktopWidget::numScreens() const
{
    return d->screenCount;
}

QWidget *QDesktopWidget::screen( int screen )
{
    if (d->use_xinerama)
	return this;

    if ( screen < 0 || screen >= d->screenCount )
	screen = d->defaultScreen;

    if ( ! d->screens ) {
	memset( (d->screens = new QWidget*[d->screenCount] ), 0,
		d->screenCount * sizeof( QWidget*) );
	d->screens[d->defaultScreen] = this;
    }

    if ( ! d->screens[screen] ||               // not created yet
	 ! d->screens[screen]->isDesktop() ) { // reparented away
	qt_x11_create_desktop_on_screen = screen;
	d->screens[screen] = new QWidget( 0, "desktop", WType_Desktop );
	qt_x11_create_desktop_on_screen = -1;
    }

    return d->screens[screen];
}

const QRect& QDesktopWidget::screenGeometry( int screen ) const
{
    if ( screen < 0 || screen >= d->screenCount )
	screen = d->defaultScreen;

    return d->rects[ screen ];
}

int QDesktopWidget::screenNumber( QWidget *widget ) const
{
    if ( !widget )
	return d->defaultScreen;
    QRect frame = widget->frameGeometry();
    if ( !widget->isTopLevel() )
	frame.moveTopLeft( widget->mapToGlobal( frame.topLeft() ) );

    int maxSize = -1;
    int maxScreen = -1;

    for ( int i = 0; i < d->screenCount; ++i ) {
	QRect sect = d->rects[i].intersect( frame );
	int size = sect.width() * sect.height();
	if ( size > maxSize && sect.width() > 0 && sect.height() > 0 ) {
	    maxSize = size;
	    maxScreen = i;
	}
    }
    return maxScreen;
}

int QDesktopWidget::screenNumber( const QPoint &point ) const
{
    for ( int i = 0; i < d->screenCount; ++i ) {
	if ( d->rects[i].contains( point ) )
	    return i;
    }
    return -1;
}
