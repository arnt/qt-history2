/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qtooltip.cpp#7 $
**
** Tool Tips (or Balloon Help) for any widget or rectangle
**
** Copyright (C) 1996 by Troll Tech AS.	 All rights reserved.
**
*****************************************************************************/

#include "qtooltip.h"
#include "qintdict.h"
#include "qstring.h"
#include "qwidget.h"
#include "qcolor.h"
#include "qlabel.h"
#include "qpoint.h"

RCSTAG("$Id: //depot/qt/main/src/widgets/qtooltip.cpp#7 $");

// what comes out of the dict
struct QTip
{
    QRect rect;
    QString text;
    bool autoDelete;
    QTip * next;
};


// the class which does all the work
// just one instance of this class exists -
class QTipManager : public QObject
{
public:
    QTipManager();
    ~QTipManager();

    bool eventFilter( QObject * o, QEvent * e );
    void add( QWidget *, const QRect &, const char *, bool );
    void remove( QWidget *, const QRect & );

    void remove( QToolTipGroup * );

public slots:
    void someWidgetDestroyed();

private:
    void maybeTip( const QPoint & = 0 );
    void timerEvent( QTimerEvent * );

    void up( QWidget *, QTip *, const QPoint & );
    void down();
    void showTip();
    void hideTip();

    QIntDict<QTip> * tips;
    QTip * currentTip;
    QLabel * label;
    QPoint pos;

    enum { dormant, wakingUp, active, fallingAsleep, wakingUpAgain } state;

};


// - and here it is.  a real workhorse.
static QTipManager * tipManager;


QTipManager::QTipManager()
    : QObject( 0, "tool tip workhorse object" )
{
    tips = new QIntDict<QTip>( 313 );
    currentTip = 0;
    label = 0;
    state = dormant;
}


QTipManager::~QTipManager()
{
    delete tips;
    delete label;
}


void QTipManager::add( QWidget * w, const QRect & r, const char * s, bool a )
{
    QTip * t = (*tips)[ (long)w ];
    if ( !t ) {
	// the first one for this widget
	t = new QTip;
	t->next = 0;
	w->installEventFilter( tipManager );
	tips->insert( (long)w, t );
    } else {
	while( t && t->rect != r && t->next != 0 )
	    t = t->next;
	if ( t->rect != r ) {
	    t->next = new QTip;
	    t = t->next;
	    t->next = 0;
	}
    }

    t->text = s;
    t->autoDelete = a;
    t->rect = r;
}


void QTipManager::remove( QWidget *w, const QRect & r )
{
    QTip * t = (*tips)[ (long)w ];
    if ( t == 0 )
	return;

    if ( t->rect == r ) {
	(void) tips->remove( (long)w );
	if ( t->next )
	    tips->insert( (long)w, t->next );
	else
	    w->removeEventFilter( tipManager );
    } else {
	while( t->next && t->next->rect != r )
	    t = t->next;
	if ( t->next ) {
	    delete t->next;
	    t->next = 0;
	}
    }

    if ( tips->isEmpty() ) {
	// the manager will be recreated if needed
	delete tipManager;
	tipManager = 0;
    }
}


bool QTipManager::eventFilter( QObject * o, QEvent * e )
{
    // avoid dumping core in case of application madness
    if ( !tips || !e || !o || !o->isWidgetType() )
	return FALSE;
    QWidget * w = (QWidget *)o;
    QTip * t = (*tips)[ (long int)w ];
    if ( !t )
	return FALSE;

    // with that out of the way, let's get down to action

    switch( e->type() ) {
    case Event_Paint:
	// no processing at all
	break;
    case Event_Enter:
	down();
	// up( w, t, QPoint( -1, -1 ) ); let the mouse move event do it
	break;
    case Event_Leave:
	// remove any lingering tip
	down();
	break;
    case Event_MouseMove:
	{ // a whole scope just for m
	    QMouseEvent * m = (QMouseEvent *)e;
	    if ( currentTip && !currentTip->rect.contains( m->pos() ) ) {
		down();
	    }
	    if ( m->state() == 0 ) {
		up( w, t, m->pos() );
		return TRUE;
	    }
	}
	break;
    default:
	 // is this the right thing to do?
	killTimers();
	state = dormant;
	break;
    }
    return FALSE;
}


/* okay, here comes the state machine

   dormant: tip isn't active

   wakingUp: tip isn't shown, but will be shown in a second or so
   unless the mouse moves

   active: tip is shown

   fallingAsleep: tip has been hidden, but the state machine can enter
   wakingUpAgain state

   wakingUpAgain: the tip isn't shown, but will be shown in just a
   tenth of a second unless the mouse moves

   up, down, event, showTip and hideTip all modify the state.

*/


void QTipManager::up( QWidget * w, QTip * t, const QPoint & p )
{
    if ( currentTip )
	return;

    while ( t && !t->rect.contains( p ) )
	t = t->next;
    currentTip = t;
    if ( !t ) {
	state = fallingAsleep;
	return;
    }

    pos = w->mapToGlobal( p ) + QPoint( 0, 16 );
    if ( state == dormant ) {
	state = wakingUp;
	startTimer( 1000 );
    } else {
	state = wakingUpAgain;
	startTimer( 100 );
    }
}


void QTipManager::down()
{
    hideTip();
    if ( currentTip->autoDelete ) {
	// delete the tip here, and clean it up
    }
    currentTip = 0;
}


void QTipManager::timerEvent( QTimerEvent * )
{
    killTimers();

    if ( state == fallingAsleep )
	state = dormant;
    else if ( state == active )
	hideTip();
    else
	showTip();
}


void QTipManager::showTip()
{
    if ( !currentTip ) {
	// error of some sort
	state = dormant;
	killTimers();
	return;
    }

    if ( label ) {
	label->setText( currentTip->text );
    } else {
	label = new QLabel( 0, "tool tip tip",
			    WStyle_Customize | WStyle_NoBorder | WStyle_Tool );
	CHECK_PTR( label );
	label->setText( currentTip->text );
	label->setFrameStyle( QFrame::Plain | QFrame::Box );
	label->setLineWidth( 1 );
	label->setMargin( 3 );
	label->setAlignment( AlignLeft | AlignTop );
	label->setAutoResize( TRUE );
	label->setBackgroundColor( QColor(255,255,220) );
    }
    label->move( pos );
    label->show();
    killTimers();
    startTimer( 5000 );
}


void QTipManager::hideTip()
{
    if ( label && label->isVisible() )
	label->hide();

    killTimers();
    if ( state == active ) {
	state = fallingAsleep;
	startTimer( 2000 );
    } else {
	state = dormant;
    }

    currentTip = 0;
}


void QTipManager::remove( QToolTipGroup * )
{
    // hanord! implementer, vær så snill!
}



/*! \class QToolTip qtooltip.h

  \brief The QToolTip class provides tool tips (sometimes called
  balloon help) for any widget or rectangular part of a widget.

  The tip is a short, one-line text reminding the user of the widget's
  or rectangle's function.  It is drawn immediately below the region,
  in a distinctive black on yellow combination.

  At any point in time, QToolTip is either dormant or active.  In
  dormant mode the tips are not shown, and in active mode they are.
  The mode is global, not particular to any one widget.

  QToolTip swittches from dormant to active mode when the user lets
  the mouse rest on a tip-equipped region for a second or so, and
  remains in active mode until the user either clicks a mouse button,
  lets the mouse rest for five seconds, or moves the mouse outside \e
  all tip-equpped regions for at least a second.

  There are no less than three APIs for QToolTip: <ol> <li> Adding a
  tip to an entire widget. <li> Adding a tip to a fixed rectangle
  within a widget. <li> Adding a tip to a dynamic rectangle within a
  widget. </ol>

  To add a tip to a widget, call the static function QToolTip::add()
  with the widget and tip as arguments.

  To add a tip to a fixed rectangle within a widget, call the static
  function QToolTip::add with the widget, rectangle and tip as
  arguments.

  Both of the above are one-liners and should cover the vast majority
  of cases.  The third, most general, API uses a pure virtual function
  to decide whether to pop up a tool tip.  This mode can be used to
  implement e.g. tips for text that can move as the user scrolls.

  To use this API, you need to subclass QToolTip and reimplement
  maybeTip().  maybeTip() will be called when there's a chance that a
  tip should pop up.  It must decide whether to show a tip, and
  possibly call add() with the rectangle the tip applies to, and the
  tip's text.  The tip will disappear once the mouse moves outside the
  rectangle you supply, and \e not \e reappear - maybeTip() will be
  called again if the user lets the mouse rest within the same
  rectangle again.  You can forcibly remove the tip by calling
  remove() with no arguments.  This is handy if the widget scrolls.  */


/*!  Creates a tool tip object.	 This is necessary only if you need
  tool tips on regions that can move within the widget (most often
  because the widget's contents can scroll).

  \sa maybeHit().
*/

QToolTip::QToolTip( QWidget * )
{
    if ( !tipManager )
	tipManager = new QTipManager();
    // stuff and nonsense
}


/*!  Adds a tool tip to an entire widget.  \e widget

  The normal entry point to the QToolTip class.


*/
void QToolTip::add( QWidget * widget, const char * text )
{
    if ( !tipManager )
	tipManager = new QTipManager();
    tipManager->add( widget, QRect( QCOORD_MIN, QCOORD_MIN,
				    QCOORD_MAX-QCOORD_MIN,
				    QCOORD_MAX-QCOORD_MIN ),
		     text, FALSE );
}


/*!

*/

void QToolTip::add( QWidget *, const char *,
		    QToolTipGroup *, const char * )
{
    
}

void QToolTip::remove( QWidget * widget )
{
    if ( tipManager )
	tipManager->remove( widget, QRect( QCOORD_MIN, QCOORD_MIN,
					   QCOORD_MAX-QCOORD_MIN,
					   QCOORD_MAX-QCOORD_MIN ) );
}

void QToolTip::add( QWidget * widget, const QRect & rect, const char * text )
{
    if ( !tipManager )
	tipManager = new QTipManager();
    tipManager->add( widget, rect, text, FALSE );
}


/*!

*/

void QToolTip::add( QWidget *, const QRect &, const char *,
		    QToolTipGroup *, const char * )
{
    
}



void QToolTip::remove( QWidget * widget, const QRect & rect )
{
    if ( tipManager )
	tipManager->remove( widget, rect );
}


/*! \fn virtual void QToolTip::maybeTip( const QPoint &);

*/


void QToolTip::tip( const QRect &, const char * )
{
}


void QToolTip::clear()
{
}


/*!

*/

QToolTipGroup::QToolTipGroup( QObject * parent, const char * name )
    : QObject( parent, name )
{
    initMetaObject();
}



/*!
  Destroy this tool tip groups and all tool tips in it.
*/

QToolTipGroup::~QToolTipGroup()
{
    if ( tipManager )
	tipManager->remove( this );
}
