/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qclipboard_mac.cpp
**
** Implementation of QClipboard class for mac
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
** licenses for Unix/X11/FIXME may use this file in accordance with the Qt Commercial
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

#include "qclipboard.h"

#ifndef QT_NO_CLIPBOARD

// #define QCLIPBOARD_DEBUG

#include "qapplication.h"
#include "qbitmap.h"
#include "qdatetime.h"
#include "qdragobject.h"
#include "qbuffer.h"
#include "qapplication_p.h"
#include "qt_mac.h"

/*****************************************************************************
  QClipboard member functions for mac.
 *****************************************************************************/

void QClipboard::clear()
{
    ClearCurrentScrap();
    scrap = NULL;
}


void QClipboard::ownerDestroyed()
{
}


void QClipboard::connectNotify( const char * )
{
}


bool QClipboard::event( QEvent * )
{
    return TRUE;
}


QMimeSource* QClipboard::data() const
{
    qDebug("Pasting fra clipboard..");
    return 0;
}

void QClipboard::setData( QMimeSource *src )
{
    qDebug("Copying til clipboard..");
    
}

void QClipboard::setSelectionMode(bool)
{
}


bool QClipboard::selectionModeEnabled() const
{
    return FALSE; //nei takk
}

bool QClipboard::supportsSelection() const
{
    return FALSE; //nei takk
}

void QClipboard::loadClipboard(bool)
{
    LoadScrap();
    GetCurrentScrap(&scrap);
}

void QClipboard::saveClipboard()
{
    UnloadScrap();
    scrap = NULL;
}


#endif // QT_NO_CLIPBOARD
