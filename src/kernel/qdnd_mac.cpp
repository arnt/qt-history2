/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qdnd_mac.cpp
**
** DND implementation for mac.
**
** Created : 001019
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

#include "qapplication.h"

#ifndef QT_NO_DRAGANDDROP
#include "qwidget.h"
#include "qintdict.h"
#include "qdatetime.h"
#include "qdict.h"
#include "qdragobject.h"
#include "qobjectlist.h"
#include "qbitmap.h"


bool QDropEvent::provides( const char * ) const
{
    qDebug( "QDropEvent::provides" );
    return FALSE;
}

QByteArray QDropEvent::encodedData( const char * ) const
{
    qDebug( "QDropEvent::encodedData" );
    return QByteArray();
}

const char* QDropEvent::format( int ) const
{
    qDebug( "QDropEvent::format" );
    return "text/plain";
}

void QDragManager::timerEvent( QTimerEvent* )
{
    qDebug( "QDragManager::timerEvent" );
    return;
}

bool QDragManager::eventFilter( QObject *, QEvent * )
{
    qDebug( "QDragManager::eventFilter" );
    return FALSE;
}

void QDragManager::updateMode( ButtonState )
{
    qDebug( "QDragManager::updateMode" );
}

void QDragManager::updateCursor()
{
    qDebug( "QDragManager::updateCursor" );
}

void QDragManager::cancel( bool )
{
    qDebug( "QDragManager::cancel" );
}

void QDragManager::move( const QPoint & )
{
    qDebug( "QDragManager::move" );
}

void QDragManager::drop()
{
    qDebug( "QDragManager::drop" );
}

bool QDragManager::drag( QDragObject *, QDragObject::DragMode )
{
    qDebug( "QDragManager::drag" );
    return FALSE;
}

void QDragManager::updatePixmap()
{
    qDebug( "QDragManager::updatePixmap" );
}



#endif // QT_NO_DRAGANDDROP
