/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qstatusbar.cpp#46 $
**
** Implementation of QStatusBar class
**
** Created : 980119
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#include "qstatusbar.h"

#include "qlist.h"
#include "qlayout.h"
#include "qpainter.h"
#include "qtimer.h"
#include "qdrawutil.h"
#include "qapplication.h"
#include "qsizegrip.h"

// NOT REVISED
/*! \class QStatusBar qstatusbar.h

  \brief The QStatusBar class provides a horizontal bar suitable for
  presenting status messages.

  \ingroup realwidgets
  \ingroup application

  Each status message falls into one of three classes: <ul>

  <li> \c Temporary - occupies most of the status bar briefly.  Used
  e.g. for explaining tool tip texts or menu entries.

  <li> \c Normal - occupies part of the status bar and may be hidden
  by temporary messages.  Used e.g. for displaying the page and line
  number in a word processor.

  <li> \c Permanent - is never hidden.  Used for important mode
  indications.  Some applications put a Caps Lock indicator here.

  </ul>

  QStatusBar lets you display all three sorts of messages.

  To display a temporary message, you can call message(), or connect a
  suitable signal to it.  To remove a temporary message, you can call
  clear(), or connect a signal to it.

  There are two variants of message(), one which displays the message
  until the next clear() or mesage(), and one which also has a time limit.

  Normal and permanent messages are displayed by creating a widget
  (typically a QLabel) and using addWidget() to add this widget to the
  status bar.

  Finally, in Windows style QStatusBar also provides a Windows
  standard resize handle.  In the X11 version of Qt this resize handle
  generally works differently than the one provided by the system; we
  are working to reduce this difference.

  <img src=qstatusbar-m.png> <img src=qstatusbar-w.png>

  \sa QToolBar QMainWindow QLabel
  <a href="http://www.microsoft.com/win32dev/uiguide/uigui192.htm">Microsoft Style Guide,</a>
  <a href="guibooks.html#fowler">GUI Design Handbook: Status Bar.</a>
*/


class QStatusBarPrivate
{
public:
    QStatusBarPrivate() {}

    struct SBItem {
	SBItem( QWidget* widget, int stretch, bool permanent )
	    : s( stretch ), w( widget ), p( permanent ) {}
	int s;
	QWidget * w;
	bool p;
    };

    QList<SBItem> items;
    QString tempItem;

    QBoxLayout * box;
    QTimer * timer;

    QSizeGrip * resizer;
};


/*!  Constructs an empty status bar. */

QStatusBar::QStatusBar( QWidget * parent, const char *name )
    : QWidget( parent, name )
{
    d = new QStatusBarPrivate;
    d->items.setAutoDelete( TRUE );
    d->box = 0;
    d->resizer = new QSizeGrip( this, "QStatusBar::resizer" );
    d->timer = 0;
    reformat();
}


/*! Destructs the object and frees any allocated resources.

*/

QStatusBar::~QStatusBar()
{
    delete d;
    d = 0;
}


/*!  Adds \a widget to this status bar, with a width of \a stretch.

  \a widget is permanently visible if \a permanent is TRUE, and is
  obscured by temporary messages if \a permanent is FALSE.  The
  default is FALSE.

  \a stretch is used to compute a suitable size for \a widget.

  If \a permanent is TRUE, \a widget is located at the far right of
  the status bar.  If \a permanent is FALSE (the default) \a widget is
  located just to the left of the first permanent widget.

  This function may cause some flicker.

  \sa removeWidget()
*/

void QStatusBar::addWidget( QWidget * widget, int stretch, bool permanent )
{
    if ( !widget ) {
#if defined(CHECK_NULL)
	qWarning( "QStatusBar::addWidget(): Cannot add null widget" );
#endif
	return;
    }

    QStatusBarPrivate::SBItem* item
	= new QStatusBarPrivate::SBItem( widget, stretch, permanent );

    d->items.last();
    while( !permanent && d->items.current() && d->items.current()->p )
	d->items.prev();

    d->items.insert( d->items.at() >= 0 ? d->items.at()+1 : 0, item );

    if ( !d->tempItem.isEmpty() && !permanent )
	widget->hide();

    reformat();
}


/*!  Removes \a widget from the status bar.

  This function may cause some flicker.

  Note that \a widget is not deleted.

  \sa addWidget()
*/

void QStatusBar::removeWidget( QWidget* widget )
{
    bool found = FALSE;
    QStatusBarPrivate::SBItem* item = d->items.first();
    while ( item && !found ) {
	if ( item->w == widget ) {
	    d->items.remove();
	    found = TRUE;
	}
	item = d->items.next();
    }

    if ( found )
	reformat();
#if defined(DEBUG)
    else
	qDebug( "QStatusBar::removeWidget(): Widget not found." );
#endif
}


/*!  Changes the status bar's appearance to account for item
  changes. */

void QStatusBar::reformat()
{
    if ( d->box )
	delete d->box;
    d->box = new QVBoxLayout( this );
    d->box->addSpacing( 3 );

    QBoxLayout* l = new QHBoxLayout( d->box );
    l->addSpacing( 3 );

    int maxH = fontMetrics().height();

    QStatusBarPrivate::SBItem* item = d->items.first();
    while ( item && !item->p ) {
	l->addWidget( item->w, item->s );
	l->addSpacing( 4 );
	int itemH = item->w->sizeHint().height();
	maxH = QMAX( maxH, itemH );
	item = d->items.next();
    }

    l->addStretch( 0 );


    while ( item ) {
	l->addWidget( item->w, item->s );
	l->addSpacing( 4 );
	int itemH = item->w->sizeHint().height();
	maxH = QMAX( maxH, itemH );
	item = d->items.next();
    }

    maxH = QMAX( maxH, d->resizer->sizeHint().height() );
    l->addStrut( maxH );
    l->addSpacing( 2 );
    l->addWidget( d->resizer, 0, AlignBottom );
    l->addSpacing( 2 );
    d->box->addSpacing( 2 );
    d->box->activate();
    update();
}




/*!  Hide the normal status indications and display \a message, until
  clear() or another message() is called.

  \sa clear()
*/

void QStatusBar::message( const QString &message )
{
    if ( d->tempItem == message )
	return;
    d->tempItem = message;
    if ( d->timer ) {
	delete d->timer;
	d->timer = 0;
    }
    hideOrShow();
}


/*!  Hide the normal status indications and display \a message for \a
  ms milli-seconds, or until clear() or another message() is called,
  whichever is shorter.
*/

void QStatusBar::message( const QString &message, int ms )
{
    d->tempItem = message;

    if ( !d->timer ) {
	d->timer = new QTimer( this );
	connect( d->timer, SIGNAL(timeout()), this, SLOT(clear()) );
    }
    if ( ms > 0 )
	d->timer->start( ms );
    hideOrShow();
}


/*!  Removes any temporary message being shown.

  \sa message()
*/

void QStatusBar::clear()
{
    if ( d->tempItem.isEmpty() )
	return;
    if ( d->timer ) {
	delete d->timer;
	d->timer = 0;
    }
    d->tempItem = QString::null;
    hideOrShow();
}


/*!  Ensures that the right widgets are visible.  Used by message()
  and clear().
*/

void QStatusBar::hideOrShow()
{
    bool haveMessage = !d->tempItem.isEmpty();

    QStatusBarPrivate::SBItem* item = d->items.first();

    while( item && !item->p ) {
	if ( haveMessage )
	    item->w->hide();
	else
	    item->w->show();
	item = d->items.next();
    }

    update();
}


/*!  Shows the temporary message, if appropriate. */

void QStatusBar::paintEvent( QPaintEvent * )
{
    bool haveMessage = !d->tempItem.isEmpty();

    QPainter p( this );
    QStatusBarPrivate::SBItem* item = d->items.first();
    while ( item ) {
	if ( !haveMessage || item->p )
	    qDrawShadeRect( &p, item->w->x()-1, item->w->y()-1,
			    item->w->width()+2, item->w->height()+2,
			    colorGroup(), TRUE, 1, 0, 0 );
	item = d->items.next();
    }
    if ( haveMessage ) {
	p.setPen( colorGroup().text() );
	// ### clip and add ellipsis if necessary
	p.drawText( 6, 0, width() - 12, height(), AlignVCenter + SingleLine,
		    d->tempItem );
    }
}
