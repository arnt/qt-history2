/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qtooltip.cpp#1 $
**
** Tool Tips (or Balloon Help) for any widget or rectangle
**
** Copyright (C) 1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qtooltip.h"
#include "qintdict.h"
#include "qstring.h"
#include "qwidget.h"
#include "qcolor.h"
#include "qlabel.h"
#include "qpoint.h"

RCSTAG("$Id: //depot/qt/main/src/widgets/qtooltip.cpp#1 $");

// what comes out of the dict
struct QTip
{
    QRect rect;
    QString text;
    bool autoDelete;
    QTip * next;
};


// the class which does all the work
// just one instance of this class exists...
class QTipManager : public QToolTip
{
public:
    QTipManager();
    ~QTipManager();

    bool eventFilter( QObject * o, QEvent * e );
    void add( QWidget *, const QRect &, const char *, bool );
    void remove( QWidget *, const QRect & );

protected:
    void maybeTip( const QPoint & = 0 );
    bool event( QEvent * );

private:
    void up( QWidget *, QTip *, const QPoint & );
    void down();
    void showTip();
    void hideTip();

    QIntDict<QTip> * tips;
    QTip * currentTip;
    int counter;
    QLabel * label;
    QPoint pos;

    enum { dormant, wakingUp, active, fallingAsleep, wakingUpAgain } state;
    
};


// ... and here it is.  a real workhorse.
static QTipManager * tipManager;


QTipManager::QTipManager()
    : QToolTip( 0, "tool tip workhorse object" )
{
    tips = new QIntDict<QTip>( 313 );
    currentTip = 0;
    counter = 0;
    label = 0;
    state = dormant;
    startTimer( 100 );
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
	counter = 0;
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

    counter = 0;
    pos = w->mapToGlobal( p ) + QPoint( 0, 16 );
    if ( state == dormant )
	state = wakingUp;
    else
	state = wakingUpAgain;
}


void QTipManager::down()
{
    hideTip();
    if ( currentTip->autoDelete ) {
	// delete the tip here, and clean it up
    }
    currentTip = 0;
}


bool QTipManager::event( QEvent * e )
{
    if ( e->type() != Event_Timer )
	return TRUE;

    switch( state ) {
    case wakingUp:
	if ( counter++ > 10 )
	    showTip();
	break;
    case active:
	if ( counter++ > 100 )
	    hideTip();
	break;
    case fallingAsleep:
	if ( counter++ > 200 )
	    state = dormant;
	break;
    case wakingUpAgain:
	if ( counter++ > 1 )
	    showTip();
	break;
    default:
	break;
    }
    return TRUE;
}


void QTipManager::showTip()
{
    if ( !currentTip ) {
	// error of some sort
	state = dormant;
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
    state = active;
    counter = 0;
}


void QTipManager::hideTip()
{
    if ( label && label->isVisible() )
	label->hide();

    if ( state == active )
	state = fallingAsleep;
    else
	state = dormant;

    counter = 0;
    currentTip = 0;
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


/*!  Creates a tool tip object.  This is necessary only if you need
  tool tips on regions that can move within the widget (most often
  because the widget's contents can scroll).

  \sa maybeHit().
*/

QToolTip::QToolTip( QWidget * parent, const char * name = 0 )
    : QObject( parent, name )
{



}


/*!  Destroys the tool tip object.  This need never be called
  explicitly; Qt will delete it when the parent widget is deleted.
*/

QToolTip::~QToolTip()
{
    // nothing need be done?
}


/*!  Adds a tool tip to an entire widget.  \e widget

  The normal entry point to the QToolTip class.


*/
void QToolTip::add( QWidget * widget, const char * text )
{


}


void QToolTip::remove( QWidget * )
{


}

void QToolTip::add( QWidget *, const QRect &, const char * )
{



}


void QToolTip::remove( QWidget *, const QRect & )
{


}


/*! \fn virtual void QToolTip::maybeTip( const QPoint &);

*/


void QToolTip::tip( const QRect &, const char * )
{
}


void QToolTip::clear()
{
}

