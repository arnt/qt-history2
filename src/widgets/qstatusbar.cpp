/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qstatusbar.cpp#8 $
**
** Implementation of QStatusBar class
**
** Created : 980119
**
** Copyright (C) 1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qstatusbar.h"

#include "qlist.h"
#include "qlayout.h"
#include "qpainter.h"
#include "qtimer.h"
#include "qdrawutl.h"
#include "qapp.h"

RCSTAG("$Id: //depot/qt/main/src/widgets/qstatusbar.cpp#8 $");


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

  <li> \c Permanent - occupies the far right of the statusbar and is
  never hidden.  Used for important mode indications.  Some
  applications put a Caps Lock indicator here.

  </ul>

*/


class QStatusBarPrivate
{
public:
    QStatusBarPrivate() {}

    struct Item {
	Item( QWidget * widget, int stretch, bool permanent )
	    : s( stretch ), w( widget ), p(permanent) {}
	int s;
	QWidget * w;
	bool p;
    };

    class ResizeLines: public QWidget
    {
    public:
	ResizeLines( QWidget * parent );
	
    protected:
	void paintEvent( QPaintEvent * );
	void mousePressEvent( QMouseEvent * );
	void mouseMoveEvent( QMouseEvent * );
    private:
	QPoint p;
	QSize s;
    };

    QList<Item> items;
    Item * firstPermanent;

    QString temporary;

    QBoxLayout * box;
    QTimer * timer;

    ResizeLines * resizer;
};


QStatusBarPrivate::ResizeLines::ResizeLines( QWidget * parent )
    : QWidget( parent, 0 )
{
    setCursor( sizeFDiagCursor );
    setFixedSize( 13, 13 );
}


void QStatusBarPrivate::ResizeLines::paintEvent( QPaintEvent * )
{
    QPainter p( this );
    QPointArray a;
    a.setPoints( 3, 1,12, 12,1, 12,12 );
    p.setPen( colorGroup().dark() );
    p.setBrush( colorGroup().dark() );
    p.drawPolygon( a );
    p.setPen( colorGroup().light() );
    p.drawLine(  0, 12, 12,  0 );
    p.drawLine(  5, 12, 12,  5 );
    p.drawLine( 10, 12, 12, 10 );
    p.setPen( colorGroup().background() );
    p.drawLine(  4, 12, 12,  4 );
    p.drawLine(  9, 12, 12,  9 );
}


void QStatusBarPrivate::ResizeLines::mousePressEvent( QMouseEvent * e )
{
    p = mapToGlobal( e->pos() );
    s = topLevelWidget()->size();
}


void QStatusBarPrivate::ResizeLines::mouseMoveEvent( QMouseEvent * e )
{
    if ( e->state() != LeftButton )
	return;

    QPoint np( QCursor::pos() );
    int w = np.x() - p.x() + s.width();
    int h = np.y() - p.y() + s.height();
    if ( w < 1 )
	w = 1;
    if ( h < 1 )
	h = 1;
    topLevelWidget()->resize( w, h );
    QApplication::syncX();
}


/*!  Constructs an empty status bar. */

QStatusBar::QStatusBar( QWidget * parent, const char * name )
    : QWidget( parent, name )
{
    d = new QStatusBarPrivate;
    d->box = 0;
    d->resizer = new QStatusBarPrivate::ResizeLines( this );
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
    QStatusBarPrivate::Item * item
	= new QStatusBarPrivate::Item( widget, stretch, permanent );

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

    QStatusBarPrivate::Item * i;
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

void QStatusBar::message( const char * message )
{
    d->temporary = message; // ### clip and add ellipsis if necessary
    if ( d->timer )
	delete d->timer;
    hideOrShow();
}


/*!  Hide the normal status indications and display \a message for \a
  ms milli-seconds, or until clear() or another message() is called,
  whichever is shorter.
*/

void QStatusBar::message( const char * message, int ms )
{
    d->temporary = message; // ### clip and add ellipsis if necessary

    if ( !d->timer ) {
	d->timer = new QTimer( this );
	connect( d->timer, SIGNAL(timeout()), this, SLOT(clear()) );
    }
    d->timer->start( ms );
    hideOrShow();
}


/*!  Removes any remporary message being shown.

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
    QStatusBarPrivate::Item * i;
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
    QStatusBarPrivate::Item * i;
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
