/****************************************************************************
** $Id: $
**
** Implementation of QButtonGroup class
**
** Created : 950130
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
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

#include "qbuttongroup.h"
#ifndef QT_NO_BUTTONGROUP
#include "qbutton.h"
#include "qptrlist.h"
#include "qapplication.h"



/*!
  \class QButtonGroup qbuttongroup.h
  \brief The QButtonGroup widget organizes QButton widgets in a group.

  \ingroup organizers
  \ingroup geomanagement
  \ingroup appearance
  \mainclass

  A button group widget makes it easier to deal with groups of
  buttons.  Each button in a button group has a unique
  identifier. The button group emits a clicked() signal with this
  identifier when the button is clicked. This makes a button group
  particularly useful when you have several similar buttons and want to
  connect all their clicked() signals to one slot.

  An \link setExclusive() exclusive\endlink button group switches off
  all toggle buttons except the one that was clicked. A button group
  is by default non-exclusive. By default, all radio
  buttons that are inserted will be
  mutually exclusive even if the button group is non-exclusive. (See
  setRadioButtonExclusive().)

  There are two ways of using a button group:
  \list
  \i The button group is a parent widget of a number of buttons,
  i.e., the button group is the parent argument in the button constructor.
  The buttons are assigned identifiers 0, 1, 2, etc. in the order they are
  created. A QButtonGroup can display a frame and a title because it inherits
  QGroupBox.
  \i The button group is an invisible widget and the contained buttons
  have some other parent widget.  A button must then be manually inserted
  using the insert() function with an identifier.
  \endlist

  A button can be removed from the group with remove(). A pointer to a
  button with a given id can be obtained using find(). The id of a
  button is available using id(). A button can be set on with
  setButton(). The number of buttons in the group is returned by
  count().

  <img src=qbttngrp-m.png> <img src=qbttngrp-w.png>

  \sa QButton, QPushButton, QCheckBox, QRadioButton
*/

/*! \property QButtonGroup::exclusive
    \brief whether the button group is exclusive

  If this property is TRUE, then the buttons in the group are
  toggled, and to untoggle a button you must click on another button in
  the group.
*/

/*! \property QButtonGroup::radioButtonExclusive
    \brief whether the radiobuttons in the group are exclusive

  If this property is TRUE, the radiobuttons in the group are treated
  exclusively.
*/

struct QButtonItem
{
    QButton *button;
    int	     id;
};


class QButtonList: public QPtrList<QButtonItem>
{
public:
    QButtonList() {}
   ~QButtonList() {}
};


typedef QPtrListIterator<QButtonItem> QButtonListIt;


/*!
  Constructs a button group with no title.

  The \a parent and \a name arguments are passed to the QWidget constructor.
*/

QButtonGroup::QButtonGroup( QWidget *parent, const char *name )
    : QGroupBox( parent, name )
{
    init();
}

/*!
  Constructs a button group with the title \a title.

  The \a parent and \a name arguments are passed to the QWidget constructor.
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

  The \a parent and \a name arguments are passed to the QWidget constructor.
*/

QButtonGroup::QButtonGroup( int strips, Orientation orientation,
			    QWidget *parent, const char *name )
    : QGroupBox( strips, orientation, parent, name )
{
    init();
}

/*!
  Constructs a button group with title \a title. Child widgets will be
  arranged in \a strips rows or columns (depending on \a orientation).

  The \a parent and \a name arguments are passed to the QWidget constructor.
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
    Q_CHECK_PTR( buttons );
    buttons->setAutoDelete( TRUE );
    excl_grp = FALSE;
    radio_excl = TRUE;
}

/*! \reimp */

QButtonGroup::~QButtonGroup()
{
    QButtonList * tmp = buttons;
    QButtonItem *bi = tmp->first();
    buttons = 0;
    while( bi ) {
	bi->button->setGroup(0);
	bi = tmp->next();
    }
    delete tmp;
}

bool QButtonGroup::isExclusive() const
{
    return excl_grp;
}

void QButtonGroup::setExclusive( bool enable )
{
    excl_grp = enable;
}


/*!
  Inserts the \a button with the identifier \a id into the button group.
  Returns the button identifier.

    Buttons are normally inserted into a button group automatically by
    giving the button group as the parent when the button is
    constructed. So it is not necessary to manually insert buttons that
    have this button group as their parent widget. An exception is when
    you want custom identifiers instead of the default 0, 1, 2, etc.

  The button is assigned the identifier \a id or an automatically
  generated identifier.	 It works as follows: If \a id >= 0, this
  identifier is assigned.  If \a id == -1 (default), the identifier is
  equal to the number of buttons in the group.	If \a id is any other
  negative integer, for instance -2, a unique identifier (negative
  integer \<= -2) is generated. No button has an id of -1.

  Inserting several buttons with \a id = -1 assigns the identifiers 0,
  1, 2, etc.

  \sa find(), remove(), setExclusive()
*/

int QButtonGroup::insert( QButton *button, int id )
{
    if ( button->group() )
	button->group()->remove( button );

    static int seq_no = -2;
    QButtonItem *bi = new QButtonItem;
    Q_CHECK_PTR( bi );

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

    if ( button->isToggleButton() && !button->isOn() &&
	 selected() && (selected()->focusPolicy() & TabFocus) != 0 )
	button->setFocusPolicy( (FocusPolicy)(button->focusPolicy() &
					      ~TabFocus) );

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
  Removes the \a button from the button group.
  \sa insert()
*/

void QButtonGroup::remove( QButton *button )
{
    if ( !buttons )
	return;

    QButtonListIt it( *buttons );
    QButtonItem *i;
    while ( (i=it.current()) != 0 ) {
	++it;
	if ( i->button == button ) {
	    buttons->remove( i );
	    button->setGroup(0);
	    button->disconnect( this );
	    return;
	}
    }
}


/*!
  Finds and returns a pointer to the button with the specified identifier
  \a id.

  Returns null if the button was not found.
*/

QButton *QButtonGroup::find( int id ) const
{
    // introduce a QButtonListIt if calling anything
    for ( QButtonItem *i=buttons->first(); i; i=buttons->next() )
	if ( i->id == id )
	    return i->button;
    return 0;
}


/*!
  \fn void QButtonGroup::pressed( int id )
  This signal is emitted when a button in the group is
  \link QButton::pressed() pressed\endlink.
  The \a id argument is the button's identifier.
*/

/*!
  \fn void QButtonGroup::released( int id )
  This signal is emitted when a button in the group is
  \link QButton::released() released\endlink.
  The \a id argument is the button's identifier.
*/

/*!
  \fn void QButtonGroup::clicked( int id )

  This signal is emitted when a button in the group is clicked.  The
  \a id argument is the button's identifier.

  \sa QButton::clicked() insert()
*/


/*!
  \internal
  This slot is activated when one of the buttons in the group emits the
  QButton::pressed() signal.
*/

void QButtonGroup::buttonPressed()
{
    // introduce a QButtonListIt if calling anything
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
    // introduce a QButtonListIt if calling anything
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
    // introduce a QButtonListIt if calling anything
    int id = -1;
    QButton *bt = (QButton *)sender();		// object that sent the signal
#if defined(QT_CHECK_NULL)
    Q_ASSERT( bt->inherits("QButton") );
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
    // introduce a QButtonListIt if calling anything
    if ( !on || !excl_grp && !radio_excl )
	return;
    QButton *bt = (QButton *)sender();		// object that sent the signal
#if defined(QT_CHECK_NULL)
    Q_ASSERT( bt->inherits("QButton") );
    Q_ASSERT( bt->isToggleButton() );
#endif

    if ( !excl_grp && !bt->inherits("QRadioButton") )
	return;
    QButtonItem * i = buttons->first();
    bool hasTabFocus = FALSE;

    while( i != 0 && hasTabFocus == FALSE ) {
	if ( ( excl_grp || i->button->inherits("QRadioButton") ) &&
	     (i->button->focusPolicy() & TabFocus) )
	    hasTabFocus = TRUE;
	i = buttons->next();
    }

    i = buttons->first();
    while( i ) {
	if ( bt != i->button &&
	     i->button->isToggleButton() &&
	     i->button->isOn() &&
	     ( excl_grp || i->button->inherits( "QRadioButton" ) ) )
	    i->button->setOn( FALSE );
	if ( ( excl_grp || i->button->inherits( "QRadioButton" ) ) &&
	     i->button->isToggleButton() &&
	     hasTabFocus )
	    i->button->setFocusPolicy( (FocusPolicy)(i->button->focusPolicy() &
						     ~TabFocus) );
	i = buttons->next();
    }

    if ( hasTabFocus )
	bt->setFocusPolicy( (FocusPolicy)(bt->focusPolicy() | TabFocus) );
}



/*!  Sets the button with id \a id to be on; if this is an
  exclusive group, all other buttons in the group will be set to off.
*/

void QButtonGroup::setButton( int id )
{
    QButton * b = find( id );
    if ( b )
	b->setOn( TRUE );
}

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

    if ( candidate && f && f->inherits( "QButton" ) &&
	 ((QButton*)f)->isOn() &&
	 candidate->inherits( "QButton" ) &&
	 ((QButton*)candidate)->isToggleButton() &&
	 ( isExclusive() || ( f->inherits( "QRadioButton" ) &&
			      candidate->inherits( "QRadioButton" )))) {
	if ( f->focusPolicy() & TabFocus ) {
	    f->setFocusPolicy( (FocusPolicy)(f->focusPolicy() & ~TabFocus) );
	    candidate->setFocusPolicy( (FocusPolicy)(candidate->focusPolicy()|
						     TabFocus) );
	}
	((QButton*)candidate)->setOn( TRUE );
	((QButton*)candidate)->animateClick();
	((QButton*)candidate)->animateTimeout(); // ### crude l&f hack
    }

    if ( candidate )
	candidate->setFocus();
}


/*!
  Returns a pointer to the selected toggle button if exactly one
  is selected; returns 0 otherwise.
*/

QButton * QButtonGroup::selected() const
{
    if ( !buttons )
	return 0;
    QButtonListIt it( *buttons );
    QButtonItem *i;
    QButton *candidate = 0;

    while ( (i = it.current()) != 0 ) {
	++it;
	if ( i->button && i->button->isToggleButton() && i->button->isOn() ) {
	    if ( candidate != 0 )
		return 0;
	    candidate = i->button;
	}
    }
    return candidate;
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
#endif
