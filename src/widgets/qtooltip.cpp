/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qtooltip.cpp#11 $
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

RCSTAG("$Id: //depot/qt/main/src/widgets/qtooltip.cpp#11 $");

// magic value meaning an entire widget - if someone tries to insert a
// tool tip on this part of a widget it will be interpreted as the
// entire widget

static inline const QRect entireWidget() {
    return QRect( QCOORD_MIN, QCOORD_MIN,
		  QCOORD_MAX-QCOORD_MIN, QCOORD_MAX-QCOORD_MIN );
}

// what comes out of the dict in QTipManager
struct QTip
{
    QRect rect;
    QString text;
    QString groupText;
    QToolTipGroup * group;
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
    void add( QWidget *, const QRect &,
	      const char *, 
	      QToolTipGroup *, const char *,
	      bool );
    void remove( QWidget *, const QRect & );
    void remove( QWidget * );

    void removeFromGroup( QToolTipGroup * );

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
    if ( tips ) {
	QIntDictIterator<QTip> i( *tips );
	QTip * t, * n;
	long k;

	while( (t = i.current()) != 0 ) {
	    k = i.currentKey();
	    ++i;
	    tips->take( k );
	    while ( t ) {
		n = t->next;
		delete t;
		t = n;
	    }
	}
	delete tips;
    }

    delete label;
}


void QTipManager::add( QWidget * w, const QRect & r, const char * s,
		       QToolTipGroup * g, const char * gs, bool a )
{
    QTip * t = (*tips)[ (long)w ];
    if ( !t ) {
	// the first one for this widget
	t = new QTip;
	t->next = 0;
	w->installEventFilter( tipManager );
	w->setMouseTracking( TRUE );
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

    t->groupText = gs;
    t->group = g;
}


void QTipManager::remove( QWidget *w, const QRect & r )
{
    QTip * t = (*tips)[ (long)w ];
    if ( t == 0 )
	return;

    if ( t->rect == r ) {
	(void) tips->take( (long)w );
	if ( t->next )
	    tips->insert( (long)w, t->next );
	else
	    w->removeEventFilter( tipManager );
    } else {
	while( t->next && t->next->rect != r )
	    t = t->next;
	if ( t->next ) {
	    QTip * d = t->next;
	    t->next = t->next->next;
	    t = d;
	}

	delete t;
    }

    if ( tips->isEmpty() ) {
	// the manager will be recreated if needed
	delete tipManager;
	tipManager = 0;
    }
}


void QTipManager::remove( QWidget *w )
{
    QTip * t = (*tips)[ (long)w ];
    if ( t == 0 )
	return;

    (void) tips->take( (long)w );
    QTip * d;
    while ( t ) {
	d = t->next;
	delete t;
	t = d;
    }

    if ( tips->isEmpty() ) {
	delete tipManager;
	tipManager = 0;
    }
}


void QTipManager::removeFromGroup( QToolTipGroup * g )
{
    QIntDictIterator<QTip> i( *tips );
    QTip * t;

    while( (t = i.current()) != 0 ) {
	++i;
	while ( t ) {
	    if ( t->group == g )
		t->group = 0;
	    t = t->next;
	}
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
    if ( currentTip->group && !currentTip->groupText.isEmpty() )
	emit currentTip->group->showTip( currentTip->groupText );
}


void QTipManager::hideTip()
{
    if ( label && label->isVisible() )
	label->hide();

    if ( currentTip && currentTip->group )
	emit currentTip->group->removeTip();

    killTimers();
    if ( state == active ) {
	state = fallingAsleep;
	startTimer( 2000 );
    } else {
	state = dormant;
    }

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
  remove() with no arguments.  This is handy if the widget scrolls.
*/


/*!  Creates a tool tip object.	 This is necessary only if you need
  tool tips on regions that can move within the widget (most often
  because the widget's contents can scroll).

  \a parent is widget you want to add dynamic tool tips to and \a
  group is the tool tip group they should belong to, if any.

  \sa maybeHit().
*/

QToolTip::QToolTip( QWidget * parent, QToolTipGroup * group )
{
    p = parent;
    g = group;
}


/*!  Adds a tool tip to the entire \e widget.  \e text is the text to
  be shown in the tool tip.  QToolTip makes a deep copy of this
  string.

  This is the most common entry point to the QToolTip class.
*/
void QToolTip::add( QWidget * widget, const char * text )
{
    if ( !tipManager )
	tipManager = new QTipManager();
    tipManager->add( widget, entireWidget(), text, 0, 0, FALSE );
}


/*!  Adds a tool tip to an entire \a widget, and to tool tip group \a
  group.

  \e text is the text shown in the tool tip and \a longText is the
  text emitted from \a group.  QToolTip makes deep copies of both
  strings.

  Normally, \a longText is shown in a status bar or similar.
*/

void QToolTip::add( QWidget * widget, const char * text,
		    QToolTipGroup * group, const char * longText )
{
    if ( !tipManager )
	tipManager = new QTipManager();
    tipManager->add( widget, entireWidget(), text, group, longText, FALSE );
}

/*! Remove the tool tip from \e widget.

  If there are more than one tool tip on \a widget, only the one
  covering the entire widget is removed.
*/

void QToolTip::remove( QWidget * widget )
{
    if ( tipManager )
	tipManager->remove( widget, entireWidget() );
}

/*! Adds a tool tip to a fixed rectangle within \a widget.  \a text is
  the text shown in the tool tip.  QToolTip makes a deep copy of this
  string.
*/

void QToolTip::add( QWidget * widget, const QRect & rect, const char * text )
{
    if ( !tipManager )
	tipManager = new QTipManager();
    tipManager->add( widget, rect, text, 0, 0, FALSE );
}


/*!  Adds a tool tip to an entire \a widget, and to tool tip group \a
  group.

  \e text is the text shown in the tool tip and \a longText is the
  text emitted from \a group.  QToolTip make deep copies of both
  strings.

  Normally, \a longText is shown in a status bar or similar.
*/

void QToolTip::add( QWidget * widget, const QRect & rect,
		    const char * text,
		    QToolTipGroup * group, const char * groupText )
{
    if ( !tipManager )
	tipManager = new QTipManager();
    tipManager->add( widget, rect, text, group, groupText, FALSE );
}

/*! Remove the tool tip for \e rect from \e widget.

  If there are more than one tool tip on \a widget, only the one
  covering rectangle \e rect is removed.
*/

void QToolTip::remove( QWidget * widget, const QRect & rect )
{
    if ( tipManager )
	tipManager->remove( widget, rect );
}


/*! \fn virtual void QToolTip::maybeTip( const QPoint &);

  This pure virtual function is half of the most versatile interface
  QToolTip offers.
*/


/*! Pop up a tip saying \a text right now, and remove that tip once
  the cursor moves out of rectangle \a rect.

  The tip will not come back if the cursor moves back; your maybeTip()
  has to reinstate it each time.
*/

void QToolTip::tip( const QRect & rect, const char * text )
{
    if ( !tipManager )
	tipManager = new QTipManager();
    tipManager->add( parentWidget(), rect, text, 0, 0, TRUE );
}

/*! Pop up a tip saying \a text right now, and remove that tip once
  the cursor moves out of rectangle \a rect.

  The tip will not come back if the cursor moves back; your maybeTip()
  has to reinstate it each time.
*/

void QToolTip::tip( const QRect & rect, const char * text,
		    const char * groupText )
{
    if ( !tipManager )
	tipManager = new QTipManager();
    tipManager->add( parentWidget(), rect, text, group(), groupText, TRUE );
}


/*! Remove all tool tips for this widget immediately. */

void QToolTip::clear()
{
    if ( tipManager )
	tipManager->remove( parentWidget() );
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
	tipManager->removeFromGroup( this );
}
