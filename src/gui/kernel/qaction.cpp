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
void QActionPrivate::sendDataChanged()
{
    emit q->dataChanged();
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

    In GUI applications many commands can be invoked via a menu
    option, a toolbar button and a keyboard shortcut. Since the
    same action must be performed regardless of how the action was
    invoked, and since the menu and toolbar should be kept in sync, it
    is useful to represent a command as an \e action. An action can be
    added to a menu and a toolbar and will automatically keep them in
    sync. For example, if the user presses a Bold toolbar button the
    Bold menu item will automatically be checked.

    A QAction may contain an icon, a menu text, a shortcut, a
    status text, a whats this text and a tool tip. Most of these can
    be set in the constructor. They can also be set independently with
    setIconSet(), setText(), setMenuText(), setToolTip(),
    setStatusTip(), setWhatsThis() and setShortcut().

    Actions are added to widgets using QWidget::addAction().

    Once a QAction has been created it should be added to the relevant
    menu and toolbar and then connected to the slot which will perform
    the action. For example:

    \quotefile action/application.cpp
    \skipto QPixmap(fileopen
    \printuntil connect

    We recommend that actions are created as children of the window
    that they are used in. In most cases actions will be children of
    the application's main window.

    To prevent recursion, don't create an action as a child of a
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
    Constructs an action with some \a text and a submenu given by
    \a menu for the \a parent action group. The action will be
    automatically inserted into the action group.
*/
QAction::QAction(const QString &text, QMenu *menu, QActionGroup* parent)
    : QObject(*(new QActionPrivate), parent)
{
    d->text = text;
    d->menu = menu;
    d->group = parent;
    if(parent)
        parent->addAction(this);
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
    Constructs an action with some \a text and a submenu given by
    \a menu for the \a parent widget. The action will \e not be
    automatically inserted into the widget.
*/
QAction::QAction(const QString &text, QMenu *menu, QWidget* parent)
    : QObject(*(new QActionPrivate), parent)
{
    d->text = text;
    d->menu = menu;
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
    Constructs an action with some \a text and a keyboard \shortcut
    for the \a parent action group. The action will be automatically
    inserted into the action group.
*/
QAction::QAction(const QString &text, const QKeySequence &shortcut, QActionGroup* parent)
    : QObject(*(new QActionPrivate), parent)
{
    d->text = text;
    setShortcut(shortcut);
    d->group = parent;
    if(parent)
        parent->addAction(this);
}

/*!
    Constructs an action with an \a icon, some \a text, and a keyboard
    \a shortcut for the \a parent action group. The action will
    automatically be inserted into the action group.
*/
QAction::QAction(const QIconSet &icon, const QString &text, const QKeySequence &shortcut, QActionGroup* parent)
    : QObject(*(new QActionPrivate), parent)
{
    d->text = text;
    setShortcut(shortcut);
    d->icons = new QIconSet(icon);
    d->group = parent;
    if(parent)
        parent->addAction(this);
}

/*!
    Constructs an action with some \a text and a keyboard \a shortcut
    for the \a parent widget. The action will \e not be automatically
    inserted into the widget.
*/
QAction::QAction(const QString &text, const QKeySequence &shortcut, QWidget* parent)
    : QObject(*(new QActionPrivate), parent)
{
    d->text = text;
    setShortcut(shortcut);
}

/*!
    Constructs an action with an \a icon, some \a text, and a keyboard
    \a shortcut for the \a parent widget. The action will \e not be
    automatically inserted into the widget.
*/
QAction::QAction(const QIconSet &icon, const QString &text, const QKeySequence &shortcut,
                   QWidget* parent) : QObject(*(new QActionPrivate), parent)
{
    d->text = text;
    setShortcut(shortcut);
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
    if(QWidget *parent = parentWidget()) {
        if(d->shortcutId != -1) {
            parent->releaseShortcut(d->shortcutId);
            d->shortcutId = -1;
        } else {
            parent->installEventFilter(this);
        }
        d->shortcutId = parent->grabShortcut(d->shortcut);
    }
    d->sendDataChanged();
}

QKeySequence QAction::shortcut() const
{
    return d->shortcut;
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
}

/*!
  Sets this action group to \a group. The action will be automatically
  added to the group's list of actions.

  A QActionGroup will provide mutual exclusivity as well group
  behavior of all contained actions.

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
    \property QAction::iconSet
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
  Set the submenu of this action to menu \a menu.

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

  How a separator is reprented depends on the widget this is inserted
  into. Under most circumstances the text, submenu and icon will be
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

    If \l QMainWindow::usesTextLabel is true, the text appears as a
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

    The statusTip is displayed on all status bars that this action's
    toplevel parent widget provides, and is provided as the parameter
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
    return d->statustip;
}

/*!
    \property QAction::whatsThis
    \brief the action's "What's This?" help text

    The whats this text is used to provide a brief description of the
    action. The text may contain rich text (HTML-like tags -- see
    QStyleSheet for the list of supported tags). There is no default
    "What's This" text.

    If the whats this text contains a hyperlink the whatsThisClicked()
    signal is emitted when the user clicks inside the "What's This?"
    window.

    \sa QWhatsThis whatsThisClicked()
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

    A toggle action is one which has an on/off state. For example,
    in a word processor, a "Bold" toolbar button may be either on or
    off. An action which is not a toggle action is a command action;
    a command action is simply executed, e.g. file save.
    By default, this property is false.

    In some situations, the state of one toggle action should depend
    on the state of others. For example, "Left Align", "Center" and
    "Right Align" toggle actions are mutually exclusive. To achieve
    exclusive toggling, add the relevant toggle actions to a
    QActionGroup with the \l QActionGroup::exclusive property set to
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
#ifdef QT_COMPAT
    if(isCheckable())
        emit toggled(b);
#endif
}

bool QAction::isChecked() const
{
    return d->checked;
}

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
    if(d->shortcutId != -1) {
        if(QWidget *p = parentWidget())
            p->setShortcutEnabled(d->shortcutId, d->enabled && d->visible);
    }
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
    if(d->shortcutId != -1) {
        if(QWidget *p = parentWidget())
            p->setShortcutEnabled(d->shortcutId, d->enabled && d->visible);
    }
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
QAction::eventFilter(QObject *, QEvent *e)
{
    if (e->type() == QEvent::Shortcut) {
        QShortcutEvent *se = static_cast<QShortcutEvent *>(e);
        if (d->shortcutId != se->shortcutId())
            return false;
        if (se->isAmbiguous())
            qWarning("QAction: Ambigious shortcut overload!");
        else
            activate(Trigger);
        return true;
    }
    return false;
}

/*!

  Sends the relevant signals for ActionEvent \a event.

  Action based widgets will use this API to cause the QAction's
  signals to be sent as well as emit'ing their own signals
*/
void QAction::activate(ActionEvent event)
{
    if(event == Trigger) {
        if(isCheckable())
            setChecked(!isChecked());
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

    This signal is emitted when an action is activated by the user,
    e.g. when the user clicks a menu option or a toolbar button or
    presses an action's shortcut key combination.

    Connect to this signal for command actions. Connect to the
    toggled() signal for checkable actions.

    \sa QAction::activate()
*/

/*!
    \fn void QAction::hovered()

    This signal is emitted when an action is highlighted by the user,
    e.g. when the user pauses with the cursor over a menu option or a
    toolbar button or presses an action's shortcut key combination.

    \sa QAction::activate()
*/

/*!
    \fn void QAction::dataChanged()

    This signal is emitted when an action has changed. If only
    interested in actions in a given widget you can watch for
    QWidget::actionEvent() sent with an QEvent::ActionChanged.

    \sa QWidget::actionEvent()
*/

/*!
    \enum QAction::ActionEvent

    This enum type is used when calling QAction::activate()

    \value Trigger this will cause the QAction::triggered() signal to be emitted.

    \value Hover this will cause the QAction::hovered() signal to be emitted.
*/

/* ************** QActionGroup code ***************** */
/*!
    \class QActionGroup qaction.h
    \brief The QActionGroup class groups actions together.

    \ingroup basic
    \ingroup application

    In some situations it is useful to group actions together. For
    example, if you have a left justify action, a right justify action
    and a center action, only one of these actions should be active at
    any one time, and one simple way of achieving this is to group the
    actions together in an action group.

    An action group can also be added to a menu or a toolbar as a
    single unit, with all the actions within the action group
    appearing as separate menu options and toolbar buttons.

    Here's an example from examples/textedit:
    \quotefile textedit/textedit.cpp
    \skipto QActionGroup
    \printuntil connect

    Here we create a new action group. Since the action group is exclusive
    by default, only one of the actions in the group is ever active at any
    one time. We then connect the group's selected() signal to our
    textAlign() slot.

    \printuntil actionAlignLeft->setToggleAction

    We create a left align action, add it to the toolbar and the menu
    and make it a toggle action. We create center and right align
    actions in exactly the same way.

    A QActionGroup emits an triggered() signal when one of its actions
    is activated. The actions in an action group emit their
    triggered() signal as usual.

    The setExclusive() function is used to ensure that only one action
    is active at any one time: it should be used with actions which
    have their \c checkable set to true.

##### need to actually figure this out. After discussion with matthias
    Actions can be added to an action group using add(), but normally
    they are added by creating the action with the action group as
    parent. Actions can have separators dividing them using
    addSeparator(). Action groups are added to widgets with addTo().
*/

/*!
    Constructs an action group for the \a parent object.

    The action group is exclusive by default. Call setExclusive(false) to make
    the action group non-exclusive.
*/
QActionGroup::QActionGroup(QObject* parent) : QObject(*new QActionGroupPrivate, parent)
{
}

/*!
    Destroys the object and frees allocated resources.
*/
QActionGroup::~QActionGroup()
{
}

/*!
    \fn QAction *QActionGroup::addAction(QAction *action)

    Adds the \a action to this group.

    Normally an action is added to a group by creating it with the
    group as its parent, so this function is not usually used.

    \sa QAction::setActionGroup()
*/
QAction *QActionGroup::addAction(QAction* a)
{
    if(!d->actions.contains(a)) {
        d->actions.append(a);
        QObject::connect(a, SIGNAL(triggered()), this, SLOT(internalTriggered()));
        QObject::connect(a, SIGNAL(dataChanged()), this, SLOT(internalDataChanged()));
        QObject::connect(a, SIGNAL(hovered()), this, SLOT(internalHovered()));
    }
    if(d->exclusive)
        a->setCheckable(true);
    if(!a->d->forceDisabled) {
        a->setEnabled(d->enabled);
        a->d->forceDisabled = false;
    }
    if(!a->d->forceInvisible) {
        a->setVisible(d->visible);
        a->d->forceInvisible = false;
    }
    if(a->actionGroup() != this)
        a->setActionGroup(this);
    return a;
}

/*!
    Adds an action with \a text and an shortcut of \a
    shortcut. This newly created action's parent is set to this action
    group and it is returned.

    Normally an action is added to a group by creating it with the
    group as parent, so this function is not usually used.

    \sa QAction::setActionGroup()
*/
QAction *QActionGroup::addAction(const QString &text, const QKeySequence &shortcut)
{
    return new QAction(text, shortcut, this);
}

/*!
    Creates and returns an action with \a text, an \a icon, and
    a \a shortcut. The newly created action is a child of this action
    group.

    Normally an action is added to a group by creating it with the
    group as its parent, so this function is not usually used.

    \sa QAction::setActionGroup()
*/
QAction *QActionGroup::addAction(const QIconSet &icon, const QString &text, const QKeySequence &shortcut)
{
    return new QAction(icon, text, shortcut, this);
}

/*!
  Removes action \a action from this group. The action \a action's
  action group is set to 0.

  \sa QAction::setActionGroup()
*/
void QActionGroup::removeAction(QAction *action)
{
    action->setActionGroup(0);
}

/*!
    Returns the (possibly empty) list of this groups's actions.
*/
QList<QAction*> QActionGroup::actions() const
{
    return d->actions;
}

/*!
    \property QActionGroup::exclusive
    \brief whether the action group does exclusive checking

    If exclusive is true only one checkable action in the action group
    can ever be active at any one time. If the user chooses another
    checkable action in the group the one they chose becomes active and
    the one that was active becomes inactive.

    \sa Q3Action::checkable
*/
void QActionGroup::setExclusive(bool b)
{
    d->exclusive = b;
    for(QList<QAction*>::Iterator it = d->actions.begin(); it != d->actions.end(); ++it)
        (*it)->setCheckable(b);
}

bool QActionGroup::isExclusive() const
{
    return d->exclusive;
}

/*!
    \property QActionGroup::enabled
    \brief whether the action group is enabled

    All action's in the action group will match the enabled state of
    this action group if the action has not been explicitly disabled.

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
  Returns the currently checked action in the group, or 0 if none are checked
*/
QAction *QActionGroup::checkedAction() const
{
    return d->current;
}

/*!
    \property QActionGroup::visible
    \brief whether the action group is visible

    All action's in the action group will match the visible state of
    this action group if the action has not been explicitly hidden.

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
  \internal
*/
void QActionGroup::internalDataChanged()
{
    QAction *action = qt_cast<QAction*>(sender());
    if(!action)
        qWarning("not possible..");
    if(d->exclusive && action->isChecked() && action != d->current) {
        if(d->current)
            d->current->setChecked(false);
        d->current = action;
    }
}

/*!
  \internal
*/
void QActionGroup::internalTriggered()
{
    QAction *action = qt_cast<QAction*>(sender());
    if(!action)
        qWarning("not possible..");
    emit triggered(action);
    emit selected(action);
}

/*!
  \internal
*/
void QActionGroup::internalHovered()
{
    QAction *action = qt_cast<QAction*>(sender());
    if(!action)
        qWarning("not possible..");
    emit hovered(action);
}


