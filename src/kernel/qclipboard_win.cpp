/****************************************************************************
** $Id: $
**
** Implementation of QClipboard class for Win32
**
** Created : 960430
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Windows may use this file in accordance with the Qt Commercial
** License Agreement provided with the Software.
**
** This file is not available for use under any other license without
** express written permission from the copyright holder.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qclipboard.h"

#ifndef QT_NO_CLIPBOARD

#include "qapplication.h"
#include "qapplication_p.h"
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
static QWidget *qt_cb_owner = 0;

static QWidget *clipboardOwner()
{
    if ( !qt_cb_owner )
	qt_cb_owner = new QWidget( 0, "internal clipboard owner" );
    return qt_cb_owner;
}


typedef uint ClipboardFormat;

#define CFText	    CF_TEXT
#define CFPixmap    CF_BITMAP
#define CFNothing   0


/*****************************************************************************
  QClipboard member functions for Win32.
 *****************************************************************************/

bool QClipboard::supportsSelection() const
{
    return FALSE;
}


bool QClipboard::ownsSelection() const
{
    return FALSE;
}


bool QClipboard::ownsClipboard() const
{
    qWarning("QClipboard::ownsClipboard: UNIMPLEMENTED!");
    return FALSE;
}


void QClipboard::setSelectionMode(bool)
{
}


bool QClipboard::selectionModeEnabled() const
{
    return FALSE;
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
    if ( qstrcmp(signal,SIGNAL(dataChanged())) == 0 && !inClipboardChain ) {
	QWidget *owner = clipboardOwner();
	inClipboardChain = TRUE;
	nextClipboardViewer = SetClipboardViewer( owner->winId() );
#ifndef Q_NO_DEBUG
	if ( !nextClipboardViewer )
	    qSystemWarning( "QClipboard: Failed to set clipboard viewer" );
#endif
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
#ifndef Q_NO_DEBUG
	    if ( !CloseClipboard() )
		qSystemWarning( "QClipboard: Failed to close clipboard" );
#else
	    CloseClipboard();
#endif
	}
#ifndef Q_NO_DEBUG
	else
	    qSystemWarning( "QClipboardWatcher: Failed to open clipboard" );
#endif
	return r;
    }

    // Not efficient to iterate over this (so we provide provides() above).

    // In order to be able to use UniCode characters, we have to give
    // it a higher priority than single-byte characters.  To do this, we
    // treat CF_TEXT as a special case and postpone its processing until
    // we're sure we do not have CF_UNICODETEXT
    const char* format( int n ) const
    {
	const char* mime = 0;
	bool sawSBText( FALSE );

	if ( n >= 0 ) {
	    if ( OpenClipboard( clipboardOwner()->winId() ) ) {
		int cf = 0;
		QPtrList<QWindowsMime> all = QWindowsMime::all();
		while (cf = EnumClipboardFormats(cf)) {
#ifdef Q_OS_TEMP
			if ( cf == CF_TEXT )
				sawSBText = TRUE;
#else
#if defined(UNICODE)
                    if ( qWinVersion() & Qt::WV_NT_based && cf == CF_TEXT ) {
			sawSBText = TRUE;
		    } else 
#endif
		    {
			mime = QWindowsMime::cfToMime(cf);
			if ( mime ) {
			    if ( !n )
				break; // COME FROM HERE
			    n--;
			    mime = 0;
			}
		    }
#endif
		}
		// COME FROM BREAK

		// If we did not find a suitable mime type, yet skipped
		// CF_TEXT due to the priorities above, give it a shot
                if ( qWinVersion() & Qt::WV_NT_based && !mime && sawSBText ) {
		    mime = QWindowsMime::cfToMime( CF_TEXT );
		    if ( mime ) {
			n--;
		    }
		}
		CloseClipboard();
	    }
	}
	if ( !n )
	    return mime;
	return 0;
    }

    QByteArray encodedData( const char* mime ) const
    {
	QByteArray r;
	if ( OpenClipboard( clipboardOwner()->winId() ) ) {
	    QPtrList<QWindowsMime> all = QWindowsMime::all();
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
#ifndef Q_NO_DEBUG
		    else
			qSystemWarning( "QClipboard: Failed to read clipboard data" );
#endif
		}
	    }
	    CloseClipboard();
	}
#ifndef Q_NO_DEBUG
	else
	    qSystemWarning( "QClipboard: Failed to open Clipboard" );
#endif
	return r;
    }
};



class QClipboardData
{
public:
    QClipboardData();
   ~QClipboardData();

    void setSource(QMimeSource* s)
    {
	delete src;
	src = s;
	if ( src )
	    src->clearCache();
    }
    QMimeSource* source()
    {
	if ( src )
	    src->clearCache();
	return src;
    }
    QMimeSource* provider()
    {
	if (!prov)
	    prov = new QClipboardWatcher();
	prov->clearCache();
	return prov;
    }

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

static QClipboardData *internalCbData = 0;

static void cleanupClipboardData()
{
    delete internalCbData;
    internalCbData = 0;
}

static QClipboardData *clipboardData()
{
    if ( internalCbData == 0 ) {
	internalCbData = new QClipboardData;
	Q_CHECK_PTR( internalCbData );
	qAddPostRoutine( cleanupClipboardData );
    }
    return internalCbData;
}


//#define QT_DEBUG_CB

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
	    HANDLE res = SetClipboardData( cf, h );
#ifndef Q_NO_DEBUG
	    if ( !res )
		qSystemWarning( "QClipboard: Failed to write data" );
#endif
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

    if ( !qt_cb_owner )
	return;
    if ( !OpenClipboard( qt_cb_owner->winId() ) ) {
#if defined(QT_CHECK_STATE)
	qSystemWarning( "renderAllFormats: couldn't open clipboard" );
#endif
	return;
    }

    EmptyClipboard();

    const char* mime;
    QPtrList<QWindowsMime> all = QWindowsMime::all();
    for (int i = 0; mime = s->format(i); i++) {
	for (QWindowsMime* c = all.first(); c; c = all.next()) {
	    if ( c->cfFor(mime) ) {
		for (int j = 0; j < c->countCf(); j++) {
		    int cf = c->cf(j);
		    if ( c->canConvert(mime,cf) ) {
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
		    }
		}
	    }
	}
    }

    CloseClipboard();
}

QClipboard::~QClipboard()
{
    renderAllFormats();
    delete qt_cb_owner;
    qt_cb_owner = 0;
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
#if defined(UNICODE)
#ifndef Q_OS_TEMP
	if ( qWinVersion() & Qt::WV_NT_based )
#endif
	    SendMessage( nextClipboardViewer, m->message,
			 m->wParam, m->lParam );
#ifndef Q_OS_TEMP
	else
#endif
#endif
#ifndef Q_OS_TEMP
	    SendMessageA( nextClipboardViewer, m->message,
			 m->wParam, m->lParam );
#endif
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
    if ( !OpenClipboard(clipboardOwner()->winId()) ) {
#ifndef Q_NO_DEBUG
	qSystemWarning( "QClipboard: Failed to open clipboard" );
#endif
	return;
    }

    QClipboardData *d = clipboardData();
    d->setSource( src );

    int res = EmptyClipboard();
#ifndef Q_NO_DEBUG
    if ( !res )
	qSystemWarning( "QClipboard: Failed to empty clipboard" );
#endif

    // Register all the formats of src that we can render.
    const char* mime;
    QPtrList<QWindowsMime> all = QWindowsMime::all();
    for (int i = 0; mime = src->format(i); i++) {
	for (QWindowsMime* c = all.first(); c; c = all.next()) {
	    if ( c->cfFor(mime) ) {
		for (int j = 0; j < c->countCf(); j++) {
		    UINT cf = c->cf(j);
		    if ( c->canConvert(mime,cf) )
			SetClipboardData( cf, 0 ); // 0 == ask me later
		}
	    }
	}
    }

    CloseClipboard();
}

#endif // QT_NO_CLIPBOARD
