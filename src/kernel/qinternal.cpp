/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qinternal.cpp $
**
** Implementation of some internal classes
**
** Created : 010427
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
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
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
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

#include "qinternal_p.h"
#include "qwidget.h"
#include "qpixmap.h"
#include "qpainter.h"
#include "qcleanuphandler.h"

static QPixmap* qdb_shared_pixmap = 0;
static QSharedDoubleBuffer* qdb_owner = 0;
static QCleanupHandler<QPixmap> qdb_cleanup_pixmap;

static const bool buffer_disabled = FALSE;

QSharedDoubleBuffer::QSharedDoubleBuffer()
    : wid(0), rx(0), ry(0), rw(0), rh(0), p(0), xp(0), pix(0)
{
}


QSharedDoubleBuffer::QSharedDoubleBuffer( QWidget* widget, int x, int y, int w, int h )
    : wid(0), rx(0), ry(0), rw(0), rh(0), p(0), xp(0), pix(0)
{
    begin( widget, x, y, w, h );
}

QSharedDoubleBuffer::QSharedDoubleBuffer( QPainter* painter, int x, int y, int w, int h )
    : wid(0), rx(0), ry(0), rw(0), rh(0), p(0), xp(0), pix(0)
{
    begin( painter, x, y, w, h );
}

QSharedDoubleBuffer::QSharedDoubleBuffer( QWidget *widget, const QRect &r )
    : wid(0), rx(0), ry(0), rw(0), rh(0), p(0), xp(0), pix(0)
{
    begin( widget, r.x(), r.y(), r.width(), r.height() );
}

QSharedDoubleBuffer::QSharedDoubleBuffer( QPainter *painter, const QRect &r )
    : wid(0), rx(0), ry(0), rw(0), rh(0), p(0), xp(0), pix(0)
{
    begin( painter, r.x(), r.y(), r.width(), r.height() );
}


QSharedDoubleBuffer::~QSharedDoubleBuffer()
{
    if ( wid )
	end();
}

bool QSharedDoubleBuffer::begin( QPainter* painter, int x, int y, int w, int h )
{
    if ( isActive() ) {
#if defined(QT_CHECK_STATE)
        qWarning( "QSharedDoubleBuffer::begin: Buffer is already active."
                  "\n\tYou must end() the buffer before a second begin()" );
#endif
        return FALSE;
    }

    xp = painter;

    if ( xp->device()->devType() == QInternal::Widget ) {
	if (!buffer_disabled )
	    return begin( (QWidget*) xp->device(), x, y, w, h );
	if ( w < 0 )
	    w = wid->width();
	if ( h < 0 )
	    w = wid->height();
	( (QWidget*) xp->device() )->erase( x, y, w, h );
    }
    p = xp;
    return TRUE;
}

bool QSharedDoubleBuffer::begin( QWidget* widget, int x, int y, int w, int h )
{
    if ( isActive() ) {
#if defined(QT_CHECK_STATE)
        qWarning( "QSharedDoubleBuffer::begin: Buffer is already active."
                  "\n\tYou must end() the buffer before a second begin()" );
#endif
        return FALSE;
    }

    wid = widget;
    if ( !wid )
	return FALSE;

    if ( w < 0 )
	w = wid->width();
    if ( h < 0 )
	w = wid->height();

    if ( buffer_disabled  ) {
	wid->erase( x, y, w, h );
	p = new QPainter( widget );
	return TRUE;
    }

    rx = x;
    ry = y;
    rw = w;
    rh = h;
    if ( w * h <=  64000 ) {
	if ( qdb_owner ) {
	    qdb_cleanup_pixmap.remove( qdb_shared_pixmap );
	    pix = qdb_shared_pixmap = new QPixmap( w, h );
	    qdb_cleanup_pixmap.add( qdb_shared_pixmap );
	    qdb_owner = this;
	} else {
	    if ( !qdb_shared_pixmap  )
		qdb_shared_pixmap = new QPixmap( w, h );
	    else if ( qdb_shared_pixmap->width() < w  || qdb_shared_pixmap->height() < h)
		qdb_shared_pixmap->resize( w, h );
	    pix = qdb_shared_pixmap;
	    qdb_owner = this;
	}
    } else {
	pix = new QPixmap( w, h );
    }

    qdb_owner = this;
    pix->fill( wid, rx, ry );
    p = new QPainter( pix, wid );
    p->setBrushOrigin( -rx, -ry );
    p->translate( -rx, -ry );
    return TRUE;
}

bool QSharedDoubleBuffer::end()
{
    if ( !p )
	return FALSE;
    flush();
    wid = 0;
    if ( this == qdb_owner )
	qdb_owner = 0;
    if ( p != xp )
	delete p;
    p = 0;
    xp = 0;
    if ( pix != qdb_shared_pixmap )
	delete pix;
    pix = 0;
    return TRUE;
}

bool QSharedDoubleBuffer::isActive() const
{
    return p != 0;
}

void QSharedDoubleBuffer::flush()
{
    if ( !pix )
	return;
    if ( xp )
	xp->drawPixmap( rx, ry, *pix, 0, 0, rw, rh );
    else if ( wid && wid->isVisible() )
	bitBlt( wid, rx, ry, pix, 0, 0, rw, rh );


}
