/****************************************************************************
** $Id: $
**
** Implementation of QAction class
**
** Created : 000000
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
#include "qptrlist.h"
#include "qpopupmenu.h"
#include "qaccel.h"
#include "qtoolbutton.h"
#include "qcombobox.h"
#include "qtooltip.h"
#include "qwhatsthis.h"
#include "qstatusbar.h"
#include "qobjectlist.h"


/*! \class QAction qaction.h
    \ingroup basic
    \ingroup application
  \mainclass
  \brief The QAction class provides an abstract user interface action that can
  appear both in menus and tool bars.

  In GUI applications many commands can be invoked via a menu option, a
  toolbar button and a keyboard accelerator. Since the same action must
  be performed regardless of how the action was invoked and since the
  menu and toolbar should be kept in sync it is useful to represent a
  command as an \e action. An action can be added to a menu and a
  toolbar and will automatically be kept in sync, for example, if the
  user presses a Bold toolbar button the Bold menu item will be checked.

  A QAction may contain an icon, a menu text, an accelerator, a status
  text, a whats this text and a tool tip. Most of these can be set in
  the constructor. They can all be set independently with setIconSet(),
  setText(), setMenuText(), setToolTip(), setStatusTip(), setWhatsThis()
  and setAccel().

  An action may be a toggle action e.g. a Bold toolbar button, or a
  command action, e.g. 'Open File' which invokes an open file dialog.
  Toggle actions emit the toggled() signal when their state changes.
  Both command and toggle actions emit the activated() signal when they
  are invoked. Use setToggleAction() to set an action's toggled status.
  To see if an action is a toggle action use isToggleAction(). A toggle
  action may be "on", isOn() returns TRUE, or "off", isOn() returns
  FALSE.

  Actions are added to widgets (menus or toolbars) using addTo(), and
  removed using removeFrom().

    Once a QAction has been created it should be added to the relevant
    menu and toolbar and then connected to the slot which will perform
    the action. For example:

    \quotefile action/application.cpp
    \skipto Save File
    \printuntil connect

    We create a "Save File" action with a menu text of "&Save" and
    \e{Ctrl+S} as the keyboard accelerator. We connect the
    fileSaveAction's activated() signal to our save() slot. Note that at
    this point there is no menu or toolbar action, we'll add them next:

    \skipto new QToolBar
    \printline
    \skipto fileSaveAction->addTo
    \printline
    \skipto new QPopupMenu
    \printuntil insertItem
    \skipto fileSaveAction->addTo
    \printline

    We create a toolbar and add our fileSaveAction to it. Similarly we
    create a menu, add a top-level menu item, and add our
    fileSaveAction.

  (See the \link simple-application-action.html Simple Application
  Walkthrough featuring QAction \endlink for a detailed example.)

  We recommend that actions are created as children of the window that
  they are used in. In most cases actions will be children of the
  application's main window.

  To prevent recursion don't create an action as a child of a widget
  that the action is later added to.
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
    QKeySequence key;
#ifndef QT_NO_ACCEL
    QAccel* accel;
    int accelid;
#endif
    uint enabled : 1;
    uint toggleaction :1;
    uint on : 1;
#ifndef QT_NO_TOOLTIP
    QToolTipGroup* tipGroup;
#endif

    struct MenuItem {
	MenuItem():popup(0),id(0){}
	QPopupMenu* popup;
	int id;
    };
    // ComboItem is only necessary for actions that are
    // in dropdown/exclusive actiongroups. The actiongroup
    // will clean this up
    struct ComboItem {
	ComboItem():combo(0), id(0) {}
	QComboBox *combo;
	int id;
    };
    QPtrList<MenuItem> menuitems;
    QPtrList<QToolButton> toolbuttons;
    QPtrList<ComboItem> comboitems;

    enum Update { Everything, Icons, State }; // Everything means everything but icons and state
    void update( Update upd = Everything );

    QString menuText() const;
    QString toolTip() const;
    QString statusTip() const;
};

QActionPrivate::QActionPrivate()
{
    iconset = 0;
#ifndef QT_NO_ACCEL
    accel = 0;
    accelid = 0;
#endif
    key = 0;
    enabled = 1;
    toggleaction  = 0;
    on = 0;
    menuitems.setAutoDelete( TRUE );
    comboitems.setAutoDelete( TRUE );
#ifndef QT_NO_TOOLTIP
    tipGroup = new QToolTipGroup( 0 );
#endif
}

QActionPrivate::~QActionPrivate()
{
    QPtrListIterator<QToolButton> ittb( toolbuttons );
    QToolButton *tb;

    while ( ( tb = ittb.current() ) ) {
	++ittb;
	delete tb;
    }

    QPtrListIterator<QActionPrivate::MenuItem> itmi( menuitems);
    QActionPrivate::MenuItem* mi;
    while ( ( mi = itmi.current() ) ) {
	++itmi;
	QPopupMenu* menu = mi->popup;
	if ( menu->findItem( mi->id ) )
	    menu->removeItem( mi->id );
    }

#ifndef QT_NO_ACCEL
    delete accel;
#endif
    delete iconset;
#ifndef QT_NO_TOOLTIP
    delete tipGroup;
#endif
}

void QActionPrivate::update( Update upd )
{
    for ( QPtrListIterator<MenuItem> it( menuitems); it.current(); ++it ) {
	MenuItem* mi = it.current();
	QString t = menuText();
#ifndef QT_NO_ACCEL
	if ( key )
	    t += '\t' + QAccel::keyToString( key );
#endif
	switch ( upd ) {
	case State:
	    mi->popup->setItemEnabled( mi->id, enabled );
	    if ( toggleaction )
		mi->popup->setItemChecked( mi->id, on );
	    break;
	case Icons:
	    if ( iconset )
		mi->popup->changeItem( mi->id, *iconset, t );
	    break;
	default:
	    mi->popup->changeItem( mi->id, t );
	    if ( !whatsthis.isEmpty() )
		mi->popup->setWhatsThis( mi->id, whatsthis );
	    if ( toggleaction ) {
		mi->popup->setCheckable( TRUE );
		mi->popup->setItemChecked( mi->id, on );
	    }
	}
    }
    for ( QPtrListIterator<QToolButton> it2( toolbuttons); it2.current(); ++it2 ) {
	QToolButton* btn = it2.current();
	switch ( upd ) {
	case State:
	    btn->setEnabled( enabled );
	    if ( toggleaction )
		btn->setOn( on );
	    break;
	case Icons:
	    if ( iconset )
		btn->setIconSet( *iconset );
	    break;
	default:
	    btn->setToggleButton( toggleaction );
	    if ( !text.isEmpty() )
		btn->setTextLabel( text, FALSE );
#ifndef QT_NO_TOOLTIP
	    QToolTip::remove( btn );
	    QToolTip::add( btn, toolTip(), tipGroup, statusTip() );
#endif
#ifndef QT_NO_WHATSTHIS
	    QWhatsThis::remove( btn );
	    if ( !whatsthis.isEmpty() )
		QWhatsThis::add( btn, whatsthis );
#endif
	}
    }
    // Only used by actiongroup
    for ( QPtrListIterator<ComboItem> it3( comboitems ); it3.current(); ++it3 ) {
	ComboItem *ci = it3.current();
	if ( !ci->combo )
	    return;
	if ( iconset )
	    ci->combo->changeItem( iconset->pixmap(), text, ci->id );
	else
	    ci->combo->changeItem( text, ci->id );
    }
}

QString QActionPrivate::menuText() const
{
    if ( menutext.isNull() )
	return text;
    return menutext;
}

QString QActionPrivate::toolTip() const
{
    if ( tooltip.isNull() ) {
#ifndef QT_NO_ACCEL
	if ( accel )
	    return text + " (" + QAccel::keyToString( accel->key( accelid )) + ")";
#endif
	return text;
    }
    return tooltip;
}

QString QActionPrivate::statusTip() const
{
    if ( statustip.isNull() )
	return toolTip();
    return statustip;
}



/*! Constructs an action with parent \a parent and name \a name.

  If \a toggle is TRUE the action will be a toggle action otherwise it
  will be a command action.

  If \a parent is a QActionGroup, the new action inserts itself into \a parent.

  Note: for accelerators and status tips to work, \a parent
  must be a widget.
*/
QAction::QAction( QObject* parent, const char* name, bool toggle )
    : QObject( parent, name )
{
    d = new QActionPrivate;
    d->toggleaction = toggle;
    init();
}


/*! This constructor creates an action with the following properties:
  the description \a text, the icon or iconset \a icon, the menu text
  \a menuText and keyboard accelerator \a accel. It is a child of \a parent
  and named \a name. If \a toggle is TRUE the action will be a toggle
  action otherwise it will be a command action.

    The \a parent should be a widget for accelerators and status tips to
    work.

  If  \a parent is a QActionGroup, the action automatically becomes a
  member of it.

  The \a text and \a accel will be used for tool tips and status tips
  unless you provide specific text for these using setToolTip() and
  setStatusTip().
*/
QAction::QAction( const QString& text, const QIconSet& icon, const QString& menuText, QKeySequence accel, QObject* parent, const char* name, bool toggle )
    : QObject( parent, name )
{
    d = new QActionPrivate;
    d->toggleaction = toggle;
    if ( !icon.isNull() )
	setIconSet( icon );

    d->text = text;
    d->menutext = menuText;
    setAccel( accel );
    init();
}

/*! This constructor results in an iconless action with the description
  \a text, the menu text \a menuText and the keyboard accelerator \a accel.
  Its parent is \a parent and its name \a
  name. If \a toggle is TRUE the action will be a toggle
  action otherwise it will be a command action.

  The action automatically becomes a member of \a parent if \a parent
  is a QActionGroup.

    The \a parent should be a widget for accelerators and status tips to
    work.

  The \a text and \a accel will be used for tool tips and status tips
  unless you provide specific text for these using setToolTip() and
  setStatusTip().
*/
QAction::QAction( const QString& text, const QString& menuText, QKeySequence accel, QObject* parent, const char* name, bool toggle )
    : QObject( parent, name )
{
    d = new QActionPrivate;
    d->toggleaction = toggle;
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
    if ( parent() && parent()->inherits("QActionGroup") ) {
	((QActionGroup*) parent())->add( this );		// insert into action group
    }
}

/*! Destroys the object and frees allocated resources. */

QAction::~QAction()
{
    delete d;
}

/*! \property QAction::iconSet
  \brief  the action's icon

  The icon is used as tool button icon and in the menu to the left of
  the menu text.

  (See the action/toggleaction/toggleaction.cpp example.)

*/
void QAction::setIconSet( const QIconSet& icon )
{
    if ( icon.isNull() )
	return;

    register QIconSet *i = d->iconset;
    d->iconset = new QIconSet( icon );
    delete i;
    d->update( QActionPrivate::Icons );
}

QIconSet QAction::iconSet() const
{
    if ( d->iconset )
	return *d->iconset;
    return QIconSet();
}

/*! \property QAction::text
  \brief the action's descriptive text

  If \l QMainWindow::usesTextLabel is TRUE, the text appears as a label in
  the relevant toolbutton. It also serves as the default text
  in menus and tips if these have not been specifically defined.

  \sa setMenuText() setToolTip() setStatusTip()
*/
void QAction::setText( const QString& text )
{
    d->text = text;
    d->update();
}

QString QAction::text() const
{
    return d->text;
}


/*! \property QAction::menuText
  \brief the action's menu text

    If the action is added to a menu the menu option will consist of the
    icon (if there is one), the menu text and the accelerator (if there
    is one). If the menu text is not explicitly set in the constructor
    or using setMenuText() the action's description text will be used as
    the menu text.

  \sa text
*/
void QAction::setMenuText( const QString& text )
{
    d->menutext = text;
    d->update();
}

QString QAction::menuText() const
{
    return d->menuText();
}

/*! \property QAction::toolTip
  \brief the action's tool tip

    This text is used for the tool tip. If no status tip has been set
    the tool tip will be used for the status tip.

    If no tool tip is specified the action's text and accelerator
    description are used as a default tool tip.

  \sa setStatusTip() setAccel()
*/
void QAction::setToolTip( const QString& tip )
{
    d->tooltip = tip;
    d->update();
}

QString QAction::toolTip() const
{
    return d->toolTip();
}

/*! \property QAction::statusTip
  \brief the action's status tip

  The statusTip is displayed on all status bars that the toplevel
  widget parenting this action provides.

  If no status tip is defined, the action uses the tool tip text.

  \sa setStatusTip() setToolTip()
*/
//#### Please reimp for QActionGroup!
//#### For consistency reasons even action groups should show
//#### status tips (as they already do with tool tips)
//#### Please change QActionGroup class doc appropriately after
//#### reimplementation.
void QAction::setStatusTip( const QString& tip )
{
    d->statustip = tip;
    d->update();
}

QString QAction::statusTip() const
{
    return d->statusTip();
}

/*!\property QAction::whatsThis
  \brief the action's "What's This ?" help text

  The whats this text is used to provide a brief description of the
  action. The text may contain rich text (i.e. HTML tags -- see
  QStyleSheet for the list of supported tags).

  \sa QWhatsThis
*/
void QAction::setWhatsThis( const QString& whatsThis )
{
    if ( d->whatsthis == whatsThis )
	return;
    d->whatsthis = whatsThis;
#ifndef QT_NO_ACCEL
    if ( !d->whatsthis.isEmpty() && d->accel )
	d->accel->setWhatsThis( d->accelid, d->whatsthis );
#endif
    d->update();
}

QString QAction::whatsThis() const
{
    return d->whatsthis;
}


/*! \property QAction::accel
  \brief the action's accelerator key

  The keycodes can be found in \l Qt::Key and \l
  Qt::Modifier.


*/
//#### Please reimp for QActionGroup!
//#### For consistency reasons even QActionGroups should respond to
//#### their accelerators and e.g. open the relevant submenu.
//#### Please change appropriate QActionGroup class doc after
//#### reimplementation.
void QAction::setAccel( const QKeySequence& key )
{
    d->key = key;
#ifndef QT_NO_ACCEL
    delete d->accel;
    d->accel = 0;
#endif

    if ( !(int)key ) {
	d->update();
	return;
    }

#ifndef QT_NO_ACCEL
    QObject* p = parent();
    while ( p && !p->isWidgetType() ) {
	p = p->parent();
    }
    if ( p ) {
	d->accel = new QAccel( (QWidget*)p, 0, "qt_action_accel" );
	d->accelid = d->accel->insertItem( d->key );
	d->accel->connectItem( d->accelid, this, SLOT( internalActivation() ) );
	if ( !d->whatsthis.isEmpty() )
	    d->accel->setWhatsThis( d->accelid, d->whatsthis );
    }
#if defined(QT_CHECK_STATE)
    else
	qWarning( "QAction::setAccel()  (%s) requires widget in parent chain.", name( "unnamed" ) );
#endif
#endif
    d->update();
}


QKeySequence QAction::accel() const
{
    return d->key;
}


/*! \property QAction::toggleAction
  \brief whether the action is a toggle action

    A toggle action is one which has an on/off state. For example a Bold
    toolbar button is either on or off. An action which is not a toggle
    action is a command action; a command action is simply executed. For
    example a file open toolbar button would invoke a file open dialog.

  For exclusive toggling, add toggle actions to
  a QActionGroup with the \l QActionGroup::exclusive property set to TRUE.

*/
void QAction::setToggleAction( bool enable )
{
    if ( enable == (bool)d->toggleaction )
	return;
    d->toggleaction = enable;
    d->update();
}

bool QAction::isToggleAction() const
{
    return d->toggleaction;
}

/*!
  \property QAction::on
  \brief whether a toggle action is on

  This property is always on for command actions and QActionGroups.
  setOn() has no effect on them.

  \sa toggleAction
*/
void QAction::setOn( bool enable )
{
    if ( !isToggleAction() ) {
#if defined(QT_CHECK_STATE)
	qWarning( "QAction::setOn() (%s) Only toggle actions "
		  "may be switched", name( "unnamed" ) );
#endif
	return;
    }
    if ( enable == (bool)d->on )
	return;
    d->on = enable;
    d->update( QActionPrivate::State );
    emit toggled( enable );
}

bool QAction::isOn() const
{
    return d->on;
}

/*! \property QAction::enabled
  \brief whether the action is enabled

  Disabled actions can't be chosen by the user. They don't
  disappear from the menu/tool bar but are displayed in a way which
  indicates that they are unavailable, e.g. they might be displayed
  greyed out.

  What's this? help on disabled actions is still available
  provided the \l QAction::whatsThis property is set.

*/
void QAction::setEnabled( bool enable )
{
    d->enabled = enable;
#ifndef QT_NO_ACCEL
    if ( d->accel )
	d->accel->setEnabled( enable );
#endif
    d->update( QActionPrivate::State );
}

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

  Currently actions may be added to QToolBar and QPopupMenu widgets.

  An action added to a tool bar is automatically displayed
  as a tool button; an action added to a pop up menu appears
  as a menu option.

  addTo() returns TRUE if the action was added successfully and FALSE
  otherwise. (If \a w is not a QToolBar or QPopupMenu the action will
  not be added and FALSE will be returned.)

  \sa removeFrom()
*/
bool QAction::addTo( QWidget* w )
{
#ifndef QT_NO_TOOLBAR
    if ( w->inherits( "QToolBar" ) ) {
	if ( !qstrcmp( name(), "qt_separator_action" ) ) {
	    ((QToolBar*)w)->addSeparator();
	} else {
	    QCString bname = name() + QCString( "_action_button" );
	    QToolButton* btn = new QToolButton( (QToolBar*) w, bname );
	    addedTo( btn, w );
	    btn->setToggleButton( d->toggleaction );
	    d->toolbuttons.append( btn );
	    if ( d->iconset )
		btn->setIconSet( *d->iconset );
	    d->update( QActionPrivate::State );
	    d->update( QActionPrivate::Everything );
	    connect( btn, SIGNAL( clicked() ), this, SIGNAL( activated() ) );
	    connect( btn, SIGNAL( toggled(bool) ), this, SLOT( toolButtonToggled(bool) ) );
	    connect( btn, SIGNAL( destroyed() ), this, SLOT( objectDestroyed() ) );
#ifndef QT_NO_TOOLTIP
	    connect( d->tipGroup, SIGNAL(showTip(const QString&)), this, SLOT(showStatusText(const QString&)) );
	    connect( d->tipGroup, SIGNAL(removeTip()), this, SLOT(clearStatusText()) );
#endif
	}
    } else
#endif
    if ( w->inherits( "QPopupMenu" ) ) {
	if ( !qstrcmp( name(), "qt_separator_action" ) ) {
	    ((QPopupMenu*)w)->insertSeparator();
	} else {
	    QActionPrivate::MenuItem* mi = new QActionPrivate::MenuItem;
	    mi->popup = (QPopupMenu*) w;
	    QIconSet* diconset = d->iconset;
	    if ( diconset )
		mi->id = mi->popup->insertItem( *diconset, QString::fromLatin1("") );
	    else
		mi->id = mi->popup->insertItem( QString::fromLatin1("") );
	    addedTo( mi->popup->indexOf( mi->id ), mi->popup );
	    mi->popup->connectItem( mi->id, this, SLOT(internalActivation()) );
	    d->menuitems.append( mi );
	    d->update( QActionPrivate::State );
	    d->update( QActionPrivate::Everything );
	    w->topLevelWidget()->className();
	    connect( mi->popup, SIGNAL(highlighted( int )), this, SLOT(menuStatusText( int )) );
	    connect( mi->popup, SIGNAL(aboutToHide()), this, SLOT(clearStatusText()) );
	    connect( mi->popup, SIGNAL( destroyed() ), this, SLOT( objectDestroyed() ) );
	}
    // Makes only sense when called by QActionGroup::addTo
    } else if ( w->inherits( "QComboBox" ) ) {
	if ( qstrcmp( name(), "qt_separator_action" ) ) {
	    QActionPrivate::ComboItem *ci = new QActionPrivate::ComboItem;
	    ci->combo = (QComboBox*)w;
	    connect( ci->combo, SIGNAL( destroyed() ), this, SLOT( objectDestroyed() ) );
	    ci->id = ci->combo->count();
	    if ( d->iconset )
		ci->combo->insertItem( d->iconset->pixmap(), text() );
	    else
		ci->combo->insertItem( text() );
	    d->comboitems.append( ci );
	}
    } else {
	qWarning( "QAction::addTo(), unknown object" );
	return FALSE;
    }
    return TRUE;
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

/*! Sets the status message to \a text */
void QAction::showStatusText( const QString& text )
{
#ifndef QT_NO_STATUSBAR
    QObject* par;
    if ( ( par = parent() ) && par->inherits( "QActionGroup" ) )
	par = par->parent();
    if ( !par || !par->isWidgetType() )
	return;
    QStatusBar *bar = 0;
    bar = (QStatusBar*)( (QWidget*)par )->topLevelWidget()->child( 0, "QStatusBar", FALSE );
    if ( !bar ) {
	QObjectList *l = ( (QWidget*)par )->topLevelWidget()->queryList( "QStatusBar" );
	if ( !l )
	    return;
	// #### hopefully the last one is the one of the mainwindow...
	bar = (QStatusBar*)l->last();
	delete l;
    }
    if ( bar ) {
	if ( text.isEmpty() )
	    bar->clear();
	else
	    bar->message( text );
    }
#endif
}

/*! Sets the status message to the menu item's status text, or
  to the tooltip, if there is no status text.
*/
void QAction::menuStatusText( int id )
{
    QString text;
    QPtrListIterator<QActionPrivate::MenuItem> it( d->menuitems);
    QActionPrivate::MenuItem* mi;
    while ( ( mi = it.current() ) ) {
	++it;
	if ( mi->id == id ) {
	    text = statusTip();
	    break;
	}
    }

    if ( !text.isEmpty() )
	showStatusText( text );
}

/*! Clears the status text.
*/
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
#ifndef QT_NO_TOOLBAR
    if ( w->inherits( "QToolBar" ) ) {
	QPtrListIterator<QToolButton> it( d->toolbuttons);
	QToolButton* btn;
	while ( ( btn = it.current() ) ) {
	    ++it;
	    if ( btn->parentWidget() == w ) {
		d->toolbuttons.removeRef( btn );
		disconnect( btn, SIGNAL( destroyed() ), this, SLOT( objectDestroyed() ) );
		delete btn;
		// no need to disconnect from statusbar
	    }
	}
    } else
#endif
    if ( w->inherits( "QPopupMenu" ) ) {
	QPtrListIterator<QActionPrivate::MenuItem> it( d->menuitems);
	QActionPrivate::MenuItem* mi;
	while ( ( mi = it.current() ) ) {
	    ++it;
	    if ( mi->popup == w ) {
		disconnect( mi->popup, SIGNAL(highlighted( int )), this, SLOT(menuStatusText(int)) );
		disconnect( mi->popup, SIGNAL(aboutToHide()), this, SLOT(clearStatusText()) );
		disconnect( mi->popup, SIGNAL( destroyed() ), this, SLOT( objectDestroyed() ) );
		mi->popup->removeItem( mi->id );
		d->menuitems.removeRef( mi );
	    }
	}
    } else if ( w->inherits( "QComboBox" ) ) {
	QPtrListIterator<QActionPrivate::ComboItem> it( d->comboitems );
	QActionPrivate::ComboItem *ci;
	while ( ( ci = it.current() ) ) {
	    ++it;
	    if ( ci->combo == w ) {
		disconnect( ci->combo, SIGNAL(destroyed()), this, SLOT(objectDestroyed()) );
		d->comboitems.removeRef( ci );
	    }
	}
    } else {
	qWarning( "QAction::removeFrom(), unknown object" );
	return FALSE;
    }
    return TRUE;
}

/*!
  \internal
*/
void QAction::objectDestroyed()
{
    const QObject* obj = sender();
    QPtrListIterator<QActionPrivate::MenuItem> it( d->menuitems );
    QActionPrivate::MenuItem* mi;
    while ( ( mi = it.current() ) ) {
	++it;
	if ( mi->popup == obj )
	    d->menuitems.removeRef( mi );
    }
    QActionPrivate::ComboItem *ci;
    QPtrListIterator<QActionPrivate::ComboItem> it2( d->comboitems );
    while ( ( ci = it2.current() ) ) {
	++it2;
	if ( ci->combo == obj )
	    d->comboitems.removeRef( ci );
    }
    d->toolbuttons.removeRef( (QToolButton*) obj );
}

/*! \fn void QAction::activated()

  This signal is emitted when an action is activated by the user, i.e.
  when the user clicks a menu option or a toolbar button or presses an
  action's accelerator key combination.

  Connect to this signal for command actions. Connect to the toggled()
  signal for toggle actions.
*/

/*! \fn void QAction::toggled(bool)

  This signal is emitted when a toggle action changes state;
  command actions and QActionGroups don't emit toggled().

  The argument denotes the new state; i.e. TRUE
  if the toggle action was switched on and FALSE if
  it was switched off.

  To trigger a user command depending on whether a toggle action has
  been switched on or off connect it to a slot that takes a bool to
  indicate the state, e.g.

  \quotefile action/toggleaction/toggleaction.cpp
  \skipto QMainWindow * window
  \printline QMainWindow * window
  \skipto labelonoffaction
  \printline labelonoffaction
  \skipto connect
  \printuntil setUsesTextLabel

  \sa activated() setToggleAction() setOn()
*/



class QActionGroupPrivate
{
public:
    uint exclusive: 1;
    uint dropdown: 1;
    QPtrList<QAction> actions;
    QAction* selected;
    QAction* separatorAction;

    struct MenuItem {
	MenuItem():popup(0),id(0){}
	QPopupMenu* popup;
	int id;
    };

    QPtrList<QComboBox> comboboxes;
    QPtrList<QToolButton> menubuttons;
    QPtrList<MenuItem> menuitems;
    QPtrList<QPopupMenu> popupmenus;

    void update( const QActionGroup * );
};

void QActionGroupPrivate::update( const QActionGroup* that )
{
    for ( QPtrListIterator<QAction> it( actions ); it.current(); ++it ) {
	it.current()->setEnabled( that->isEnabled() );
    }
    for ( QPtrListIterator<QComboBox> cb( comboboxes ); cb.current(); ++cb ) {
	cb.current()->setEnabled( that->isEnabled() );

#ifndef QT_NO_TOOLTIP
	QToolTip::remove( cb.current() );
	if ( !!that->toolTip() )
	    QToolTip::add( cb.current(), that->toolTip() );
#endif
#ifndef QT_NO_WHATSTHIS
	QWhatsThis::remove( cb.current() );
	if ( !!that->whatsThis() )
	    QWhatsThis::add( cb.current(), that->whatsThis() );
#endif
    }
    for ( QPtrListIterator<QToolButton> mb( menubuttons ); mb.current(); ++mb ) {
	mb.current()->setEnabled( that->isEnabled() );

	if ( !that->text().isNull() )
	    mb.current()->setTextLabel( that->text() );
	if ( !that->iconSet().isNull() )
	    mb.current()->setIconSet( that->iconSet() );

#ifndef QT_NO_TOOLTIP
	QToolTip::remove( mb.current() );
	if ( !!that->toolTip() )
	    QToolTip::add( mb.current(), that->toolTip() );
#endif
#ifndef QT_NO_WHATSTHIS
	QWhatsThis::remove( mb.current() );
	if ( !!that->whatsThis() )
	    QWhatsThis::add( mb.current(), that->whatsThis() );
#endif
    }
    for ( QPtrListIterator<QActionGroupPrivate::MenuItem> pu( menuitems ); pu.current(); ++pu ) {
	QWidget* parent = pu.current()->popup->parentWidget();
	if ( parent->inherits( "QPopupMenu" ) ) {
	    QPopupMenu* ppopup = (QPopupMenu*)parent;
	    ppopup->setItemEnabled( pu.current()->id, that->isEnabled() );
	} else {
	    pu.current()->popup->setEnabled( that->isEnabled() );
	}
    }
    for ( QPtrListIterator<QPopupMenu> pm( popupmenus ); pm.current(); ++pm ) {
	QPopupMenu *popup = pm.current();
	QPopupMenu *parent = popup->parentWidget()->inherits( "QPopupMenu" ) ? (QPopupMenu*)popup->parentWidget() : 0;
	if ( !parent )
	    continue;

	int index;
	parent->findPopup( popup, &index );
	int id = parent->idAt( index );
	parent->changeItem( id, that->iconSet(), that->menuText() );
	parent->setItemEnabled( id, that->isEnabled() );
	parent->setAccel( that->accel(), id );
    }
}

/*! \class QActionGroup qaction.h
    \ingroup basic
    \ingroup application

  \brief The QActionGroup class groups actions together.

    In some situations it is useful to group actions together. For
    example, if you have a left justify action, a right justify action
    and a center action, only one of these actions should be active at
    any one time, and one simple way of achieving this is to group the
    actions together in an action group and setExclusive(TRUE).

    An action group can also be added to a menu or a toolbar as a single
    unit, with all the actions within the action group appearing as
    separate menu options and toolbar buttons.

    Here's an example from examples/textedit:
    \quotefile textedit/textedit.cpp
    \skipto QActionGroup
    \printuntil connect

    We create a new action  group, call setExclusive() to ensure that
    only one of the actions in the group is ever active at any one time.
    We then connect the group to our textAlign() slot.

    \printuntil actionAlignLeft->setToggleAction

    We create a left align action, add it to the toolbar and the menu
    and make it a toggle action. We create center and right align
    actions in exactly the same way.

    A QActionGroup emits an activated() signal when one of its actions
    is activated. The actions in an action group emit their activated()
    (and for toggle actions, toggled()) signals as usual.

    The setExclusive() function is used to ensure that only one action
    is active at any one time: it should be used with actions which have
    their toggleAction set to TRUE.

    Action group actions appear as individual menu options and toolbar
    buttons. For exclusive action groups use setUsesDropDown() to
    display the actions in a subwidget of any widget the action group is
    added to. For example, the actions would appear in a combobox in a
    toolbar or as a submenu in a menu.

    Actions can be added to an action group using add(), but normally
    they are added by creating the action with the action group as
    parent. Actions can have separators dividing them using
    addSeparator(). Action groups are added to widgets with addTo().

*/

/*! Constructs an action group with parent \a parent and name \a name.

    If \a exclusive is TRUE only one toggle action in the group will
    ever be active.

*/
QActionGroup::QActionGroup( QObject* parent, const char* name, bool exclusive )
    : QAction( parent, name )
{
    d = new QActionGroupPrivate;
    d->exclusive = exclusive;
    d->dropdown = FALSE;
    d->selected = 0;
    d->separatorAction = 0;

    connect( this, SIGNAL(selected(QAction*)), SLOT(internalToggle(QAction*)) );
}

/*! Destroys the object and frees allocated resources. */

QActionGroup::~QActionGroup()
{
    QPtrListIterator<QActionGroupPrivate::MenuItem> mit( d->menuitems );
    while ( mit.current() ) {
	QActionGroupPrivate::MenuItem *mi = mit.current();
	++mit;
	if ( mi->popup )
	    mi->popup->disconnect( SIGNAL(destroyed()), this, SLOT(objectDestroyed()) );
    }

    QPtrListIterator<QComboBox> cbit( d->comboboxes );
    while ( cbit.current() ) {
	QComboBox *cb = cbit.current();
	++cbit;
	cb->disconnect(  SIGNAL(destroyed()), this, SLOT(objectDestroyed()) );
    }
    QPtrListIterator<QToolButton> mbit( d->menubuttons );
    while ( mbit.current() ) {
	QToolButton *mb = mbit.current();
	++mbit;
	mb->disconnect(  SIGNAL(destroyed()), this, SLOT(objectDestroyed()) );
    }
    QPtrListIterator<QPopupMenu> pmit( d->popupmenus );
    while ( pmit.current() ) {
	QPopupMenu *pm = pmit.current();
	++pmit;
	pm->disconnect(  SIGNAL(destroyed()), this, SLOT(objectDestroyed()) );
    }

    delete d->separatorAction;
    d->menubuttons.setAutoDelete( TRUE );
    d->comboboxes.setAutoDelete( TRUE );
    d->menuitems.setAutoDelete( TRUE );
    d->popupmenus.setAutoDelete( TRUE );
    delete d;
}

/*! \property QActionGroup::exclusive
  \brief whether the action group does exclusive toggling

    If exclusive is TRUE only one toggle action in the action group can
    ever be active at any one time. If the user chooses another toggle
    action in the group the one they chose becomes active and the one
    that was active becomes inactive.

  \sa QAction::toggleAction
*/
void QActionGroup::setExclusive( bool enable )
{
    d->exclusive = enable;
}

bool QActionGroup::isExclusive() const
{
    return d->exclusive;
}

/*!  \property QActionGroup::usesDropDown
  \brief whether the group's actions are displayed in a
  subwidget of the widgets the action group is added to

  Exclusive action groups added to a toolbar display their actions in
  a combobox with the action's \l QAction::text and \l
  QAction::iconSet properties shown. Non-exclusive groups are
  represented by a tool button showing their \l QAction::iconSet and
  -- depending on \l QMainWindow::usesTextLabel() -- text() property.

  In a popup menu the member actions are displayed in a
  submenu.

  Changing usesDropDown effects subsequent calls to addTo() only.

*/
void QActionGroup::setUsesDropDown( bool enable )
{
    d->dropdown = enable;
}

bool QActionGroup::usesDropDown() const
{
    return d->dropdown;
}

/*! Adds action \a action to this group.

    Normally an action is added to a group by creating it with the group
    as parent, so this function is not usually used.

  \sa addTo()
*/
void QActionGroup::add( QAction* action )
{
    if ( d->actions.containsRef( action ) )
	return;

    d->actions.append( action );

    if ( action->whatsThis().isNull() )
	action->setWhatsThis( whatsThis() );
    if ( action->toolTip().isNull() )
	action->setToolTip( toolTip() );
    action->setEnabled( isEnabled() );

    connect( action, SIGNAL( destroyed() ), this, SLOT( childDestroyed() ) );
    connect( action, SIGNAL( activated() ), this, SIGNAL( activated() ) );
    connect( action, SIGNAL( toggled( bool ) ), this, SLOT( childToggled( bool ) ) );

    for ( QPtrListIterator<QComboBox> cb( d->comboboxes ); cb.current(); ++cb ) {
	cb.current()->insertItem( action->iconSet().pixmap(), action->text() );
    }
    for ( QPtrListIterator<QToolButton> mb( d->menubuttons ); mb.current(); ++mb ) {
	QPopupMenu* popup = mb.current()->popup();
	if ( !popup )
	    continue;
	action->addTo( popup );
    }
    for ( QPtrListIterator<QActionGroupPrivate::MenuItem> mi( d->menuitems ); mi.current(); ++mi ) {
	QPopupMenu* popup = mi.current()->popup;
	if ( !popup )
	    continue;
	action->addTo( popup );
    }
}

/*! Adds a separator to the group. */
void QActionGroup::addSeparator()
{
    if ( !d->separatorAction )
	d->separatorAction = new QAction( 0, "qt_separator_action" );
    d->actions.append( d->separatorAction );
}


/*! \fn void QActionGroup::insert( QAction* a )

  \obsolete

  Use add() instead, or better still create the action with the action
  group as its parent.
 */

/*! Adds this action group to the widget \a w.

    If usesDropDown() is TRUE and exclusive is TRUE (see setExclusive())
    the actions are presented in a combobox if \a w is a toolbar and as
    a submenu if \a w is a menu. Otherwise (the default) the actions
    within the group are added to the widget individually, for example
    if the widget is a menu the actions will appear as individual menu
    options and if the widget is a toolbar the actions will appear as
    toolbar buttons.

    It is recommended that actions is action groups, especially where
    usesDropDown() is TRUE, have their menuText() or text() property set.

    All actions should be added to the action group \e before the action
    group is added to the widget. If actions are added to the action
    group \e after the action group has been added to the widget these
    later actions will \e not appear.

  \sa setExclusive() setUsesDropDown() removeFrom()
*/
bool QActionGroup::addTo( QWidget* w )
{
#ifndef QT_NO_TOOLBAR
    if ( w->inherits( "QToolBar" ) ) {
	if ( d->dropdown ) {
	    if ( !d->exclusive ) {
		QPtrListIterator<QAction> it( d->actions);
		if ( !it.current() )
		    return TRUE;

		QAction *defAction = it.current();

		QToolButton* btn = new QToolButton( (QToolBar*) w, "qt_actiongroup_btn" );
		addedTo( btn, w );
		connect( btn, SIGNAL(destroyed()), SLOT(objectDestroyed()) );
		d->menubuttons.append( btn );

		if ( !iconSet().isNull() )
		    btn->setIconSet( iconSet() );
		else if ( !defAction->iconSet().isNull() )
		    btn->setIconSet( defAction->iconSet() );
		if ( !!text() )
		    btn->setTextLabel( text() );
		else if ( !!defAction->text() )
		    btn->setTextLabel( defAction->text() );
#ifndef QT_NO_TOOLTIP
		if ( !!toolTip() )
		    QToolTip::add( btn, toolTip() );
		else if ( !!defAction->toolTip() )
		    QToolTip::add( btn, defAction->toolTip() );
#endif
#ifndef QT_NO_WHATSTHIS
		if ( !!whatsThis() )
		    QWhatsThis::add( btn, whatsThis() );
		else if ( !!defAction->whatsThis() )
		    QWhatsThis::add( btn, defAction->whatsThis() );
#endif

		connect( btn, SIGNAL( clicked() ), defAction, SIGNAL( activated() ) );
		connect( btn, SIGNAL( toggled(bool) ), defAction, SLOT( toolButtonToggled(bool) ) );
		connect( btn, SIGNAL( destroyed() ), defAction, SLOT( objectDestroyed() ) );

		QPopupMenu *menu = new QPopupMenu( btn, "qt_actiongroup_menu" );
		btn->setPopupDelay( 0 );
		btn->setPopup( menu );

		while( it.current() ) {
		    it.current()->addTo( menu );
		    ++it;
		}
		return TRUE;
	    } else {
		QComboBox *box = new QComboBox( FALSE, w, "qt_actiongroup_combo" );
		addedTo( box, w );
		connect( box, SIGNAL(destroyed()), SLOT(objectDestroyed()) );
		d->comboboxes.append( box );
#ifndef QT_NO_TOOLTIP
		if ( !!toolTip() )
		    QToolTip::add( box, toolTip() );
#endif
#ifndef QT_NO_WHATSTHIS
		if ( !!whatsThis() )
		    QWhatsThis::add( box, whatsThis() );
#endif

		for ( QPtrListIterator<QAction> it( d->actions); it.current(); ++it ) {
		    it.current()->addTo( box );
		}
		connect( box, SIGNAL(activated(int)), this, SLOT( internalComboBoxActivated(int)) );
		return TRUE;
	    }
	}
    } else
#endif
    if ( w->inherits( "QPopupMenu" ) ) {
	QPopupMenu *popup;
	if ( d->dropdown ) {
	    QPopupMenu *menu = (QPopupMenu*)w;
	    popup = new QPopupMenu( w, "qt_actiongroup_menu" );
	    d->popupmenus.append( popup );
	    connect( popup, SIGNAL(destroyed()), SLOT(objectDestroyed()) );

	    int id;
	    if ( !iconSet().isNull() ) {
		if ( menuText().isEmpty() )
		    id = menu->insertItem( iconSet(), text(), popup );
		else
		    id = menu->insertItem( iconSet(), menuText(), popup );
	    } else {
		if ( menuText().isEmpty() )
		    id = menu->insertItem( text(), popup );
		else
		    id = menu->insertItem( menuText(), popup );
	    }

	    addedTo( menu->indexOf( id ), menu );

	    QActionGroupPrivate::MenuItem *item = new QActionGroupPrivate::MenuItem;
	    item->id = id;
	    item->popup = popup;
	    d->menuitems.append( item );
	} else {
	    popup = (QPopupMenu*)w;
	}
	for ( QPtrListIterator<QAction> it( d->actions); it.current(); ++it ) {
	    // #### do an addedTo( index, popup, action), need to find out index
	    it.current()->addTo( popup );
	}
	return TRUE;
    }

    for ( QPtrListIterator<QAction> it( d->actions); it.current(); ++it ) {
	// #### do an addedTo( index, popup, action), need to find out index
	it.current()->addTo( w );
    }

    return TRUE;
}

/*! \reimp
*/
bool QActionGroup::removeFrom( QWidget* w )
{
    for ( QPtrListIterator<QAction> it( d->actions); it.current(); ++it ) {
	it.current()->removeFrom( w );
    }

#ifndef QT_NO_TOOLBAR
    if ( w->inherits( "QToolBar" ) ) {
	QPtrListIterator<QComboBox> cb( d->comboboxes );
	while( cb.current() ) {
	    QComboBox *box = cb.current();
	    ++cb;
	    if ( box->parentWidget() == w )
		delete box;
	}
	QPtrListIterator<QToolButton> mb( d->menubuttons );
	while( mb.current() ) {
	    QToolButton *btn = mb.current();
	    ++mb;
	    if ( btn->parentWidget() == w )
		delete btn;
	}
    } else
#endif
    if ( w->inherits( "QPopupMenu" ) ) {
	QPtrListIterator<QActionGroupPrivate::MenuItem> pu( d->menuitems );
	while ( pu.current() ) {
	    QActionGroupPrivate::MenuItem *mi = pu.current();
	    ++pu;
	    if ( d->dropdown && mi->popup )
		( (QPopupMenu*)w )->removeItem( mi->id );
	    delete mi->popup;
	}
    }

    return TRUE;
}

/*! \internal
*/
void QActionGroup::childToggled( bool b )
{
    if ( !isExclusive() )
	return;
    QAction* s = (QAction*) sender();
    if ( b ) {
	if ( s != d->selected ) {
	    d->selected = s;
	    for ( QPtrListIterator<QAction> it( d->actions); it.current(); ++it ) {
		if ( it.current()->isToggleAction() && it.current() != s )
		    it.current()->setOn( FALSE );
	    }
	    emit activated();
	    emit selected( s );
	}
    } else {
	if ( s == d->selected ) {
	    // at least one has to be selected
	    s->setOn( TRUE );
	}
    }
}

/*! \internal
*/
void QActionGroup::childDestroyed()
{
    d->actions.removeRef( (QAction*) sender() );
    if ( d->selected == sender() )
	d->selected = 0;
}

/*! \reimp
*/
void QActionGroup::setEnabled( bool enable )
{
    if ( enable == isEnabled() )
	return;

    QAction::setEnabled( enable );
    d->update( this );
}

/*! \reimp
*/
void QActionGroup::setIconSet( const QIconSet& icon )
{
    QAction::setIconSet( icon );
    d->update( this );
}

/*! \reimp
*/
void QActionGroup::setText( const QString& txt )
{
    if ( txt == text() )
	return;

    QAction::setText( txt );
    d->update( this );
}

/*! \reimp
*/
void QActionGroup::setMenuText( const QString& text )
{
    if ( text == menuText() )
	return;

    QAction::setMenuText( text );
    d->update( this );
}

/*! \reimp
*/
void QActionGroup::setToolTip( const QString& text )
{
    if ( text == toolTip() )
	return;
    for ( QPtrListIterator<QAction> it( d->actions); it.current(); ++it ) {
	if ( it.current()->toolTip().isNull() )
	    it.current()->setToolTip( text );
    }
    QAction::setToolTip( text );
    d->update( this );
}

/*! \reimp
*/
void QActionGroup::setWhatsThis( const QString& text )
{
    if ( text == whatsThis() )
	return;
    for ( QPtrListIterator<QAction> it( d->actions); it.current(); ++it ) {
	if ( it.current()->whatsThis().isNull() )
	    it.current()->setWhatsThis( text );
    }
    QAction::setWhatsThis( text );
    d->update( this );
}

/*! \reimp
*/
void QActionGroup::childEvent( QChildEvent *e )
{
    if ( !e->child()->inherits( "QAction" ) )
	return;

    QAction *action = (QAction*)e->child();

    if ( !e->removed() )
	return;

    for ( QPtrListIterator<QComboBox> cb( d->comboboxes ); cb.current(); ++cb ) {
	for ( int i = 0; i < cb.current()->count(); i++ ) {
	    if ( cb.current()->text( i ) == action->text() ) {
		cb.current()->removeItem( i );
		break;
	    }
	}
    }
    for ( QPtrListIterator<QToolButton> mb( d->menubuttons ); mb.current(); ++mb ) {
	QPopupMenu* popup = mb.current()->popup();
	if ( !popup )
	    continue;
	action->removeFrom( popup );
    }
    for ( QPtrListIterator<QActionGroupPrivate::MenuItem> mi( d->menuitems ); mi.current(); ++mi ) {
	QPopupMenu* popup = mi.current()->popup;
	if ( !popup )
	    continue;
	action->removeFrom( popup );
    }
}

/*! \fn void QActionGroup::selected( QAction* )

  This signal is emitted from exclusive groups when toggle actions
  change state.

  The argument is the action whose state changed to "on".

  \quotefile action/actiongroup/editor.cpp
  \skipto QActionGroup
  \printline QActionGroup
  \skipto QObject::connect
  \printuntil SLOT

  In this example we connect the selected() signal to our setFontColor()
  slot, passing the QAction so that we know which action was chosen by
  the user.

  (See the \link actiongroup.html QActionGroup Walkthrough. \endlink)

  \sa setExclusive(), isOn()
*/

/*! \internal
*/
void QActionGroup::internalComboBoxActivated( int index )
{
    QAction *a = d->actions.at( index );
    if ( a ) {
	if ( a != d->selected ) {
	    d->selected = a;
	    for ( QPtrListIterator<QAction> it( d->actions); it.current(); ++it ) {
		if ( it.current()->isToggleAction() && it.current() != a )
		    it.current()->setOn( FALSE );
	    }
	    if ( a->isToggleAction() )
		a->setOn( TRUE );

	    emit activated();
	    emit selected( d->selected );
	    emit ((QActionGroup*)a)->activated();
	}
    }
}

/*! \internal
*/
void QActionGroup::internalToggle( QAction *a )
{
    for ( QPtrListIterator<QComboBox> it( d->comboboxes); it.current(); ++it ) {
	int index = d->actions.find( a );
	if ( index != -1 )
	    it.current()->setCurrentItem( index );
    }
}

/*! \internal
*/
void QActionGroup::objectDestroyed()
{
    const QObject* obj = sender();
    d->menubuttons.removeRef( (QToolButton*)obj );
    for ( QPtrListIterator<QActionGroupPrivate::MenuItem> mi( d->menuitems ); mi.current(); ++mi ) {
	if ( mi.current()->popup == obj ) {
	    d->menuitems.removeRef( mi.current() );
	    break;
	}
    }
    d->popupmenus.removeRef( (QPopupMenu*)obj );
    d->comboboxes.removeRef( (QComboBox*)obj );
}

/*! This function is called from the addTo() function when it created
  a widget (\a actionWidget) for the child action \a a in the \a
  container.
*/

void QActionGroup::addedTo( QWidget *actionWidget, QWidget *container, QAction *a )
{
    Q_UNUSED( actionWidget );
    Q_UNUSED( container );
    Q_UNUSED( a );
}

/*! \overload

  This function is called from the addTo() function when it created a
  menu item for the child action at the index \a index in the popup
  menu \a menu.
*/

void QActionGroup::addedTo( int index, QPopupMenu *menu, QAction *a )
{
    Q_UNUSED( index );
    Q_UNUSED( menu );
    Q_UNUSED( a );
}

/*! \reimp
    \overload
  This function is called from the addTo() function when it created
  a widget (\a actionWidget) in the \a container.
*/

void QActionGroup::addedTo( QWidget *actionWidget, QWidget *container )
{
    Q_UNUSED( actionWidget );
    Q_UNUSED( container );
}

/*! \reimp
    \overload
  This function is called from the addTo() function when it created a
  menu item at the index \a index in the popup menu \a menu.

*/

void QActionGroup::addedTo( int index, QPopupMenu *menu )
{
    Q_UNUSED( index );
    Q_UNUSED( menu );
}

#endif
