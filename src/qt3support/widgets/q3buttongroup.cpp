/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "q3buttongroup.h"
#include "qabstractbutton.h"
#include "qmap.h"
#include "qapplication.h"
#include "qradiobutton.h"
#include "qevent.h"
#include "qset.h"


/*!
    \class Q3ButtonGroup
    \brief The Q3ButtonGroup widget organizes QAbstractButton widgets in a group.

    \compat

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
    in the order they are created. A Q3ButtonGroup can display a frame
    and a title because it inherits Q3GroupBox.
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

    \sa QPushButton, QCheckBox, QRadioButton
*/

/*!
    \property Q3ButtonGroup::exclusive
    \brief whether the button group is exclusive

    If this property is true, then the buttons in the group are
    toggled, and to untoggle a button you must click on another button
    in the group. The default value is false.
*/

/*!
    \property Q3ButtonGroup::radioButtonExclusive
    \brief whether the radio buttons in the group are exclusive

    If this property is true (the default), the \link QRadioButton
    radio buttons\endlink in the group are treated exclusively.
*/


/*!
    Constructs a button group with no title.

    The \a parent and \a name arguments are passed to the QWidget
    constructor.
*/

Q3ButtonGroup::Q3ButtonGroup(QWidget *parent, const char *name)
    : Q3GroupBox(parent, name)
{
    init();
}

/*!
    Constructs a button group with the title \a title.

    The \a parent and \a name arguments are passed to the QWidget
    constructor.
*/

Q3ButtonGroup::Q3ButtonGroup(const QString &title, QWidget *parent,
                            const char *name)
    : Q3GroupBox(title, parent, name)
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

Q3ButtonGroup::Q3ButtonGroup(int strips, Qt::Orientation orientation,
                            QWidget *parent, const char *name)
    : Q3GroupBox(strips, orientation, parent, name)
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

Q3ButtonGroup::Q3ButtonGroup(int strips, Qt::Orientation orientation,
                            const QString &title, QWidget *parent,
                            const char *name)
    : Q3GroupBox(strips, orientation, title, parent, name)
{
    init();
}

/*!
    Initializes the button group.
*/

void Q3ButtonGroup::init()
{
    group.setExclusive(false);
    radio_excl = true;
}

/*! Destructor. */

Q3ButtonGroup::~Q3ButtonGroup()
{
}

bool Q3ButtonGroup::isExclusive() const
{
    return group.exclusive();
}

void Q3ButtonGroup::setExclusive(bool enable)
{
    group.setExclusive(enable);
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

int Q3ButtonGroup::insert(QAbstractButton *button, int id)
{
    remove(button);
    return insert_helper(button, id);
}

int Q3ButtonGroup::insert_helper(QAbstractButton *button, int id)
{
    if (isExclusive() || !qobject_cast<QRadioButton*>(button))
        group.addButton(button);

    static int seq_no = -2;
    if (id < -1)
        id = seq_no--;
    else if (id == -1)
        id = buttonIds.count();
    buttonIds.insert(id, button);
    connect(button, SIGNAL(pressed()) , SLOT(buttonPressed()));
    connect(button, SIGNAL(released()), SLOT(buttonReleased()));
    connect(button, SIGNAL(clicked()) , SLOT(buttonClicked()));
    return id;
}

/*!
    Returns the number of buttons in the group.
*/
int Q3ButtonGroup::count() const
{
    fixChildren();
    return buttonIds.count();
}

/*!
    Removes the \a button from the button group.

    \sa insert()
*/

void Q3ButtonGroup::remove(QAbstractButton *button)
{
    fixChildren();
    QMap<int, QAbstractButton*>::Iterator it = buttonIds.begin();
    while (it != buttonIds.end()) {
        if (it.value() == button) {
            buttonIds.erase(it);
            button->disconnect(this);
            group.removeButton(button);
            break;
        }
        ++it;
    }
}


/*!
    Returns the button with the specified identifier \a id, or 0 if
    the button was not found.
*/

QAbstractButton *Q3ButtonGroup::find(int id) const
{
    fixChildren();
    return buttonIds.value(id);
}


/*!
    \fn void Q3ButtonGroup::pressed(int id)

    This signal is emitted when a button in the group is \link
    QAbstractButton::pressed() pressed\endlink. The \a id argument is the
    button's identifier.

    \sa insert()
*/

/*!
    \fn void Q3ButtonGroup::released(int id)

    This signal is emitted when a button in the group is \link
    QAbstractButton::released() released\endlink. The \a id argument is the
    button's identifier.

    \sa insert()
*/

/*!
    \fn void Q3ButtonGroup::clicked(int id)

    This signal is emitted when a button in the group is \link
    QAbstractButton::clicked() clicked\endlink. The \a id argument is the
    button's identifier.

    \sa insert()
*/


/*!
  \internal
  This slot is activated when one of the buttons in the group emits the
  QAbstractButton::pressed() signal.
*/

void Q3ButtonGroup::buttonPressed()
{
    QAbstractButton *senderButton = ::qobject_cast<QAbstractButton *>(sender());
    Q_ASSERT(senderButton);
    int senderId = id(senderButton);
    if (senderId != -1)
        emit pressed(senderId);
}

/*!
  \internal
  This slot is activated when one of the buttons in the group emits the
  QAbstractButton::released() signal.
*/

void Q3ButtonGroup::buttonReleased()
{
    QAbstractButton *senderButton = ::qobject_cast<QAbstractButton *>(sender());
    Q_ASSERT(senderButton);
    int senderId = id(senderButton);
    if (senderId != -1)
        emit released(senderId);
}

/*!
  \internal
  This slot is activated when one of the buttons in the group emits the
  QAbstractButton::clicked() signal.
*/

void Q3ButtonGroup::buttonClicked()
{
    QAbstractButton *senderButton = ::qobject_cast<QAbstractButton *>(sender());
    Q_ASSERT(senderButton);
    int senderId = id(senderButton);
    if (senderId != -1)
        emit clicked(senderId);
}

void Q3ButtonGroup::setButton(int id)
{
    QAbstractButton *b = find(id);
    if (b)
        b->setOn(true);
}

void Q3ButtonGroup::setRadioButtonExclusive(bool on)
{
    radio_excl = on;
}


/*!
    Returns the selected toggle button if exactly one is selected;
    otherwise returns 0.

    \sa selectedId()
*/

QAbstractButton *Q3ButtonGroup::selected() const
{
    fixChildren();
    QAbstractButton *candidate = 0;
    QMap<int, QAbstractButton*>::ConstIterator it = buttonIds.constBegin();
    while (it != buttonIds.constEnd()) {
        if (it.value()->isCheckable() && it.value()->isChecked()) {
            if (candidate)
                return 0;
            candidate = it.value();
        }
        ++it;
    }
    return candidate;
}

/*!
    \property Q3ButtonGroup::selectedId
    \brief The id of the selected toggle button.

    If no toggle button is selected, id() returns -1.

    If setButton() is called on an exclusive group, the button with
    the given id will be set to on and all the others will be set to
    off.

    \sa selected()
*/

int Q3ButtonGroup::selectedId() const
{
    return id(selected());
}


/*!
    Returns the id of \a button, or -1 if \a button is not a member of
    this group.

    \sa selectedId()
*/

int Q3ButtonGroup::id(QAbstractButton *button) const
{
    fixChildren();
    QMap<int, QAbstractButton*>::ConstIterator it = buttonIds.constBegin();
    while (it != buttonIds.constEnd()) {
        if (it.value() == button)
            return it.key();
        ++it;
    }
    return -1;
}


/*!
    \reimp
*/
bool Q3ButtonGroup::event(QEvent * e)
{
    if (e->type() == QEvent::ChildInserted) {
        QChildEvent * ce = (QChildEvent *) e;
        if (QAbstractButton *button = qobject_cast<QAbstractButton*>(ce->child())) {
            button->setAutoExclusive(false);
            if (group.exclusive() || qobject_cast<QRadioButton*>(button)) {
                button->setAutoExclusive(true);
                QMap<int, QAbstractButton*>::ConstIterator it = buttonIds.constBegin();
                while (it != buttonIds.constEnd()) {
                    if (it.value() == button)
                        return Q3GroupBox::event(e);
                    ++it;
                }
            }
            insert(button, id(button));
        }
    }
    return Q3GroupBox::event(e);
}

void Q3ButtonGroup::fixChildren() const
{
    if (children().count() == buttonIds.count())
        return; // small optimization, all our children have ids.

    QList<QAbstractButton *> list = ::qFindChildren<QAbstractButton*>(this);
    QSet<QAbstractButton*> set;
    for (QMap<int, QAbstractButton*>::ConstIterator it = buttonIds.constBegin();
            it != buttonIds.constEnd(); ++it)
        set.insert(*it);
    for (int i = 0; i < list.count(); ++i)
        if (!set.contains(list.at(i)))
            const_cast<Q3ButtonGroup*>(this)->insert_helper(list.at(i));
}


/*!
    \class Q3HButtonGroup
    \brief The Q3HButtonGroup widget organizes button widgets in a
    group with one horizontal row.

    \compat

    Q3HButtonGroup is a convenience class that offers a thin layer on
    top of Q3ButtonGroup. From a layout point of view it is effectively
    a Q3HBoxWidget that offers a frame with a title and is specifically
    designed for buttons. From a functionality point of view it is a
    Q3ButtonGroup.

    \sa Q3VButtonGroup
*/

/*!
    \fn Q3HButtonGroup::Q3HButtonGroup(QWidget *parent, const char *name)

    Constructs a horizontal button group with no title.

    The \a parent and \a name arguments are passed to the QWidget
    constructor.
*/

/*!
    \fn Q3HButtonGroup::Q3HButtonGroup(const QString &title, QWidget *parent,
                                       const char *name)

    Constructs a horizontal button group with the title \a title.

    The \a parent and \a name arguments are passed to the QWidget
    constructor.
*/

/*!
    \class Q3VButtonGroup
    \brief The Q3VButtonGroup widget organizes button widgets in a
    vertical column.

    \compat

    Q3VButtonGroup is a convenience class that offers a thin layer on top
    of Q3ButtonGroup. Think of it as a QVBoxWidget that offers a frame with a
    title and is specifically designed for buttons.

    \sa Q3HButtonGroup
*/

/*!
    \fn Q3VButtonGroup::Q3VButtonGroup(QWidget *parent, const char *name)

    Constructs a vertical button group with no title.

    The \a parent and \a name arguments are passed on to the QWidget
    constructor.
*/

/*!
    \fn Q3VButtonGroup::Q3VButtonGroup(const QString &title, QWidget *parent,
			               const char *name)

    Constructs a vertical button group with the title \a title.

    The \a parent and \a name arguments are passed on to the QWidget
    constructor.
*/
