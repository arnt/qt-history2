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
#include "qactiongroup.h"

#include "qaction_p.h"
#include "qapplication.h"
#include "qevent.h"
#include "qlist.h"

#define d d_func()
#define q q_func()

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
    \fn void QAction::trigger()

    This is a convenience slot that calls activate(Trigger).
*/

/*!
    \fn void QAction::hover()

    This is a convenience slot that calls activate(Hover).
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
    Constructs an action with some \a menuText for the \a parent action
    group. The action will be automatically inserted into the
    action group.

    The action uses a stripped version of \a menuText (e.g. "\&Menu
    Option..." becomes "Menu Option") as descriptive text for
    toolbuttons. You can override this by setting a specific
    description with setText(). The same text will be used for
    tool tips unless you specify a different test using
    setToolTip().

*/
QAction::QAction(const QString &menuText, QActionGroup* parent)
    : QObject(*(new QActionPrivate), parent)
{
    d->menuText = menuText;
    d->group = parent;
    if(parent)
        parent->addAction(this);
}

/*!
    Constructs an action with an \a icon and some \a menuText for the
    \a parent action group. The action will be automatically inserted
    into the action group.

    The action uses a stripped version of \a menuText (e.g. "\&Menu
    Option..." becomes "Menu Option") as descriptive text for
    toolbuttons. You can override this by setting a specific
    description with setText(). The same text will be used for
    tool tips unless you specify a different test using
    setToolTip().
*/
QAction::QAction(const QIconSet &icon, const QString &menuText, QActionGroup* parent)
    : QObject(*(new QActionPrivate), parent)
{
    d->icons = new QIconSet(icon);
    d->menuText = menuText;
    d->group = parent;
    if(parent)
        parent->addAction(this);
}

/*!
    Constructs an action with some \a menuText for the \a parent widget.
    The action will \e not be automatically inserted into the widget.

    The action uses a stripped version of \a menuText (e.g. "\&Menu
    Option..." becomes "Menu Option") as descriptive text for
    toolbuttons. You can override this by setting a specific
    description with setText(). The same text will be used for
    tool tips unless you specify a different test using
    setToolTip().

*/
QAction::QAction(const QString &menuText, QWidget* parent)
    : QObject(*(new QActionPrivate), parent)
{
    d->menuText = menuText;
}

/*!
    Constructs an action with the given \a icon and \a menuText for the
    \a parent widget. The action will \e not be automatically inserted
    into the widget.

    The action uses a stripped version of \a menuText (e.g. "\&Menu
    Option..." becomes "Menu Option") as descriptive text for
    toolbuttons. You can override this by setting a specific
    description with setText(). The same text will be used for
    tool tips unless you specify a different test using
    setToolTip().

*/
QAction::QAction(const QIconSet &icon, const QString &menuText, QWidget* parent)
    : QObject(*(new QActionPrivate), parent)
{
    d->menuText = menuText;
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

    The font property is used to render the text set on the
    QAction. The font will can be considered a hint as it will not be
    consulted in all cases based upon application and style.

    \sa QAction::setText() QStyle
*/
void QAction::setFont(const QFont &font)
{
    if (d->font == font)
        return;

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
    if (d->menu == menu)
        return;

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
    if (d->separator == b)
        return;

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
    if (d->text == text)
        return;

    d->text = text;
    d->sendDataChanged();
}




QString QAction::text() const
{
    if (d->text.isEmpty()) {
        QString s = d->menuText;
        s.remove(QLatin1String("..."));
        s.remove(QChar('&')); //## for loop because of &&
        return s.trimmed();
    }
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
    if (d->menuText == text)
        return;

    d->menuText = text;
    d->sendDataChanged();
}

QString QAction::menuText() const
{
    QString s = d->menuText;
    if(s.isEmpty()) {
        s = d->text;
        s.replace('&', "&&");
    }
    return s;
}


/*!
    \property QAction::toolTip
    \brief the action's tool tip

    This text is used for the tool tip. If no status tip has been set
    the tool tip will be used for the status tip.

    If no tool tip is specified the action's text is used.

    \sa setStatusTip() setShortcut()
*/
void QAction::setToolTip(const QString &tooltip)
{
    if (d->tooltip == tooltip)
        return;

    d->tooltip = tooltip;
    d->sendDataChanged();
}

QString QAction::toolTip() const
{
    if (d->tooltip.isEmpty())
        return text();
    return d->tooltip;
}

/*!
    \property QAction::statusTip
    \brief the action's status tip

    The statusTip is displayed on all status bars provided by the
    action's top-level parent widget. It can be set, and is provided as the parameter
    of the showStatusMessage() signal.

    There is no default statusTip text.

    \sa setStatusTip() setToolTip() showStatusMessage()
*/
void QAction::setStatusTip(const QString &statustip)
{
    if (d->statustip == statustip)
        return;

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

    The "What's This?" text is used to provide a brief description of
    the action. The text may contain rich text. There is no default
    "What's This?" text.

    If the text contains a hyperlink, the whatsThisClicked() signal is
    emitted when the user clicks inside the "What's This?" window.

    \sa QWhatsThis whatsThisClicked() QStyleSheet
*/
void QAction::setWhatsThis(const QString &whatsthis)
{
    if (d->whatsthis == whatsthis)
        return;

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
    if (d->checkable == b)
        return;

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
    if (d->checked == b)
        return;

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
    if (d->forceDisabled != b)
        return;

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
    if (d->forceInvisible != b)
        return;

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

#include "moc_qaction.cpp"

