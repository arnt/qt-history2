/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
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
#include "qdebug.h"
#include <private/qshortcutmap_p.h>
#include <private/qapplication_p.h>

/*
  internal: guesses a descriptive text from a text suited for a menu entry
 */
static QString qt_strippedText(QString s)
{
    s.remove( QString::fromLatin1("...") );
    int i = 0;
    while (i < s.size()) {
        ++i;
        if (s.at(i-1) != QLatin1Char('&'))
            continue;
        if (i < s.size() && s.at(i) == QLatin1Char('&'))
            ++i;
        s.remove(i-1,1);
    }
    return s.trimmed();
};


QActionPrivate::QActionPrivate() : group(0), enabled(1), forceDisabled(0),
                                   visible(1), forceInvisible(0), checkable(0), checked(0), separator(0)
{
#ifdef QT3_SUPPORT
    static int qt_static_action_id = -1;
    param = id = --qt_static_action_id;
    act_signal = 0;
#endif
    shortcutId = 0;
    shortcutContext = Qt::WindowShortcut;
}

QActionPrivate::~QActionPrivate()
{
}

void QActionPrivate::sendDataChanged()
{
    Q_Q(QAction);
    QActionEvent e(QEvent::ActionChanged, q);
    for (int i = 0; i < widgets.size(); ++i) {
        QWidget *w = widgets.at(i);
        QApplication::sendEvent(w, &e);
    }
    QApplication::sendEvent(q, &e);

    emit q->changed();
}

void QActionPrivate::redoGrab(QShortcutMap &map)
{
    Q_Q(QAction);
    if (shortcutId)
        map.removeShortcut(shortcutId, q);
    if (shortcut.isEmpty())
        return;
    shortcutId = map.addShortcut(q, shortcut, shortcutContext);
    if (!enabled)
        map.setShortcutEnabled(false, shortcutId, q);
}

void QActionPrivate::setShortcutEnabled(bool enable, QShortcutMap &map)
{
    Q_Q(QAction);
    if (shortcutId)
        map.setShortcutEnabled(enable, shortcutId, q);
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
    setIcon(), setText(), setIconText(), setShortcut(),
    setStatusTip(), setWhatsThis(), and setToolTip(). For menu items,
    it is possible to set an individual font with setFont().

    Actions are added to widgets using QWidget::addAction().

    Once a QAction has been created it should be added to the relevant
    menu and toolbar, then connected to the slot which will perform
    the action. For example:

    \quotefile mainwindows/application/mainwindow.cpp
    \skipto openAct
    \printuntil connect
    \skipto fileMenu->addAction(openAct
    \printuntil fileMenu->addAction(openAct
    \skipto fileToolBar->addAction(openAct
    \printuntil fileToolBar->addAction(openAct

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
    Constructs an action with \a parent. If \a parent is an action
    group the action will be automatically inserted into the group.
*/
QAction::QAction(QObject* parent)
    : QObject(*(new QActionPrivate), parent)
{
    Q_D(QAction);
    d->group = qobject_cast<QActionGroup *>(parent);
    if (d->group)
        d->group->addAction(this);
}


/*!
    Constructs an action with some \a text and \a parent. If \a
    parent is an action group the action will be automatically
    inserted into the group.

    The action uses a stripped version of \a text (e.g. "\&Menu
    Option..." becomes "Menu Option") as descriptive text for
    toolbuttons. You can override this by setting a specific
    description with setText(). The same text will be used for
    tool tips unless you specify a different test using
    setToolTip().

*/
QAction::QAction(const QString &text, QObject* parent)
    : QObject(*(new QActionPrivate), parent)
{
    Q_D(QAction);
    d->text = text;
    d->group = qobject_cast<QActionGroup *>(parent);
    if (d->group)
        d->group->addAction(this);
}

/*!
    Constructs an action with an \a icon and some \a text and \a
    parent. If \a parent is an action group the action will be
    automatically inserted into the group.

    The action uses a stripped version of \a text (e.g. "\&Menu
    Option..." becomes "Menu Option") as descriptive text for
    toolbuttons. You can override this by setting a specific
    description with setText(). The same text will be used for
    tool tips unless you specify a different test using
    setToolTip().
*/
QAction::QAction(const QIcon &icon, const QString &text, QObject* parent)
    : QObject(*(new QActionPrivate), parent)
{
    Q_D(QAction);
    d->icon = icon;
    d->text = text;
    d->group = qobject_cast<QActionGroup *>(parent);
    if (d->group)
        d->group->addAction(this);
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
    Q_D(QAction);
    if (d->shortcut == shortcut)
        return;

    d->shortcut = shortcut;
    d->redoGrab(qApp->d_func()->shortcutMap);
    d->sendDataChanged();
}

QKeySequence QAction::shortcut() const
{
    Q_D(const QAction);
    return d->shortcut;
}

/*!
    \property QAction::shortcutContext
    \brief the context for the action's shortcut

    Valid values for this property can be found in \l Qt::ShortcutContext.
    The default value is Qt::WindowShortcut.
*/

void QAction::setShortcutContext(Qt::ShortcutContext context)
{
    Q_D(QAction);
    if (d->shortcutContext == context)
        return;
    d->shortcutContext = context;
    d->redoGrab(qApp->d_func()->shortcutMap);
    d->sendDataChanged();
}

Qt::ShortcutContext QAction::shortcutContext() const
{
    Q_D(const QAction);
    return d->shortcutContext;
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
    Q_D(QAction);
    if (d->font == font)
        return;

    d->font = font;
    d->sendDataChanged();
}

QFont QAction::font() const
{
    Q_D(const QAction);
    return d->font;
}

#ifdef QT3_SUPPORT
/*!
    Use one of the QAction constructors that doesn't take a \a name
    argument and call setObjectName() instead.
*/
QAction::QAction(QObject* parent, const char* name)
 : QObject(*(new QActionPrivate), parent)
{
    Q_D(QAction);
    setObjectName(name);
    d->group = qobject_cast<QActionGroup *>(parent);
    if (d->group)
        d->group->addAction(this);
}


/*!
    Use one of the QAction constructors that doesn't take a \a name
    argument and call setObjectName() instead.
*/
QAction::QAction(const QString &text, const QKeySequence &shortcut, QObject* parent, const char* name)
 : QObject(*(new QActionPrivate), parent)
{
    Q_D(QAction);
    setObjectName(name);
    d->text = text;
    setShortcut(shortcut);
    d->group = qobject_cast<QActionGroup *>(parent);
    if (d->group)
        d->group->addAction(this);
}

/*!
    Use one of the QAction constructors that doesn't take a \a name
    argument and call setObjectName() instead.
*/
QAction::QAction(const QIcon &icon, const QString &text, const QKeySequence &shortcut,
                 QObject* parent, const char* name)
    : QObject(*(new QActionPrivate), parent)
{
    Q_D(QAction);
    setObjectName(name);
    d->text = text;
    setShortcut(shortcut);
    d->icon = icon;
    d->group = qobject_cast<QActionGroup *>(parent);
    if (d->group)
        d->group->addAction(this);
}
#endif

/*!
    Destroys the object and frees allocated resources.
*/
QAction::~QAction()
{
    Q_D(QAction);
    for (int i = d->widgets.size()-1; i >= 0; --i) {
        QWidget *w = d->widgets.at(i);
        w->removeAction(this);
    }
    if (d->group)
        d->group->removeAction(this);

    if (d->shortcutId && qApp)
        qApp->d_func()->shortcutMap.removeShortcut(d->shortcutId, this);
}

/*!
  Sets this action group to \a group. The action will be automatically
  added to the group's list of actions.

  Actions within the group will be mutually exclusive.

  \sa QActionGroup, QAction::actionGroup()
*/
void QAction::setActionGroup(QActionGroup *group)
{
    Q_D(QAction);
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
    Q_D(const QAction);
    return d->group;
}


/*!
    \property QAction::icon
    \brief the action's icon

    In toolbars, the icon is used as the tool button icon; in menus,
    it is displayed to the left of the menu text. There is no default
    icon.

    If a null icon (QIcon::isNull() is passed into this function,
    the icon of the action is cleared.
*/
void QAction::setIcon(const QIcon &icon)
{
    Q_D(QAction);
    d->icon = icon;
    d->sendDataChanged();
}

QIcon QAction::icon() const
{
    Q_D(const QAction);
    return d->icon;
}

/*!
  Returns this action's submenu.

  \sa QAction::setMenu()
*/
QMenu *QAction::menu() const
{
    Q_D(const QAction);
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
    Q_D(QAction);
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
    Q_D(const QAction);
    return d->separator;
}

/*!
    \property QAction::text
    \brief the action's descriptive text

    If the action is added to a menu, the menu option will consist of
    the icon (if there is one), the text, and the shortcut (if there
    is one). If the text is not explicitly set in the constructor, or
    by using setText(), the action's description icon text will be
    used as text. There is no default text.

    \sa iconText
*/
void QAction::setText(const QString &text)
{
    Q_D(QAction);
    if (d->text == text)
        return;

    d->text = text;
    d->sendDataChanged();
}

QString QAction::text() const
{
    Q_D(const QAction);
    QString s = d->text;
    if(s.isEmpty()) {
        s = d->iconText;
        s.replace('&', "&&");
    }
    return s;
}





/*!
    \property QAction::iconText
    \brief the action's descriptive icon text

    If QMainWindow::usesTextLabel is true, the text appears as a label
    in the relevant tool button. It also serves as the default text in
    menus and tool tips if these have not been specifically defined
    with setText() or setToolTip(). If the icon text is not explicitly
    set in the by using setIconText(), the action's normal text will
    be used as icon text. There is no default icon text.

    \sa setToolTip() setStatusTip()
*/
void QAction::setIconText(const QString &text)
{
    Q_D(QAction);
    if (d->iconText == text)
        return;

    d->iconText = text;
    d->sendDataChanged();
}

QString QAction::iconText() const
{
    Q_D(const QAction);
    if (d->iconText.isEmpty())
        return qt_strippedText(d->text);
    return d->iconText;
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
    Q_D(QAction);
    if (d->tooltip == tooltip)
        return;

    d->tooltip = tooltip;
    d->sendDataChanged();
}

QString QAction::toolTip() const
{
    Q_D(const QAction);
    if (d->tooltip.isEmpty()) {
        if (!d->text.isEmpty())
            return qt_strippedText(d->text);
        return d->iconText;
    }
    return d->tooltip;
}

/*!
    \property QAction::statusTip
    \brief the action's status tip

    The statusTip is displayed on all status bars provided by the
    action's top-level parent widget. It can be set, and is provided as the parameter
    of the showStatusMessage() signal.

    There is no default statusTip text.

    \sa setStatusTip() setToolTip() showStatusText()
*/
void QAction::setStatusTip(const QString &statustip)
{
    Q_D(QAction);
    if (d->statustip == statustip)
        return;

    d->statustip = statustip;
    d->sendDataChanged();
}

QString QAction::statusTip() const
{
    Q_D(const QAction);
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
    Q_D(QAction);
    if (d->whatsthis == whatsthis)
        return;

    d->whatsthis = whatsthis;
    d->sendDataChanged();
}

QString QAction::whatsThis() const
{
    Q_D(const QAction);
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
    Q_D(QAction);
    if (d->checkable == b)
        return;

    d->checkable = b;
    d->checked = false;
    d->sendDataChanged();
}

bool QAction::isCheckable() const
{
    Q_D(const QAction);
    return d->checkable;
}

/*!
    \fn void QAction::toggle()

    This is a convenience function for the \l checked property.
    Connect to it to change the checked state to its opposite state.
*/
void QAction::toggle()
{
    Q_D(QAction);
    setChecked(!d->checked);
}

/*!
    \property QAction::checked
    \brief whether the action is checked.

    Only checkable actions can be checked.  By default, this is false
    (the action is unchecked).

    \sa checkable
*/
void QAction::setChecked(bool b)
{
    Q_D(QAction);
    if (!d->checkable || d->checked == b)
        return;

    QObject *guard = this;
    QMetaObject::addGuard(&guard);
    d->checked = b;
    d->sendDataChanged();
    if (guard)
        emit toggled(b);
    QMetaObject::removeGuard(&guard);
}

bool QAction::isChecked() const
{
    Q_D(const QAction);
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
    Q_D(QAction);
    if (b == d->enabled && b != d->forceDisabled)
        return;
    d->forceDisabled = !b;
    if (b && d->group && !d->group->isEnabled())
        return;
    d->enabled = b;
    d->setShortcutEnabled(b, qApp->d_func()->shortcutMap);
    d->sendDataChanged();
}

bool QAction::isEnabled() const
{
    Q_D(const QAction);
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
    Q_D(QAction);
    if (b == d->visible && b != d->forceInvisible)
        return;
    d->forceInvisible = !b;
    d->visible = b;
    d->sendDataChanged();
}


bool QAction::isVisible() const
{
    Q_D(const QAction);
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
        Q_ASSERT_X(se->key() == d_func()->shortcut,
                   "QAction::event",
                   "Received shortcut event from incorrect shortcut");
        if (se->isAmbiguous())
            qWarning("QAction::eventFilter: ambiguous shortcut overload: %s", QString(se->key()).toLatin1().constData());
        else
            activate(Trigger);
        return true;
    }
    return false;
}

/*!
  Returns the user data as set in QAction::setData.

  \sa setData(const QVariant &d)
*/
QVariant
QAction::data() const
{
    Q_D(const QAction);
    return d->userData;
}

/*!
  Sets internal data to \a data. This can be used for user data to store anything that a
  QVariant can store. The ownership of anything the the user data will remain with the
  variant and thus be referenced counted as appropriate.

  \sa data()
*/
void
QAction::setData(const QVariant &data)
{
    Q_D(QAction);
    d->userData = data;
    d->sendDataChanged();
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
    Q_D(QAction);
    if(event == Trigger) {
        QObject *guard = this;
        QMetaObject::addGuard(&guard);
        if(d->checkable) {
            // the checked action of an exclusive group cannot be  unchecked
            if (d->checked && (d->group && d->group->isExclusive()
                               && d->group->checkedAction() == this)) {
                QMetaObject::removeGuard(&guard);
                return;
            }
            setChecked(!d->checked);
        }
        if (guard)
            emit triggered(d->checked);
#ifdef QT3_SUPPORT
        if (guard)
            emit activated(d->param);
#endif
        QMetaObject::removeGuard(&guard);
    } else if(event == Hover) {
        emit hovered();
    }
}

/*!
    \fn void QAction::triggered(bool checked)

    This signal is emitted when an action is activated by the user;
    for example, when the user clicks a menu option, toolbar button,
    or presses an action's shortcut key combination, or when trigger()
    was called. Notably, it is \e not emitted when setChecked() or
    toggle() is called.

    If the action is checkable, \a checked is true if the action is
    checked, or false if the action is unchecked.

    \sa QAction::activate(), QAction::checked(), QAction::toggled()
*/

/*!
    \fn void QAction::toggled(bool checked)

    This signal is emitted whenever a checkable action changes its
    isChecked() status. This can be the result of a user interaction,
    or because setChecked() was called.

    \a checked is true if the action is checked, or false if the
    action is unchecked.

    \sa QAction::activate(), QAction::triggered(), QAction::checked(),
    QAction::setChecked()
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
    \enum QAction::ActionEvent

    This enum type is used when calling QAction::activate()

    \value Trigger this will cause the QAction::triggered() signal to be emitted.

    \value Hover this will cause the QAction::hovered() signal to be emitted.
*/

/*!
    \fn void QAction::setMenuText(const QString &text)

    Use setText() instead.
*/

/*!
    \fn QString QAction::menuText() const

    Use text() instead.
*/

/*!
    \fn bool QAction::isOn() const

    Use isChecked() instead.
*/

/*!
    \fn void QAction::setOn(bool b)

    Use setChecked() instead.
*/

/*!
    \fn bool QAction::isToggleAction() const

    Use isCheckable() instead.
*/

/*!
    \fn void QAction::setToggleAction(bool b)

    Use setCheckable() instead.
*/

/*!
    \fn void QAction::setIconSet(const QIcon &i)

    Use setIcon() instead.
*/

/*!
    \fn bool QAction::addTo(QWidget *w)

    Use QWidget::addAction() instead.

    \oldcode
    action->addTo(widget);
    \newcode
    widget->addAction(action);
    \endcode
*/

/*!
    \fn bool QAction::removeFrom(QWidget *w)

    Use QWidget::removeAction() instead.

    \oldcode
    action->removeFrom(widget);
    \newcode
    widget->removeAction(action);
    \endcode
*/

/*!
    \fn void QAction::setAccel(const QKeySequence &shortcut)

    Use setShortcut() instead.
*/

/*!
    \fn QIcon QAction::iconSet() const

    Use icon() instead.
*/

/*!
    \fn QKeySequence QAction::accel() const

    Use shortcut() instead.
*/

/*!
    \fn void QAction::activated(int i);

    Use triggered() instead.
*/


#include "moc_qaction.cpp"

