/****************************************************************************
**
** Implementation of QButtonGroup class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qbuttongroup.h"
#ifndef QT_NO_BUTTONGROUP
#include "qbutton.h"
#include "qmap.h"
#include "qapplication.h"
#include "qradiobutton.h"
#include "qevent.h"


/*!
    \class QButtonGroup qbuttongroup.h
    \brief The QButtonGroup widget organizes QButton widgets in a group.

    \ingroup organizers
    \ingroup geomanagement
    \ingroup appearance
    \mainclass

    A button group widget makes it easier to deal with groups of
    buttons. Each button in a button group has a unique identifier.
    The button group emits a clicked() signal with this identifier
    when a button in the group is clicked. This makes a button group
    particularly useful when you have several similar buttons and want
    to connect all their clicked() signals to a single slot.

    An \link setExclusive() exclusive\endlink button group switches
    off all toggle buttons except the one that was clicked. A button
    group is, by default, non-exclusive. Note that all radio buttons
    that are inserted into a button group are mutually exclusive even
    if the button group is non-exclusive. (See
    setRadioButtonExclusive().)

    There are two ways of using a button group:
    \list
    \i The button group is the parent widget of a number of buttons,
    i.e. the button group is the parent argument in the button
    constructor. The buttons are assigned identifiers 0, 1, 2, etc.,
    in the order they are created. A QButtonGroup can display a frame
    and a title because it inherits QGroupBox.
    \i The button group is an invisible widget and the contained
    buttons have some other parent widget. In this usage, each button
    must be manually inserted, using insert(), into the button group
    and given an identifier.
    \endlist

    A button can be removed from the group with remove(). A pointer to
    a button with a given id can be obtained using find(). The id of a
    button is available using id(). A button can be set \e on with
    setButton(). The number of buttons in the group is returned by
    count().

    <img src=qbttngrp-m.png> <img src=qbttngrp-w.png>

    \sa QPushButton, QCheckBox, QRadioButton
*/

/*!
    \property QButtonGroup::exclusive
    \brief whether the button group is exclusive

    If this property is TRUE, then the buttons in the group are
    toggled, and to untoggle a button you must click on another button
    in the group. The default value is FALSE.
*/

/*!
    \property QButtonGroup::radioButtonExclusive
    \brief whether the radio buttons in the group are exclusive

    If this property is TRUE (the default), the \link QRadioButton
    radiobuttons\endlink in the group are treated exclusively.
*/

// ### 4.0: get rid of QButtonMap class
class QButtonMap : public QMap<int, QButton *>
{
public:
    QButtonMap() {}
    ~QButtonMap() {}
};


/*!
    Constructs a button group with no title.

    The \a parent and \a name arguments are passed to the QWidget
    constructor.
*/

QButtonGroup::QButtonGroup( QWidget *parent, const char *name )
    : QGroupBox( parent, name )
{
    init();
}

/*!
    Constructs a button group with the title \a title.

    The \a parent and \a name arguments are passed to the QWidget
    constructor.
*/

QButtonGroup::QButtonGroup( const QString &title, QWidget *parent,
			    const char *name )
    : QGroupBox( title, parent, name )
{
    init();
}

/*!
    Constructs a button group with no title. Child widgets will be
    arranged in \a strips rows or columns (depending on \a
    orientation).

    The \a parent and \a name arguments are passed to the QWidget
    constructor.
*/

QButtonGroup::QButtonGroup( int strips, Orientation orientation,
			    QWidget *parent, const char *name )
    : QGroupBox( strips, orientation, parent, name )
{
    init();
}

/*!
    Constructs a button group with title \a title. Child widgets will
    be arranged in \a strips rows or columns (depending on \a
    orientation).

    The \a parent and \a name arguments are passed to the QWidget
    constructor.
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
    buttons = new QButtonMap;
    excl_grp = FALSE;
    radio_excl = TRUE;
}

/*! \reimp */

QButtonGroup::~QButtonGroup()
{
    QButtonMap *tmp = buttons;
    buttons = 0;

    QButtonMap::ConstIterator it = tmp->constBegin();
    while (it != tmp->constEnd()) {
	it.value()->setGroup(0);
	++it;
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
    Inserts the \a button with the identifier \a id into the button
    group. Returns the button identifier.

    Buttons are normally inserted into a button group automatically by
    passing the button group as the parent when the button is
    constructed. So it is not necessary to manually insert buttons
    that have this button group as their parent widget. An exception
    is when you want custom identifiers instead of the default 0, 1,
    2, etc., or if you want the buttons to have some other parent.

    The button is assigned the identifier \a id or an automatically
    generated identifier. It works as follows: If \a id >= 0, this
    identifier is assigned. If \a id == -1 (default), the identifier
    is equal to the number of buttons in the group. If \a id is any
    other negative integer, for instance -2, a unique identifier
    (negative integer \<= -2) is generated. No button has an id of -1.

    \sa find(), remove(), setExclusive()
*/

int QButtonGroup::insert( QButton *button, int id )
{
    if ( button->group() )
	button->group()->remove( button );

    static int seq_no = -2;

    if ( id < -1 )
	id = seq_no--;
    else if ( id == -1 )
	id = buttons->count();

    button->setGroup(this);
    buttons->insert(id, button);

    connect( button, SIGNAL(pressed()) , SLOT(buttonPressed()) );
    connect( button, SIGNAL(released()), SLOT(buttonReleased()) );
    connect( button, SIGNAL(clicked()) , SLOT(buttonClicked()) );
    connect( button, SIGNAL(toggled(bool)) , SLOT(buttonToggled(bool)) );

    if (button->isToggleButton() && !button->isOn()
	    && selected() && (selected()->focusPolicy() & TabFocus))
	button->setFocusPolicy((FocusPolicy)(button->focusPolicy() & ~TabFocus));
    return id;
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
    QButtonMap::Iterator it = buttons->begin();
    while (it != buttons->end()) {
        if (it.value() == button) {
	    buttons->erase(it);
	    button->setGroup(0);
	    button->disconnect(this);
	    break;
	}
        ++it;
    }
}


/*!
    Returns the button with the specified identifier \a id, or 0 if
    the button was not found.
*/

QButton *QButtonGroup::find( int id ) const
{
    return buttons->value(id);
}


/*!
    \fn void QButtonGroup::pressed( int id )

    This signal is emitted when a button in the group is \link
    QButton::pressed() pressed\endlink. The \a id argument is the
    button's identifier.

    \sa insert()
*/

/*!
    \fn void QButtonGroup::released( int id )

    This signal is emitted when a button in the group is \link
    QButton::released() released\endlink. The \a id argument is the
    button's identifier.

    \sa insert()
*/

/*!
    \fn void QButtonGroup::clicked( int id )

    This signal is emitted when a button in the group is \link
    QButton::clicked() clicked\endlink. The \a id argument is the
    button's identifier.

    \sa insert()
*/


/*!
  \internal
  This slot is activated when one of the buttons in the group emits the
  QButton::pressed() signal.
*/

void QButtonGroup::buttonPressed()
{
    QButton *senderButton = ::qt_cast<QButton*>(sender());
    Q_ASSERT(senderButton);
    int senderId = id(senderButton);
    if (senderId != -1)
	emit pressed(senderId);
}

/*!
  \internal
  This slot is activated when one of the buttons in the group emits the
  QButton::released() signal.
*/

void QButtonGroup::buttonReleased()
{
    QButton *senderButton = ::qt_cast<QButton*>(sender());
    Q_ASSERT(senderButton);
    int senderId = id(senderButton);
    if (senderId != -1)
	emit released(senderId);
}

/*!
  \internal
  This slot is activated when one of the buttons in the group emits the
  QButton::clicked() signal.
*/

void QButtonGroup::buttonClicked()
{
    QButton *senderButton = ::qt_cast<QButton*>(sender());
    Q_ASSERT(senderButton);
    int senderId = id(senderButton);
    if (senderId != -1)
	emit clicked(senderId);
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
    QButton *senderButton = ::qt_cast<QButton*>(sender());
    Q_ASSERT( senderButton );
    Q_ASSERT( senderButton->isToggleButton() );

    if ( !excl_grp && !::qt_cast<QRadioButton*>(senderButton) )
	return;
    
    bool hasTabFocus = false;

    QButtonMap::ConstIterator it = buttons->constBegin();
    while (it != buttons->constEnd()) {
	if ((excl_grp || ::qt_cast<QRadioButton*>(it.value()))
		&& (it.value()->focusPolicy() & TabFocus)) {
	    hasTabFocus = true;
	    break;
	}
        ++it;
    }

    it = buttons->constBegin();
    while (it != buttons->constEnd()) {
	if (senderButton != it.value() && it.value()->isToggleButton()
		&& it.value()->isOn() && (excl_grp || qt_cast<QRadioButton *>(it.value())))
	    it.value()->setOn(false);
	if ((excl_grp || ::qt_cast<QRadioButton *>(it.value()))
		&& it.value()->isToggleButton() && hasTabFocus)
	    it.value()->setFocusPolicy((FocusPolicy)(it.value()->focusPolicy() & ~TabFocus));	
	++it;
    }
    if (hasTabFocus)
	senderButton->setFocusPolicy((FocusPolicy)(senderButton->focusPolicy() | TabFocus));
}



void QButtonGroup::setButton( int id )
{
    QButton *b = find(id);
    if (b)
	b->setOn( TRUE );
}

void QButtonGroup::setRadioButtonExclusive( bool on)
{
    radio_excl = on;
}


/*!
    Moves the keyboard focus according to \a key, and if appropriate
    checks the new focus item.

    This function does nothing unless the keyboard focus points to one
    of the button group members and \a key is one of \c Key_Up, \c
    Key_Down, \c Key_Left and \c Key_Right.
*/

void QButtonGroup::moveFocus( int key )
{
    QWidget *f = qApp->focusWidget();
    QButton *fb = ::qt_cast<QButton *>(f);
    if (!fb)
	return;
    int fid = id(fb);
    if (fid == -1)
	return;

    QWidget * candidate = 0;
    int bestScore = -1;

    QPoint goal( f->mapToGlobal( f->geometry().center() ) );

    QButtonMap::ConstIterator it = buttons->constBegin();
    while (it != buttons->constEnd()) {
	if (it.value() != f && it.value()->isEnabled()) {
	    QPoint p(it.value()->mapToGlobal(it.value()->geometry().center()));
	    int score = (p.y() - goal.y())*(p.y() - goal.y()) +
			(p.x() - goal.x())*(p.x() - goal.x());
	    bool betterScore = score < bestScore || !candidate;
	    switch( key ) {
	    case Key_Up:
		if ( p.y() < goal.y() && betterScore ) {
		    if ( QABS( p.x() - goal.x() ) < QABS( p.y() - goal.y() ) ) {
			candidate = it.value();
			bestScore = score;
		    } else if ( it.value()->x() == f->x() ) {
			candidate = it.value();
			bestScore = score/2;
		    }
		}
		break;
	    case Key_Down:
		if ( p.y() > goal.y() && betterScore ) {
		    if ( QABS( p.x() - goal.x() ) < QABS( p.y() - goal.y() ) ) {
			candidate = it.value();
			bestScore = score;
		    } else if ( it.value()->x() == f->x() ) {
			candidate = it.value();
			bestScore = score/2;
		    }
		}
		break;
	    case Key_Left:
		if ( p.x() < goal.x() && betterScore ) {
		    if ( QABS( p.y() - goal.y() ) < QABS( p.x() - goal.x() ) ) {
			candidate = it.value();
			bestScore = score;
		    } else if ( it.value()->y() == f->y() ) {
			candidate = it.value();
			bestScore = score/2;
		    }
		}
		break;
	    case Key_Right:
		if ( p.x() > goal.x() && betterScore ) {
		    if ( QABS( p.y() - goal.y() ) < QABS( p.x() - goal.x() ) ) {
			candidate = it.value();
			bestScore = score;
		    } else if ( it.value()->y() == f->y() ) {
			candidate = it.value();
			bestScore = score/2;
		    }
		}
		break;
	    }
	}
        ++it;
    }

    QButton *buttoncand = qt_cast<QButton*>(candidate);
    if ( buttoncand && qt_cast<QButton*>(f) &&
	 ((QButton*)f)->isOn() &&
	 buttoncand->isToggleButton() &&
	 ( isExclusive() || ( qt_cast<QRadioButton*>(f) &&
			      qt_cast<QRadioButton*>(candidate)))) {
	if ( f->focusPolicy() & TabFocus ) {
	    f->setFocusPolicy( (FocusPolicy)(f->focusPolicy() & ~TabFocus) );
	    candidate->setFocusPolicy( (FocusPolicy)(candidate->focusPolicy()|
						     TabFocus) );
	}
	buttoncand->setOn( TRUE );
	buttoncand->animateClick();
	buttoncand->animateTimeout(); // ### crude l&f hack
    }

    if ( candidate ) {
	if (key == Key_Up || key == Key_Left)
	    QFocusEvent::setReason(QFocusEvent::Backtab);
	else
	    QFocusEvent::setReason(QFocusEvent::Tab);
	candidate->setFocus();
	QFocusEvent::resetReason();
    }
}


/*!
    Returns the selected toggle button if exactly one is selected;
    otherwise returns 0.

    \sa selectedId()
*/

QButton *QButtonGroup::selected() const
{
    QButton *candidate = 0;
    QButtonMap::ConstIterator it = buttons->constBegin();
    while (it != buttons->constEnd()) {
	if (it.value()->isToggleButton() && it.value()->isOn()) {
 	    if (candidate)
 		return 0;
 	    candidate = it.value();
	}
        ++it;
    }
    return candidate;
}

/*!
    \property QButtonGroup::selectedId
    \brief The id of the selected toggle button.

    If no toggle button is selected, id() returns -1.

    If setButton() is called on an exclusive group, the button with
    the given id will be set to on and all the others will be set to
    off.

    \sa selected()
*/

int QButtonGroup::selectedId() const
{
    return id( selected() );
}


/*!
    Returns the id of \a button, or -1 if \a button is not a member of
    this group.

    \sa selectedId();
*/

int QButtonGroup::id(QButton *button) const
{
    QButtonMap::ConstIterator it = buttons->constBegin();
    while (it != buttons->constEnd()) {
	if (it.value() == button)
	    return it.key();
	++it;
    }
    return -1;
}


/*!
    \reimp
*/
bool QButtonGroup::event( QEvent * e )
{
    if ( e->type() == QEvent::ChildInserted ) {
	QChildEvent * ce = (QChildEvent *) e;
	if (radio_excl) {
	    QButton *button = qt_cast<QRadioButton*>(ce->child());
	    if (button && button->isToggleButton() && !button->isOn()
		    && selected() && (selected()->focusPolicy() & TabFocus))
		button->setFocusPolicy((FocusPolicy)(button->focusPolicy() & ~TabFocus));
	}
    }
    return QGroupBox::event( e );
}
#endif
