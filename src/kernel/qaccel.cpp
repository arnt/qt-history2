/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qaccel.cpp#58 $
**
** Implementation of QAccel class
**
** Created : 950419
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

#define QAccelList QListM_QAccelItem
#include "qaccel.h"
#include "qapplication.h"
#include "qwidget.h"
#include "qlist.h"
#include "qsignal.h"

/*!
  \class QAccel qaccel.h
  \brief The QAccel class handles keyboard accelerator keys.

  \ingroup kernel

  A QAccel contains a list of accelerator items. Each accelerator item
  consists of an identifier and a keyboard code combined with modifiers
  (\c SHIFT, \c CTRL, \c ALT or \c ASCII_ACCEL).

  For example, <code>CTRL + Key_P</code> could be a shortcut for printing
  a document. The key codes are listed in qkeycode.h.

  When pressed, an accelerator key sends out the signal activated() with a
  number that identifies this particular accelerator item.  Accelerator
  items can also be individually connected, so that two different keys
  will activate two different slots (see connectItem()).

  A QAccel object handles key events to the
  \link QWidget::topLevelWidget() top level window\endlink
  containing \a parent, and hence to any child widgets of that window.
  Note that the accelerator will be deleted only when the \a parent
  is deleted, and will consume relevant key events until then.

  Example:
  \code
     QAccel *a = new QAccel( myWindow );	// create accels for myWindow
     a->connectItem( a->insertItem(Key_P+CTRL), // adds Ctrl+P accelerator
		     myWindow,			// connected to myWindow's
		     SLOT(printDoc()) );	// printDoc() slot
  \endcode

  \sa QKeyEvent QWidget::keyPressEvent() QMenuData::setAccel()
  QButton::setAccel()
  <a href="guibooks.html#fowler">GUI Design Handbook: Keyboard Shortcuts,</a>
*/


struct QAccelItem {				// internal accelerator item
    QAccelItem( int k, int i ) { key=k; id=i; enabled=TRUE; signal=0; }
   ~QAccelItem()	       { delete signal; }
    int		id;
    int		key;
    bool	enabled;
    QSignal    *signal;
};


typedef QList<QAccelItem> QAccelList; // internal accelerator list


class QAccelPrivate {
public:
    QAccelPrivate() { aitems.setAutoDelete( TRUE ); }
    ~QAccelPrivate() {}
    QAccelList aitems;
    bool enabled;
    QWidget *tlw;
};


// ### hack: we cast merrily.
static QAccelItem *find_id( QAccelList &list, int id )
{
    register QAccelItem *item = list.first();
    while ( item && item->id != id )
	item = list.next();
    return item;
}

static QAccelItem *find_key( QAccelList &list, int key, int ascii )
{
    register QAccelItem *item = list.first();
    while ( item ) {
	int k = item->key;
	if ( (k & Qt::ASCII_ACCEL) != 0 && (k & 0xff) == ascii ) {
	    break;
	} else {
	    if ( k == key )
		break;
	}
	item = list.next();
    }
    return item;
}


/*!
  Creates a QAccel object with a parent widget and a name.
*/

QAccel::QAccel( QWidget *parent, const char *name )
    : QObject( parent, name )
{
    d = new QAccelPrivate;
    d->enabled = TRUE;
    if ( parent ) {				// install event filter
#if defined(CHECK_RANGE)
	ASSERT( parent->isWidgetType() );
#endif
	d->tlw = parent->topLevelWidget();
	d->tlw->installEventFilter( this );
	connect( d->tlw, SIGNAL(destroyed()), SLOT(tlwDestroyed()) );
    } else {
	d->tlw = 0;
#if defined(CHECK_NULL)
	warning( "QAccel: An accelerator must have a parent widget" );
#endif
    }
}

/*!
  Destroys the accelerator object.
*/

QAccel::~QAccel()
{
    if ( d->tlw )
	d->tlw->removeEventFilter( this );
    delete (QAccelList *)d;
}


/*!
  \fn void QAccel::activated( int id )
  This signal is emitted when an accelerator key is pressed. \e id is
  a number that identifies this particular accelerator item.
*/

/*!
  Returns TRUE if the accelerator is enabled, or FALSE if it is disabled.
  \sa setEnabled(), isItemEnabled()
*/

bool QAccel::isEnabled() const
{
    return d->enabled;
}


/*!
  Enables the accelerator if \e enable is TRUE, or disables it if
  \e enable is FALSE.

  Individual keys can also be enabled or disabled.

  \sa isEnabled(), setItemEnabled()
*/

void QAccel::setEnabled( bool enable )
{
    d->enabled = enable;
}


/*!
  Returns the number of accelerator items.
*/

uint QAccel::count() const
{
    return d->aitems.count();
}


/*!
  Inserts an accelerator item and returns the item's identifier.

  \arg \e key is a key code plus a combination of SHIFT, CTRL and ALT.
  \arg \e id is the accelerator item id.

  If \e id is negative, then the item will be assigned a unique
  identifier.

  \code
    QAccel *a = new QAccel( myWindow );		// create accels for myWindow
    a->insertItem( Key_P + CTRL, 200 );		// Ctrl+P to print document
    a->insertItem( Key_X + ALT , 201 );		// Alt+X  to quit
    a->insertItem( ASCII_ACCEL + 'q', 202 );	// ASCII 'q' to quit
    a->insertItem( Key_D );			// gets id 3
    a->insertItem( Key_P + CTRL + SHIFT );	// gets id 4
  \endcode
*/

int QAccel::insertItem( int key, int id )
{
    if ( id == -1 )
	id = d->aitems.count();
    d->aitems.insert( 0, new QAccelItem(key,id) );
    return id;
}

/*!
  Removes the accelerator item with the identifier \e id.
*/

void QAccel::removeItem( int id )
{
    if ( find_id( d->aitems, id) )
	d->aitems.remove();
}


/*!
  Removes all accelerator items.
*/

void QAccel::clear()
{
    d->aitems.clear();
}


/*!
  Returns the key code of the accelerator item with the identifier \e id,
  or zero if the id cannot be found.
*/

int QAccel::key( int id )
{
    QAccelItem *item = find_id( d->aitems, id);
    return item ? item->key : 0;
}


/*!
  Returns the identifier of the accelerator item with the key code \e key, or
  -1 if the item cannot be found.
*/

int QAccel::findKey( int key ) const
{
    QAccelItem *item = find_key( d->aitems, key, key & 0xff );
    return item ? item->id : -1;
}


/*!
  Returns TRUE if the accelerator item with the identifier \e id is enabled.
  Returns FALSE if the item is disabled or cannot be found.
  \sa setItemEnabled(), isEnabled()
*/

bool QAccel::isItemEnabled( int id ) const
{
    QAccelItem *item = find_id( d->aitems, id);
    return item ? item->enabled : FALSE;
}


/*!
  Enables or disables an accelerator item.
  \arg \e id is the item identifier.
  \arg \e enable specifies whether the item should be enabled or disabled.

  \sa isItemEnabled(), isEnabled()
*/

void QAccel::setItemEnabled( int id, bool enable )
{
    QAccelItem *item = find_id( d->aitems, id);
    if ( item )
	item->enabled = enable;
}


/*!
  Connects an accelerator item to a slot/signal in another object.

  \arg \e id is the accelerator item id.
  \arg \e receiver is the object to receive a signal.
  \arg \e member is a slot or signal function in the receiver.

  \code
    a->connectItem( 201, mainView, SLOT(quit()) );
  \endcode

  \sa disconnectItem()
*/

bool QAccel::connectItem( int id, const QObject *receiver, const char *member )
{
    QAccelItem *item = find_id( d->aitems, id);
    if ( item ) {
	if ( !item->signal ) {
	    item->signal = new QSignal;
	    CHECK_PTR( item->signal );
	}
	return item->signal->connect( receiver, member );
    }
    return FALSE;
}

/*!
  Disconnects an accelerator item from a function in another
  object.
  \sa connectItem()
*/

bool QAccel::disconnectItem( int id, const QObject *receiver,
			     const char *member )
{
    QAccelItem *item = find_id( d->aitems, id);
    if ( item && item->signal )
	return item->signal->disconnect( receiver, member );
    return FALSE;
}


/*!
  Make sure that the accelerator is watching the correct event
  filter.  Used by QWidget::reparent().
*/

void QAccel::repairEventFilter()
{
    QWidget *ntlw;
    if ( parent() ) {
#if defined(CHECK_RANGE)
	ASSERT( parent()->isWidgetType() );
#endif
	ntlw = ((QWidget*)parent())->topLevelWidget();
    } else {
	ntlw = 0;
    }
    if ( d->tlw != ntlw ) {
	if ( d->tlw ) {
	    d->tlw->removeEventFilter( this );
	    disconnect( d->tlw, SIGNAL(destroyed()), this, SLOT(tlwDestroyed()) );
	}
	d->tlw = ntlw;
	if ( d->tlw ) {
	    d->tlw->installEventFilter( this );
	    connect( d->tlw, SIGNAL(destroyed()), this, SLOT(tlwDestroyed()) );
	}
    }
}


/*!
  Processes accelerator events intended for the top level widget.
*/

bool QAccel::eventFilter( QObject *, QEvent *e )
{
    if ( d->enabled &&
	 ( e->type() == QEvent::Accel || e->type() == QEvent::AccelAvailable) &&
	 parent() && parent()->isWidgetType() &&
	 ((QWidget *)parent())->isVisibleToTLW() ) {
	QKeyEvent *k = (QKeyEvent *)e;
	int key = k->key();
	if ( k->state() & ShiftButton )
	    key |= SHIFT;
	if ( k->state() & ControlButton )
	    key |= CTRL;
	if ( k->state() & AltButton )
	    key |= ALT;
	QAccelItem *item = find_key( d->aitems, key, k->ascii() );
	if ( item && item->enabled ) {
	    if (e->type() == QEvent::Accel) {
		if ( item->signal )
		    item->signal->activate();
		else
		    emit activated( item->id );
	    }
	    k->accept();
	    return TRUE;
	}
    }
    return FALSE;
}


/*!
  \internal
  This slot is called when the top level widget that owns the accelerator
  is destroyed.
*/

void QAccel::tlwDestroyed()
{
    d->tlw = 0;
}


/* \page shortcuts.html

<title>Standard Accelerators</title>
\postheader

<h1 align="center">Standard Accelerator Keys</h1>

Microsoft defines a large number of standard accelerators; the Open
Group defines a somewhat smaller number.  Here is a list of the ones
that involve letter keys, sorted alphabetically.  The boldfaced letter
(A in About) together with Alt is Microsoft's accelerator; where the
Open Group has a different standard we explain the difference in
parentheses.

<ul>
<li><b><u>A</u></b>bout
<li>Always on <b><u>T</u></b>op
<li><b><u>A</u></b>pply
<li><b><u>B</u></b>ack
<li><b><u>B</u></b>rowse
<li><b><u>C</u></b>lose (CDE: Alt-F4.  Alt-F4 is "close window" in Windows.)
<li><b><u>C</u></b>opy (CDE: Ctrl-C, Ctrl-Insert)
<li><b><u>C</u></b>opy Here
<li>Create <b><u>S</u></b>hortcut
<li>Create <b><u>S</u></b>hortcut Here
<li>Cu<b><u>t</u></b>
<li><b><u>D</u></b>elete
<li><b><u>E</u></b>dit
<li><b><u>E</u></b>xit
<li><b><u>E</u></b>xplore
<li><b><u>F</u></b>ile
<li><b><u>F</u></b>ind
<li><b><u>H</u></b>elp
<li>Help <b><u>T</u></b>opics
<li><b><u>H</u></b>ide
<li><b><u>I</u></b>nsert
<li>Insert <b><u>O</u></b>bject
<li><b><u>L</u></b>ink Here
<li>Ma<b><u>x</u></b>imize
<li>Mi<b><u>n</u></b>imize
<li><b><u>M</u></b>ove
<li><b><u>M</u></b>ove Here
<li><b><u>N</u></b>ew
<li><b><u>N</u></b>ext
<li><b><u>N</u></b>o
<li><b><u>O</u></b>pen
<li>Open <b><u>W</u></b>ith
<li>Page Set<b><u>u</u></b>p
<li><b><u>P</u></b>aste
<li>Paste <b><u>L</u></b>ink
<li>Paste <b><u>S</u></b>hortcut
<li>Paste <b><u>S</u></b>pecial
<li><b><u>P</u></b>ause
<li><b><u>P</u></b>lay
<li><b><u>P</u></b>rint
<li><b><u>P</u></b>rint Here
<li>P<b><u>r</u></b>operties
<li><b><u>Q</u></b>uick View
<li><b><u>R</u></b>edo (CDE: Ctrl-Y, Alt-Backspace)
<li><b><u>R</u></b>epeat
<li><b><u>R</u></b>estore
<li><b><u>R</u></b>esume
<li><b><u>R</u></b>etry
<li><b><u>R</u></b>un
<li><b><u>S</u></b>ave
<li>Save <b><u>A</u></b>s
<li>Select <b><u>A</u></b>ll
<li>Se<b><u>n</u></b>d To
<li><b><u>S</u></b>how
<li><b><u>S</u></b>ize
<li>S<b><u>p</u></b>lit
<li><b><u>S</u></b>top
<li><b><u>U</u></b>ndo (CDE says Ctrl-Z or Alt-Backspace)
<li><b><u>V</u></b>iew
<li><b><u>W</u></b>hat's This?
<li><b><u>W</u></b>indow
<li><b><u>Y</u></b>es
	</ul>

<a href="http://www.amazon.com/exec/obidos/ASIN/1556156790/trolltech/t">
<a href="http://www.amazon.com/exec/obidos/ASIN/1859121047/trolltech/t">

*/
