/****************************************************************************
** $Id: //depot/qt/fb/src/kernel/qdnd_fb.cpp#2 $
**
** XDND implementation for Qt.  See http://www.cco.caltech.edu/~jafl/xdnd/
**
** Created : 991026
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing.
**
*****************************************************************************/

#include "qapplication.h"

#ifndef QT_NO_DRAGANDDROP

#include "qwidget.h"
#include "qintdict.h"
#include "qdatetime.h"
#include "qdict.h"
#include "qdragobject.h"
#include "qobjectlist.h"
#include "qbitmap.h"

#if 0
static QPixmap *defaultPm = 0;

#define noDropCursorWidth 20
#define noDropCursorHeight 20
static unsigned char noDropCutBits[] = {
 0x00,0x00,0x00,0x80,0x1f,0x00,0xe0,0x7f,0x00,0xf0,0xf0,0x00,0x38,0xc0,0x01,
 0x7c,0x80,0x03,0xec,0x00,0x03,0xce,0x01,0x07,0x86,0x03,0x06,0x06,0x07,0x06,
 0x06,0x0e,0x06,0x06,0x1c,0x06,0x0e,0x38,0x07,0x0c,0x70,0x03,0x1c,0xe0,0x03,
 0x38,0xc0,0x01,0xf0,0xe0,0x00,0xe0,0x7f,0x00,0x80,0x1f,0x00,0x00,0x00,0x00};

static unsigned char noDropCutMask[] = {
 0x80,0x1f,0x00,0xe0,0x7f,0x00,0xf0,0xff,0x00,0xf8,0xff,0x01,0xfc,0xf0,0x03,
 0xfe,0xc0,0x07,0xfe,0x81,0x07,0xff,0x83,0x0f,0xcf,0x07,0x0f,0x8f,0x0f,0x0f,
 0x0f,0x1f,0x0f,0x0f,0x3e,0x0f,0x1f,0xfc,0x0f,0x1e,0xf8,0x07,0x3e,0xf0,0x07,
 0xfc,0xe0,0x03,0xf8,0xff,0x01,0xf0,0xff,0x00,0xe0,0x7f,0x00,0x80,0x1f,0x00};

static const int default_pm_hotx = -2;
static const int default_pm_hoty = -16;
static const char* default_pm[] = {
"13 9 3 1",
".      c None",
"       c #000000",
"X      c #FFFFFF",
"X X X X X X X",
" X X X X X X ",
"X ......... X",
" X.........X ",
"X ......... X",
" X.........X ",
"X ......... X",
" X X X X X X ",
"X X X X X X X",
};
#endif
void QDragManager::timerEvent( QTimerEvent*  )
{
}

bool QDragManager::eventFilter( QObject * , QEvent * )
{
    return FALSE;
}


void QDragManager::updateMode( ButtonState /*newstate*/ )
{
}


void QDragManager::updateCursor()
{
}


void QDragManager::cancel( bool /*deleteSource*/ )
{
}

void QDragManager::move( const QPoint & /*globalPos*/ )
{
}


void QDragManager::drop()
{
}

bool QDropEvent::provides( const char */*mimeType*/ ) const
{
    return FALSE;
}

QByteArray QDropEvent::encodedData( const char */*format*/ ) const
{
    return QByteArray();
}

const char* QDropEvent::format( int ) const
{
    return 0;
}

bool QDragManager::drag( QDragObject *, QDragObject::DragMode )
{
    return FALSE;
}

void QDragManager::updatePixmap()
{
}

#endif // QT_NO_DRAGANDDROP
