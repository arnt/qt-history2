/****************************************************************************
**
** Implementation of QAction class.
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
#include "qaction.h"


#include "qaction_p.h"
#include "qapplication.h"
#include "qevent.h"
#include "qlist.h"

#define d d_func()
#define q q_func()

/* QAction code */

QActionPrivate::QActionPrivate() : group(0), icons(0), enabled(1), forceDisabled(0),
                                   visible(1), forceInvisible(0), checkable(0), checked(0), separator(0)
{
#ifdef QT_COMPAT
    static int qt_static_action_id = -1;
    param = id = --qt_static_action_id;
    act_signal = 0;
#endif
}

QActionPrivate::~QActionPrivate()
{
    delete icons;
    if(menu)
        delete menu;
}

void QActionPrivate::sendDataChanged()
{
    emit q->changed();
    QActionEvent e(QEvent::ActionChanged, q);
    QApplication::sendEvent(q, &e);
}

/*!
    \class QAction qaction.h
    \brief The QAction class provides an abstract user interface
    action that can be inserted into widgets with
    QWidget::addAction().

    \ingroup basic
    \ingroup application
    \mainclass

    In applications many common commands can be invoked via menus,
    toolbar buttons, and keyboard shortcuts. Since the user expects
    each command to be performed in the same way, regardless of the
    user interface used, it is useful to represent each command as
    an \e action.

    Actions can be added to menus and toolbars, and will
    automatically keep them in sync. For example, in a word processor,
    if the user presses a Bold toolbar button, the Bold menu item
    will automatically be checked.

    Actions can be created as independent objects, but they may
    also be created during the construction of menus; the QMenu class
    contains convenience functions for creating actions suitable for
    use as menu items.

    A QAction may contain an icon, menu text, a shortcut, status text,
    "What's This?" text, and a tool tip. Most of these can be set in
    the constructor. They can also be set independently with
    setIconSet(), setText(), setMenuText(), setShortcut(),
    setStatusTip(), setWhatsThis(), and setToolTip(). For menu items,
    it is possible to set an individual font with setFont().

    Actions are added to widgets using QWidget::addAction().

    Once a QAction has been created it should be added to the relevant
    menu and toolbar, then connected to the slot which will perform
    the action. For example:

    \quotefile action/application.cpp
    \skipto QPixmap(fileopen
    \printuntil connect

    We recommend that actions are created as children of the window
    they are used in. In most cases actions will be children of
    the application's main window.

    To prevent recursion, do not create an action as a child of a
    widget that the action is later added to.
*/


/*!
    Constructs an action for a \a parent action group. The action will
    be automatically inserted into the \a parent.
*/
QAction::QAction(QActionGroup* parent)
    : QObject(*(new QActionPrivate), parent)
{
    d->group = parent;
    if(parent)
        parent->addAction(this);
}

/*!
    Constructs an action for a \a parent widget. The action will \e not
    be automatically inserted into the widget.
*/
QAction::QAction(QWidget* parent)
    : QObject(*(new QActionPrivate), parent)
{
}

/*!
    Constructs an action with some \a text for the \a parent action
    group. The action will be automatically inserted into the
    action group.
*/
QAction::QAction(const QString &text, QActionGroup* parent)
    : QObject(*(new QActionPrivate), parent)
{
    d->text = text;
    d->group = parent;
    if(parent)
        parent->addAction(this);
}

/*!
    Constructs an action with an \a icon and some \a text for the
    \a parent action group. The action will be automatically inserted
    into the action group.
*/
QAction::QAction(const QIconSet &icon, const QString &text, QActionGroup* parent)
    : QObject(*(new QActionPrivate), parent)
{
    d->icons = new QIconSet(icon);
    d->text = text;
    d->group = parent;
    if(parent)
        parent->addAction(this);
}

/*!
    Constructs an action with some \a text for the \a parent widget.
    The action will \e not be automatically inserted into the widget.
*/
QAction::QAction(const QString &text, QWidget* parent)
    : QObject(*(new QActionPrivate), parent)
{
    d->text = text;
}

/*!
    Constructs an action with an \a icon and some \a text for the
    \a parent widget. The action will \e not be automatically inserted
    into the widget.
*/
QAction::QAction(const QIconSet &icon, const QString &text, QWidget* parent)
    : QObject(*(new QActionPrivate), parent)
{
    d->text = text;
    d->icons = new QIconSet(icon);
}

/*!
    Returns the parent widget.
*/
QWidget *QAction::parentWidget() const
{
    QObject *ret = parent();
    while (ret && !ret->isWidgetType())
        ret = ret->parent();
    return (QWidget*)ret;
}

/*!
    \property QAction::shortcut
    \brief the action's shortcut key

    Valid keycodes for this property can be found in \l Qt::Key and
    \l Qt::Modifier. There is no default shortcut key.
*/
void QAction::setShortcut(const QKeySequence &shortcut)
{
    if (d->shortcut == shortcut)
        return;

    d->shortcut = shortcut;
    d->sendDataChanged();
}

QKeySequence QAction::shortcut() const
{
    return d->shortcut;
}

/*!
    \property QAction::font
    \brief the action's font

    The font property is used to draw menu items.
*/
void QAction::setFont(const QFont &font)
{
    d->font = font;
    d->sendDataChanged();
}

QFont QAction::font() const
{
    return d->font;
}

#ifdef QT_COMPAT
QAction::QAction(QWidget* parent, const char* name)
 : QObject(*(new QActionPrivate), parent)
{
    setObjectName(name);
}

QAction::QAction(QActionGroup* parent, const char* name)
 : QObject(*(new QActionPrivate), parent)
{
    setObjectName(name);
    d->group = parent;
    if(parent)
        parent->addAction(this);
}

QAction::QAction(const QString &text, const QKeySequence &shortcut, QWidget* parent, const char* name)
 : QObject(*(new QActionPrivate), parent)
{
    setObjectName(name);
    d->text = text;
    setShortcut(shortcut);
}

QAction::QAction(const QIconSet &icon, const QString &text, const QKeySequence &shortcut,
                 QWidget* parent, const char* name)
 : QObject(*(new QActionPrivate), parent)
{
    setObjectName(name);
    d->text = text;
    setShortcut(shortcut);
    d->icons = new QIconSet(icon);
}

QAction::QAction(const QString &text, const QKeySequence &shortcut, QActionGroup* parent, const char* name)
 : QObject(*(new QActionPrivate), parent)
{
    setObjectName(name);
    d->text = text;
    setShortcut(shortcut);
    d->group = parent;
    if(parent)
        parent->addAction(this);
}

QAction::QAction(const QIconSet &icon, const QString &text, const QKeySequence &shortcut,
                 QActionGroup* parent, const char* name)
 : QObject(*(new QActionPrivate), parent)
{
    setObjectName(name);
    d->text = text;
    setShortcut(shortcut);
    d->icons = new QIconSet(icon);
    d->group = parent;
    if(parent)
        parent->addAction(this);
}
#endif

/*!
    Destroys the object and frees allocated resources.
*/
QAction::~QAction()
{
    /* We need to be able to tell when a QAction is about to be destroyed (ie before the QObject::~QObject)
       so that the QAction can properly be removed */
    emit deleted();
}

/*!
  Sets this action group to \a group. The action will be automatically
  added to the group's list of actions.

  Actions within the group will be mutually exclusive.

  \sa QActionGroup, QAction::actionGroup()
*/
void QAction::setActionGroup(QActionGroup *group)
{
    if(group == d->group)
        return;

    if(d->group)
        d->group->removeAction(this);
    d->group = group;
    if(group)
        group->addAction(this);
}

/*!
  Returns the action group for this action. If no action group manages
  this action then 0 will be returned.

  \sa QActionGroup, QAction::setActionGroup()
*/
QActionGroup *QAction::actionGroup() const
{
    return d->group;
}


/*!
    \property QAction::icon
    \brief the action's icon

    In toolbars, the icon is used as the tool button icon; in menus,
    it is displayed to the left of the menu text. There is no default
    icon.

    If a null icon (QIconSet::isNull() is passed into this function,
    the icon of the action is cleared.
*/
void QAction::setIcon(const QIconSet &icons)
{
    d->icons = new QIconSet(icons);
    d->sendDataChanged();
}

QIconSet QAction::icon() const
{
    if(d->icons)
        return *d->icons;
    return QIconSet();
}

/*!
  Set the submenu of this action to the \a menu given.

  \sa QAction::menu()
*/
void QAction::setMenu(QMenu *menu)
{
    d->menu = menu;
    d->sendDataChanged();
}

/*!
  Returns this action's submenu.

  \sa QAction::setMenu()
*/
QMenu *QAction::menu() const
{
    return d->menu;
}

/*!
  If \a b is true then this action will be considered a separator.

  How a separator is represented depends on the widget it is inserted
  into. Under most circumstances the text, submenu, and icon will be
  ignored for separator actions.

  \sa QAction::separator()
*/
void QAction::setSeparator(bool b)
{
    d->separator = b;
    d->sendDataChanged();
}

/*!
  Returns true if this action is a separator action; otherwise it
  returns false.

  \sa QAction::setSeparator()
*/
bool QAction::isSeparator() const
{
    return d->separator;
}

/*!
    \property QAction::text
    \brief the action's descriptive text

    If QMainWindow::usesTextLabel is true, the text appears as a
    label in the relevant tool button. It also serves as the default
    text in menus and tool tips if these have not been specifically
    defined. There is no default text.

    \sa setToolTip() setStatusTip()
*/
void QAction::setText(const QString &text)
{
    d->text = text;
    d->sendDataChanged();
}


QString QAction::text() const
{
    return d->text;
}

/*!
    \property QAction::menuText
    \brief the action's menu text

    If the action is added to a menu, the menu option will consist of
    the icon (if there is one), the menu text, and the shortcut (if
    there is one). If the menu text is not explicitly set in the
    constructor, or by using setMenuText(), the action's description
    text will be used as the menu text. There is no default menu text.

    \sa text
*/
void QAction::setMenuText(const QString &text)
{
    d->menuText = text;
    d->sendDataChanged();
}

QString QAction::menuText() const
{
    QString ret = d->menuText;
    if(ret.isEmpty()) {
        ret = d->text;
        ret.replace('&', "&&");
    }
    return ret;
}


/*!
    \property QAction::toolTip
    \brief the action's tool tip

    This text is used for the tool tip. If no status tip has been set
    the tool tip will be used for the status tip.

    If no tool tip is specified the action's text is used, and if that
    hasn't been specified the description text is used as the tool tip
    text.

    There is no default tool tip text.

    \sa setStatusTip() setShortcut()
*/
void QAction::setToolTip(const QString &tooltip)
{
    d->tooltip = tooltip;
    d->sendDataChanged();
}

QString QAction::toolTip() const
{
    return d->tooltip;
}

/*!
    \property QAction::statusTip
    \brief the action's status tip

    The statusTip is displayed on all status bars provided by the
    action's top-level parent widget. It can be set, and is provided as the parameter
    of the showStatusMessage() signal.

    If no status tip is defined, the action uses the tool tip text.

    There is no default statusTip text.

    \sa setStatusTip() setToolTip() showStatusMessage()
*/
void QAction::setStatusTip(const QString &statustip)
{
    d->statustip = statustip;
    d->sendDataChanged();
}

QString QAction::statusTip() const
{
    if(d->statustip.isNull())
        return d->tooltip;
    return d->statustip;
}

/*!
    \property QAction::whatsThis
    \brief the action's "What's This?" help text

    The "What's This?" text is used to provide a brief description of
    the action. The text may contain rich text. There is no default
    "What's This?" text.

    If the text contains a hyperlink, the whatsThisClicked() signal is
    emitted when the user clicks inside the "What's This?" window.

    \sa QWhatsThis whatsThisClicked() QStyleSheet
*/
void QAction::setWhatsThis(const QString &whatsthis)
{
    d->whatsthis = whatsthis;
    d->sendDataChanged();
}

QString QAction::whatsThis() const
{
    return d->whatsthis;
}


/*!
    \property QAction::checkable
    \brief whether the action is a checkable action

    A checkable action is one which has an on/off state. For example,
    in a word processor, a Bold toolbar button may be either on or
    off. An action which is not a toggle action is a command action;
    a command action is simply executed, e.g. file save.
    By default, this property is false.

    In some situations, the state of one toggle action should depend
    on the state of others. For example, "Left Align", "Center" and
    "Right Align" toggle actions are mutually exclusive. To achieve
    exclusive toggling, add the relevant toggle actions to a
    QActionGroup with the QActionGroup::exclusive property set to
    true.

    \sa QAction::setChecked()
*/
void QAction::setCheckable(bool b)
{
    d->checkable = b;
    d->sendDataChanged();
}

bool QAction::isCheckable() const
{
    return d->checkable;
}

/*!
    \fn void QAction::toggle()

    This is a convenience function for the \l checked property.
    Connect to it to change the checked state to its opposite state.
*/

/*!
    \property QAction::checked
    \brief whether a toggle action is on

    This property is always on (true) for command actions and
    \l{QActionGroup}s; setOn() has no effect on them. For actions
    where isCheckable() is true, this property's default value is
    off (false).

    \sa checkable
*/
void QAction::setChecked(bool b)
{
    d->checked = b;
    d->sendDataChanged();
    if(d->checkable) {
        emit checked(b);
#ifdef QT_COMPAT
        emit toggled(b);
#endif
    }
}

bool QAction::isChecked() const
{
    return d->checked;
}

/*!
    \fn void QAction::setDisabled(bool b)

    This is a convenience function for the \l enabled property, that
    is useful for signals--slots connections. If \a b is true the
    action is disabled; otherwise it is enabled.
*/

/*!
    \property QAction::enabled
    \brief whether the action is enabled

    Disabled actions cannot be chosen by the user. They do not
    disappear from menus or toolbars, but they are displayed in a way
    which indicates that they are unavailable. For example, they might
    be displayed using only shades of gray.

    What's this? help on disabled actions is still available, provided
    that the QAction::whatsThis property is set.
*/
void QAction::setEnabled(bool b)
{
    d->enabled = b;
    d->forceDisabled = !b;
    d->sendDataChanged();
}

bool QAction::isEnabled() const
{
    return d->enabled;
}

/*!
    \property QAction::visible
    \brief whether the action can be seen (e.g. in menus and toolbars)

    If \e visible is true the action can be seen (e.g. in menus and
    toolbars) and chosen by the user; if \e visible is false the
    action cannot be seen or chosen by the user.

    Actions which are not visible are \e not grayed out; they do not
    appear at all.
*/
void QAction::setVisible(bool b)
{
    d->forceInvisible = !b;
    d->visible = b;
    d->sendDataChanged();
}


bool QAction::isVisible() const
{
    return d->visible;
}

/*!
  \reimp
*/
bool
QAction::event(QEvent *e)
{
    if (e->type() == QEvent::Shortcut) {
        QShortcutEvent *se = static_cast<QShortcutEvent *>(e);
        Q_ASSERT_X(se->key() == d->shortcut,
                   "QAction::event",
                   "Received shortcut event from incorrect shortcut");
        if (se->isAmbiguous())
            qWarning("QAction::eventFilter: ambiguous shortcut overload");
        else
            activate(Trigger);
        return true;
    }
    return false;
}

/*!
  Updates the status bar for \a widget. If widget is an appropriate
  QStatusBar found for for this action based on the parent heirarchy will be used.

  \sa setStatusTip(QString&)

*/
bool
QAction::showStatusText(QWidget *widget)
{
    if(QObject *object = widget ? widget : parent()) {
        QStatusTipEvent tip(statusTip());
        QApplication::sendEvent(object, &tip);
        return true;
    }
    return false;
}

/*!
  Sends the relevant signals for ActionEvent \a event.

  Action based widgets use this API to cause the QAction
  to emit signals as well as emitting their own.
*/
void QAction::activate(ActionEvent event)
{
    if(event == Trigger) {
        if(d->checkable) {
            // the checked action of an exclusive group cannot be  unchecked
            if (d->checked && (d->group && d->group->isExclusive()
                               && d->group->checkedAction() == this))
                return;
            setChecked(!d->checked);
        }
        emit triggered();
#ifdef QT_COMPAT
        emit activated();
#endif
    } else if(event == Hover) {
        emit hovered();
    }
}

/*!
    \fn void QAction::triggered()

    This signal is emitted when an action is activated by the user;
    for example, when the user clicks a menu option, toolbar button,
    or presses an action's shortcut key combination.

    Connect to this signal for command actions.

    \sa QAction::activate(), QAction::checked()
*/

/*!
    \fn void QAction::checked(bool state)

    This signal is emitted when an action is activated by the user
    which causes the isChecked() status to change. The new \a state
    of the action is given.

    Connect to this signal for checkable actions.

    \sa QAction::activate(), QAction::triggered(), QAction::setChecked()
*/

/*!
    \fn void QAction::hovered()

    This signal is emitted when an action is highlighted by the user;
    for example, when the user pauses with the cursor over a menu option,
    toolbar button, or presses an action's shortcut key combination.

    \sa QAction::activate()
*/

/*!
    \fn void QAction::changed()

    This signal is emitted when an action has changed. If you
    are only interested in actions in a given widget, you can
    watch for QWidget::actionEvent() sent with an
    QEvent::ActionChanged.

    \sa QWidget::actionEvent()
*/

/*!
    \fn void QAction::deleted()
    \internal

    This signal is emitted when an action has been deleted.

    \sa QWidget::actionEvent()
*/

/*!
    \enum QAction::ActionEvent

    This enum type is used when calling QAction::activate()

    \value Trigger this will cause the QAction::triggered() signal to be emitted.

    \value Hover this will cause the QAction::hovered() signal to be emitted.
*/

/* ************** QActionGroup code ***************** */
void QActionGroupPrivate::actionChanged()
{
    QAction *action = qt_cast<QAction*>(q->sender());
    Q_ASSERT_X(action != 0, "QWidgetGroup::actionChanged", "internal error");
    if(exclusive && action->isChecked() && action != current) {
        if(current)
            current->setChecked(false);
        current = action;
    }
}

void QActionGroupPrivate::actionTriggered()
{
    QAction *action = qt_cast<QAction*>(q->sender());
    Q_ASSERT_X(action != 0, "QWidgetGroup::actionTriggered", "internal error");
    emit q->triggered(action);
    emit q->selected(action);
}

void QActionGroupPrivate::actionHovered()
{
    QAction *action = qt_cast<QAction*>(q->sender());
    Q_ASSERT_X(action != 0, "QWidgetGroup::actionHovered", "internal error");
    emit q->hovered(action);
}

void QActionGroupPrivate::actionDeleted()
{
    QAction *action = qt_cast<QAction*>(q->sender());
    Q_ASSERT_X(action != 0, "QWidgetGroup::actionDeleted", "internal error");
    q->removeAction(action);
}

/*!
    \class QActionGroup qaction.h
    \brief The QActionGroup class groups actions together.

    \ingroup basic
    \ingroup application

    In some situations it is useful to group actions together. For
    example, if you have a left justify action, a right justify action
    and a center action, only one of these actions should be active at
    any one time. One simple way of achieving this is to group the
    actions together in an action group.

    An action group can also be added to a menu or a toolbar as a
    single unit, with all the actions within the action group
    appearing as separate menu options or toolbar buttons.

    Here's an example from examples/textedit:
    \quotefile textedit/textedit.cpp
    \skipto QActionGroup
    \printuntil connect

    Here we create a new action group. Since the action group is exclusive
    by default, only one of the actions in the group is ever active at any
    one time. We then connect the group's triggered() signal to our
    textAlign() slot.

    \printuntil actionAlignLeft->setCheckable

    We create a left align action, add it to the toolbar and the menu,
    and make it a toggle action. We create center and right align
    actions in exactly the same way.

    A QActionGroup emits an triggered() signal when one of its actions
    is activated. Each action in an action group emits its
    triggered() signal as usual.

    As stated above, an action group is \l exclusive by default; it
    ensures that only one \c checkable action is active at any one
    time. If you want to group checkable actions without making them
    exclusive, you can turn of exclusiveness by calling
    setExclusive(false).

    Actions can be added to an action group using addAction(), but it
    is usually more convenient to specify a group when creating
    actions; this ensures that actions are automatically created with
    a parent. Actions can be visually separated from each other
    using addSeparator(). Action groups are added to widgets with
    addTo().
*/

/*!
    Constructs an action group for the \a parent object.

    The action group is exclusive by default. Call setExclusive(false)
    to make the action group non-exclusive.
*/
QActionGroup::QActionGroup(QObject* parent) : QObject(*new QActionGroupPrivate, parent)
{
}

/*!
    Destroys the action group.
*/
QActionGroup::~QActionGroup()
{
}

/*!
    \fn QAction *QActionGroup::addAction(QAction *action)

    Adds the \a action to this group, and returns it.

    Normally an action is added to a group by creating it with the
    group as its parent, so this function is not usually used.

    \sa QAction::setActionGroup()
*/
QAction *QActionGroup::addAction(QAction* a)
{
    if(!d->actions.contains(a)) {
        d->actions.append(a);
        QObject::connect(a, SIGNAL(triggered()), this, SLOT(actionTriggered()));
        QObject::connect(a, SIGNAL(changed()), this, SLOT(actionChanged()));
        QObject::connect(a, SIGNAL(hovered()), this, SLOT(actionHovered()));
        QObject::connect(a, SIGNAL(deleted()), this, SLOT(actionDeleted()));
    }
    if(!a->d->forceDisabled) {
        a->setEnabled(d->enabled);
        a->d->forceDisabled = false;
    }
    if(!a->d->forceInvisible) {
        a->setVisible(d->visible);
        a->d->forceInvisible = false;
    }
    if(a->d->group != this)
        a->d->group = this;
    return a;
}

/*!
    Creates and returns an action with \a text.  The newly created
    action is a child of this action group.

    Normally an action is added to a group by creating it with the
    group as parent, so this function is not usually used.

    \sa QAction::setActionGroup()
*/
QAction *QActionGroup::addAction(const QString &text)
{
    return new QAction(text, this);
}

/*!
    Creates and returns an action with \a text and an \a icon. The
    newly created action is a child of this action group.

    Normally an action is added to a group by creating it with the
    group as its parent, so this function is not usually used.

    \sa QAction::setActionGroup()
*/
QAction *QActionGroup::addAction(const QIconSet &icon, const QString &text)
{
    return new QAction(icon, text, this);
}

/*!
  Removes the \a action from this group. The action will have no
  parent as a result.

  \sa QAction::setActionGroup()
*/
void QActionGroup::removeAction(QAction *action)
{
    if (d->actions.removeAll(action)) {
        QObject::disconnect(action, SIGNAL(triggered()), this, SLOT(actionTriggered()));
        QObject::disconnect(action, SIGNAL(changed()), this, SLOT(actionChanged()));
        QObject::disconnect(action, SIGNAL(hovered()), this, SLOT(actionHovered()));
        QObject::disconnect(action, SIGNAL(deleted(QObject*)), this, SLOT(actionDeleted()));
        action->d->group = 0;
    }
}

/*!
    Returns the list of this groups's actions. This may be empty.
*/
QList<QAction*> QActionGroup::actions() const
{
    return d->actions;
}

/*!
    \property QActionGroup::exclusive
    \brief whether the action group does exclusive checking

    If exclusive is true, only one checkable action in the action group
    can ever be active at any time. If the user chooses another
    checkable action in the group, the one they chose becomes active and
    the one that was active becomes inactive.

    \sa QAction::checkable
*/
void QActionGroup::setExclusive(bool b)
{
    d->exclusive = b;
}

bool QActionGroup::isExclusive() const
{
    return d->exclusive;
}

/*!
    \fn void QActionGroup::setDisabled(bool b)

    This is a convenience function for the \l enabled property, that
    is useful for signals--slots connections. If \a b is true the
    action group is disabled; otherwise it is enabled.
*/

/*!
    \property QActionGroup::enabled
    \brief whether the action group is enabled

    Each action in the group will be enabled or disabled unless it
    has been explicitly disabled.

    \sa QAction::setEnabled()
*/
void QActionGroup::setEnabled(bool b)
{
    d->enabled = b;
    for(QList<QAction*>::Iterator it = d->actions.begin(); it != d->actions.end(); ++it) {
        if(!(*it)->d->forceDisabled) {
            (*it)->setEnabled(b);
            (*it)->d->forceDisabled = false;
        }
    }
}

bool QActionGroup::isEnabled() const
{
    return d->enabled;
}

/*!
  Returns the currently checked action in the group, or 0 if none
  are checked.
*/
QAction *QActionGroup::checkedAction() const
{
    return d->current;
}

/*!
    \property QActionGroup::visible
    \brief whether the action group is visible

    Each action in the action group will match the visible state of
    this group unless it has been explicitly hidden.

    \sa QAction::setEnabled()
*/
void QActionGroup::setVisible(bool b)
{
    d->visible = b;
    for(QList<QAction*>::Iterator it = d->actions.begin(); it != d->actions.end(); ++it) {
        if(!(*it)->d->forceInvisible) {
            (*it)->setVisible(b);
            (*it)->d->forceInvisible = false;
        }
    }
}

bool QActionGroup::isVisible() const
{
    return d->visible;
}

/*!
  \reimp
*/
void QActionGroup::childEvent(QChildEvent* e)
{
    if(e->type() == QEvent::ChildAdded) {
        if(QAction *action = qt_cast<QAction*>(e->child()))
            addAction(action);
    } else if(e->type() == QEvent::ChildRemoved) {
        if(QAction *action = qt_cast<QAction*>(e->child()))
            removeAction(action);
    }
    QObject::childEvent(e);
}

/*!
    \fn void QActionGroup::triggered(QAction *action)

    This signal is emitted when the given \a action in the action
    group is activated by the user; for example, when the user clicks
    a menu option, toolbar button, or presses an action's shortcut key
    combination.

    Connect to this signal for command actions.

    \sa QAction::activate(), QAction::checked()
*/

/*!
    \fn void QActionGroup::hovered(QAction *action)

    This signal is emitted when the given \a action in the action
    group is highlighted by the user; for example, when the user
    pauses with the cursor over a menu option, toolbar button, or
    presses an action's shortcut key combination.

    \sa QAction::activate()
*/

#include "moc_qaction.cpp"

