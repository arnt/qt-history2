/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qtoolbar.cpp#56 $
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

  \brief The QAction class provides an abstract user interface action that can
  appear both in menus and tool bars.

  In modern GUIs many user commands can be invoked via a menu entry, a
  tool button and/or a keyboard accelerator.
  To make the program less error-prone it is advisable to not
  implement all these incidences separately but use an abstraction that ties
  together all invokation methods. These abstractions are called \e actions.

  A QAction may contain items like an icon set, an appropriate menu text,
  an accelerator, a help text, a tool tip etc. that \e represent the user command.
  These items are referred to with the term \e properties. Although the term
  \e action might suggest so they do not include the function to be invoked itself.
  On user interaction QActions emit signals that should be used to tie the
  functionality to the user interface.

  There are two basic kinds of user interface actions, command actions
  and toggle actions. The former triggers the execution of a command (for example
  "open file"). To do this it emits the activated() signal when the user
  invokes the command. The application
  programmer is responsible for connecting activated() to a slot that executes
  the respective function.

  Toggle actions on the other hand give the user the possibility to opt on/off
  for a certain tool or modus. The drawing tools in a
  paint program or the setting of font characteristics like bold/underlined/italics
  in a text processor are examples that should be represented by toggle actions.
  A toggle action emits a toggled() signal whenever it changes state.

  Whether an action is a command action or a toggle action can be defined via
  the setToggleAction() function.

  To insert an action into a menu or a tool bar, use addTo().
  It will appear as either a menu entry or a tool button:

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

  (Refer to the \link simple-application-action.html Simple Application Walkthrough
  featuring QAction \endlink for a detailed explanation of the above code.)

  You can add an action to an arbitrary number of menus and toolbars
  and remove it again with removeFrom().
  Several actions can be combined in a QActionGroup.

  Changing an action's properties using one of the set-functions shows instant
  effect on all representations. Whilst properties like isEnabled(), text(),
  menuText(), toolTip(), statusTip(), accel(), iconSet() and
  whatsThis() are equally useful
  for both command and toggle actions, the isOn() property does show effect
  only when isToggleAction() returns TRUE.

  Since accelerators and status tips are window specific, the application
  window has to be an ancestor of an action with accel() or statusTip()
  holding TRUE. Therefore it is a good idea to always create actions as direct
  children of the main window.

  To prevent recursions it is advisable to never create an action as a child of a widget
  it is added to later.
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
    uint enabled : 1;
    uint toggleaction :1;
    uint on : 1;
    QToolTipGroup* tipGroup;

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
    QList<MenuItem> menuitems;
    QList<QToolButton> toolbuttons;
    QList<ComboItem> comboitems;

    enum Update { Everything, Icons, State }; // Everything means everything but icons and state
    void update( Update upd = Everything );

    QString menuText() const;
    QString toolTip() const;
    QString statusTip() const;
};

QActionPrivate::QActionPrivate()
{
    iconset = 0;
    accel = 0;
    accelid = 0;
    key = 0;
    enabled = 1;
    toggleaction  = 0;
    on = 0;
    menuitems.setAutoDelete( TRUE );
    tipGroup = new QToolTipGroup( 0 );
}

QActionPrivate::~QActionPrivate()
{
    QListIterator<QToolButton> ittb( toolbuttons );
    QToolButton *tb;

    while ( ( tb = ittb.current() ) ) {
	++ittb;
	delete tb;
    }

    QListIterator<QActionPrivate::MenuItem> itmi( menuitems);
    QActionPrivate::MenuItem* mi;
    while ( ( mi = itmi.current() ) ) {
	++itmi;
	QPopupMenu* menu = mi->popup;
	if ( menu->findItem( mi->id ) )
	    menu->removeItem( mi->id );
    }

    delete accel;
    delete iconset;
    delete tipGroup;
}

void QActionPrivate::update( Update upd )
{
    for ( QListIterator<MenuItem> it( menuitems); it.current(); ++it ) {
	MenuItem* mi = it.current();
	QString t = menuText();
	if ( key )
	    t += '\t' + QAccel::keyToString( key );
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
    for ( QListIterator<QToolButton> it2( toolbuttons); it2.current(); ++it2 ) {
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
	    QToolTip::remove( btn );
	    QToolTip::add( btn, toolTip(), tipGroup, statusTip() );
	    QWhatsThis::remove( btn );
	    if ( !whatsthis.isEmpty() )
		QWhatsThis::add( btn, whatsthis );
	}
    }
    // Only used by actiongroup
    for ( QListIterator<ComboItem> it3( comboitems ); it3.current(); ++it3 ) {
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
	if ( accel )
	    return text + " (" + QAccel::keyToString( accel->key( accelid )) + ")";
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



/*! Constructs an action skeleton with parent \a parent and name \a name.

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
    d->toggleaction = toggle;
    init();
}


/*! This constructor creates an action with the following properties:
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
    d->toggleaction = toggle;
    if ( !icon.pixmap().isNull() )
	setIconSet( icon );
    d->text = text;
    d->menutext = menuText;
    setAccel( accel );
    init();
}

/*! This constructor results in an iconless action with the description text
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
	((QActionGroup*) parent())->insert( this );		// insert into action group
    }
}

/*! Destructs the object and frees allocated resources. */

QAction::~QAction()
{
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
    d->update( QActionPrivate::Icons );
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
    d->text = text;
    d->update();
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
    d->menutext = text;
    d->update();
}

/*! Returns the text used in menu entries.

  If no menu text has been defined, this is the same as text().

  \sa setMenuText(),  text()
*/
QString QAction::menuText() const
{
    return d->menuText();
}

/*! Sets the tool tip text to \a tip.

  As long as statusTip() hasn't been set using setStatusTip()
  \a tip serves as message in the status bar as well.

  \sa toolTip(), statusTip()
*/
void QAction::setToolTip( const QString& tip )
{
    d->tooltip = tip;
    d->update();
}

/*! Returns the current tool tip.

  If no tool tip has been defined yet, it returns text()
  and the accelerator description as returned by QAccel::keyToString().

  toolTip() serves as statusTip() as long as no
  separate message for the status bar has been set.

  \sa setToolTip(), setStatusTip(), text(), accel(), QAccel::keyToString()
*/
QString QAction::toolTip() const
{
    return d->toolTip();
}

/*! Sets the status tip to \a tip. It is displayed on
  all status bars the toplevel widget parenting this action provides.

  Note that QActions that were created with only non-windows as
  ancestors can't display status tips.

  \sa statusTip(), toolTip()
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

/*!  Returns the current status tip.

  If no status tip has been defined yet, this is the same as toolTip().

  \sa setStatusTip(), toolTip()
*/
QString QAction::statusTip() const
{
    return d->statusTip();
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
    d->update();
}

/*! Returns the What's This help text for this action.

  Unlike for tips and menu entries text() does not serve
  as default value here.

  \sa setWhatsThis()
*/
QString QAction::whatsThis() const
{
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
//#### Please reimp for QActionGroup!
//#### For consistency reasons even QActionGroups should respond to
//#### their accelerators and e.g. open the relevant submenu.
//#### Please change appropriate QActionGroup class doc after
//#### reimplementation.
void QAction::setAccel( int key )
{
    d->key = key;
    delete d->accel;
    d->accel = 0;

    if ( !key )
	return;

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
    d->update();
}


/*! Returns the acceleration key.

  The hexadecimal keycodes can be found in \l Qt::Key and \l Qt::Modifier.

  \sa setAccel()
*/
int QAction::accel() const
{
    return d->key;
}


/*! Makes the action a toggle action if \a enable is TRUE, or a
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
    if ( enable == (bool)d->toggleaction )
	return;
    d->toggleaction = enable;
    d->update();
}

/*! Returns whether the action is a toggle action or not.

  \sa setToggleAction()
*/
bool QAction::isToggleAction() const
{
    return d->toggleaction;
}

/*! Switches a toggle action on if \a enable is TRUE or off if \e enable is
  FALSE.

  This function has no effect on command actions and QActionGroups.

  \sa isOn(), isToggleAction(), setToggleAction()
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
    d->enabled = enable;
    if ( d->accel )
	d->accel->setEnabled( enable );
    d->update( QActionPrivate::State );
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

  Currently supported widget types are QToolBar and QPopupMenu.

  An action added to a tool bar is automatically displayed
  as a tool button; an action added to a pop up menu appears
  as a menu entry:

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

  \sa removeFrom()
*/
bool QAction::addTo( QWidget* w )
{
    if ( w->inherits( "QToolBar" ) ) {
	if ( !qstrcmp( name(), "qt_separator_action" ) ) {
	    ((QToolBar*)w)->addSeparator();
	} else {
	    QToolButton* btn = new QToolButton( (QToolBar*) w );
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
	    connect( d->tipGroup, SIGNAL(showTip(const QString&)), this, SLOT(showStatusText(const QString&)) );
	    connect( d->tipGroup, SIGNAL(removeTip()), this, SLOT(clearStatusText()) );
	}
    } else if ( w->inherits( "QPopupMenu" ) ) {
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
}

/*! Sets the status message to the menuitem's status text, or
  to the tooltip, if there is no status text.
*/
void QAction::menuStatusText( int id )
{
    QString text;
    QListIterator<QActionPrivate::MenuItem> it( d->menuitems);
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
    if ( w->inherits( "QToolBar" ) ) {
	QListIterator<QToolButton> it( d->toolbuttons);
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
    } else if ( w->inherits( "QPopupMenu" ) ) {
	QListIterator<QActionPrivate::MenuItem> it( d->menuitems);
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
	QListIterator<QActionPrivate::ComboItem> it( d->comboitems );
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
    QListIterator<QActionPrivate::MenuItem> it( d->menuitems );
    QActionPrivate::MenuItem* mi;
    while ( ( mi = it.current() ) ) {
	++it;
	if ( mi->popup == obj )
	    d->menuitems.removeRef( mi );
    }
    QActionPrivate::ComboItem *ci;
    QListIterator<QActionPrivate::ComboItem> it2( d->comboitems );
    while ( ( ci = it2.current() ) ) {
	++it2;
	if ( ci->combo == obj )
	    d->comboitems.removeRef( ci );
    }
    d->toolbuttons.removeRef( (QToolButton*) obj );
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



class QActionGroupPrivate
{
public:
    uint exclusive: 1;
    uint dropdown: 1;
    QList<QAction> actions;
    QAction* selected;
    QAction* separatorAction;

    struct MenuItem {
	MenuItem():popup(0),id(0){}
	QPopupMenu* popup;
	int id;
    };

    QList<QComboBox> comboboxes;
    QList<QToolButton> menubuttons;
    QList<MenuItem> menuitems;
    QList<QPopupMenu> popupmenus;

    void update( const QActionGroup * );
};

void QActionGroupPrivate::update( const QActionGroup* that )
{
    for ( QListIterator<QAction> it( actions ); it.current(); ++it ) {
	it.current()->setEnabled( that->isEnabled() );
    }
    for ( QListIterator<QComboBox> cb( comboboxes ); cb.current(); ++cb ) {
	cb.current()->setEnabled( that->isEnabled() );

	QToolTip::remove( cb.current() );
	QWhatsThis::remove( cb.current() );
	if ( !!that->toolTip() )
	    QToolTip::add( cb.current(), that->toolTip() );
	if ( !!that->whatsThis() )
	    QWhatsThis::add( cb.current(), that->whatsThis() );
    }
    for ( QListIterator<QToolButton> mb( menubuttons ); mb.current(); ++mb ) {
	mb.current()->setEnabled( that->isEnabled() );

	mb.current()->setTextLabel( that->text() );
	mb.current()->setIconSet( that->iconSet() );

	QToolTip::remove( mb.current() );
	QWhatsThis::remove( mb.current() );
	if ( !!that->toolTip() )
	    QToolTip::add( mb.current(), that->toolTip() );
	if ( !!that->whatsThis() )
	    QWhatsThis::add( mb.current(), that->whatsThis() );
    }
    for ( QListIterator<QActionGroupPrivate::MenuItem> pu( menuitems ); pu.current(); ++pu ) {
	QWidget* parent = pu.current()->popup->parentWidget();
	if ( parent->inherits( "QPopupMenu" ) ) {
	    QPopupMenu* ppopup = (QPopupMenu*)parent;
	    ppopup->setItemEnabled( pu.current()->id, that->isEnabled() );
	} else {
	    pu.current()->popup->setEnabled( that->isEnabled() );
	}
    }
    for ( QListIterator<QPopupMenu> pm( popupmenus ); pm.current(); ++pm ) {
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
    : QAction( parent, name )
{
    d = new QActionGroupPrivate;
    d->exclusive = exclusive;
    d->dropdown = FALSE;
    d->selected = 0;
    d->separatorAction = 0;

    connect( this, SIGNAL(selected(QAction*)), SLOT(internalToggle(QAction*)) );
}

/*! Destructs the object and frees allocated resources. */

QActionGroup::~QActionGroup()
{
    QListIterator<QActionGroupPrivate::MenuItem> mit( d->menuitems );
    while ( mit.current() ) {
	QActionGroupPrivate::MenuItem *mi = mit.current();
	++mit;
	if ( mi->popup )
	    mi->popup->disconnect( SIGNAL(destroyed()), this, SLOT(objectDestroyed()) );
    }

    QListIterator<QComboBox> cbit( d->comboboxes );
    while ( cbit.current() ) {
	QComboBox *cb = cbit.current();
	++cbit;
	cb->disconnect(  SIGNAL(destroyed()), this, SLOT(objectDestroyed()) );
    }
    QListIterator<QToolButton> mbit( d->menubuttons );
    while ( mbit.current() ) {
	QToolButton *mb = mbit.current();
	++mbit;
	mb->disconnect(  SIGNAL(destroyed()), this, SLOT(objectDestroyed()) );
    }
    QListIterator<QPopupMenu> pmit( d->popupmenus );
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
    d->exclusive = enable;
}

/*! Returns TRUE if the action group is exclusive, otherwise FALSE.

  \sa setExclusive()
*/

bool QActionGroup::isExclusive() const
{
    return d->exclusive;
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
void QActionGroup::setUsesDropDown( bool enable )
{
    d->dropdown = enable;
}

/*! Returns whether this group uses a subwidget to represent its member actions.

  \sa setUsesDropDown
*/
bool QActionGroup::usesDropDown() const
{
    return d->dropdown;
}

/*! Adds action \a action to this group.

  QActions with this action group as their parent object became members
  at creation time and don't have to be added manually.

  Note that all members of an action group must be
  added before the group is added to a widget.

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

    connect( action, SIGNAL( destroyed() ), this, SLOT( childDestroyed() ) );
    connect( action, SIGNAL( activated() ), this, SIGNAL( activated() ) );
    connect( action, SIGNAL( toggled( bool ) ), this, SLOT( childToggled( bool ) ) );

    for ( QListIterator<QComboBox> cb( d->comboboxes ); cb.current(); ++cb ) {
	cb.current()->insertItem( action->iconSet().pixmap(), action->text() );
    }
    for ( QListIterator<QToolButton> mb( d->menubuttons ); mb.current(); ++mb ) {
	QPopupMenu* popup = mb.current()->popup();
	if ( !popup )
	    continue;
	action->addTo( popup );
    }
    for ( QListIterator<QActionGroupPrivate::MenuItem> mi( d->menuitems ); mi.current(); ++mi ) {
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


/*! \fn void insert( QAction* a )
  
  \obsolete
  
  Use add() instead.
 */

/*! Adds this action group to the widget \a w.

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
bool QActionGroup::addTo( QWidget* w )
{
    if ( w->inherits( "QToolBar" ) ) {
	if ( d->dropdown ) {
	    if ( !d->exclusive ) {
		QListIterator<QAction> it( d->actions);
		if ( !it.current() )
		    return TRUE;

		QAction *defAction = it.current();

		QToolButton* btn = new QToolButton( (QToolBar*) w );
		addedTo( btn, w, defAction );
		connect( btn, SIGNAL(destroyed()), SLOT(objectDestroyed()) );
		d->menubuttons.append( btn );

		if ( !iconSet().pixmap().isNull() )
		    btn->setIconSet( iconSet() );
		else
		    btn->setIconSet( defAction->iconSet() );
		if ( !!text() )
		    btn->setTextLabel( text() );
		else if ( !!defAction->text() )
		    btn->setTextLabel( defAction->text() );
		if ( !!toolTip() )
		    QToolTip::add( btn, toolTip() );
		else if ( !!defAction->toolTip() )
		    QToolTip::add( btn, defAction->toolTip() );
		if ( !!whatsThis() )
		    QWhatsThis::add( btn, whatsThis() );
		else if ( !!defAction->whatsThis() )
		    QWhatsThis::add( btn, defAction->whatsThis() );

		connect( btn, SIGNAL( clicked() ), defAction, SIGNAL( activated() ) );
		connect( btn, SIGNAL( toggled(bool) ), defAction, SLOT( toolButtonToggled(bool) ) );
		connect( btn, SIGNAL( destroyed() ), defAction, SLOT( objectDestroyed() ) );

		QPopupMenu *menu = new QPopupMenu( btn );
		btn->setPopupDelay( 0 );
		btn->setPopup( menu );

		while( it.current() ) {
		    it.current()->addTo( menu );
		    ++it;
		}
		return TRUE;
	    } else {
		QComboBox *box = new QComboBox( FALSE, w );
		addedTo( box, w );
		connect( box, SIGNAL(destroyed()), SLOT(objectDestroyed()) );
		d->comboboxes.append( box );
		if ( !!toolTip() )
		    QToolTip::add( box, toolTip() );
		if ( !!whatsThis() )
		    QWhatsThis::add( box, whatsThis() );

		for ( QListIterator<QAction> it( d->actions); it.current(); ++it ) {
		    it.current()->addTo( box );
		}
		connect( box, SIGNAL(activated(int)), this, SLOT( internalComboBoxActivated(int)) );
		return TRUE;
	    }
	}
    } else if ( w->inherits( "QPopupMenu" ) ) {
	QPopupMenu *popup;
	if ( d->dropdown ) {
	    QPopupMenu *menu = (QPopupMenu*)w;
	    popup = new QPopupMenu( w );
	    d->popupmenus.append( popup );
	    connect( popup, SIGNAL(destroyed()), SLOT(objectDestroyed()) );

	    int id;
	    if ( !iconSet().pixmap().isNull() ) {
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
	for ( QListIterator<QAction> it( d->actions); it.current(); ++it ) {
	    // #### do an addedTo( index, popup, action), need to find out index
	    it.current()->addTo( popup );
	}
	return TRUE;
    }

    for ( QListIterator<QAction> it( d->actions); it.current(); ++it ) {
	// #### do an addedTo( index, popup, action), need to find out index
	it.current()->addTo( w );
    }

    return TRUE;
}

/*! \reimp
*/
bool QActionGroup::removeFrom( QWidget* w )
{
    for ( QListIterator<QAction> it( d->actions); it.current(); ++it ) {
	it.current()->removeFrom( w );
    }

    if ( w->inherits( "QToolBar" ) ) {
	QListIterator<QComboBox> cb( d->comboboxes );
	while( cb.current() ) {
	    QComboBox *box = cb.current();
	    ++cb;
	    if ( box->parentWidget() == w )
		delete box;
	}
	QListIterator<QToolButton> mb( d->menubuttons );
	while( mb.current() ) {
	    QToolButton *btn = mb.current();
	    ++mb;
	    if ( btn->parentWidget() == w )
		delete btn;
	}
    } else if ( w->inherits( "QPopupMenu" ) ) {
	QListIterator<QActionGroupPrivate::MenuItem> pu( d->menuitems );
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
	    for ( QListIterator<QAction> it( d->actions); it.current(); ++it ) {
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
    for ( QListIterator<QAction> it( d->actions); it.current(); ++it ) {
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
    for ( QListIterator<QAction> it( d->actions); it.current(); ++it ) {
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

    for ( QListIterator<QComboBox> cb( d->comboboxes ); cb.current(); ++cb ) {
	for ( int i = 0; i < cb.current()->count(); i++ ) {
	    if ( cb.current()->text( i ) == action->text() ) {
		cb.current()->removeItem( i );
		break;
	    }
	}
    }
    for ( QListIterator<QToolButton> mb( d->menubuttons ); mb.current(); ++mb ) {
	QPopupMenu* popup = mb.current()->popup();
	if ( !popup )
	    continue;
	action->removeFrom( popup );
    }
    for ( QListIterator<QActionGroupPrivate::MenuItem> mi( d->menuitems ); mi.current(); ++mi ) {
	QPopupMenu* popup = mi.current()->popup;
	if ( !popup )
	    continue;
	action->removeFrom( popup );
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
void QActionGroup::internalComboBoxActivated( int index )
{
    QAction *a = d->actions.at( index );
    if ( a ) {
	if ( a != d->selected ) {
	    d->selected = a;
	    for ( QListIterator<QAction> it( d->actions); it.current(); ++it ) {
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
    for ( QListIterator<QComboBox> it( d->comboboxes); it.current(); ++it ) {
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
    for ( QListIterator<QActionGroupPrivate::MenuItem> mi( d->menuitems ); mi.current(); ++mi ) {
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

/* \reimp */

void QActionGroup::addedTo( QWidget *actionWidget, QWidget *container )
{
    Q_UNUSED( actionWidget );
    Q_UNUSED( container );
}

/* \reimp */

void QActionGroup::addedTo( int index, QPopupMenu *menu )
{
    Q_UNUSED( index );
    Q_UNUSED( menu );
}

#endif
