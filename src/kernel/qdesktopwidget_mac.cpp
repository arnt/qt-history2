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
#include <qapplication.h>
#include "qt_mac.h"

class QDesktopWidgetPrivate
{
public:
    QDesktopWidgetPrivate();

    int appScreen;
    int screenCount;

    QMemArray<QRect> rects;
    QPtrList<GDPtr> devs;
};

QDesktopWidgetPrivate::QDesktopWidgetPrivate()
{
    appScreen = 0;
    QPtrList<QRect> rs;
    rs.setAutoDelete(TRUE);
    for(GDHandle g = GetMainDevice(); g; g = GetNextDevice(g)) {
	rs.append(new QRect((*g)->gdRect.left,    (*g)->gdRect.top,
			    (*g)->gdRect.right -  (*g)->gdRect.left,
			    (*g)->gdRect.bottom - (*g)->gdRect.top));
	devs.append(g);
    }
    int i = 0;
    rects.resize( screenCount = rs.count() );
    for(QPtrListIterator<QRect> it(rs); it.current(); ++it) 
	rects[i++] = *(*it);
}

QDesktopWidget::QDesktopWidget()
: QWidget( 0, "desktop", WType_Desktop )
{
    d = new QDesktopWidgetPrivate;
    setWState( WState_Visible );
}

QDesktopWidget::~QDesktopWidget()
{
    delete d;
}

bool QDesktopWidget::isVirtualDesktop() const
{
    return TRUE;
}

int QDesktopWidget::primaryScreen() const
{
    return d->appScreen;
}

int QDesktopWidget::numScreens() const
{
    return d->screenCount;
}

QWidget *QDesktopWidget::screen( int )
{
    return this;
}

const QRect& QDesktopWidget::screenGeometry( int screen ) const
{
    if ( screen < 0 || screen >= d->screenCount )
	screen = d->appScreen;
    return d->rects[ screen ];
}

int QDesktopWidget::screenNumber( QWidget *widget ) const
{
    if ( !widget )
	return d->appScreen;
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

GDHandle QDesktopWidget::handle( int screen ) const 
{
    if(screen > (int)d->devs.count())
	return NULL;
    GDHandle ret = d->devs.first();
    for(int i = 0; i < screen; i++)	
	ret = d->devs.next();
    return ret;
}
