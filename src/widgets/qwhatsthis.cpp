/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qwhatsthis.cpp#3 $
**
** C++ file skeleton
**
** Copyright (C) 1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qwhatsthis.h"

#include "qapp.h"
#include "qpdevmet.h"
#include "qpixmap.h"
#include "qpainter.h"
#include "qtimer.h"
#include "qptrdict.h"
#include "qtoolbutton.h"
#include "qshared.h"
#include "qkeycode.h"


RCSTAG("$Id: //depot/qt/main/src/widgets/qwhatsthis.cpp#3 $");


class QWhatsThisPrivate: public QObject
{
public:
    // a special button
    struct Button: public QToolButton
    {
	Button( QWidget * parent, const char * name );
	~Button();

	// reimplemented because of QButton's lack of virtuals
	void mouseReleaseEvent( QMouseEvent * );
	
	// reimplemented because, well, because I'm evil.
	const char *className() const;
    };

    // an item for storing texts
    struct Item: public QShared
    {
	Item(): QShared(), dc(FALSE), s(0) {}
	~Item();
	bool dc;
	const char * s;
    };

    // the state machine
    enum State { Inactive, Waiting, Displaying, FinalPress };

    QWhatsThisPrivate();
    ~QWhatsThisPrivate();

    bool eventFilter( QObject *, QEvent * );

    // say it.
    void say( QWidget *, const char * );

    // setup and teardown
    static void tearDownWhatsThis();
    static void setUpWhatsThis();

    // variables
    QWidget * whatsThat;
    QPtrDict<Item> * dict;
    QPtrDict<QWidget> * tlw;
    QPtrDict<Button> * buttons;
    State state;
};


// static, but static the less-typing way
static QWhatsThisPrivate * wt = 0;


// the item
QWhatsThisPrivate::Item::~Item()
{
    if ( count )
	fatal( "Internal error #10%d in What's This", count );
    if ( dc )
	delete[] s;
}


static const char * fucking_button_image_and_you_too[] = {
"16 16 3 1",
" 	c None",
"o	c #000000",
"a	c #000080",
"o        aaaaa  ",
"oo      aaa aaa ",
"ooo    aaa   aaa",
"oooo   aa     aa",
"ooooo  aa     aa",
"oooooo  a    aaa",
"ooooooo     aaa ",
"oooooooo   aaa  ",
"ooooooooo aaa   ",
"ooooo     aaa   ",
"oo ooo          ",
"o  ooo    aaa   ",
"    ooo   aaa   ",
"    ooo         ",
"     ooo        ",
"     ooo        "};


// the button class
QWhatsThisPrivate::Button::Button( QWidget * parent, const char * name )
    : QToolButton( parent, name )
{
    QPixmap p( fucking_button_image_and_you_too );
    setPixmap( p );
    setToggleButton( TRUE );
    wt->buttons->insert( (void *)this, this );
}


QWhatsThisPrivate::Button::~Button()
{
    if ( wt && wt->buttons )
	wt->buttons->take( (void *)this );
}


void QWhatsThisPrivate::Button::mouseReleaseEvent( QMouseEvent * e )
{
    QToolButton::mouseReleaseEvent( e );
    if ( isOn() ) {
	setUpWhatsThis();
	wt->state = Waiting;
	qApp->installEventFilter( wt );
    }
}


const char *QWhatsThisPrivate::Button::className() const
{
    return "QWhatsThisPrivate::Button";
}


// the what's this manager class
QWhatsThisPrivate::QWhatsThisPrivate()
    : QObject( 0, "global what's this object" )
{
    qAddPostRoutine( tearDownWhatsThis );
    whatsThat = 0;
    dict = new QPtrDict<QWhatsThisPrivate::Item>;
    tlw = new QPtrDict<QWidget>;
    wt = this;
    buttons = new QPtrDict<Button>;
    state = Inactive;
}

QWhatsThisPrivate::~QWhatsThisPrivate()
{
    // the two straight-and-simple dicts
    delete tlw;
    delete buttons;
    
    
    // then delete the complex one.
    QPtrDictIterator<Item> it( *dict );
    Item * i;
    QWidget * w;
    while( (i=it.current()) != 0 ) {
	w = (QWidget *)it.currentKey();
	++it;
	dict->take( w );
	i->deref();
	if ( !i->dc || !i->count )
	    delete i;
    }
    
    // and finally lose wt
    wt = 0;
}    

bool QWhatsThisPrivate::eventFilter( QObject * o, QEvent * e )
{
    if ( !o || !e )
	return FALSE;

    switch( state ) {
    case FinalPress:
	if( e->type() == Event_MouseButtonRelease ) {
	    state = Inactive;
	    qApp->removeEventFilter( this );
	    if ( whatsThat )
		whatsThat->hide();
	    return TRUE;
	} else if ( e->type() == Event_MouseMove ) {
	    return TRUE;
	}
	break;
    case Displaying:
	if ( e->type() == Event_MouseButtonPress ) {
	    if ( !qstrcmp( "QWhatsThisPrivate::Button", o->className() ) ) {
		state = Inactive;
		qApp->removeEventFilter( this );
	    } else {
		state = FinalPress;
	    }
	    if ( whatsThat )
		whatsThat->hide();
	    return TRUE;
	} else if ( e->type() == Event_MouseButtonRelease ||
		    e->type() == Event_MouseMove ) {
	    return TRUE;
	} else if ( e->type() == Event_Accel ) {
	    if ( whatsThat )
		whatsThat->hide();
	    ((QKeyEvent *)e)->accept();
	    state = Inactive;
	    qApp->removeEventFilter( this );
	} else if ( e->type() == Event_FocusOut ||
		    e->type() == Event_FocusIn ) {
	    if ( whatsThat )
		whatsThat->hide();
	    state = Inactive;
	    qApp->removeEventFilter( this );
	}
	break;
    case Waiting:
	if ( e->type() == Event_MouseButtonPress && o->isWidgetType() ) {
	    QWidget * w = (QWidget *) o;
	    QWhatsThisPrivate::Item * i = dict->find( w );
	    QPtrDictIterator<Button> it( *(wt->buttons) );
	    Button * b;
	    while( (b=it.current()) != 0 ) {
		++it;
		b->setOn( FALSE );
	    }
	    if ( i ) {
		state = Displaying;
		say( w, i->s );
	    } else {
		state = FinalPress;
	    }
	    return TRUE;
	} else if ( e->type() == Event_FocusOut ||
		    e->type() == Event_FocusIn ||
		    e->type() == Event_Accel ||
		    e->type() == Event_KeyPress ) {
	    if ( whatsThat )
		whatsThat->hide();
	    state = Inactive;
	    qApp->removeEventFilter( this );
	}
	break;
    case Inactive:
	if ( e->type() == Event_Accel &&
	     ((QKeyEvent *)e)->key() == Key_F1 &&
	     !o->parent() &&
	     o->isWidgetType() &&
	     ((QKeyEvent *)e)->state() == ShiftButton ) {
	    ((QKeyEvent *)e)->accept();
	    QWidget * w = ((QWidget *)o)->focusWidget();
	    QWhatsThisPrivate::Item * i = 0;
	    if ( w && (i=dict->find( w )) != 0 && i->s ) {
		say( w, i->s );
		state = Displaying;
		qApp->installEventFilter( this );
	    }
	    return TRUE;
	}
	break;
    }
    return FALSE;
}



void QWhatsThisPrivate::setUpWhatsThis()
{
    if ( !wt )
	wt = new QWhatsThisPrivate();
}


void QWhatsThisPrivate::tearDownWhatsThis()
{
    delete wt;
    wt = 0;
}


void QWhatsThisPrivate::say( QWidget * widget, const char * text )
{
    const int shadowWidth = 6;   // also used as '5' and '6' and even '8' below
    const int normalMargin = 12; // *2
    const int leftMargin = 18;   // *3

    QWidget * desktop = QApplication::desktop();

    int w = desktop->width() / 3;
    if ( w < 200 )
	w = 200;
    else if ( w > 300 )
	w = 300;

    QPixmap pm( 1,1 );
    QPainter p( &pm );
    QRect r = p.boundingRect( 0, 0, w, 1000,
			      AlignLeft + AlignTop + WordBreak + ExpandTabs,
			      text );
    p.end();
    int h = r.height() + normalMargin + normalMargin;
    w = w + leftMargin + normalMargin;

    // okay, now to find a suitable location

    QPoint pos = widget->mapToGlobal( QPoint( 0,0 ) );
    int x;

    // first try locating the widget immediately above/below,
    // with nice alignment if possible.
    if ( w > widget->width() + 16 )
	x = pos.x() + widget->width()/2 - w/2;
    else
	x = pos.x();

    // squeeze it in if that would result in part of what's this
    // being only partially visible
    if ( x + w > QApplication::desktop()->width() )
	x = QMIN(QApplication::desktop()->width(),
		 pos.x() + widget->width())
	    - w;

    if ( x < 0 )
	x = 0;

    int y =pos.y() + widget->height() + 2; // below, two pixels spacing
    // what's this is above or below, wherever there's most space
    if ( y + h + 10 > QApplication::desktop()->height() )
	y = pos.y() + 2 - shadowWidth - h; // above, overlap

#if 0
    // should try to fit the whats this widget onto the same
    // top-level widget here, if possible.

    // if there wasn't enough space either above or below, try the sides
    if ( y + height() > m.height() || y < 0 ) {
	//
	if ( pos.x() > width() + 2 )
	    x = pos.x() + widget->width() + 8;
	else
	    x = pos.x() - width() - 2;

	if ( pos.y() + height() < m.height() )
	    y = pos.y();
	else if ( pos.y() + widget->height() > height() )
	    y = pos.y() + widget->height() - height();
	else
	    y = m.height() - height();
    }
#endif

    // make the widget, and set it up
    if ( !whatsThat ) {
	whatsThat = new QWidget( 0, "automatic what's this? widget",
				 WStyle_Customize +
				 WStyle_NoBorder + WStyle_Tool );
	whatsThat->setBackgroundMode( QWidget::NoBackground );
    }
    whatsThat->setGeometry( x, y, w + shadowWidth, h + shadowWidth );
    whatsThat->show();

    // now for super-clever shadow stuff.  super-clever mostly in
    // how many window system problems it skirts around.
    p.begin( whatsThat );

    p.setPen( QApplication::palette()->normal().foreground() );
    p.drawRect( 0, 0, w, h );
    p.setPen( QApplication::palette()->normal().mid() );
    p.setBrush( QColor( 255, 255, 240 ) );
    p.drawRect( 1, 1, w-2, h-2 );
    p.setPen( QApplication::palette()->normal().text() );
    p.drawText( leftMargin, normalMargin, r.width(), r.height(),
		AlignLeft + AlignTop + WordBreak + ExpandTabs,
		text );
    p.setPen( black );
    p.drawPoint( w + 5, 6 );
    p.drawLine( w + 3, 6,
		w + 5, 8 );
    p.drawLine( w + 1, 6,
		w + 5, 10 );
    int i;
    for( i=7; i < h; i += 2 )
	p.drawLine( w, i,
		    w + 5, i + 5 );
    for( i = w - i + h; i > 6; i -= 2 )
	p.drawLine( i, h,
		    i + 5, h + 5 );
    for( ; i > 0 ; i -= 2 )
	p.drawLine( 6, h + 6 - i,
		    i + 5, h + 5 );
    p.end();
}



// and finally the What's This class itself

/*!

*/

void QWhatsThis::add( QWidget * widget, const char * text, bool deepCopy )
{
    QWhatsThisPrivate::setUpWhatsThis();
    QWhatsThisPrivate::Item * i = wt->dict->find( (void *)widget );
    if ( i )
	remove( widget );

    i = new QWhatsThisPrivate::Item;
    i->dc = deepCopy;
    if ( deepCopy ) {
	i->s = new char[ qstrlen(text) + 1 ];
	qstrcpy( (char *)i->s, text );
    } else {
	i->s = text;
    }
    wt->dict->insert( (void *)widget, i );
    QWidget * tlw = widget->topLevelWidget();
    if ( !wt->tlw->find( (void *)tlw ) ) {
	wt->tlw->insert( (void *)tlw, tlw );
	tlw->installEventFilter( wt );
    }
}


/*!

*/

void QWhatsThis::remove( QWidget * widget )
{
    QWhatsThisPrivate::setUpWhatsThis();
    QWhatsThisPrivate::Item * i = wt->dict->find( (void *)widget );
    if ( !i )
	return;

    wt->dict->take( (void *)widget );

    i->deref();
    if ( !i->dc || !i->count )
	delete i;
}


/*!

*/

const char * QWhatsThis::textFor( QWidget * widget )
{
    QWhatsThisPrivate::setUpWhatsThis();
    QWhatsThisPrivate::Item * i = wt->dict->find( widget );
    return i ? i->s : 0;
}


/*!

*/

QToolButton * QWhatsThis::whatsThisButton( QWidget * parent )
{
    QWhatsThisPrivate::setUpWhatsThis();
    return new QWhatsThisPrivate::Button( parent,
					  "automatic what's this? button" );
}



