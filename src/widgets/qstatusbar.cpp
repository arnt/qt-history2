/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qstatusbar.cpp#31 $
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
** Licensees with valid Qt Professional Edition licenses may distribute and
** use this file in accordance with the Qt Professional Edition License
** provided at sale or upon request.
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
#include "qresizecorner.h"

/*! \class QStatusBar qstatusbar.h

  \brief The QStatusBar class provides a horizontal bar suitable for
  presenting status messages.

  \ingroup realwidgets
  \ingroup application

  Each status message falls into one of three classes: <ul>

  <li> \c Temporary - occupies most of the status bar briefly.  Used
  e.g. for explaining \link QToolTip tool tip \endlink texts or menu
  entries)

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
  standard resize handle.  In the X11 version Qt 1.40 this resize
  handle generally works differently than the one provided by the
  system; we hope to reduce this difference in the future.

  <img src=qstatusbar-m.gif> <img src=qstatusbar-w.gif>

  \sa QToolBar QMainWindow QLabel
  <a href="http://www.microsoft.com/win32dev/uiguide/uigui192.htm">Microsoft Style Guide,</a>
  <a href="guibooks.html#fowler">GUI Design Handbook: Status Bar.</a>
*/


class QStatusBarPrivate
{
public:
    QStatusBarPrivate() {}

    struct StatusBarPrivateItem {
	StatusBarPrivateItem( QWidget * widget, int stretch, bool permanent )
	    : s( stretch ), w( widget ), p(permanent) {}
	int s;
	QWidget * w;
	bool p;
    };

    QList<StatusBarPrivateItem> items;
    StatusBarPrivateItem * firstPermanent;

    QString temporary;

    QBoxLayout * box;
    QTimer * timer;

    QResizeCorner * resizer;
};


/*!  Constructs an empty status bar. */

QStatusBar::QStatusBar( QWidget * parent, const char *name )
    : QWidget( parent, name )
{
    d = new QStatusBarPrivate;
    d->items.setAutoDelete( TRUE );
    d->box = 0;
    d->resizer = new QResizeCorner( this );
    d->timer = 0;
    reformat();
}


/*! Destroys the object and frees any allocated resources.

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
    QStatusBarPrivate::StatusBarPrivateItem * item
	= new QStatusBarPrivate::StatusBarPrivateItem( widget, stretch, permanent );

    d->items.last();
    while( !permanent && d->items.current() && d->items.current()->p )
	d->items.prev();

    d->items.insert( d->items.at() >= 0 ? d->items.at()+1 : 0, item );

    reformat();
}


/*!  Removes \a widget from the status bar.

  This function may cause some flicker.

  Note that \a widget is not deleted.

  \sa addWidget()
*/

void QStatusBar::removeWidget( QWidget * widget )
{
    d->items.first();
    do {
	if ( d->items.current() &&
	     d->items.current()->w == widget ) {
	    d->items.remove();
	    reformat();
	    return;
	}
    } while( d->items.next() );
}


/*!  Changes the status bar's appearance to account for item
  changes. */

void QStatusBar::reformat()
{
    if ( d->box ) {
	delete d->box;
	d->box = 0;
    }

    d->box = new QBoxLayout( this, QBoxLayout::Down );
    d->box->addSpacing( 3 );
    QBoxLayout * l = new QBoxLayout( QBoxLayout::LeftToRight );
    d->box->addLayout( l );

    QStatusBarPrivate::StatusBarPrivateItem * i;
    d->items.first();
    int space = 1;

    while( (i=d->items.current()) != 0 ) {
	d->items.next();
	l->addSpacing( space );
	space = 4;
	l->addWidget( i->w, i->s );
    }
    QBoxLayout * vproxy;
    if ( space == 1 ) {
	l->addStretch( 1 );
	vproxy = new QBoxLayout( QBoxLayout::Down );
	l->addLayout( vproxy );
	vproxy->addSpacing( 3 + fontMetrics().height() + 3 );
    }
    l->addSpacing( 2 );
    vproxy = new QBoxLayout( QBoxLayout::Down );
    l->addLayout( vproxy );
    vproxy->addStretch( 1 );
    vproxy->addWidget( d->resizer, 0 );
    d->box->activate();
}




/*!  Hide the normal status indications and display \a message, until
  clear() or another message() is called.

  \sa clear()
*/

void QStatusBar::message( const QString &message )
{
    d->temporary = message; // ### clip and add ellipsis if necessary
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
    d->temporary = message; // ### clip and add ellipsis if necessary

    if ( !d->timer ) {
	d->timer = new QTimer( this );
	connect( d->timer, SIGNAL(timeout()), this, SLOT(clear()) );
    }
    d->timer->start( ms );
    hideOrShow();
}


/*!  Removes any temporary message being shown.

  \sa message()
*/

void QStatusBar::clear()
{
    if ( d->temporary.isEmpty() )
	return;
    if ( d->timer ) {
	delete d->timer;
	d->timer = 0;
    }
    d->temporary = "";
    hideOrShow();
}


/*!  Ensures that the right widgets are visible.  Used by message()
  and clear().
*/

void QStatusBar::hideOrShow()
{
    QStatusBarPrivate::StatusBarPrivateItem * i;
    bool b = (d->temporary.length() == 0);

    d->items.first();
    update();
    while( (i=d->items.current()) != 0 ) {
	d->items.next();
	if ( b || i->p )
	    i->w->show();
	else
	    i->w->hide();
    }
}


/*!  Shows the temporary message, if appropriate. */

void QStatusBar::paintEvent( QPaintEvent * )
{
    QStatusBarPrivate::StatusBarPrivateItem * i;
    bool b = (d->temporary.length() == 0);
    QPainter p( this );
    d->items.first();
    while( (i=d->items.current()) != 0 ) {
	d->items.next();
	if ( b || i->p )
	    qDrawShadeRect( &p,
			    i->w->x()-1, i->w->y()-1,
			    i->w->width()+2, i->w->height()+2,
			    colorGroup(), TRUE, 1, 0, 0 );
    }
    if ( !b ) {
	p.setPen( colorGroup().text() );
	p.drawText( 6, 0, width() - 12, height(), AlignVCenter + SingleLine,
		    d->temporary );
    }
}
