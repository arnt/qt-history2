/****************************************************************************
** $Id: $
**
** Implementation of QAccel class
**
** Created : 950419
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qaccel.h"

#ifndef QT_NO_ACCEL

#include "qsignal.h"
#include "qapplication.h"
#include "qwidget.h"
#include "qptrlist.h"
#include "qwhatsthis.h"
#include "qguardedptr.h"

/*!
  \class QAccel qaccel.h
  \brief The QAccel class handles keyboard accelerator and shortcut keys.

  \ingroup misc

  A keyboard accelerator triggers an action when a certain key
  combination is pressed. The accelerator handles all keyboard
  activity for all children of one top-level widget, so it is not
  affected by the keyboard focus.

  In most cases, you will not need to use this class directly. Use the
  QAction class to create actions with accelerators that can be used in
  both menus and toolbars. If you're only interested in menus use
  QMenuData::insertItem() or QMenuData::setAccel() to make accelerators
  for operations that are also available on menus.  Many widgets
  automatically generate accelerators, such as QButton, QGroupBox,
  QLabel (with QLabel::setBuddy()), QMenuBar and QTabBar.
  Example:
  \code
     QPushButton p( "&Exit", parent ); //automatic shortcut ALT+Key_E
     QPopupMenu *fileMenu = new fileMenu( parent );
     fileMenu->insertItem( "Undo", parent, SLOT(undo()), CTRL+Key_Z );
  \endcode

  A QAccel contains a list of accelerator items that can be
  manipulated using insertItem(), removeItem(), clear(),
  key() and findKey().

  Each accelerator item consists of an identifier and a \l
  QKeySequence. A single key sequence consists of a keyboard code
  combined with modifiers (\c SHIFT, \c CTRL, \c ALT or \c
  UNICODE_ACCEL).  For example, <code>CTRL + Key_P</code> could be a
  shortcut for printing a document. The key codes are listed in
  qnamespace.h. As an alternative, use \c UNICODE_ACCEL with the
  unicode code point of the character. For example,
  <code>UNICODE_ACCEL + 'A'</code> gives the same accelerator as \c
  Key_A.

  When an accelerator key is pressed, the accelerator sends out the
  signal activated() with a number that identifies this particular
  accelerator item.  Accelerator items can also be individually
  connected, so that two different keys will activate two different
  slots (see connectItem() and disconnectItem()).

  Use setEnabled() to enable/disable all items in the accelerator, or
  setItemEnabled() to enable/disable individual items.  An item is
  active only when the QAccel is enabled and the item itself is.

  The function setWhatsThis() specifies a help text that appears when
  the user presses an accelerator key in What's This mode.

  A QAccel object handles key events to the QWidget::topLevelWidget()
  containing \e parent, and hence to any child widgets of that window.
  The accelerator will be deleted when \e parent is deleted, and will
  consume relevant key events until then.

  Example:
  \code
     QAccel *a = new QAccel( myWindow );	// create accels for myWindow
     a->connectItem( a->insertItem(Key_P+CTRL), // adds Ctrl+P accelerator
		     myWindow,			// connected to myWindow's
		     SLOT(printDoc()) );	// printDoc() slot
  \endcode

  \sa QKeyEvent QWidget::keyPressEvent() QMenuData::setAccel()
  QButton::setAccel() QLabel::setBuddy()
  \link guibooks.html#fowler GUI Design Handbook: Keyboard Shortcuts \endlink.
*/


struct QAccelItem {				// internal accelerator item
    QAccelItem( int k, int i )
	{ key=k; id=i; enabled=TRUE; signal=0; }
   ~QAccelItem()	       { delete signal; }
    int		id;
    int		key;
    bool	enabled;
    QSignal    *signal;
    QString whatsthis;
};


typedef QPtrList<QAccelItem> QAccelList; // internal accelerator list


class QAccelPrivate {
public:
    QAccelPrivate() { aitems.setAutoDelete( TRUE ); ignorewhatsthis = FALSE; }
    ~QAccelPrivate() {}
    QAccelList aitems;
    bool enabled;
    QGuardedPtr<QWidget> tlw;
    QGuardedPtr<QWidget> watch;
    bool ignorewhatsthis;
};


static QAccelItem *find_id( QAccelList &list, int id )
{
    register QAccelItem *item = list.first();
    while ( item && item->id != id )
	item = list.next();
    return item;
}

static QAccelItem *find_key( QAccelList &list, int key, QChar ch )
{
    register QAccelItem *item = list.first();
    while ( item ) {
	int k = item->key;
	int km = k & Qt::MODIFIER_MASK;
	QChar kc = QChar(k & 0xffff);
	if ( k & Qt::UNICODE_ACCEL )
	{
	    if ( km ) {
		// Modifiers must match...
		QChar c;
		if ( (key & Qt::CTRL) && (ch < ' ') )
		    c = ch.unicode()+'@'+' '; // Ctrl+A is ASCII 001, etc.
		else
		    c = ch;
		if ( kc.lower() == c.lower()
		  && (key & Qt::MODIFIER_MASK) == km )
		    break;
		else if ( kc.lower() == c.lower()
		    && (key & (Qt::MODIFIER_MASK^Qt::SHIFT)) == km )
		    break;
	    } else {
		// No modifiers requested, ignore Shift but require others...
		if ( kc == ch
		  && (key & (Qt::MODIFIER_MASK^Qt::SHIFT)) == km )
		    break;
	    }
	} else if ( k == key ) {
	    break;
	}
	item = list.next();
    }
    return item;
}


/*!
  Constructs a QAccel object with parent \a parent and name \a name. The
  accelerator operates on \a parent.
*/

QAccel::QAccel( QWidget *parent, const char *name )
    : QObject( parent, name )
{
    d = new QAccelPrivate;
    d->enabled = TRUE;
    d->watch = parent;
    if ( d->watch ) {				// install event filter
	d->watch->installEventFilter( this );
	d->tlw = d->watch->topLevelWidget();
	if ( d->tlw != d->watch )
	    d->tlw->installEventFilter( this );
    } else {
#if defined(QT_CHECK_NULL)
	qWarning( "QAccel: An accelerator must have a parent or a watch widget" );
#endif
    }
}

/*!
  Constructs a QAccel object that operates on \a watch, but is a child of
  \a parent. The object is called \a name.

  This constructor is not needed for normal application programming.
*/
QAccel::QAccel( QWidget* watch, QObject *parent, const char *name )
    : QObject( parent, name )
{
    d = new QAccelPrivate;
    d->enabled = TRUE;
    d->watch = watch;
    if ( watch ) {				// install event filter
	d->watch->installEventFilter( this );
	d->tlw = d->watch->topLevelWidget();
	if ( d->tlw != d->watch )
	    d->tlw->installEventFilter( this );
    } else {
#if defined(QT_CHECK_NULL)
	qWarning( "QAccel: An accelerator must have a parent or a watch widget" );
#endif
    }
}

/*!
  Destroys the accelerator object and frees all allocated resources.
*/

QAccel::~QAccel()
{
    delete d;
}


/*!
  \fn void QAccel::activated( int id )

  This signal is emitted when an accelerator key is pressed. \a id is
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
  Enables the accelerator if \a enable is TRUE, or disables it if
  \a enable is FALSE.

  Individual keys can also be enabled or disabled using
  setItemEnabled().  To work, a key must be an enabled item in an
  enabled QAccel.

  \sa isEnabled(), setItemEnabled()
*/

void QAccel::setEnabled( bool enable )
{
    d->enabled = enable;
}


/*!
  Returns the number of accelerator items in this accelerator.
*/

uint QAccel::count() const
{
    return d->aitems.count();
}


static int get_seq_id()
{
    static int seq_no = -2;  // -1 is used as return value in findKey()
    return seq_no--;
}

/*!
  Inserts an accelerator item and returns the item's identifier.

  \a key is a key code plus a combination of SHIFT, CTRL and ALT.
  \a id is the accelerator item id.

  If \a id is negative, then the item will be assigned a unique
  negative identifier less than -1.

  \code
    QAccel *a = new QAccel( myWindow );		// create accels for myWindow
    a->insertItem( Key_P + CTRL, 200 );		// Ctrl+P to print document
    a->insertItem( Key_X + ALT , 201 );		// Alt+X  to quit
    a->insertItem( UNICODE_ACCEL + 'q', 202 );	// Unicode 'q' to quit
    a->insertItem( Key_D );			// gets a unique negative id < -1
    a->insertItem( Key_P + CTRL + SHIFT );	// gets a unique negative id < -1
  \endcode
*/

int QAccel::insertItem( const QKeySequence& key, int id )
{
    if ( id == -1 )
	id = get_seq_id();
    d->aitems.insert( 0, new QAccelItem(key,id) );
    return id;
}

/*!
  Removes the accelerator item with the identifier \a id.
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
  Returns the key code of the accelerator item with the identifier \a id,
  or zero if the id cannot be found.
*/

QKeySequence QAccel::key( int id )
{
    QAccelItem *item = find_id( d->aitems, id);
    return QKeySequence( item ? item->key : 0 );
}


/*!
  Returns the identifier of the accelerator item with the key code \a key, or
  -1 if the item cannot be found.
*/

int QAccel::findKey( const QKeySequence& key ) const
{
    QAccelItem *item = find_key( d->aitems, key, QChar(key & 0xffff) );
    return item ? item->id : -1;
}


/*!
  Returns TRUE if the accelerator item with the identifier \a id is enabled.
  Returns FALSE if the item is disabled or cannot be found.
  \sa setItemEnabled(), isEnabled()
*/

bool QAccel::isItemEnabled( int id ) const
{
    QAccelItem *item = find_id( d->aitems, id);
    return item ? item->enabled : FALSE;
}


/*!
  Enables the accelerator item with the identifier \a id if \a enable is
  TRUE, and disables \a id if \a enable is FALSE.

  To work, an item must be enabled and be in an enabled QAccel.

  \sa isItemEnabled(), isEnabled()
*/

void QAccel::setItemEnabled( int id, bool enable )
{
    QAccelItem *item = find_id( d->aitems, id);
    if ( item )
	item->enabled = enable;
}


/*!
  Connects the accelerator item \a id to the slot \a member of \a receiver.

  \code
    a->connectItem( 201, mainView, SLOT(quit()) );
  \endcode

  Of course, you can also send a signal as \a member.

  \sa disconnectItem()
*/

bool QAccel::connectItem( int id, const QObject *receiver, const char *member )
{
    QAccelItem *item = find_id( d->aitems, id);
    if ( item ) {
	if ( !item->signal ) {
	    item->signal = new QSignal;
	    Q_CHECK_PTR( item->signal );
	}
	return item->signal->connect( receiver, member );
    }
    return FALSE;
}

/*!
  Disconnects an accelerator item with id \a id from the function
  called \a member in the \a receiver object.

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


/*!  Makes sure that the accelerator is watching the correct event filter.
  This function is called automatically; you should not need to call it in
  application code.
*/

void QAccel::repairEventFilter()
{
    QWidget *ntlw;
    if ( d->watch )
	ntlw = d->watch->topLevelWidget();
    else
	ntlw = 0;

    if ( (QWidget*) d->tlw != ntlw ) {
	if ( d->tlw )
	    d->tlw->removeEventFilter( this );
	d->tlw = ntlw;
	if ( d->tlw )
	    d->tlw->installEventFilter( this );
    }
}


/*!
  Processes accelerator events intended for the top level widget.
  \a e is the event that occurred on object \a o.
*/

bool QAccel::eventFilter( QObject *o, QEvent *e )
{
    if ( e->type() == QEvent::Reparent && d->watch == o ) {
	repairEventFilter();
    } else  if ( d->enabled &&
	 ( e->type() == QEvent::Accel ||
	   e->type() == QEvent::AccelAvailable) &&
	 d->watch && d->watch->isVisible() ) {
	QKeyEvent *k = (QKeyEvent *)e;
	int key = k->key();
	if ( k->state() & ShiftButton )
	    key |= SHIFT;
	if ( k->state() & ControlButton )
	    key |= CTRL;
	if ( k->state() & AltButton )
	    key |= ALT;
	QAccelItem *item = find_key( d->aitems, key, k->text()[0] );
	if ( key == Key_unknown )
	    item = 0;
#ifndef QT_NO_WHATSTHIS
	bool b = QWhatsThis::inWhatsThisMode();
#else
	bool b = FALSE;
#endif
	if ( item && ( item->enabled || b )) {
	    if (e->type() == QEvent::Accel) {
		if ( b && !d->ignorewhatsthis ) {
#ifndef QT_NO_WHATSTHIS
		    QWhatsThis::leaveWhatsThisMode( item->whatsthis );
#endif
		}
		else if ( item->enabled ){
		    if ( item->signal )
			item->signal->activate();
		    else
			emit activated( item->id );
		}
	    }
	    k->accept();
	    return TRUE;
	}
    }
    return QObject::eventFilter( o, e );
}



/*!
  Returns the shortcut key for \a str, or 0 if \a str has no
  shortcut sequence.

  For example, shortcutKey("E&amp;xit") returns ALT+Key_X,
  shortcutKey("&amp;Exit") returns ALT+Key_E and shortcutKey("Exit")
  returns 0.  (In code that does not inherit the Qt namespace class,
  you need to write e.g. Qt::ALT+Qt::Key_X.)

  We provide a \link accelerators.html list of common accelerators
  \endlink in English.  At the time of this writing, Microsoft and The
  Open Group do not appear to have issued equivalent recommendations for
  other languages.
*/

QKeySequence QAccel::shortcutKey( const QString &str )
{
    int p = 0;
    while ( p >= 0 ) {
	p = str.find( '&', p ) + 1;
	if ( p <= 0 || p >= (int)str.length() )
	    return 0;
	if ( str[p] != '&' ) {
	    QChar c = str[p];
	    if ( c.isPrint() ) {
		c = c.upper();
		return QKeySequence( c.unicode() + ALT + UNICODE_ACCEL );
	    }
	}
	p++;
    }
    return QKeySequence(0);
}


#ifdef Q_WS_MAC
#define QMAC_CTRL (QString(QChar(0x2318)))
#define QMAC_ALT  (QString(QChar(0x2325)))
#endif

/*! \obsolete

   Creates an accelerator string for the key \a k.
   For instance CTRL+Key_O gives "Ctrl+O".  The "Ctrl" etc.
   are translated (using QObject::tr()) in the "QAccel" scope.

   The function is superfluous.  Cast the QKeySequence \a k to a
   QString for the same effect.

   \sa stringToKey()
*/
QString QAccel::keyToString( QKeySequence k )
{
    return (QString) k;
}

/*!\obsolete

  Returns an accelerator code for the string \a s.  For example
  "Ctrl+O" gives CTRL+UNICODE_ACCEL+'O'.  The strings "Ctrl",
  "Shift", "Alt" are recognized, as well as their translated
  equivalents in the "QAccel" scope (using QObject::tr()). Returns 0
  if \a s is not recognized.

  This function is typically used with \link QObject::tr() tr
  \endlink(), so that accelerator keys can be replaced in
  translations:

  \code
    QPopupMenu *file = new QPopupMenu( this );
    file->insertItem( p1, tr("&Open..."), this, SLOT(open()),
		      QAccel::stringToKey(tr("Ctrl+O", "File|Open")) );
  \endcode

  Notice the \c "File|Open" translator comment. It is by no means
  necessary, but it provides some context for the human translator.


  The function is superfluous.  Construct a QKeySequence from the
  string \a s for the same effect.

  \sa QObject::tr()
      \link i18n.html Internationalization with Qt \endlink
*/
QKeySequence QAccel::stringToKey( const QString & s )
{
    return QKeySequence( s );
}


/*!
  Sets a What's This help for the accelerator item \a id to \a text.

  The text will be shown when the application is in What's This mode
  and the user hits the accelerator key.

  To set What's This help on a menu item (with or without an
  accelerator key), use QMenuData::setWhatsThis().

  \sa whatsThis(), QWhatsThis::inWhatsThisMode(), QMenuData::setWhatsThis(),
      QAction::setWhatsThis()
*/
void QAccel::setWhatsThis( int id, const QString& text )
{

    QAccelItem *item = find_id( d->aitems, id);
    if ( item )
	item->whatsthis = text;
}

/*!
  Returns the What's This help text for the specified item \a id or
  QString::null if no text has been defined yet.

  \sa setWhatsThis()
 */
QString QAccel::whatsThis( int id ) const
{

    QAccelItem *item = find_id( d->aitems, id);
    return item? item->whatsthis : QString::null;
}

/*!\internal */
void QAccel::setIgnoreWhatsThis( bool b)
{
    d->ignorewhatsthis = b;
}

/*!\internal */
bool QAccel::ignoreWhatsThis() const
{
    return d->ignorewhatsthis;
}


/*! \page accelerators.html

\title Standard Accelerator Keys

Applications invariably need to define accelerator keys for actions,
and Qt provides functions to help with that, most importantly \l
QAccel::shortcutKey().

Here is Microsoft's recommendations for accelerator key choice, with
comments about the Open Group's recommendations where they exist and
differ.  For most commands, the Open Group either has no advice or
agrees with Microsoft.

The boldfaced letter plus Alt is Microsoft's recommended choice, and
we recommend supporting it.  For an Apply button, for example, we
recommend QButton::setText( \link QWidget::tr() tr \endlink("&Apply") );

If you have conflicting commands (e.g. About and Apply buttons in the
same dialog), you're on your own.

\list
\i <b><u>A</u></b>bout
\i Always on <b><u>T</u></b>op
\i <b><u>A</u></b>pply
\i <b><u>B</u></b>ack
\i <b><u>B</u></b>rowse
\i <b><u>C</u></b>lose (CDE: Alt+F4; Alt+F4 is "close window" in Windows)
\i <b><u>C</u></b>opy (CDE: Ctrl+C, Ctrl+Insert)
\i <b><u>C</u></b>opy Here
\i Create <b><u>S</u></b>hortcut
\i Create <b><u>S</u></b>hortcut Here
\i Cu<b><u>t</u></b>
\i <b><u>D</u></b>elete
\i <b><u>E</u></b>dit
\i <b><u>E</u></b>xit
\i <b><u>E</u></b>xplore
\i <b><u>F</u></b>ile
\i <b><u>F</u></b>ind
\i <b><u>H</u></b>elp
\i Help <b><u>T</u></b>opics
\i <b><u>H</u></b>ide
\i <b><u>I</u></b>nsert
\i Insert <b><u>O</u></b>bject
\i <b><u>L</u></b>ink Here
\i Ma<b><u>x</u></b>imize
\i Mi<b><u>n</u></b>imize
\i <b><u>M</u></b>ove
\i <b><u>M</u></b>ove Here
\i <b><u>N</u></b>ew
\i <b><u>N</u></b>ext
\i <b><u>N</u></b>o
\i <b><u>O</u></b>pen
\i Open <b><u>W</u></b>ith
\i Page Set<b><u>u</u></b>p
\i <b><u>P</u></b>aste
\i Paste <b><u>L</u></b>ink
\i Paste <b><u>S</u></b>hortcut
\i Paste <b><u>S</u></b>pecial
\i <b><u>P</u></b>ause
\i <b><u>P</u></b>lay
\i <b><u>P</u></b>rint
\i <b><u>P</u></b>rint Here
\i P<b><u>r</u></b>operties
\i <b><u>Q</u></b>uick View
\i <b><u>R</u></b>edo (CDE: Ctrl+Y, Alt+Backspace)
\i <b><u>R</u></b>epeat
\i <b><u>R</u></b>estore
\i <b><u>R</u></b>esume
\i <b><u>R</u></b>etry
\i <b><u>R</u></b>un
\i <b><u>S</u></b>ave
\i Save <b><u>A</u></b>s
\i Select <b><u>A</u></b>ll
\i Se<b><u>n</u></b>d To
\i <b><u>S</u></b>how
\i <b><u>S</u></b>ize
\i S<b><u>p</u></b>lit
\i <b><u>S</u></b>top
\i <b><u>U</u></b>ndo (CDE: Ctrl+Z or Alt+Backspace)
\i <b><u>V</u></b>iew
\i <b><u>W</u></b>hat's This?
\i <b><u>W</u></b>indow
\i <b><u>Y</u></b>es
\endlist

There are also a lot of other keys and actions (that use other
modifier keys than Alt).  See the Microsoft and Open Group
documentation for details.

The \link http://www.amazon.com/exec/obidos/ASIN/0735605661/trolltech/t
Microsoft book \endlink has ISBN 0735605661.  The corresponding Open Group
book is very hard to find, rather expensive and we cannot recommend
it. However, if you really want it, OGPubs@opengroup.org might be able
to help. Ask then for ISBN 1859121047.

*/

#endif // QT_NO_ACCEL
