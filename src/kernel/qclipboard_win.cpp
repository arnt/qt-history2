/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qclipboard_win.cpp#37 $
**
** Implementation of QClipboard class for Win32
**
** Created : 960430
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit Professional Edition.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing.
**
*****************************************************************************/

#include "qclipboard.h"

#ifndef QT_NO_CLIPBOARD

#include "qapplication.h"
#include "qpixmap.h"
#include "qdatetime.h"
#include "qimage.h"
#include "qmime.h"
#include "qt_windows.h"


/*****************************************************************************
  Internal QClipboard functions for Win32.
 *****************************************************************************/

static HWND nextClipboardViewer = 0;
static bool inClipboardChain = FALSE;
extern Qt::WindowsVersion qt_winver;		// in qapplication_win.cpp


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


/*****************************************************************************
  QClipboard member functions for Win32.
 *****************************************************************************/

void QClipboard::ownerDestroyed()
{
    if ( inClipboardChain ) {
	QWidget *owner = (QWidget *)sender();
	ChangeClipboardChain( owner->winId(), nextClipboardViewer );
    }
}


void QClipboard::connectNotify( const char *signal )
{
    if ( qstrcmp(signal,SIGNAL(dataChanged())) == 0 && !inClipboardChain ) {
	QWidget *owner = clipboardOwner();
	inClipboardChain = TRUE;
	nextClipboardViewer = SetClipboardViewer( owner->winId() );
	connect( owner, SIGNAL(destroyed()), SLOT(ownerDestroyed()) );
    }
}

class QClipboardWatcher : public QMimeSource {
public:
    QClipboardWatcher()
    {
    }

    bool provides(const char* mime) const
    {
	bool r = FALSE;
	if ( OpenClipboard( clipboardOwner()->winId() ) ) {
	    int cf = 0;
	    while (!r && (cf = EnumClipboardFormats(cf))) {
		if ( QWindowsMime::convertor(mime,cf) )
		    r = TRUE;
	    }
	    CloseClipboard();
	}
	return r;
    }

    // Not efficient to iterate over this (so we provide provides() above).
    const char* format( int n ) const
    {
	const char* mime = 0;
	if ( n >= 0 ) {
	    if ( OpenClipboard( clipboardOwner()->winId() ) ) {
		int cf = 0;
		QList<QWindowsMime> all = QWindowsMime::all();
		while (cf = EnumClipboardFormats(cf)) {
		    mime = QWindowsMime::cfToMime(cf);
		    if ( mime ) {
			if ( !n )
			    break; // COME FROM HERE
			n--;
		    }
		}
		// COME FROM BREAK
		CloseClipboard();
	    }
	}
	return mime;
    }

    QByteArray encodedData( const char* mime ) const
    {
	QByteArray r;
	if ( OpenClipboard( clipboardOwner()->winId() ) ) {
	    QList<QWindowsMime> all = QWindowsMime::all();
	    for (QWindowsMime* c = all.first(); c; c = all.next()) {
		int cf = c->cfFor(mime);
		if ( cf ) {
		    HANDLE h = GetClipboardData(cf);
		    if ( h ) {
			char *src = (char *)GlobalLock(h);
			int s = GlobalSize(h);
			r.setRawData(src,s);
			QByteArray tr = c->convertToMime(r,mime,cf);
			tr.detach();
			r.resetRawData(src,s);
			GlobalUnlock(h);
			r = tr;
			break;
		    }
		}
	    }
	    CloseClipboard();
	}
	return r;
    }
};



class QClipboardData
{
public:
    QClipboardData();
   ~QClipboardData();

    void setSource(QMimeSource* s)
	{ delete src; src = s; }
    QMimeSource* source()
	{ return src; }
    QMimeSource* provider()
	{ if (!prov) prov = new QClipboardWatcher(); return prov; }

private:
    QMimeSource* src;
    QMimeSource* prov;
};

QClipboardData::QClipboardData()
{
    src = 0;
    prov = 0;
}

QClipboardData::~QClipboardData()
{
    delete src;
    delete prov;
}

static QClipboardData *internalCbData;

static void cleanupClipboardData()
{
    delete internalCbData;
}

static QClipboardData *clipboardData()
{
    if ( internalCbData == 0 ) {
	internalCbData = new QClipboardData;
	CHECK_PTR( internalCbData );
	qAddPostRoutine( cleanupClipboardData );
    }
    return internalCbData;
}


//# QT_DEBUG_CB

static void renderFormat(int cf)
{
#if defined(QT_DEBUG_CB)
    qDebug("renderFormat(%d)",cf);
#endif
    if ( !internalCbData )			// Spurious Windows message
	return;
    QMimeSource *s = internalCbData->source();
    if ( !s )					// Spurious Windows message
	return;
    const char* mime;
    for (int i=0; (mime=s->format(i)); i++) {
	QWindowsMime* c = QWindowsMime::convertor(mime,cf);
	if ( c ) {
	    QByteArray md = s->encodedData(mime);
#if defined(QT_DEBUG_CB)
	    qDebug("source is %d bytes of %s",md.size(),mime);
#endif
	    md = c->convertFromMime(md,mime,cf);
	    int len = md.size();
#if defined(QT_DEBUG_CB)
	    qDebug("rendered %d bytes of CF %d by %s",len,cf,c->convertorName());
#endif
	    HANDLE h = GlobalAlloc( GHND, len );
	    char *d = (char *)GlobalLock( h );
	    memcpy( d, md.data(), len );
	    SetClipboardData( cf, h );		
	    GlobalUnlock( h );
	    return;
	}
    }
}

static void renderAllFormats()
{
#if defined(QT_DEBUG_CB)
    qDebug("renderAllFormats");
#endif
    if ( !internalCbData )			// Spurious Windows message
	return;
    QMimeSource *s = internalCbData->source();
    if ( !s )					// Spurious Windows message
	return;
    // ###### Warwick?
}


bool QClipboard::event( QEvent *e )
{
    if ( e->type() != QEvent::Clipboard )
	return QObject::event( e );

    MSG *m = (MSG *)((QCustomEvent*)e)->data();

    bool propagate=FALSE;
    switch ( m->message ) {

	case WM_CHANGECBCHAIN:
	    if ( (HWND)m->wParam == nextClipboardViewer )
		nextClipboardViewer = (HWND)m->lParam;
	    else
		propagate = TRUE;
	    break;

	case WM_DRAWCLIPBOARD:
	    propagate = TRUE;
	    emit dataChanged();
	    break;

	case WM_RENDERFORMAT:
	    renderFormat(m->wParam);
	    break;
	case WM_RENDERALLFORMATS:
	    renderAllFormats();
	    break;
    }
    if ( propagate && nextClipboardViewer ) {
	if ( qt_winver & Qt::WV_NT_based )
	    SendMessage( nextClipboardViewer, m->message,
			 m->wParam, m->lParam );
	else
	    SendMessageA( nextClipboardViewer, m->message,
			 m->wParam, m->lParam );
    }

    return TRUE;
}


void QClipboard::clear()
{
    if ( OpenClipboard( clipboardOwner()->winId() ) ) {
	EmptyClipboard();
	CloseClipboard();
    }
}


QMimeSource* QClipboard::data() const
{
    QClipboardData *d = clipboardData();
    return d->provider();
}

void QClipboard::setData( QMimeSource* src )
{
    if ( !OpenClipboard(clipboardOwner()->winId()) )
	return;

    QClipboardData *d = clipboardData();
    d->setSource( src );

    EmptyClipboard();

    // Register all the formats of src that we can render.
    const char* mime;
    QList<QWindowsMime> all = QWindowsMime::all();
    for (int i = 0; mime = src->format(i); i++) {
	for (QWindowsMime* c = all.first(); c; c = all.next()) {
	    if ( c->cfFor(mime) ) {
		for (int j = 0; j < c->countCf(); j++) {
		    int cf = c->cf(j);
		    if ( c->canConvert(mime,cf) ) {
			SetClipboardData( cf, 0 ); // 0 == ask me later
		    }
		}
	    }
	}
    }

    CloseClipboard();
}

#endif // QT_NO_CLIPBOARD
