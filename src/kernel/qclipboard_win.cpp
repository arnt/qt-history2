/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qclipboard_win.cpp#22 $
**
** Implementation of QClipboard class for Win32
**
** Created : 960430
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees with valid Qt Professional Edition licenses may distribute and
** use this file in accordance with the Qt Professional Edition License
** provided at sale or upon request.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#include "qclipboard.h"
#include "qapplication.h"
#include "qpixmap.h"
#include "qdatetime.h"
#include "qt_windows.h"


/*****************************************************************************
  Internal QClipboard functions for Win32.
 *****************************************************************************/

static HWND nextClipboardViewer = 0;
static bool inClipboardChain = FALSE;


static QWidget *clipboardOwner()
{
    static QWidget *owner = 0;
    if ( owner )				// owner already created
	return owner;
    if ( qApp->mainWidget() )			// there's a main widget
	owner = qApp->mainWidget();
    else					// otherwise create fake widget
	owner = new QWidget( 0, "internalClipboardOwner" );
    return owner;
}


typedef uint ClipboardFormat;

#define CFText	    CF_TEXT
#define CFPixmap    CF_BITMAP
#define CFNothing   0


static ClipboardFormat getFormat( const char *format )
{
    if ( strcmp(format,"TEXT") == 0 )
	 return CFText;
    else if ( strcmp(format,"PIXMAP") == 0 )
	return CFPixmap;
    return CFNothing;
}


/*****************************************************************************
  QClipboard member functions for Win32.
 *****************************************************************************/

void QClipboard::clear()
{
    OpenClipboard( clipboardOwner()->winId() );
    EmptyClipboard();
    CloseClipboard();
}


void *QClipboard::data( const char *format ) const
{
#if defined(CHECK_RANGE)
    warning( "QClipboard::data: Unknown format: %s", format );
#endif
    return 0;
}


void QClipboard::ownerDestroyed()
{
    if ( inClipboardChain ) {
	QWidget *owner = (QWidget *)sender();
	ChangeClipboardChain( owner->winId(), nextClipboardViewer );
    }
}


void QClipboard::connectNotify( const char *signal )
{
    if ( strcmp(signal,SIGNAL(dataChanged())) == 0 && !inClipboardChain ) {
	QWidget *owner = clipboardOwner();
	inClipboardChain = TRUE;
	nextClipboardViewer = SetClipboardViewer( owner->winId() );
	connect( owner, SIGNAL(destroyed()), SLOT(ownerDestroyed()) );
    }
}


bool QClipboard::event( QEvent *e )
{
    if ( e->type() != QEvent::Clipboard )
	return QObject::event( e );

    MSG *m = (MSG *)((QCustomEvent*)e)->data();

    switch ( m->message ) {

	case WM_CHANGECBCHAIN:
	    if ( (HWND)m->wParam == nextClipboardViewer )
		nextClipboardViewer = (HWND)m->lParam;
	    else if ( nextClipboardViewer )
		SendMessage( nextClipboardViewer, m->message,
			     m->wParam, m->lParam );
	    break;

	case WM_DRAWCLIPBOARD:
	    if ( nextClipboardViewer )
		SendMessage( nextClipboardViewer, m->message,
			     m->wParam, m->lParam );
	    emit dataChanged();
	    break;
    }

    return TRUE;
}

QString QClipboard::text() const
{
    // #### Only ASCII at the moment.  Add Unicode CF format.

    ClipboardFormat f = CFText;

    if ( !OpenClipboard(clipboardOwner()->winId()) )
	return 0;

    HANDLE h = GetClipboardData( f );
    if ( h == 0 ) {				// no clipboard data
	CloseClipboard();
	return 0;
    }

    QString text;
    HANDLE htext = GlobalAlloc(GMEM_MOVEABLE, GlobalSize(h));
    char *src = (char *)GlobalLock(h);
    char *dst = (char *)GlobalLock(htext);
    strcpy( dst, src );
    text = dst;
    GlobalUnlock(h);
    GlobalUnlock(htext);
    GlobalFree(htext);

    CloseClipboard();

    return text;
}

void QClipboard::setText( const QString &text )
{
    // #### Only ASCII at the moment.  Add Unicode CF format.

    ClipboardFormat f = CFText;

    if ( !OpenClipboard(clipboardOwner()->winId()) )
	return;

    EmptyClipboard();

    int len = text.length();
    if ( len > 0 ) {
	HANDLE h = GlobalAlloc( GHND, len+1 );
	char *d = (char *)GlobalLock( h );
	memcpy( d, text.ascii(), len+1 );
	GlobalUnlock( h );
	SetClipboardData( f, h );
    }

    CloseClipboard();
}


QPixmap QClipboard::pixmap() const
{
    ClipboardFormat f = CFPixmap;

    if ( !OpenClipboard(clipboardOwner()->winId()) )
	return 0;

    HANDLE h = GetClipboardData( f );
    if ( h == 0 ) {				// no clipboard data
	CloseClipboard();
	return 0;
    }

    BITMAP bm;
    HDC    hdc = GetDC( 0 );
    HDC    hdcMemSrc = CreateCompatibleDC( hdc );
    GetObject( h, sizeof(BITMAP), &bm );
    SelectObject( hdcMemSrc, h );
    QPixmap pixmap( bm.bmWidth, bm.bmHeight );
    BitBlt( pixmap.handle(), 0,0, pixmap.width(), pixmap.height(),
	    hdcMemSrc, 0, 0, SRCCOPY );
    DeleteDC( hdcMemSrc );
    ReleaseDC( 0, hdc );

    CloseClipboard();

    return pixmap;
}

void QClipboard::setPixmap( const QPixmap &pixmap )
{
    ClipboardFormat f = CFPixmap;

    if ( !OpenClipboard(clipboardOwner()->winId()) )
	return;

    EmptyClipboard();

    if ( !pixmap.isNull() ){
	BITMAP bm;
	GetObject( pixmap.hbm(), sizeof(BITMAP), &bm );
	HBITMAP hbm = CreateBitmapIndirect( &bm );
	HDC hdc = GetDC( 0 );
	HDC hdcMemDst = CreateCompatibleDC( hdc );
	SelectObject( hdcMemDst, hbm );
	BitBlt( hdcMemDst, 0,0, bm.bmWidth, bm.bmHeight,
		pixmap.handle(), 0, 0, SRCCOPY );
	DeleteDC( hdcMemDst );
	ReleaseDC( 0, hdc );
	SetClipboardData( f, hbm );
    }

    CloseClipboard();
}
