/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qclipboard_win.cpp#12 $
**
** Implementation of QClipboard class for Win32
**
** Created : 960430
**
** Copyright (C) 1996-1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qclipbrd.h"
#include "qapp.h"
#include "qpixmap.h"
#include "qdatetm.h"

#if defined(_CC_BOOL_DEF_)
#undef	bool
#include <windows.h>
#define bool int
#else
#include <windows.h>
#endif

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
    ClipboardFormat f = getFormat( format );
    switch ( f ) {
	case CFText:
	    break;
	case CFPixmap:
	    break;
	default:
#if defined(CHECK_RANGE)
	    warning( "QClipboard::data: Unknown format: %s", format );
#endif
	    return 0;
    }

    static QString *text   = 0;
    static QPixmap *pixmap = 0;

    if ( !OpenClipboard(clipboardOwner()->winId()) )
	return 0;

    HANDLE h = GetClipboardData( f );
    if ( h == 0 ) {				// no clipboard data
	CloseClipboard();
	return 0;
    }

    void *ptr = 0;
    if ( f == CFText ) {
	delete pixmap;
	pixmap = 0;
#if 1
	HANDLE htext = GlobalAlloc(GMEM_MOVEABLE, GlobalSize(h));
	char *src = (char *)GlobalLock(h);
	char *dst = (char *)GlobalLock(htext);
	strcpy( dst, src );
	delete text;
	text = new QString( dst );
	GlobalUnlock(h);
	GlobalUnlock(htext);
	GlobalFree(htext);
#else
	char *d = (char *)GlobalLock( h );
	delete text;
	text = new QString( d );
	GlobalUnlock( h );
#endif
	ptr = text->data();
    } else if ( f == CFPixmap ) {
	delete text;
	text = 0;
	BITMAP bm;
	HDC    hdc = GetDC( 0 );
	HDC    hdcMemSrc = CreateCompatibleDC( hdc );
	GetObject( h, sizeof(BITMAP), &bm );
	SelectObject( hdcMemSrc, h );
	delete pixmap;
	pixmap = new QPixmap( bm.bmWidth, bm.bmHeight );
	pixmap->detach();
	BitBlt( pixmap->handle(), 0,0, pixmap->width(), pixmap->height(),
		hdcMemSrc, 0, 0, SRCCOPY );
	DeleteDC( hdcMemSrc );
	ReleaseDC( 0, hdc );
	ptr = pixmap;
    }
    CloseClipboard();

    return ptr;
}


void QClipboard::setData( const char *format, void *data )
{
    ClipboardFormat f = getFormat( format );

    switch ( f ) {
	case CFText:
	    break;
	case CFPixmap:
	    break;
	default:
#if defined(CHECK_RANGE)
	    warning( "QClipboard::setData: Unknown format: %s", format );
#endif
	    return;
    }

    if ( !OpenClipboard(clipboardOwner()->winId()) )
	return;

    EmptyClipboard();

    if ( f == CFText ) {
	int len = strlen((char*)data);
	if ( len > 0 ) {
	    HANDLE h = GlobalAlloc( GHND, len+1 );
	    char *d = (char *)GlobalLock( h );
	    memcpy( d, data, len+1 );
	    GlobalUnlock( h );
	    SetClipboardData( f, h );
	}
    } else if ( f == CFPixmap ) {
	QPixmap *pixmap = (QPixmap *)data;
	if ( pixmap && !pixmap->isNull() ) {
	    BITMAP bm;
	    GetObject( pixmap->hbm(), sizeof(BITMAP), &bm );
	    HANDLE hbm = CreateBitmapIndirect( &bm );
	    HDC	   hdc = GetDC( 0 );
	    HDC	   hdcMemDst = CreateCompatibleDC( hdc );
	    SelectObject( hdcMemDst, hbm );
	    BitBlt( hdcMemDst, 0,0, bm.bmWidth, bm.bmHeight,
		    pixmap->handle(), 0, 0, SRCCOPY );
	    DeleteDC( hdcMemDst );
	    ReleaseDC( 0, hdc );
	    SetClipboardData( f, hbm );
	}
    }

    CloseClipboard();
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
    if ( e->type() != Event_Clipboard )
	return QObject::event( e );

    MSG *m = (MSG *)Q_CUSTOM_EVENT(e)->data();

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
