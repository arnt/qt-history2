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

// REVISED: arnt


/*****************************************************************************
  QClipboard member functions for mac.
 *****************************************************************************/

/*!
  Clears the clipboard contents.
*/

void QClipboard::clear()
{
}


/*!
  \internal
  Internal cleanup for Windows.
*/

void QClipboard::ownerDestroyed()
{
}


/*!
  \internal
  Internal optimization for Windows.
*/

void QClipboard::connectNotify( const char * )
{
}


/*!\reimp
*/

bool QClipboard::event( QEvent * )
{
    return TRUE;
}






/*!
  Returns a reference to a QMimeSource representation of the current
  clipboard data.
*/
QMimeSource* QClipboard::data() const
{
    return 0;
}

/*!
  Sets the clipboard data.  Ownership of the data is transferred to
  the clipboard - the only ways to remove this data is to set
  something else, or to call clear().  The QDragObject subclasses are
  reasonable things to put on the clipboard (but do not try to call
  QDragObject::drag() on the same object).  Any QDragObject placed in
  the clipboard should have a parent of 0.  Do not put QDragMoveEvent
  or QDropEvent subclasses on the clipboard, as they do not belong to
  the event handler which receives them.

  The setText() and setPixmap() functions are simpler wrappers for this.
*/
void QClipboard::setData( QMimeSource* )
{
}

void QClipboard::setSelectionMode(bool)
{
}


bool QClipboard::selectionModeEnabled() const
{
    return FALSE;
}

bool QClipboard::supportsSelection() const
{
    return FALSE;
}

#endif // QT_NO_CLIPBOARD
