/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qtoolbar.cpp#56 $
**
** Implementation of QAction class
**
** Created : 241200
**
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
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

#include "qaction.h"

#ifndef QT_NO_ACTION

#include "qtoolbar.h"
#include "qlist.h"
#include "qpopupmenu.h"
#include "qaccel.h"
#include "qtoolbutton.h"
#include "qcombobox.h"
#include "qtooltip.h"
#include "qwhatsthis.h"
#include "qstatusbar.h"
#include "qobjectlist.h"


/*! \class QAction qaction.h

  \brief The QAction class provides an abstract user interface action
  that can appear both in menus and tool bars.

  In modern GUIs most user commands can be invoked via a menu entry, a
  tool button and/or a keyboard accelerator.  To make the program
  simpler and more robust you can combine these in one object: an \e
  action.

  A QAction may have properties like an icon set, an appropriate menu
  text, an accelerator, a help text, a tool tip etc. Although the term
  action might suggest so they do not include the function to be
  invoked.  QAction emits signals; the functionality must be located
  in corresponding slots.

  There are two basic kinds of user interface actions, command actions
  and toggle actions. The former triggers the execution of a command
  (for example "open file"). To do this it emits the activated()
  signal when the user invokes the command. The application programmer
  is responsible for connecting activated() to a slot that executes
  the respective function.

  Toggle actions on the other hand give the user the possibility to
  opt on/off for a certain tool or modus. The drawing tools in a paint
  program or the setting of font characteristics like
  bold/underlined/italics in a text processor are examples that should
  be represented by toggle actions.  A toggle action emits a toggled()
  signal whenever it changes state.

  Both kinds are often organized in groups, and QAction also provides
  support for that. The drawing tools mentioned would typically be an
  ExclusiveGroup, and the tree tools for bold, underlined and italics
  could be a group.

  To insert an action into a menu or a tool bar, use addTo().  It will
  appear as either a menu entry or a tool button:

  \walkthrough action/application.cpp
  \skipto fileOpenAction
  \printline fileOpenAction
  \skipto fileOpenAction
  \printuntil "open"

  \skipto fileTools
  \printline fileTools
  \skipto fileOpenAction
  \printline fileOpenAction

  \skipto QPopupMenu
  \printuntil &File
  \skipto fileOpenAction
  \printline fileOpenAction

  (Refer to the \link simple-application-action.html Simple
  Application Walkthrough featuring QAction \endlink for a detailed
  explanation of this code.)

  You can add an action to an arbitrary number of menus and toolbars
  and remove it again with removeFrom(). If you change a property, the
  change shows up instantly in all representations. (Note that this
  may cause some flicker.)

  While properties like isEnabled(), text(), menuText(), toolTip(),
  statusTip(), accel(), iconSet() and whatsThis() are equally useful
  for all types of actions, some are specific to one type of
  action. isOn() is specific to toggle actions. ...

  Since accelerators and status tips are specific to one window, a
  QAction has to be the child of a QWidget for these features to
  work. We strongly recommend making QAction objects children of the
  relevant top-level widget.
*/


class QActionPrivate
{
public:
    QActionPrivate();
    ~QActionPrivate();
    QIconSet *iconset;
    QString text;
    QString menutext;
    QString tooltip;
    QString statustip;
    QString whatsthis;
    int key;
    QAccel* accel;
    int accelid;
    QAction::Type type;
    uint enabled : 1;
    uint on : 1;
    uint dropdown : 1;
    QAction * group;

    QToolTipGroup* tipGroup;

    struct MenuItem {
	MenuItem() : popup(0), child(0), id(0) {}
	QPopupMenu * popup;
	QPopupMenu * child;
	int id;
    };
    // ComboItem is only necessary for actions that are in
    // dropdown/exclusive actiongroups. The actiongroup will clean
    // this up
    struct ComboItem {
	ComboItem():combo(0), id(0) {}
	QComboBox* combo;
	int id;
    };
    QList<MenuItem> menuitems;
    QList<ComboItem> comboitems;// ### new
    QList<QWidget> widgets;

    QList<QAction> actions;
    QAction* selected;
};


QActionPrivate::QActionPrivate()
{
    iconset = 0;
    accel = 0;
    accelid = 0;
    dropdown = 0;
    key = 0;
    type = QAction::Command;
    enabled = 1;
    on = 0;
    group = 0;
    menuitems.setAutoDelete( TRUE );
    selected = 0;
    tipGroup = new QToolTipGroup( 0 );
    comboitems.setAutoDelete( TRUE );
    widgets.setAutoDelete( TRUE );
}


QActionPrivate::~QActionPrivate()
{
    // first do the menu items, then the widget - wouldn't want to
    // firste delete a menu and then access a menu item in it.

    if ( !menuitems.isEmpty() ) {
	QListIterator<QActionPrivate::MenuItem> it( menuitems );
	QActionPrivate::MenuItem* mi;
	while ( (mi=it.current()) != 0 ) {
	    ++it;
	    QPopupMenu* menu = mi->popup;
	    if ( menu->findItem( mi->id ) )
		menu->removeItem( mi->id );
	}
    }

    if ( !widgets.isEmpty() ) {
	QListIterator<QWidget> it( widgets );
	QWidget *tb;
	while ( (tb=it.current()) != 0 ) {
	    ++it;
	    delete tb;
	}
    }

    delete accel;
    delete iconset;
    delete tipGroup;
}


void QAction::updateVectors()
{
    /* long complex function to update all properties of all shapes of
       this action.  not terribly well optimized; functions like
       QToolButton::setText() are supposed to guard against flicker,
       so we don't do it here.	(we used to do about half of it, but
       not any more.) */

    if ( !d->actions.isEmpty() ) {
	QListIterator<QAction> it( d->actions );
	QAction * a;
	while( (a=it.current()) != 0 ) {
	    ++it;
	    a->setEnabled( d->enabled );
	}
    }

    QString wt( whatsThis() );

    if ( !d->menuitems.isEmpty() ) {
	QListIterator<QActionPrivate::MenuItem> it( d->menuitems );
	QActionPrivate::MenuItem* mi;
	while( (mi=it.current()) != 0 ) {
	    ++it;
	    QString t = menuText();
	    if ( d->key )
		t += '\t' + QAccel::keyToString( d->key );
	    mi->popup->changeItem( mi->id, t );
	    // ### dubious - how do we clear the iconset?
	    if ( d->iconset )
		mi->popup->changeItem( mi->id, *d->iconset, t );
	    if ( !wt.isEmpty() )
		mi->popup->setWhatsThis( mi->id, wt );
	    mi->popup->setCheckable( d->type == QAction::Toggle );
	    if ( d->type == QAction::Toggle )
		mi->popup->setItemChecked( mi->id, d->on );
	    QPopupMenu * popup = mi->popup;
	    // ### if the next if() is necessary, there's a bug in the
	    // menu system.
	    if ( popup->parentWidget() &&
		 popup->parentWidget()->inherits( "QPopupMenu" ) )
		popup = (QPopupMenu*)(popup->parentWidget());
	    popup->setItemEnabled( mi->id, d->enabled );
	}
    }

    if ( !d->widgets.isEmpty() ) {
	QListIterator<QWidget> it( d->widgets );
	QWidget * w;
	while( (w=it.current()) != 0 ) {
	    ++it;
	    if ( w->inherits( "QToolButton" ) ) {
		QToolButton* btn = (QToolButton*)w;
		if ( !d->text.isEmpty() )
		    btn->setTextLabel( d->text, FALSE );
		if ( d->iconset )
		    btn->setIconSet( *d->iconset );
		btn->setEnabled( d->enabled );
		btn->setToggleButton( d->type == QAction::Toggle );
		if ( d->type == QAction::Toggle )
		    btn->setOn( d->on );
		QToolTip::remove( btn );
		QToolTip::add( btn, toolTip(), d->tipGroup, statusTip() );
		QWhatsThis::remove( btn );
		if ( !wt.isEmpty() )
		    QWhatsThis::add( btn, wt );
	    } else if ( w->inherits( "QComboBox" ) ) {
		QComboBox * cb = (QComboBox*)w;
		cb->setEnabled( d->enabled );
		QToolTip::remove( cb );
		QWhatsThis::remove( cb );
		QString tmp = toolTip();
		if ( tmp.length() )
		    QToolTip::add( cb, tmp );
		if ( !wt.isEmpty() )
		    QWhatsThis::add( cb, wt );
		// ### cb->setCurrent() ?
	    } else if ( w->inherits( "QPopupMenu" ) ) {
		// nothing should be necessary
	    }
	}
    }
    if ( d->accel ) {
	d->accel->setEnabled( d->enabled );
	if ( !wt.isEmpty() )
	    d->accel->setWhatsThis( d->accelid, wt );
    }

    if ( !d->comboitems.isEmpty() ) {
	QListIterator<QActionPrivate::ComboItem> it( d->comboitems );
	QActionPrivate::ComboItem * ci;
	while( (ci=it.current()) != 0 ) {
	    ++it;
	    if ( d->iconset )
		ci->combo->changeItem( d->iconset->pixmap(), text(), ci->id );
	    else
		ci->combo->changeItem( text(), ci->id );
	}
    }
}


/*!  Constructs an action of type \a t, with parent \a parent and name
  \a name. If \a parent is a QAction, the new action inserts itself
  into \a parent, otherwise \a parent and \a name are as for QObject.
*/

QAction::QAction( Type t, QObject* parent, const char* name )
    : QObject( parent, name )
{
    d = new QActionPrivate;
    d->type = t;
    init();
}

/*! \obsolete

  Constructs an action skeleton with parent \a parent and name \a name.

  The \a toggle parameter defines whether the action becomes a toggle action
  (TRUE) or a command action (FALSE).

  If \a parent is a QActionGroup, the new action inserts itself into \a parent.

  Note: for accelerators and status tips to work, \a parent
  must be a widget.
*/
QAction::QAction( QObject* parent, const char* name, bool toggle )
    : QObject( parent, name )
{
    d = new QActionPrivate;
    d->type = toggle ? Toggle : Command;
    init();
}


/*! \obsolete

  This constructor creates an action with the following properties:
  the description \a text, the icon or iconset \a icon, the menu text
  \a menuText and a keyboard accelerator \a accel. It is a child of \a parent
  and named \a name. As long as \a toggle isn't set to TRUE the new
  object is a command action.

  Note that with a non-widget as \a parent the QAction created
  neither displays status tips nor responds to accelerators.

  If  \a parent is a QActionGroup, the action automatically becomes a
  member of it.

  Unless you call setToolTip() to define a separate tip text, \a text
  and \a accel will show up in tool tips. They will also serve as status tip
  as long as no statusTip() and no toolTip() have been defined.
*/
QAction::QAction( const QString& text, const QIconSet& icon, const QString& menuText, int accel, QObject* parent, const char* name, bool toggle )
    : QObject( parent, name )
{
    d = new QActionPrivate;
    d->type = toggle ? Toggle : Command;
    if ( !icon.pixmap().isNull() )
	setIconSet( icon );
    d->text = text;
    d->menutext = menuText;
    setAccel( accel );
    init();
}

void QAction::setType( Type t )
{
    d->type = t;
}

QAction::Type QAction::type() const
{
    return d->type;
}

/*! \obsolete

  This constructor results in an iconless action with the description text
  \a text, the menu text \a menuText and the keyboard accelerator \a accel.
  Its parent is \a parent and its name \a
  name. Depending on the value of \a toggle a command action (default,
  FALSE) or a toggle action (TRUE) is created.

  \a text and \a accel will show up in tool tips unless you call
  setToolTip() to define a specific tip text. As long as toolTip()
  and statusTip() are not explicitly set this combination
  serves as status tip, too.

  The action automatically becomes a member of \a parent if \a parent
  is a QActionGroup.

  For accelerators and status tips to work, \a parent must be a widget.
*/
QAction::QAction( const QString& text, const QString& menuText, int accel, QObject* parent, const char* name, bool toggle )
    : QObject( parent, name )
{
    d = new QActionPrivate;
    d->type = toggle ? Toggle : Command;
    d->text = text;
    d->menutext = menuText;
    setAccel( accel );
    init();
}

/*!
  \internal
*/
void QAction::init()
{
    if ( parent() && parent()->inherits("QAction") ) {
	d->group = (QAction*)parent();
	// insert() should be done by the parent's childEvent(), not
	// here. by that time, the child's properly set up and can be
	// added to things.
	((QAction*) parent())->insert( this ); // insert into action group
    }
}

/*! Destructs the object and frees all the widgets allocated. */

QAction::~QAction()
{
    if ( !d->menuitems.isEmpty() ) {
	QListIterator<QActionPrivate::MenuItem> it( d->menuitems );
	QActionPrivate::MenuItem *mi;
	while ( (mi=it.current()) != 0 ) {
	    ++it;
	    if ( mi->popup )
		mi->popup->disconnect( SIGNAL(destroyed()),
				       this, SLOT(objectDestroyed()) );
	}
    }

    if ( !d->widgets.isEmpty() ) {
	QListIterator<QWidget> it( d->widgets );
	QWidget * w;
	while( (w=it.current()) != 0 ) {
	    ++it;
	    disconnect( w, SIGNAL(destroyed()),
			this, SLOT(objectDestroyed()) );
	}
    }

    delete d;
}

/*! Sets the icon set to \a icon, e.g.:

  \walkthrough action/toggleaction/toggleaction.cpp
  \skipto labelonoffaction
  \printline labelonoffaction
  \skipto setIconSet
  \printline setIconSet

  (c.f. the action/toggleaction/toggleaction.cpp example)

  \a icon is used as tool button icon and in the menu entry.

  \sa iconSet();
*/
void QAction::setIconSet( const QIconSet& icon )
{
    register QIconSet *i = d->iconset;
    d->iconset = new QIconSet( icon );
    delete i;
    updateVectors();
}

/*! Returns the icon set.

  \sa setIconSet();
*/
QIconSet QAction::iconSet() const
{
    if ( d->iconset )
	return *d->iconset;
    return QIconSet();
}

/*! Attaches a descriptive \a text to the action.

  If \l QMainWindow::usesTextLabel() is TRUE \a text shows up as
  label in the relevant tool-button. Furthermore \a text serves
  as the default text in menus and tips if
  the relevant properties are not defined otherwise.

  \sa text(), setMenuText(), setToolTip(), setStatusTip()
*/
void QAction::setText( const QString& text )
{
    if ( d->text == text )
	return;
    d->text = text;
    updateVectors();
}

/*! Returns the current description text.

  This is used as tool button label provided
  \l QMainWindow::usesTextLabel() is TRUE.
  If menuText() or toolTip() and statusTip() are not set,
  text() in combination with
  accel() serves as menu text, tool tip and status tip.

  \sa setText(), setMenuText(), setToolTip(), setStatusTip()
*/
QString QAction::text() const
{
    return d->text;
}


/*! Sets a special text \a text for menu entries.

  The entire menu entry will consist of iconSet() (if defined), \a text
  and accel() (if defined).

  Use setMenuText() whenever different wording
  in menu items, tool tips and button text is appropriate.

  \sa menuText(), setText(), text()
*/
void QAction::setMenuText( const QString& text )
{
    if ( d->menutext == text )
	return;
    d->menutext = text;
    updateVectors();
}

/*! Returns the text used in menu entries.

  If no menu text has been defined, this is the same as text().

  \sa setMenuText(),  text()
*/
QString QAction::menuText() const
{
    if ( !d->menutext.isEmpty() )
	return d->menutext;
    return text();
}

/*! Sets the tool tip text to \a tip.

  As long as statusTip() hasn't been set using setStatusTip()
  \a tip serves as message in the status bar as well.

  \sa toolTip(), statusTip()
*/
void QAction::setToolTip( const QString& tip )
{
    if ( d->tooltip == tip )
	return;
    d->tooltip = tip;
    updateVectors();
}

/*! Returns the current tool tip.

  If no tool tip has been defined yet, toolTip() tries to build a
  suitable text based on the text(), the accel() and (if this QAction
  is part of a group) the group's toolTip().

  toolTip() also serves as statusTip() if no separate message for the
  status bar has been set.

  \sa setToolTip(), setStatusTip(), text(), accel(), QAccel::keyToString()
*/
QString QAction::toolTip() const
{
    if ( !d->tooltip.isEmpty() )
	return d->tooltip;
    QString result = text();
    if ( d->group && d->group->d->tooltip )
	result = d->group->d->tooltip;
    if ( d->accel ) {
	if ( result.length() > 0 )
	    result += ' ';
	result = result + "(" +
		 QAccel::keyToString( d->accel->key( d->accelid )) + ")";
    }
    return result;

}

/*! Sets the status tip to \a tip. It is displayed on all status bars
  the toplevel widget parenting this action provides.

  Note that QActions that were created with only non-windows as
  ancestors can't display status tips.

  \sa statusTip(), toolTip()
*/

void QAction::setStatusTip( const QString& tip )
{
    if ( d->statustip == tip )
	return;
    d->statustip = tip;
    updateVectors();
}

/*!  Returns the current status tip.

  If no status tip has been defined yet, this is the same as toolTip().

  \sa setStatusTip(), toolTip()
*/
QString QAction::statusTip() const
{
    if ( !d->statustip.isNull() )
	return d->statustip;
    return toolTip();
}

/*!
  Sets What's This help to \a whatsThis.

  \a whatsThis may contain rich-text elements, e.g.:

  \walkthrough action/application.cpp
  \skipto filePrintText
  \printuntil setWhatsThis

  (For a detailed explanation of the above code refer to the
  \link simple-application-action.html Simple Application Walkthrough
  featuring QAction \endlink.)

  \sa whatsThis(), QStyleSheet, QWhatsThis
*/
void QAction::setWhatsThis( const QString& whatsThis )
{
    if ( d->whatsthis == whatsThis )
	return;
    d->whatsthis = whatsThis;
    if ( !d->whatsthis.isEmpty() && d->accel )
	d->accel->setWhatsThis( d->accelid, d->whatsthis );
    updateVectors();
}

/*! Returns the What's This help text for this action.

  If no What's This text has been set and this action is a child of
  another, the parent's What's This text is returned. This makes it
  easy to to have the same What's This text for all actions in an
  action group.

  Note that text() does not serve as default value here.

  \sa setWhatsThis()
*/
QString QAction::whatsThis() const
{
    if ( d->whatsthis.isNull() && parent()->inherits( "QAction" ) )
	return ((QAction*)parent())->whatsThis();
    return d->whatsthis;
}


/*! Sets the action's accelerator to \a key, e.g.:

  \walkthrough action/toggleaction/toggleaction.cpp
  \skipto labelonoffaction
  \printline labelonoffaction
  \skipto setAccel
  \printline setAccel

  (c.f. the action/toggleaction/toggleaction.cpp example)

  For accelerators to work, the action's parent or one of its ancestors
  has to be the application window.

  \sa accel()
*/
void QAction::setAccel( int key )
{
    d->key = key;
    delete d->accel;
    d->accel = 0;

    if ( !key )
	return;

    QObject* p = parent();
    while ( p && !p->isWidgetType() )
	p = p->parent();
    if ( p ) {
	QString n = QString::fromLatin1( "created by " ) + name();
	d->accel = new QAccel( (QWidget*)p, 0, n.latin1() );
	d->accelid = d->accel->insertItem( d->key );
	d->accel->connectItem( d->accelid,
			       this, SLOT( internalActivation() ) );
    } else {
#if defined(QT_CHECK_STATE)
	qWarning( "QAction::setAccel() (%s) requires widget in parent chain.",
		  name() );
#endif
    }
    updateVectors();
}


/*! Returns the acceleration key.

  The hexadecimal keycodes can be found in \l Qt::Key and \l Qt::Modifier.

  \sa setAccel()
*/
int QAction::accel() const
{
    return d->key;
}


/*! \obsolete

  setType( QAction::Toggle ) is the recommended API.

  Makes the action a toggle action if \a enable is TRUE, or a
  command action if \a enable is FALSE.

  You may want to add toggle actions to a QActionGroup for exclusive
  toggling.

  Keep in mind that it is advisable to connect a command actions'
  user command to the activated() signal whereas toggled() is the
  preferred signal for toggle actions.

  \sa isToggleAction()
*/
void QAction::setToggleAction( bool enable )
{
    if ( d->type == Toggle || d->type == Command )
	setType( enable ? Toggle : Command );
}

/*! Returns whether the action is a toggle action or not.

  \sa setToggleAction()
*/
bool QAction::isToggleAction() const
{
    return d->type == QAction::Toggle;
}

/*! Switches a toggle action on if \a enable is TRUE or off if \e enable is
  FALSE.

  This function has no effect on command actions and QActionGroups.

  \sa isOn(), isToggleAction(), setToggleAction()
*/
void QAction::setOn( bool enable )
{
    if ( d->type == QAction::Toggle ) {
#if defined(QT_CHECK_STATE)
	qWarning( "QAction::setOn() (%s) Only toggle actions "
		  "may be switched", name( "unnamed" ) );
#endif
	return;
    }
    if ( enable == (bool)d->on )
	return;
    d->on = enable;
    updateVectors();
    emit toggled( enable );
}

/*! Returns TRUE if this toggle action is switched on, or FALSE if it is
  switched off.

  For command actions isOn() is always FALSE.

  \sa setOn(), isToggleAction()
*/
bool QAction::isOn() const
{
    return d->on;
}

/*! Enables the action if \a enable is TRUE, otherwise disables it.

  Disabled actions can't be chosen by the user. They don't
  disappear from the menu/tool bar but are immediately presented
  in a manner indicating their unavailability.

  What's this? help on disabled actions is still available
  provided whatsThis() is set.

  \sa isEnabled()
*/
void QAction::setEnabled( bool enable )
{
    if ( enable == d->enabled )
	return;
    d->enabled = enable;
    updateVectors();
}


/*! Returns TRUE if the action is enabled, or FALSE if it is disabled.

  Disabled actions appear in a way indicating their unavailability
  and can't be chosen by the user.

  \sa setEnabled()
*/
bool QAction::isEnabled() const
{
    return d->enabled;
}

/*! \internal
*/
void QAction::internalActivation()
{
    if ( isToggleAction() )
	setOn( !isOn() );
    emit activated();
}

/*! \internal
*/
void QAction::toolButtonToggled( bool on )
{
    if ( !isToggleAction() )
	return;
    setOn( on );
}

/*! Adds this action to widget \a w.

  Currently, QToolBar and QPopupMenu are supported.

  An action added to a tool bar is automatically displayed as a tool
  button; an action added to a pop up menu appears as a menu entry:

  \walkthrough action/application.cpp
  \skipto fileTools
  \printline fileTools
  \skipto addTo( fileTools )
  \printline addTo( fileTools )

  \skipto new QPopupMenu
  \printuntil menuBar()
  \skipto fileOpenAction->addTo
  \printline fileOpenAction->addTo

  (cf. the \link simple-application-action.html Simple Application Walkthrough
  featuring QAction \endlink)

  addTo() returns TRUE if the action was added successfully and FALSE
  if \a w is of an unsupported class.

  Note that if you call addTo() with one of the action's parent or
  grandparent objects as argument, there is a danger of recursion.

  \sa removeFrom()
*/
bool QAction::addTo( QWidget* w )
{
    bool needUpdate = FALSE;
    QWidget * addChildrenTo = 0;

    if ( w->inherits( "QToolBar" ) ) {
	QToolBar * t = (QToolBar*)w;
	if ( d->type == Command || d->type == Toggle ) {
	    QToolButton* btn = new QToolButton( t );
	    addedTo( btn, w );
	    btn->setToggleButton( d->type == Toggle );
	    d->widgets.append( btn );
	    if ( d->iconset )
		btn->setIconSet( *d->iconset );
	    needUpdate = TRUE;
	    connect( btn, SIGNAL( clicked() ),
		     this, SIGNAL( activated() ) );
	    connect( btn, SIGNAL( toggled(bool) ),
		     this, SLOT( toolButtonToggled(bool) ) );
	    connect( btn, SIGNAL( destroyed() ),
		     this, SLOT( objectDestroyed() ) );
	    connect( d->tipGroup, SIGNAL(showTip(const QString&)),
		     this, SLOT(showStatusText(const QString&)) );
	    connect( d->tipGroup, SIGNAL(removeTip()),
		     this, SLOT(clearStatusText()) );
	} else if ( d->type == Separator ) {
	    t->addSeparator();
	} else if ( d->type == Group ) {
	    if ( d->dropdown ) {
		QAction *defAction = d->actions.first();

		QToolButton* btn = new QToolButton( (QToolBar*) w );
		addedTo( btn, w, defAction );
		connect( btn, SIGNAL(destroyed()), SLOT(objectDestroyed()) );
		d->widgets.append( btn );
		needUpdate = TRUE;
		connect( btn, SIGNAL( clicked() ),
			 defAction, SIGNAL( activated() ) );
		connect( btn, SIGNAL( toggled(bool) ),
			 defAction, SLOT( toolButtonToggled(bool) ) );
		connect( btn, SIGNAL( destroyed() ),
			 defAction, SLOT( objectDestroyed() ) );

		QPopupMenu *menu = new QPopupMenu( btn );
		btn->setPopupDelay( 0 );
		btn->setPopup( menu );
		addChildrenTo = menu;
	    } else {
		// in the non-dropdown case, the children will insert
		// themselves and there's nothing to do except perhaps
		// to make sure they're exclusive.
	    }
	} else if ( d->type == ExclusiveGroup ) {
	    if ( d->dropdown ) {
		QComboBox *box = new QComboBox( w );
		addedTo( box, w );
		connect( box, SIGNAL(destroyed()), SLOT(objectDestroyed()) );
		d->widgets.append( box );
		needUpdate = TRUE;
		QListIterator<QAction> it( d->actions);
		QAction * a;
		while( (a=it.current()) != 0 ) {
		    ++it;
		    box->insertItem( a->iconSet().pixmap(), a->text() );
		}
		connect( box, SIGNAL(activated(int)),
			 this, SLOT( internalComboBoxActivated(int)) );
	    } else {
		// nothing here either.
	    }
	}
    } else if ( w->inherits( "QPopupMenu" ) ) {
	QPopupMenu * p = (QPopupMenu *)w;
	if ( d->type == Command || d->type == Toggle ) {
	    QActionPrivate::MenuItem* mi = new QActionPrivate::MenuItem;
	    QIconSet* diconset = d->iconset; // stupid GCC 2.7.x compiler
	    if ( diconset )
		mi->id = p->insertItem( *diconset, QString::fromLatin1("") );
	    else
		mi->id = p->insertItem( QString::fromLatin1("") );
	    needUpdate = TRUE; // very, very TRUE :)
	    mi->popup = p;
	    d->menuitems.append( mi );
	    mi->popup->connectItem( mi->id, this, SLOT(internalActivation()) );
	    connect( p, SIGNAL(highlighted( int )),
		     this, SLOT(menuStatusText( int )) );
	    connect( p, SIGNAL(aboutToHide()),
		     this, SLOT(clearStatusText()) );
	    connect( p, SIGNAL( destroyed() ),
		     this, SLOT( objectDestroyed() ) );
	    addedTo( p->indexOf( mi->id ), p );
	    // ##### why was next line here?
	    // w->topLevelWidget()->className();
	    // ##### has to be a bug?
	} else if ( d->type == Separator ) {
	    p->insertSeparator();
	} else if ( d->type == Group || d->type == ExclusiveGroup ) {
	    if ( d->dropdown ) {
		QActionPrivate::MenuItem* mi = new QActionPrivate::MenuItem;
		QPopupMenu *popup = new QPopupMenu( p );
		connect( popup, SIGNAL(destroyed()), SLOT(objectDestroyed()) );
		mi->id = p->insertItem( QString::null, popup );
		mi->popup = p;
		mi->child = popup;
		needUpdate = TRUE;
		d->menuitems.append( mi );

		addedTo( p->indexOf( mi->id ), p );
		addChildrenTo = popup;
	    } else {
		addChildrenTo = w;
	    }
	}
    // Makes only sense when called by QActionGroup::addTo
    } else if ( w->inherits( "QComboBox" ) ) {
	if ( qstrcmp( name(), "qt_separator_action" ) ) {
	    QActionPrivate::ComboItem *ci = new QActionPrivate::ComboItem;
	    ci->combo = (QComboBox*)w;
	    ci->id = ci->combo->count();
	    if ( d->iconset )
		ci->combo->insertItem( d->iconset->pixmap(), text() );
	    else
		ci->combo->insertItem( text() );
	    d->comboitems.append( ci );
	}
    } else {
	qWarning( "QAction::addTo(): cannot add to type %s", w->className() );
	return FALSE;
    }

    if ( !addChildrenTo )
	return TRUE;

    QListIterator<QAction> it( d->actions );
    QAction * a;
    bool ok = TRUE;
    while ( (a=it.current()) != 0 ) {
	++it;
	ok = a->addTo( addChildrenTo ) && ok; // && ok must be last!
    }
    if ( needUpdate )
	updateVectors();
    return ok;
}

/*! This function is called from the addTo() function when it created
  a widget (\a actionWidget) for the action in the \a container.
*/

void QAction::addedTo( QWidget *actionWidget, QWidget *container )
{
    Q_UNUSED( actionWidget );
    Q_UNUSED( container );
}

/*! \overload

  This function is called from the addTo() function when it created
  a menu item at the index \a index in the popup menu \a menu.
*/

void QAction::addedTo( int index, QPopupMenu *menu )
{
    Q_UNUSED( index );
    Q_UNUSED( menu );
}

void QAction::addedTo( QWidget *actionWidget, QWidget *container, QAction *a )
{
    Q_UNUSED( actionWidget );
    Q_UNUSED( container );
    Q_UNUSED( a );
}

void QAction::addedTo( int index, QPopupMenu *menu, QAction *a )
{
    Q_UNUSED( index );
    Q_UNUSED( menu );
    Q_UNUSED( a );
}

/*! Sets the status message to \a text, if possible. */
void QAction::showStatusText( const QString& text )
{
    QObject* par = this;
    while( par && !par->isWidgetType() )
	par = par->parent();
    if ( !par )
	return;
    QWidget * w = (QWidget*)par;
    QStatusBar*bar = (QStatusBar*)(w->topLevelWidget()->child( 0, "QStatusBar",
							       FALSE ));
    if ( !bar ) {
	QObjectList *l = w->topLevelWidget()->queryList( "QStatusBar" );
	if ( !l )
	    return;
	// #### hopefully the last one is the one of the mainwindow...
	bar = (QStatusBar*)(l->last());
	delete l;
    }
    if ( bar ) {
	if ( text.isEmpty() )
	    bar->clear();
	else
	    bar->message( text );
    }
}

/*! Sets the status message to the text that is most appropriate for
  menu item \a id. */
void QAction::menuStatusText( int id )
{
    QString text;
    QListIterator<QActionPrivate::MenuItem> it( d->menuitems);
    QActionPrivate::MenuItem* mi;
    while ( text.isEmpty() && (mi=it.current()) != 0 ) {
	++it;
	if ( mi->id == id )
	    text = statusTip();
    }

    if ( !text.isEmpty() )
	showStatusText( text );
}


/*! Clears the status text. */
void QAction::clearStatusText()
{
    showStatusText( QString::null );
}

/*! Removes the action from widget \a w.

  Returns TRUE if the action was removed successfully, FALSE
  otherwise.

  \sa addTo()
*/
bool QAction::removeFrom( QWidget* w )
{
    if ( d->actions.count() > 0 && !d->dropdown ) {
	QListIterator<QAction> it( d->actions);
	QAction * a;
	while( (a=it.current()) != 0 ) {
	    ++it;
	    a->removeFrom( w );
	}
    }

    QList<QWidget> deletia;

    if ( w->inherits( "QToolButton" ) ) {
	d->widgets.first();
	QWidget * c;
	while( (c=d->widgets.current()) != 0 ) {
	    if ( c->parentWidget() == w ) {
		d->widgets.take();
		deletia.append( w );
	    } else {
		d->widgets.next();
	    }
	}
    } else if ( w->inherits( "QPopupMenu" ) ) {
	QListIterator<QActionPrivate::MenuItem> it( d->menuitems );
	QActionPrivate::MenuItem* mi;
	while ( (mi=it.current()) != 0 ) {
	    ++it;
	    if ( mi->popup == w ) {
		disconnect( mi->popup, SIGNAL(highlighted( int )),
			    this, SLOT(menuStatusText(int)) );
		disconnect( mi->popup, SIGNAL(aboutToHide()),
			    this, SLOT(clearStatusText()) );
		disconnect( mi->popup, SIGNAL( destroyed() ),
			    this, SLOT( objectDestroyed() ) );
		if ( mi->child ) {
		    disconnect( mi->child, SIGNAL( destroyed() ),
				this, SLOT( objectDestroyed() ) );
		    deletia.append( mi->child );
		    QListIterator<QAction> it( d->actions);
		    QAction * a;
		    while( (a=it.current()) != 0 ) {
			++it;
			a->removeFrom( mi->child );
		    }
		}
		mi->popup->removeItem( mi->id );
		d->menuitems.removeRef( mi );
	    }
	}
    } else {
	qWarning( "QAction::removeFrom(), unknown object" );
	return FALSE;
    }

    QWidget * c;
    while( (c=deletia.current()) != 0 ) {
	disconnect( c, SIGNAL( destroyed() ),
		    this, SLOT( objectDestroyed() ) );
	delete c;
	deletia.next();
    }

    return TRUE;
}

/*!
  \internal
*/
void QAction::objectDestroyed()
{
    const QObject * obj = sender();
    if ( obj->isWidgetType() )
	d->widgets.removeRef( (QWidget *)obj );

    QListIterator<QActionPrivate::MenuItem> it( d->menuitems);
    QActionPrivate::MenuItem* mi;
    while ( ( mi = it.current() ) ) {
	++it;
	if ( mi->popup == obj )
	    d->menuitems.removeRef( mi );
    }
}

/*! \fn void QAction::activated()

  This signal is emitted when an action is activated by the user.

  For toggle actions activated() is emitted regardless whether they are
  switched on or off. When dealing with this type of action it
  is preferable to connect the toggled() signal to the
  desired slot and ignore activated().
*/

/*! \fn void QAction::toggled(bool)

  This signal is emitted when a toggle action changes state;
  command actions and QActionGroups don't emit toggled().

  The carried argument denotes to the new state; i.e. TRUE
  if the toggle action was switched on and FALSE if
  it was switched off.

  To trigger a user command depending on whether a toggle action has
  been switched on or off connect it to a slot that takes care
  about its state, e.g.:

  \walkthrough action/toggleaction/toggleaction.cpp
  \skipto QMainWindow * window
  \printline QMainWindow * window
  \skipto labelonoffaction
  \printline labelonoffaction
  \skipto connect

  \printuntil setUsesTextLabel

  (c.f. the action/toggleaction/toggleaction.cpp example)

  \sa activated(), setToggleAction(), setOn()
*/


/*! \class QActionGroup qaction.h

  \brief The QActionGroup class combines actions into a group.

  This makes it easier to deal with actions that
  belong together. As a QActionGroup they can be
  added to and removed from a menu or a tool bar with a single call:

  \walkthrough action/actiongroup/editor.cpp
  \skipto QActionGroup
  \printuntil redfontcolor
  \skipto colors->addTo
  \printline colors->addTo

  (Please refer to the \link actiongroup.html QActionGroup Walkthrough \endlink
  for a detailed explanation.)

  The order in which member actions appear in a widget follows the
  sequence they were inserted into QActionGroup. A QAction
  created as a child of an action group is inserted at creation time.

  QActionGroup is an action on its own and thus can be treated as
  such. Standard action functions like addTo(), removeFrom() and
  setEnabled() are automatically performed on all members of the
  group. Thus adding a group to a tool bar creates a
  tool bar entry for each child action.

  Whilst a QActionGroup emits an activated() signal when
  one of its members is activated, the QAction::toggled() signal
  is not propagated as an action group can't
  be a toggle action on its own. Therefore isToggleAction()
  and isOn() are not useful in connection with action groups.

  For toggle actions an action group provides "one of many" choice
  similar to a group of radio buttons (see QRadioButton).
  This is controlled by the \e exclusive flag
  which is TRUE by default. An exclusive group switches off all
  toggle actions except the one that was activated, thereby
  emitting the selected() signal. On command action members of an
  exclusive QActionGroup this property has no effect.

  By default member actions of a QActionGroup can't be visibly
  distinguished from single actions in a menu or a tool bar:

  <IMG SRC="qactiongroup_menu.png" ALT="[ a default QActionGroup added to a popup menu ]">
  <IMG SRC="qactiongroup_toolbar.png" ALT="[ a default QActionGroup added to a tool bar ]">

  To place
  group members in a separate subwidget use setUsesDropDown().
  This is where iconSet(), text() and menuText() come handy to
  illustrate and describe the subwidget. For action groups with
  usesDropDown() being TRUE whatsThis() help and toolTip()s are
  available where appropriate. The pictures below illustrate how the
  subwidgets look like.

  <TABLE BORDER=1 CELLSPACING=1 CELLPADDING=2 WIDTH=100%>
  <TR>
  <TH>&nbsp;</TH>
  <TH COLSPAN=2>
  QActionGroups with usesDropDown() TRUE added to a
  </TH>
  </TR>
  <TR>
  <TH>exclusive()</TH>
  <TH>popup menu</TH>
  <TH>tool bar</TH>
  </TR>
  <TR>
  <TH>TRUE</TH>
  <TD ROWSPAN=2>
  <IMG SRC="qactiongroup_menu_subwidget.png" ALT="[ QActionGroup using subwidgets added to a popup menu ]">
  </TD>
  <TD>
  <IMG SRC="qactiongroup_toolbar_exclusive_subwidget.png" ALT="[ exclusive QActionGroup using subwidgets added to a tool bar ]">
  </TD>
  </TR>
  <TR>
  <TH>FALSE</TH>
  <TD>
  <IMG SRC="qactiongroup_toolbar_nonexclusive_subwidget.png" ALT="[ non-exclusive QActionGroup using subwidgets added to a tool bar ]">
  </TD>
  </TR>
  </TABLE>

  Other QAction properties like statusTip()
  and accel() have no effect with action groups.

*/

/*! Constructs an action group with parent \a parent and name \a name.

  If \a exclusive is TRUE, any toggle action that is a member of this
  group is toggled off by another member action being toggled on.

*/
QActionGroup::QActionGroup( QObject* parent, const char* name, bool exclusive )
    : QAction( (exclusive ? ExclusiveGroup : Group), parent, name )
{
    // nothing
}

/*! Destructs the object and frees allocated resources. */

QActionGroup::~QActionGroup()
{
}

/*! Makes this action group exclusive if \a enable is TRUE
  or non-exclusive if \a enable is FALSE.

  Exclusive groups can't have more than one toggle action set on
  at a time.
  Whenever a toggle action member of an exclusive group is toggled on,
  every other toggle action member changes its isOn() property to FALSE.
  Command action members are not affected.

  \sa isExclusive(), setToggleAction()
*/
void QActionGroup::setExclusive( bool enable )
{
    if ( type() == Group || type() == ExclusiveGroup )
	setType( enable ? ExclusiveGroup : Group );
}

/*! Returns TRUE if the action group is exclusive, otherwise FALSE.

  \sa setExclusive()
*/

bool QActionGroup::isExclusive() const
{
    return type() == ExclusiveGroup;
}

/*! When \a enable is TRUE, the group members are displayed in a
  logical subwidget of the widget(s) the action group is added to.

  Exclusive action groups added to a tool bar display their members
  in a combobox with the action's text() and
  iconSet() properties shown. Non-exclusive groups are represented
  by a tool button showing their iconSet() and --
  depending on QMainWindow::usesTextLabel() -- text() property. A
  submenu popup assists in displaying the member actions.

  In a popup menu the member actions are grouped in a separate submenu.
  Its menu entry can be adjusted by changing the action group's menuText()
  and iconSet() properties:

  \walkthrough action/actiongroup/editor.cpp
  \skipto QActionGroup
  \printline QActionGroup

  \skipto QPopupMenu
  \printline QPopupMenu
  \printuntil setUsesDropDown
  \printline setMenuText

  \printline colors->addTo

  (For a detailed explanation of the above code please refer to the
  \link actiongroup.html QActionGroup Walkthrough. \endlink

  Changing setUsesDropDown() effects subsequent calls to addTo() only.

  \sa usesDropDown
*/
void QAction::setUsesDropDown( bool enable )
{
    // ### remove
    d->dropdown = enable;
    // ### addto
}

/*! Returns whether this group uses a subwidget to represent its member actions.

  \sa setUsesDropDown
*/
bool QAction::usesDropDown() const
{
    return d->dropdown;
}

/*! Inserts action \a action into this group.

  QActions with this action group as their parent object became
  members at creation time and don't have to be inserted manually.

  Note that all members of an action group must be inserted before the
  group is added to a widget.

  \sa addTo()
*/
void QAction::insert( QAction* action )
{
    if ( d->actions.containsRef( action ) )
	return;

    d->actions.append( action );

    if ( action->whatsThis().isNull() )
	action->setWhatsThis( whatsThis() );
    if ( action->toolTip().isNull() )
	action->setToolTip( toolTip() );

    connect( action, SIGNAL( destroyed() ),
	     this, SLOT( childDestroyed() ) );
    connect( action, SIGNAL( activated() ),
	     this, SIGNAL( activated() ) );
    connect( action, SIGNAL( toggled( bool ) ),
	     this, SLOT( childToggled( bool ) ) );

    if ( !d->widgets.isEmpty() ) {
	QListIterator<QWidget> it( d->widgets );
	QWidget * w;
	while( (w=it.current()) != 0 ) {
	    ++it;
	    action->addTo( w );
	}
    }
    if ( !d->menuitems.isEmpty() ) {
	QListIterator<QActionPrivate::MenuItem> it( d->menuitems );
	QActionPrivate::MenuItem * mi;
	while( (mi=it.current()) != 0 ) {
	    ++it;
	    QPopupMenu* popup = mi->popup;
	    if ( popup )
		action->addTo( popup );
	}
    }
}

/*! Inserts a separator into the group. */
void QAction::insertSeparator()
{
    (void) new QAction( Separator, this, "automatic separator" );
}

/*! \fn bool QActionGroup::addTo( QWidget* w )

  Adds this action group to the widget \a w.

  Depending on the class of \a w all member actions are automatically presented
  as menu or tool bar entries.

  An exclusive action group with usesDropDown() enabled is presented in
  a combobox if \a w is a toolbar. For non-exclusive groups using the drop down
  property a tool button featuring an additional popup menu is created. To
  present it properly, the action group's iconSet() and text() must be set.
  If usesDropDown() is FALSE each member QAction in this group shows up as a
  separate tool button.

  Drop down action groups to be displayed as submenus in a popup menu should
  at least have their menuText() or text() property set. No difference is made
  in the submenu representation of exclusive and non-exclusive action groups.
  If usesDropDown() is FALSE each member action is displayed as a menu entry
  of the popup menu the group was added to.

  Note that only actions that have been inserted so far are displayed.
  Make sure to insert() all potential member actions before adding this
  action group to a widget.

  \sa setExclusive, setUsesDropDown, removeFrom()
*/


/*! \internal */
void QAction::childToggled( bool b )
{
    if ( d->type != ExclusiveGroup || !sender()->inherits( "QAction" ) )
	return;
    QAction* s = (QAction*) sender();
    if ( b ) {
	if ( s != d->selected ) {
	    d->selected = s;
	    QListIterator<QAction> it( d->actions);
	    QAction * a;
	    while( (a=it.current()) != 0 ) {
		++it;
		if ( a->isToggleAction() && a != s )
		    a->setOn( FALSE );
	    }
	    internalToggle( s );
	    emit activated();
	    emit selected( s );
	}
    } else {
	if ( s == d->selected ) {
	    // ### should this ever be executed?
	    s->setOn( TRUE );
	}
    }
}

/*! \internal */
void QAction::childDestroyed()
{
    if ( !sender()->inherits( "QAction" ) )
	return;
    QAction * a = (QAction*)sender();
    d->actions.removeRef( a );
    if ( d->selected == a )
	d->selected = 0;
    // this may leave a popup menu or combo box empty. so be it.
}


/*! \reimp */
void QAction::childEvent( QChildEvent *e )
{
    if ( !e->child()->inherits( "QAction" ) )
	return;

    QAction *action = (QAction*)e->child();

    if ( e->inserted() ) {
	// ### should iterate over the widgets and call action->addTo.
    }

    // the remainder of this function looks very fishy to arnt's
    // unskilled eyes.

    if ( e->removed() && d->widgets.count() ) {
	QListIterator<QWidget> it( d->widgets );
	QWidget * w;
	while( (w=it.current()) != 0 ) {
	    ++it;
	    if ( w->inherits( "QComboBox" ) ) {
		// this is unsound - it assumes that the actions have
		// unique texts.
		QComboBox * cb = (QComboBox*)w;
		for ( int i = 0; i < cb->count(); i++ ) {
		    if ( cb->text( i ) == action->text() ) {
			cb->removeItem( i );
			break;
		    }
		}
	    } else if ( w->inherits( "QToolButton" ) ) {
		QToolButton * p = (QToolButton*)w;
		if ( p )
		    action->removeFrom( p );
	    }
	}
    }

    if ( e->removed() && d->menuitems.count() ) {
	QListIterator<QActionPrivate::MenuItem> mi( d->menuitems );
	QActionPrivate::MenuItem * m;
	while( (m=mi.current()) != 0 ) {
	    ++mi;
	    QPopupMenu* popup = mi.current()->child;
	    if ( !popup )
		continue;
	    action->removeFrom( popup );
	}
    }
}

/*! \fn void QActionGroup::selected( QAction* )

  This signal is emitted from exclusive groups when member toggle actions
  change state.

  Its argument denotes to the action whose state changed to on.

  To call a user program depending on which action was switched on connect
  this signal to a slot that takes care of the action argument:

  \walkthrough action/actiongroup/editor.cpp
  \skipto QActionGroup
  \printline QActionGroup
  \skipto QObject::connect
  \printuntil SLOT

  (This code including the implementation of the
  \link actiongroup.html#setFontColor() setFontColor() \endlink
  slot can be found in the \link actiongroup.html QActionGroup Walkthrough. \endlink)

  \sa setExclusive(), isOn()
*/

/*! \internal
*/
void QAction::internalComboBoxActivated( int index )
{
    QAction *a = d->actions.at( index );
    if ( a ) {
	if ( a != d->selected ) {
	    d->selected = a;
	    for ( QListIterator<QAction> it( d->actions);
		  it.current(); ++it ) {
		if ( it.current()->isToggleAction() && it.current() != a )
		    it.current()->setOn( FALSE );
	    }
	    if ( a->isToggleAction() )
		a->setOn( TRUE );

	    emit activated();
	    internalToggle( d->selected );
	    emit selected( d->selected );
	    emit ((QActionGroup*)a)->activated();
	}
    }
}

/*! \internal
*/
void QAction::internalToggle( QAction *a )
{
    QListIterator<QWidget> it( d->widgets);
    QWidget * w;
    while( (w=it.current()) != 0 ) {
	++it;
	if ( w->inherits( "QComboBox" ) ) {
	    QComboBox * cb = (QComboBox*)w;
	    int index = d->actions.find( a );
	    if ( index != -1 )
		cb->setCurrentItem( index );
	}
    }
}

#endif
