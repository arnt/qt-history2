/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qwhatsthis.cpp#58 $
**
** Implementation of QWhatsThis class
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

#include "qwhatsthis.h"

#include "qapplication.h"
#include "qpaintdevicemetrics.h"
#include "qpixmap.h"
#include "qpainter.h"
#include "qtimer.h"
#include "qptrdict.h"
#include "qtoolbutton.h"
#include "qshared.h"
#include "qcursor.h"
#include "qbitmap.h"
#include "qtooltip.h"
#include "qsimplerichtext.h"
#include "qstylesheet.h"

// NOT REVISED
/*!
  \class QWhatsThis qwhatsthis.h

  \brief The QWhatsThis class provides a simple description of any
  widget, e.g. answering the question "what's this?"

  \ingroup application

  What's This help lies between tool tips and fully-blown online help
  systems: <ul><li> Tool Tips - flyweight help, extremely brief,
  entirely integrated in the user interface. <li> What's This? - also
  lightweight, but can encompass a three-paragraph explanation.  <li>
  Online Help - can encompass any amount of information, but is
  typically a little slower to call up, a little separated from the
  user's work, and often users feel that using online help is a
  digression from their real task. </ul>

  QWhatsThis, then, offers a single window with a single explanatory
  text, which pops up quickly when the user asks "what's this?", and
  goes away as soon as the user does something else.  There are two
  ways to make QWhatsThis pop up: Click a "What's This?" button and
  then click on some other widget to get help for that other widget,
  or press Shift-F1 to get help for the widget that has keyboard
  focus. But you can also connect a "what's this" entry of a help menu
  (with Shift-F1 as accelerator) to the whatsThis() slot of a
  QMainWindow, then the focus widget does not have a special meaning.

  QWhatsThis provides functions to add() and remove() What's This help
  for a widget, and it provides a function to create a What's This
  button suitable for typical tool bars.

  <img src="whatsthis.png" width="376" height="239">

  Futhermore, you can create a dedicated QWhatsThis object for a
  special widget. By subclassing and reimplementing QWhatsThis::text()
  it is possible, to have different explanatory texts depending on the
  position of the mouse click.

  If you widget needs even more control, see
  QWidget::customWhatsThis().

  Other Qt classes support What's This help directly. Keyboard
  accelerators, for example, can also provide explanatory text with
  access through both they acceleration key itself or an attached menu
  item. See QAccel::setWhatsThis() for details.

  The explanatory text can be both rich text or plain text.  If you
  specify a rich text formatted string, it will be rendered using the
  default stylesheet. This makes it also possible to embed images. See
  QStyleSheet::defaultSheet() for details.

  \sa QToolTip
*/

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
    struct WhatsThisItem: public QShared
    {
	WhatsThisItem(): QShared() { whatsthis = 0; }
	~WhatsThisItem();
	QString s;
	QWhatsThis* whatsthis;
    };

    // the (these days pretty small) state machine
    enum State { Inactive, Waiting };

    QWhatsThisPrivate();
    ~QWhatsThisPrivate();

    bool eventFilter( QObject *, QEvent * );

    // say it.
    void say( QWidget *, const QString&, const QPoint&  );

    // setup and teardown
    static void tearDownWhatsThis();
    static void setUpWhatsThis();

    void leaveWhatsThisMode();

    // variables
    QWidget * whatsThat;
    QPtrDict<WhatsThisItem> * dict;
    QPtrDict<QWidget> * tlw;
    QPtrDict<Button> * buttons;
    State state;

    QCursor * cursor;
};


// static, but static the less-typing way
static QWhatsThisPrivate * wt = 0;


// the item
QWhatsThisPrivate::WhatsThisItem::~WhatsThisItem()
{
    if ( count )
	qFatal( "Internal error #10%d in What's This", count );
}


static const char * button_image[] = {
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

#define cursor_bits_width 32
#define cursor_bits_height 32
static unsigned char cursor_bits_bits[] = {
  0x01, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x05, 0xf0, 0x07, 0x00,
  0x09, 0x18, 0x0e, 0x00, 0x11, 0x1c, 0x0e, 0x00, 0x21, 0x1c, 0x0e, 0x00,
  0x41, 0x1c, 0x0e, 0x00, 0x81, 0x1c, 0x0e, 0x00, 0x01, 0x01, 0x07, 0x00,
  0x01, 0x82, 0x03, 0x00, 0xc1, 0xc7, 0x01, 0x00, 0x49, 0xc0, 0x01, 0x00,
  0x95, 0xc0, 0x01, 0x00, 0x93, 0xc0, 0x01, 0x00, 0x21, 0x01, 0x00, 0x00,
  0x20, 0xc1, 0x01, 0x00, 0x40, 0xc2, 0x01, 0x00, 0x40, 0x02, 0x00, 0x00,
  0x80, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, };

#define cursor_mask_width 32
#define cursor_mask_height 32
static unsigned char cursor_mask_bits[] = {
  0x01, 0x00, 0x00, 0x00, 0x03, 0xf0, 0x07, 0x00, 0x07, 0xf8, 0x0f, 0x00,
  0x0f, 0xfc, 0x1f, 0x00, 0x1f, 0x3e, 0x1f, 0x00, 0x3f, 0x3e, 0x1f, 0x00,
  0x7f, 0x3e, 0x1f, 0x00, 0xff, 0x3e, 0x1f, 0x00, 0xff, 0x9d, 0x0f, 0x00,
  0xff, 0xc3, 0x07, 0x00, 0xff, 0xe7, 0x03, 0x00, 0x7f, 0xe0, 0x03, 0x00,
  0xf7, 0xe0, 0x03, 0x00, 0xf3, 0xe0, 0x03, 0x00, 0xe1, 0xe1, 0x03, 0x00,
  0xe0, 0xe1, 0x03, 0x00, 0xc0, 0xe3, 0x03, 0x00, 0xc0, 0xe3, 0x03, 0x00,
  0x80, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, };



// the button class
QWhatsThisPrivate::Button::Button( QWidget * parent, const char * name )
    : QToolButton( parent, name )
{
    QPixmap p( button_image );
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
    if ( wt->state == Inactive && isOn() ) {
	setUpWhatsThis();
	QApplication::setOverrideCursor( *wt->cursor, FALSE );
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
    dict = new QPtrDict<QWhatsThisPrivate::WhatsThisItem>;
    tlw = new QPtrDict<QWidget>;
    wt = this;
    buttons = new QPtrDict<Button>;
    state = Inactive;
    cursor = new QCursor( QBitmap( cursor_bits_width, cursor_bits_height,
				   cursor_bits_bits, TRUE ),
			  QBitmap( cursor_mask_width, cursor_mask_height,
				   cursor_mask_bits, TRUE ),
			  1, 1 );
}

QWhatsThisPrivate::~QWhatsThisPrivate()
{
    if ( state == Waiting )
	QApplication::restoreOverrideCursor();

    // the two straight-and-simple dicts
    delete tlw;
    delete buttons;

    // then delete the complex one.
    QPtrDictIterator<WhatsThisItem> it( *dict );
    WhatsThisItem * i;
    QWidget * w;
    while( (i=it.current()) != 0 ) {
	w = (QWidget *)it.currentKey();
	++it;
	dict->take( w );
	i->deref();
	if ( !i->count )
	    delete i;
    }
    delete dict;
    delete cursor;
    delete whatsThat;

    // and finally lose wt
    wt = 0;
}

bool QWhatsThisPrivate::eventFilter( QObject * o, QEvent * e )
{
    if ( !o || !e )
	return FALSE;

    if ( o == whatsThat ) {
	if (e->type() == QEvent::MouseButtonPress  || e->type() == QEvent::KeyPress ) {
	    whatsThat->hide();
	    return TRUE;
	}
	return FALSE;
    }

    switch( state ) {
    case Waiting:
	if ( e->type() == QEvent::MouseButtonPress && o->isWidgetType() ) {
	    QWidget * w = (QWidget *) o;
	    if ( ( (QMouseEvent*)e)->button() == RightButton )
		return FALSE; // ignore RMB
	    if ( w->customWhatsThis() )
		return FALSE;
	    QWhatsThisPrivate::WhatsThisItem * i = 0;
	    while( w && !i ) {
		i = dict->find( w );
		if ( !i )
		    w = w->parentWidget();
	    }
	
	    leaveWhatsThisMode();
	    if (!i )
		return TRUE;
	    QPoint pos =  ((QMouseEvent*)e)->pos();
	    if ( i->whatsthis )
		say( w, i->whatsthis->text( pos ), w->mapToGlobal(pos) );
	    else
		say( w, i->s, w->mapToGlobal(pos) );
	    return TRUE;
	} else if ( e->type() == QEvent::MouseButtonRelease ) {
	    if ( ( (QMouseEvent*)e)->button() == RightButton )
		return FALSE; // ignore RMB
	    return !o->isWidgetType() || !((QWidget*)o)->customWhatsThis();
	} else if ( e->type() == QEvent::MouseMove ) {
	    return !o->isWidgetType() || !((QWidget*)o)->customWhatsThis();
	} else if ( e->type() == QEvent::KeyPress ) {
	    QKeyEvent* kev = (QKeyEvent*)e;

	    if (kev->key() == Qt::Key_Escape) {
		leaveWhatsThisMode();
		return TRUE;
	    }
	    else if ( kev->key() == Key_Menu ||
		      ( kev->key() == Key_F10 && kev->state() == ShiftButton ) )
		return FALSE; // ignore these keys, they are used for context menus
	    else if ( kev->state() == kev->stateAfter() && kev->key() != Key_Meta )  // not a modifier key
		leaveWhatsThisMode();
	
	}
	break;
    case Inactive:
 	if ( e->type() == QEvent::Accel &&
 	     ((QKeyEvent *)e)->key() == Key_F1 &&
 	     !o->parent() &&
 	     o->isWidgetType() &&
 	     ((QKeyEvent *)e)->state() == ShiftButton ) {
 	    QWidget * w = ((QWidget *)o)->focusWidget();
 	    QWhatsThisPrivate::WhatsThisItem *i = w ? dict->find(w) : 0;
 	    if ( i && !i->s.isNull() ) {
		if ( i->whatsthis )
		    say( w, i->whatsthis->text( QPoint(0,0) ), w->mapToGlobal( w->rect().center() ));
		else
		    say( w, i->s, w->mapToGlobal( w->rect().center() ));
		((QKeyEvent *)e)->accept();
		return TRUE;
 	    }
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



void QWhatsThisPrivate::leaveWhatsThisMode()
{
    if ( state == Waiting ) {
	QPtrDictIterator<Button> it( *(wt->buttons) );
	Button * b;
	while( (b=it.current()) != 0 ) {
	    ++it;
	    b->setOn( FALSE );
	}
	QApplication::restoreOverrideCursor();
	state = Inactive;
	qApp->removeEventFilter( this );
    }
}

void QWhatsThisPrivate::say( QWidget * widget, const QString &text, const QPoint& ppos)
{
    const int shadowWidth = 6;   // also used as '5' and '6' and even '8' below
    const int vMargin = 8;
    const int hMargin = 12;
    
    if ( text.isEmpty() )
	return;

    // make the widget, and set it up
    if ( !whatsThat ) {
	whatsThat = new QWidget( 0, "automatic what's this? widget",
				 WType_Popup );
	whatsThat->setBackgroundMode( QWidget::NoBackground );
	whatsThat->setPalette( QToolTip::palette(), TRUE );
	whatsThat->installEventFilter( this );
    }


    QPainter p( whatsThat );

    QRect r;
    QSimpleRichText* qmlDoc = 0;

    if ( QStyleSheet::mightBeRichText( text ) ) {
	qmlDoc = new QSimpleRichText( text, whatsThat->font() );
	qmlDoc->adjustSize( &p );
	r.setRect( 0, 0, qmlDoc->width(), qmlDoc->height() );
    }
    else {
	int sw = QApplication::desktop()->width() / 3;
	if ( sw < 200 )
	    sw = 200;
	else if ( sw > 300 )
	    sw = 300;

	r = p.boundingRect( 0, 0, sw, 1000,
			    AlignLeft + AlignTop + WordBreak + ExpandTabs,
			    text );
    }

    int w = r.width() + 2*hMargin;
    int h = r.height() + 2*vMargin;

    // okay, now to find a suitable location

    int x;

    // first try locating the widget immediately above/below,
    // with nice alignment if possible.
    QPoint pos;
    if ( widget )
	pos = widget->mapToGlobal( QPoint( 0,0 ) );

    if ( widget && w > widget->width() + 16 )
	    x = pos.x() + widget->width()/2 - w/2;
    else
	x = ppos.x() - w/2;

    // squeeze it in if that would result in part of what's this
    // being only partially visible
    if ( x + w > QApplication::desktop()->width() )
	x = (widget? (QMIN(QApplication::desktop()->width(),
			  pos.x() + widget->width())
		     ) : QApplication::desktop()->width() )
	    - w;

    if ( x < 0 )
	x = 0;

    int y;
    if ( widget && h > widget->height() + 16 ) {
	y = pos.y() + widget->height() + 2; // below, two pixels spacing
	// what's this is above or below, wherever there's most space
	if ( y + h + 10 > QApplication::desktop()->height() )
	    y = pos.y() + 2 - shadowWidth - h; // above, overlap
    }
    y = ppos.y() + 2;

    // squeeze it in if that would result in part of what's this
    // being only partially visible
    if ( y + h > QApplication::desktop()->height() )
	y = ( widget ? (QMIN(QApplication::desktop()->height(),
			     pos.y() + widget->height())
			) : QApplication:: desktop()->height() )
	    - h;
    if ( y < 0 )
	y = 0;


    whatsThat->setGeometry( x, y, w + shadowWidth, h + shadowWidth );
    whatsThat->show();

    // now for super-clever shadow stuff.  super-clever mostly in
    // how many window system problems it skirts around.

    p.setPen( whatsThat->colorGroup().foreground() );
    p.drawRect( 0, 0, w, h );
    p.setPen( whatsThat->colorGroup().mid() );
    p.setBrush( whatsThat->colorGroup().background() );
    p.drawRect( 1, 1, w-2, h-2 );
    p.setPen( whatsThat->colorGroup().foreground() );

    if ( qmlDoc ) {
	qmlDoc->draw( &p, hMargin, vMargin, r, whatsThat->colorGroup(), 0 );
	delete qmlDoc;
    }
    else {
	p.drawText( hMargin, vMargin, r.width(), r.height(),
		    AlignLeft + AlignTop + WordBreak + ExpandTabs,
		    text );
    }
    p.setPen( whatsThat->colorGroup().shadow() );

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

  Adds \a text as What's This help for \a widget. If the text is rich
  text formatted, it will be rendered with the default stylesheet
  QStyleSheet::defaultSheet().

  \sa remove()
*/

void QWhatsThis::add( QWidget * widget, const QString &text )
{
    QWhatsThisPrivate::setUpWhatsThis();
    QWhatsThisPrivate::WhatsThisItem * i = wt->dict->find( (void *)widget );
    if ( i )
	remove( widget );

    i = new QWhatsThisPrivate::WhatsThisItem;
    i->s = text;
    wt->dict->insert( (void *)widget, i );
    QWidget * tlw = widget->topLevelWidget();
    if ( !wt->tlw->find( (void *)tlw ) ) {
	wt->tlw->insert( (void *)tlw, tlw );
	tlw->installEventFilter( wt );
    }
}


/*!  Removes the What's This help for \a widget.  \sa add() */

void QWhatsThis::remove( QWidget * widget )
{
    QWhatsThisPrivate::setUpWhatsThis();
    QWhatsThisPrivate::WhatsThisItem * i = wt->dict->find( (void *)widget );
    if ( !i )
	return;

    wt->dict->take( (void *)widget );

    i->deref();
    if ( !i->count )
	delete i;
}


/*!  Returns the text for \a widget, or a null string if there
  isn't any What's This help for \a widget.

  \sa add() */

QString QWhatsThis::textFor( QWidget * widget, const QPoint& pos)
{
    QWhatsThisPrivate::setUpWhatsThis();
    QWhatsThisPrivate::WhatsThisItem * i = wt->dict->find( widget );
    if (!i)
	return QString::null;
    return i->whatsthis? i->whatsthis->text( pos ) : i->s;
}


/*!  Returns a pointer to a specially configured QToolButton, suitable
  for use to enter What's This mode.

  \sa QToolButton
*/

QToolButton * QWhatsThis::whatsThisButton( QWidget * parent )
{
    QWhatsThisPrivate::setUpWhatsThis();
    return new QWhatsThisPrivate::Button( parent,
					  "automatic what's this? button" );
}


/*! \base64 whatsthis.png

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
Jjc3AAAAAElFTkSuQmCC
*/


/*!  Constructs a dynamic What's This object for \a widget.
*/

QWhatsThis::QWhatsThis( QWidget * widget)
{
    QWhatsThisPrivate::setUpWhatsThis();
    QWhatsThisPrivate::WhatsThisItem * i = wt->dict->find( (void *)widget );
    if ( i )
	remove( widget );

    i = new QWhatsThisPrivate::WhatsThisItem;
    i->whatsthis = this;
    wt->dict->insert( (void *)widget, i );
    QWidget * tlw = widget->topLevelWidget();
    if ( !wt->tlw->find( (void *)tlw ) ) {
	wt->tlw->insert( (void *)tlw, tlw );
	tlw->installEventFilter( wt );
    }
}


/*! Destroys the object and frees any allocated resources.

*/

QWhatsThis::~QWhatsThis()
{
}


/*!  This virtual functions returns the text for position \e p in the
  widget this What's This object documents.  If there is no What's
  This text for a position, QString::null may be returned.

  The default implementation returns QString::null.
*/

QString QWhatsThis::text( const QPoint & )
{
    return QString::null; //####
}


/*!  Enters What's This? question mode and returns immediately.

  What's This will install a special cursor and take over mouse input
  until the user click somewhere, then show any help available and
  switch out of What's This mode.  Finally, What's This removes its
  cursor and help window and restores ordinary event processing.  At
  this point the left mouse button is not pressed.

\sa inWhatsThisMode(), leaveWhatsThisMode()
*/

void QWhatsThis::enterWhatsThisMode()
{
    QWhatsThisPrivate::setUpWhatsThis();
    if ( wt->state == QWhatsThisPrivate::Inactive ) {
	QApplication::setOverrideCursor( *wt->cursor, FALSE );
	wt->state = QWhatsThisPrivate::Waiting;
	qApp->installEventFilter( wt );
    }
}


/*!
  Returns whether the application is in What's This mode.

\sa enterWhatsThisMode(), leaveWhatsThisMode()
 */
bool QWhatsThis::inWhatsThisMode()
{
    if (!wt)
	return FALSE;
    return wt->state == QWhatsThisPrivate::Waiting;
}


/*!  Leaves What's This? question mode

  This function is used internally by widgets that support
  QWidget::customWhatsThis(), applications usually never have to call
  it.  An example for such a kind of widget is QPopupMenu: Menus still
  work normally in What's This mode, but provide help texts for single
  menu items instead.

  If \e text is not a null string, then a What's This help window is
  displayed at the global position \e pos of the screen.

\sa inWhatsThisMode(), enterWhatsThisMode()
*/
void QWhatsThis::leaveWhatsThisMode( const QString& text, const QPoint& pos )
{
    if ( !inWhatsThisMode() )
	return;

    wt->leaveWhatsThisMode();
    if ( !text.isNull() )
	wt->say( 0, text, pos );
}

