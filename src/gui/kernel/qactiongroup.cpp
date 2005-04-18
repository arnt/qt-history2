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
#include "qactiongroup.h"

#include "qaction_p.h"
#include "qapplication.h"
#include "qevent.h"
#include "qlist.h"

class QActionGroupPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QActionGroup)
public:
    QActionGroupPrivate() : exclusive(1), enabled(1), visible(1)  { }
    QList<QAction *> actions;
    QPointer<QAction> current;
    uint exclusive : 1;
    uint enabled : 1;
    uint visible : 1;

private:
    void actionTriggered();  //private slot
    void actionChanged();    //private slot
    void actionHovered();    //private slot
};

void QActionGroupPrivate::actionChanged()
{
    Q_Q(QActionGroup);
    QAction *action = qobject_cast<QAction*>(q->sender());
    Q_ASSERT_X(action != 0, "QWidgetGroup::actionChanged", "internal error");
    if(exclusive && action->isChecked() && action != current) {
        if(current)
            current->setChecked(false);
        current = action;
    }
}

void QActionGroupPrivate::actionTriggered()
{
    Q_Q(QActionGroup);
    QAction *action = qobject_cast<QAction*>(q->sender());
    Q_ASSERT_X(action != 0, "QWidgetGroup::actionTriggered", "internal error");
    emit q->triggered(action);
    emit q->selected(action);
}

void QActionGroupPrivate::actionHovered()
{
    Q_Q(QActionGroup);
    QAction *action = qobject_cast<QAction*>(q->sender());
    Q_ASSERT_X(action != 0, "QWidgetGroup::actionHovered", "internal error");
    emit q->hovered(action);
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

    Here's an example from demos/textedit:
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
    using addSeparator(). Action groups are added to widgets with the
    QWidget::addActions() function.
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
    Q_D(QActionGroup);
    if(!d->actions.contains(a)) {
        d->actions.append(a);
        QObject::connect(a, SIGNAL(triggered()), this, SLOT(actionTriggered()));
        QObject::connect(a, SIGNAL(changed()), this, SLOT(actionChanged()));
        QObject::connect(a, SIGNAL(hovered()), this, SLOT(actionHovered()));
    }
    if(!a->d_func()->forceDisabled) {
        a->setEnabled(d->enabled);
        a->d_func()->forceDisabled = false;
    }
    if(!a->d_func()->forceInvisible) {
        a->setVisible(d->visible);
        a->d_func()->forceInvisible = false;
    }
    if(a->d_func()->group != this)
        a->d_func()->group = this;
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
QAction *QActionGroup::addAction(const QIcon &icon, const QString &text)
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
    Q_D(QActionGroup);
    if (d->actions.removeAll(action)) {
        QObject::disconnect(action, SIGNAL(triggered()), this, SLOT(actionTriggered()));
        QObject::disconnect(action, SIGNAL(changed()), this, SLOT(actionChanged()));
        QObject::disconnect(action, SIGNAL(hovered()), this, SLOT(actionHovered()));
        action->d_func()->group = 0;
    }
}

/*!
    Returns the list of this groups's actions. This may be empty.
*/
QList<QAction*> QActionGroup::actions() const
{
    Q_D(const QActionGroup);
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
    Q_D(QActionGroup);
    d->exclusive = b;
}

bool QActionGroup::isExclusive() const
{
    Q_D(const QActionGroup);
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
    Q_D(QActionGroup);
    d->enabled = b;
    for(QList<QAction*>::Iterator it = d->actions.begin(); it != d->actions.end(); ++it) {
        if(!(*it)->d_func()->forceDisabled) {
            (*it)->setEnabled(b);
            (*it)->d_func()->forceDisabled = false;
        }
    }
}

bool QActionGroup::isEnabled() const
{
    Q_D(const QActionGroup);
    return d->enabled;
}

/*!
  Returns the currently checked action in the group, or 0 if none
  are checked.
*/
QAction *QActionGroup::checkedAction() const
{
    Q_D(const QActionGroup);
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
    Q_D(QActionGroup);
    d->visible = b;
    for(QList<QAction*>::Iterator it = d->actions.begin(); it != d->actions.end(); ++it) {
        if(!(*it)->d_func()->forceInvisible) {
            (*it)->setVisible(b);
            (*it)->d_func()->forceInvisible = false;
        }
    }
}

bool QActionGroup::isVisible() const
{
    Q_D(const QActionGroup);
    return d->visible;
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

/*!
    \fn void QActionGroup::add(QAction* a)

    Use addAction() instead.
*/

/*!
    \fn void QActionGroup::addSeparator()

    Normally you add a separator to the menus or widgets to which
    actions are added, so this function is very rarely needed.

    \oldcode
    actionGroup->addSeparator();
    \newcode
    QAction *separator = new QAction(this);
    separator->setSeparator(true);
    actionGroup->addAction(separator);
    \endcode
*/

/*!
    \fn bool QActionGroup::addTo(QWidget *widget)

    \oldcode
    actionGroup->addTo(widget);
    \newcode
    widget->addActions(actionGroup->actions());
    \endcode
*/

/*!
    \fn void QActionGroup::selected(QAction *action);

    Use triggered() instead.

*/

#include "moc_qactiongroup.cpp"
