/****************************************************************************
** $Id: //depot/qt/fb/src/kernel/qclipboard_fb.cpp#2 $
**
** Implementation of QClipboard class for FB
**
** Created : 991026
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing.
**
*****************************************************************************/

#include "qclipboard.h"

#if QT_FEATURE_CLIPBOARD

#include "qapplication.h"
#include "qbitmap.h"
#include "qdatetime.h"
#include "qdragobject.h"
#include "qbuffer.h"

/*****************************************************************************
  Internal QClipboard functions for X11.
 *****************************************************************************/

static QWidget * owner = 0;

static void cleanup()
{
    delete owner;
    owner = 0;
}

static
void setupOwner()
{
    if ( owner )
	return;
    owner = new QWidget( 0, "internal clibpoard owner" );
    qAddPostRoutine( cleanup );
}


class QClipboardData
{
public:
    QClipboardData();
   ~QClipboardData();

    void setSource(QMimeSource* s)
	{ delete src; src = s; }
    QMimeSource* source()
	{ return src; }
    void addTransferredPixmap(QPixmap pm)
	{ /* TODO: queue them */
	    transferred[tindex] = pm;
	    tindex=(tindex+1)%2;
	}
    void clearTransfers()
	{
	    transferred[0] = QPixmap();
	    transferred[1] = QPixmap();
	}

    void clear();

private:
    QMimeSource* src;

    QPixmap transferred[2];
    int tindex;
};

QClipboardData::QClipboardData()
{
    src = 0;
    tindex=0;
}

QClipboardData::~QClipboardData()
{
    delete src;
}

void QClipboardData::clear()
{
    delete src;
    src = 0;
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


/*****************************************************************************
  QClipboard member functions for FB.
 *****************************************************************************/

/*!
  Clears the clipboard contents.
*/

void QClipboard::clear()
{
    setText( QString::null );
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


/*!
  Handles clipboard events (very platform-specific).
*/

bool QClipboard::event( QEvent *e )
{
    return QObject::event( e );
}



/*!
  Returns a reference to a QMimeSource representation
  of the current clipboard data.
*/
QMimeSource* QClipboard::data() const
{
    QClipboardData *d = clipboardData();
    return d->source();
}

/*!
  Sets the clipboard data.  Ownership of the data is transferred
  to the clipboard - the only way to remove this data is to set
  something else, or to call clear().  The QDragObject subclasses
  are reasonable things to put on the clipboard (but do not try
  to \link QDragObject::drag() drag\endlink the same object).
  Do not put QDragMoveEvent or QDropEvent subclasses on the clipboard,
  as they do not belong to the event handler which receives them.

  The setText() and setPixmap() functions are shorthand ways
  of setting the data.
*/
void QClipboard::setData( QMimeSource* src )
{
    QClipboardData *d = clipboardData();
    setupOwner();

    d->setSource( src );
    emit dataChanged();
}

#endif // QT_FEATURE_CLIPBOARD
