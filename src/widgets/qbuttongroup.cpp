/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qbuttongroup.cpp#69 $
**
** Implementation of QButtonGroup class
**
** Created : 950130
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

#include "qbuttongroup.h"
#include "qbutton.h"
#include "qlist.h"
#include "qradiobutton.h"
#include "qapplication.h"


/*!
  \class QButtonGroup qbuttongroup.h
  \brief The QButtonGroup widget organizes QButton widgets in a group.

  \ingroup realwidgets

  A button group widget makes it easier to deal with groups of
  buttons.  A button in a button group is associated with a unique
  identifer. The button group emits a clicked() signal with this
  identifier when the button is clicked. Thus, a button group is an
  ideal solution when you have several similar buttons and want to
  connect all their clicked() signals, for example, to one slot.

  An \link setExclusive() exclusive\endlink button group switches off
  all toggle buttons except the one that was clicked. A button group
  is by default non-exclusive. All \link QRadioButton radio
  buttons\endlink that are \link insert() inserted\endlink, will be
  mutually exclusive even if the button group is non-exclusive.

  There are two ways of using a button group:
  <ol>
  <li>The button group is a parent widget of a number of buttons,
  i.e. the button group is the parent argument in the button constructor.
  The buttons are assigned identifiers 0, 1, 2 etc. in the order they are
  created. A QButtonGroup can display a frame and a title because it inherits
  QGroupBox.
  <li>The button group is an invisible widget and the contained buttons
  have some other parent widget.  A button must then be manually inserted
  using the insert() function with an identifer.
  </ol>

  By default, the button group's setFont() and setPalette() functions
  do not change the appearance of the buttons, but you can use
  setFontPropagation() and setPalettePropagation() to change that.

  <img src=qbttngrp-m.png> <img src=qbttngrp-w.png>

  \sa QButton, QPushButton, QCheckBox, QRadioButton
*/


struct QButtonItem
{
    QButton *button;
    int	     id;
};

class QButtonList: public QList<QButtonItem>
{
public:
    QButtonList() {}
   ~QButtonList() {}
};


/*!
  Constructs a button group with no title.

  The \e parent and \e name arguments are passed to the QWidget constructor.
*/

QButtonGroup::QButtonGroup( QWidget *parent, const char *name )
    : QGroupBox( parent, name )
{
    init();
}

/*!
  Constructs a button group with a title.

  The \e parent and \e name arguments are passed to the QWidget constructor.
*/

QButtonGroup::QButtonGroup( const QString &title, QWidget *parent,
			    const char *name )
    : QGroupBox( title, parent, name )
{
    init();
}

/*!
  Constructs a button group with no title. Child widgets will be arranged
  in \a strips rows or columns (depending on \a orientation).

  The \e parent and \e name arguments are passed to the QWidget constructor.
*/

QButtonGroup::QButtonGroup( int strips, Orientation orientation,
			    QWidget *parent, const char *name )
    : QGroupBox( strips, orientation, parent, name )
{
    init();
}

/*!
  Constructs a button group with a \a title. Child widgets will be arranged
  in \a strips rows or columns (depending on \a orientation).

  The \e parent and \e name arguments are passed to the QWidget constructor.
*/

QButtonGroup::QButtonGroup( int strips, Orientation orientation,
			    const QString &title, QWidget *parent,
			    const char *name )
    : QGroupBox( strips, orientation, title, parent, name )
{
    init();
}

/*!
  Initializes the button group.
*/

void QButtonGroup::init()
{
    buttons = new QButtonList;
    CHECK_PTR( buttons );
    buttons->setAutoDelete( TRUE );
    excl_grp = FALSE;
    radio_excl = FALSE;
}

/*!
  Destroys the button group and its child widgets.
*/

QButtonGroup::~QButtonGroup()
{
    for ( register QButtonItem *bi=buttons->first(); bi; bi=buttons->next() )
	bi->button->setGroup(0);
    delete buttons;
}


/*!
  Returns TRUE if the button group is exclusive, otherwise FALSE.
  \sa setExclusive()
*/

bool QButtonGroup::isExclusive() const
{
    return excl_grp;
}

/*!
  Sets the button group to be exclusive if \e enable is TRUE,
  or to be non-exclusive if \e enable is FALSE.

  An exclusive button group switches off all other toggle buttons when
  one is switched on. This is ideal for groups of \link QRadioButton
  radio buttons\endlink. A non-exclusive group allow many buttons to be
  switched on at the same time.

  The default setting is FALSE.

  \sa isExclusive()
*/

void QButtonGroup::setExclusive( bool enable )
{
    excl_grp = enable;
}


/*!
  Inserts a button with the identifier \e id into the button group.
  Returns the button identifier.

  It is not necessary to manually insert buttons that have this button
  group as their parent widget. An exception is when you want custom
  identifiers instead of the default 0, 1, 2 etc.

  The button is assigned the identifier \e id or an automatically
  generated identifier.	 It works as follows: If \e id >= 0, this
  identifier is assigned.  If \e id == -1 (default), the identifier is
  equal to the number of buttons in the group.	If \e id is any other
  negative integer, for instance -2, a unique identifier (negative
  integer \<= -2) is generated.

  Inserting several buttons with \e id = -1 assigns the identifiers 0,
  1, 2, etc.

  \sa find(), remove(), setExclusive()
*/

int QButtonGroup::insert( QButton *button, int id )
{
    if ( button->group() )
	button->group()->remove( button );
    static int seq_no = -2;
    register QButtonItem *bi = new QButtonItem;
    CHECK_PTR( bi );
    if ( id < -1 )
	bi->id = seq_no--;
    else if ( id == -1 )
	bi->id = buttons->count();
    else
	bi->id = id;
    bi->button = button;
    button->setGroup(this);
    buttons->append( bi );
    connect( button, SIGNAL(pressed()) , SLOT(buttonPressed()) );
    connect( button, SIGNAL(released()), SLOT(buttonReleased()) );
    connect( button, SIGNAL(clicked()) , SLOT(buttonClicked()) );
    connect( button, SIGNAL(toggled(bool)) , SLOT(buttonToggled(bool)) );
    if ( button->inherits( "QRadioButton" ) )
	setRadioButtonExclusive( TRUE );
    return bi->id;
}

/*!
  Returns the number of buttons in the group.
*/
int QButtonGroup::count() const
{
    return buttons->count();
}

/*!
  Removes a button from the button group.
  \sa insert()
*/

void QButtonGroup::remove( QButton *button )
{
    for ( QButtonItem *i=buttons->first(); i; i=buttons->next() ) {
	if ( i->button == button ) {
	    buttons->remove();
	    button->setGroup(0);
	    button->disconnect( this );
	    button->removeEventFilter( this );
	    break;
	}
    }
}


/*!
  Finds and returns a pointer to the button with the specified identifier
  \e id.

  Returns null if the button was not found.
*/

QButton *QButtonGroup::find( int id ) const
{
    for ( QButtonItem *i=buttons->first(); i; i=buttons->next() )
	if ( i->id == id )
	    return i->button;
    return 0;
}


/*!
  \fn void QButtonGroup::pressed( int id )
  This signal is emitted when a button in the group is
  \link QButton::pressed() pressed\endlink.
  The \e id argument is the button's identifier.
*/

/*!
  \fn void QButtonGroup::released( int id )
  This signal is emitted when a button in the group is
  \link QButton::released() released\endlink.
  The \e id argument is the button's identifier.
*/

/*!
  \fn void QButtonGroup::clicked( int id )
  This signal is emitted when a button in the group is
  \link QButton::clicked() clicked\endlink.
  The \e id argument is the button's identifier.
*/


/*!
  \internal
  This slot is activated when one of the buttons in the group emits the
  QButton::pressed() signal.
*/

void QButtonGroup::buttonPressed()
{
    int id = -1;
    QButton *bt = (QButton *)sender();		// object that sent the signal
    for ( register QButtonItem *i=buttons->first(); i; i=buttons->next() )
	if ( bt == i->button ) {		// button was clicked
	    id = i->id;
	    break;
	}
    if ( id != -1 )
	emit pressed( id );
}

/*!
  \internal
  This slot is activated when one of the buttons in the group emits the
  QButton::released() signal.
*/

void QButtonGroup::buttonReleased()
{
    int id = -1;
    QButton *bt = (QButton *)sender();		// object that sent the signal
    for ( register QButtonItem *i=buttons->first(); i; i=buttons->next() )
	if ( bt == i->button ) {		// button was clicked
	    id = i->id;
	    break;
	}
    if ( id != -1 )
	emit released( id );
}

/*!
  \internal
  This slot is activated when one of the buttons in the group emits the
  QButton::clicked() signal.
*/

void QButtonGroup::buttonClicked()
{
    int id = -1;
    QButton *bt = (QButton *)sender();		// object that sent the signal
#if defined(CHECK_NULL)
    ASSERT( bt->inherits("QButton") );
#endif
    for ( register QButtonItem *i=buttons->first(); i; i=buttons->next() ) {
	if ( bt == i->button ) {			// button was clicked
	    id = i->id;
	    break;
	}
    }
    if ( id != -1 )
	emit clicked( id );
}


/*!
  \internal
  This slot is activated when one of the buttons in the group emits the
  QButton::toggled() signal.
*/

void QButtonGroup::buttonToggled( bool on )
{
    if ( !on || !excl_grp && !radio_excl )
	return;
    QButton *bt = (QButton *)sender();		// object that sent the signal
#if defined(CHECK_NULL)
    ASSERT( bt->inherits("QButton") );
    ASSERT( bt->isToggleButton() );
#endif

    if ( !excl_grp && !bt->inherits("QRadioButton") )
	return;
    QButtonItem *i = buttons->first();
    while( i ) {
	if ( bt != i->button &&
	     i->button->isToggleButton() &&
	     i->button->isOn() &&
	     (excl_grp || i->button->inherits("QRadioButton") ) ) {
	    i->button->setOn( FALSE );
	    if ( i->button->hasFocus() )
		bt->setFocus();
	}
	i = buttons->next();
    }
}



/*!  Sets the button with id \a id to be on, and if this is an
  exclusive group, all other button in the group to be off.
*/

void QButtonGroup::setButton( int id )
{
    QButton * b = find( id );
    if ( b )
	b->setOn( TRUE );
}


/*!
  \fn bool QButtonGroup::isRadioButtonExclusive () const

  Returns whether this button group will treat radio buttons as
  mutually exclusive.

  \sa setRadioButtonExclusive()
*/

/*!
  If \a on is TRUE, this button group will treat radio buttons as
  mutually exclusive, and other buttons according to
  isExclusive(). This function is called automatically whenever a
  QRadioButton is inserted, so you should normally never have to call
  it.
*/

void QButtonGroup::setRadioButtonExclusive( bool on)
{
    radio_excl = on;
}


/*!  Moves the keyboard focus according to \a key, and if appropriate
  checks the new focus item.

  This function does nothing unless the keyboard focus points to one
  of the button group members and \a key is one of \c Key_Up, \c
  Key_Down, \c Key_Left and \c Key_Right.
*/

void QButtonGroup::moveFocus( int key )
{
    QWidget * f = qApp->focusWidget();

    QButtonItem * i;
    i = buttons->first();
    while( i && i->button != f )
	i = buttons->next();

    if ( !i || !i->button )
	return;

    QWidget * candidate = 0;
    int bestScore = -1;

    QPoint goal( f->mapToGlobal( f->geometry().center() ) );

    i = buttons->first();
    while( i && i->button ) {
	if ( i->button != f &&
	     i->button->focusPolicy() &&
	     i->button->isEnabled() ) {
	    QPoint p(i->button->mapToGlobal(i->button->geometry().center()));
	    int score = (p.y() - goal.y())*(p.y() - goal.y()) +
			(p.x() - goal.x())*(p.x() - goal.x());
	    switch( key ) {
	    case Key_Up:
		if ( p.y() < goal.y() &&
		     QABS( p.x() - goal.x() ) < QABS( p.y() - goal.y() ) &&
		     ( score < bestScore || !candidate ) ) {
		    candidate = i->button;
		    bestScore = score;
		}
		break;
	    case Key_Down:
		if ( p.y() > goal.y() &&
		     QABS( p.x() - goal.x() ) < QABS( p.y() - goal.y() ) &&
		     ( score < bestScore || !candidate ) ) {
		    candidate = i->button;
		    bestScore = score;
		}
		break;
	    case Key_Left:
		if ( p.x() < goal.x() &&
		     QABS( p.y() - goal.y() ) < QABS( p.x() - goal.x() ) &&
		     ( score < bestScore || !candidate ) ) {
		    candidate = i->button;
		    bestScore = score;
		}
		break;
	    case Key_Right:
		if ( p.x() > goal.x() &&
		     QABS( p.y() - goal.y() ) < QABS( p.x() - goal.x() ) &&
		     ( score < bestScore || !candidate ) ) {
		    candidate = i->button;
		    bestScore = score;
		}
		break;
	    }
	}
	i = buttons->next();
    }

    if ( candidate && f && f->inherits( "QRadioButton" ) &&
	 ((QRadioButton*)f)->isChecked() &&
	 candidate->inherits( "QRadioButton" ) ) {
	((QRadioButton*)candidate)->setChecked( TRUE );
	((QButton*)candidate)->animateClick();
	((QButton*)candidate)->animateTimeout(); // ### crude l&f hack
    }

    if ( candidate )
	candidate->setFocus();
}


/*!  Returns a pointer to the selected radio button in this group, if
  one exists, or 0 if there is no selected radio button in this group.
*/

QButton * QButtonGroup::selected()
{
    QButtonItem *i = buttons->first();
    while ( i && !( i->button && i->button->inherits("QRadioButton") &&
		    i->button->isToggleButton() && i->button->isOn() ) )
	i = buttons->next();
    return i ? i->button : 0;
}


/*! Returns the id of \a button, or -1 if \a button is not a member of
  this group.
*/

int QButtonGroup::id( QButton * button ) const
{
    QButtonItem *i = buttons->first();
    while ( i && i->button != button )
	i = buttons->next();
    return i ? i->id : -1;
}
