/****************************************************************************
** $Id: $
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

#include "private/qinternal_p.h"
#include "qwidget.h"
#include "qpixmap.h"
#include "qpainter.h"
#include "qcleanuphandler.h"

static QPixmap* qdb_shared_pixmap = 0;
static QSharedDoubleBuffer* qdb_owner = 0;
static QCleanupHandler<QPixmap> qdb_cleanup_pixmap;

#ifdef Q_WS_MACX
bool QSharedDoubleBuffer::dblbufr = FALSE;
#else
bool QSharedDoubleBuffer::dblbufr = TRUE;
#endif


/*
  hardLimitWidth/Height: if >= 0, the maximum number of pixels that get double buffered.

  sharedLimitWidth/Height: if >= 0, the maximum number of pixels the shared double buffer can keep.

  For x with sharedLimitSize < x <= hardLimitSize, temporary buffers are constructed.

 */
static const int hardLimitWidth = -1;
static const int hardLimitHeight = -1;
#if defined( Q_WS_QWS ) || defined( Q_WS_MAC9 )
// Small in Qt/Embedded / Mac9 - 5K on 32bpp
static const int sharedLimitWidth = 64;
static const int sharedLimitHeight = 20;
#else
// 240K on 32bpp
static const int sharedLimitWidth = 640;
static const int sharedLimitHeight = 100;
#endif

QSharedDoubleBuffer::QSharedDoubleBuffer( bool mustShare, bool initializeBg, QPixmap* pm )
    : wid(0), rx(0), ry(0), rw(0), rh(0), p(0), xp(0), pix(0),xpix(pm),mustsh(mustShare),initbg(initializeBg)
{
}


QSharedDoubleBuffer::QSharedDoubleBuffer( QWidget* widget,  int x, int y, int w, int h )
    : wid(0), rx(0), ry(0), rw(0), rh(0), p(0), xp(0), pix(0),xpix(0),mustsh(1),initbg(1)
{
    begin( widget, x, y, w, h );
}

QSharedDoubleBuffer::QSharedDoubleBuffer( QPainter* painter, int x, int y, int w, int h )
    : wid(0), rx(0), ry(0), rw(0), rh(0), p(0), xp(0), pix(0),xpix(0),mustsh(1),initbg(1)
{
    begin( painter, x, y, w, h );
}

QSharedDoubleBuffer::QSharedDoubleBuffer( QWidget *widget, const QRect &r )
    : wid(0), rx(0), ry(0), rw(0), rh(0), p(0), xp(0), pix(0),xpix(0),mustsh(1),initbg(1)
{
    begin( widget, r );
}

QSharedDoubleBuffer::QSharedDoubleBuffer( QPainter *painter, const QRect &r )
    : wid(0), rx(0), ry(0), rw(0), rh(0), p(0), xp(0), pix(0),xpix(0),mustsh(1),initbg(1)
{
    begin( painter, r );
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
        if ( w < 0 )
            w = wid ? wid->width() : 0;
        if ( h < 0 )
            h = wid ? wid->height() : 0;
        if ( dblbufr &&
             ( hardLimitWidth < 0 ||  w <= hardLimitWidth ) &&
             ( hardLimitHeight < 0 ||  h <= hardLimitHeight ) &&
             ( !mustsh || ( w <= sharedLimitWidth && h <= sharedLimitHeight )
               || ( xpix && w <= xpix->width() && h <= xpix->height() ) ) )
            return begin( (QWidget*) xp->device(), x, y, w, h );
        if ( initbg )
            ( (QWidget*) xp->device() )->erase( x, y, w, h );
    }
    rx = x;
    ry = y;
    rw = w;
    rh = h;
    p = xp;
    return TRUE;
}

QPixmap* QSharedDoubleBuffer::getRawPixmap( int w, int h )
{
    if ( w > sharedLimitWidth || h > sharedLimitHeight )
        return 0;

    if ( qdb_owner ) {
        qdb_cleanup_pixmap.remove( &qdb_shared_pixmap );
        qdb_shared_pixmap = new QPixmap( w, h );
        qdb_cleanup_pixmap.add( &qdb_shared_pixmap );
        qdb_owner = 0;
    } else {
        if ( !qdb_shared_pixmap  )
            qdb_shared_pixmap = new QPixmap( w, h );
        else if ( qdb_shared_pixmap->width() < w  || qdb_shared_pixmap->height() < h)
            qdb_shared_pixmap->resize( w, h );
    }	
    return qdb_shared_pixmap;
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
        h = wid->height();

    rx = x;
    ry = y;
    rw = w;
    rh = h;

    if ( !dblbufr ||
         ( hardLimitWidth >= 0 && w > hardLimitWidth ) ||
         ( hardLimitHeight >= 0 && h > hardLimitHeight ) ||
         ( mustsh  && ( w > sharedLimitWidth || h > sharedLimitWidth )
           && ! (xpix && w <= xpix->width() && h <= xpix->height() ) )  ||
         wid->backgroundMode() == Qt::X11ParentRelative ) {
        if ( initbg )
            wid->erase( x, y, w, h );
        p = new QPainter( widget );
        return TRUE;
    }

    if ( xpix ) {
        xpix->resize( w, h );
        pix = xpix;
    } else {
        if ( ( pix = getRawPixmap( w, h ) ) )
            qdb_owner = this;
        else
            pix = new QPixmap( w, h );
    }

    if ( initbg )
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
    if ( pix != qdb_shared_pixmap && pix != xpix )
        delete pix;
    pix = 0;
    xpix = 0;
    return TRUE;
}

void QSharedDoubleBuffer::flush()
{
    if ( !pix )
        return;
    if ( xp )
        xp->drawPixmap( rx, ry, *pix, 0, 0, rw, rh );
    else if ( wid && wid->isVisible() ) {
        QPainter p(wid);
        p.drawPixmap(rx, ry, *pix, 0, 0, rw, rh);
    }
}
