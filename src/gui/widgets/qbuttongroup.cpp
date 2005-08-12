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



/*!
    \class QButtonGroup qbuttongroup.h
    \brief The QButtonGroup class provides a container to organize groups of
    button widgets.

    \ingroup organizers
    \ingroup geomanagement
    \ingroup appearance
    \mainclass

    QButtonGroup provides an abstract container into which button widgets can
    be placed. It does not provide a visual representation of this container
    (see QGroupBox for a container widget), but instead manages the states of
    each of the buttons in the group.

    An \l exclusive button group switches off all checkable (toggle)
    buttons except the one that was clicked. By default, a button
    group is exclusive. The buttons in a button group are usually
    checkable QPushButton's, \l{QCheckBox}es (normally for
    non-exclusive button groups), or \l{QRadioButton}s.

    A button is added to the group with addButton(). It can be removed
    from the group with removeButton(). If the group is exclusive, the
    currently checked button is available as checkedButton(). If a
    button is clicked the buttonClicked() signal is emitted. For a
    checkable button in an exclusive group this means that the button
    was checked. The list of buttons in the group is returned by
    buttons().

    In addition, QButtonGroup can map between integers and
    buttons. You can assign an integer id to a button with setId(),
    and retrieve it with id(). The id of the currently checked button
    is available with checkedId(), and there is an overloaded signal
    buttonClicked() which emits the id of the button. The purpose of
    the mapping mechanism is to simply the representation of enum
    values in a user interface.

    \sa QGroupBox QPushButton, QCheckBox, QRadioButton
*/

/*!
    \fn QButtonGroup::QButtonGroup(QObject *parent)

    Constructs a new, empty button group with the given \a parent.

    \sa addButton() setExclusive()
*/

/*!
    \fn QButtonGroup::~QButtonGroup()

    Destroys the button group.
*/

/*!
    \property QButtonGroup::exclusive
    \brief whether the button group is exclusive

    If this property is true, then the buttons in the group are
    checked (toggled), and to untoggle a button you must click on
    another button in the group. The default value is true.
*/

/*!
    \fn void QButtonGroup::buttonClicked(QAbstractButton *button)

    This signal is emitted when the given \a button is clicked.

    \sa checkedButton(), QAbstractButton::clicked()
*/

/*!
    \fn void QButtonGroup::buttonClicked(int id)

    This signal is emitted when a button with the given \a id is
    clicked.

    \sa checkedButton(), QAbstractButton::clicked()
*/


/*!
    \fn void QButtonGroup::addButton(QAbstractButton *button, int id = -1);

    Adds the given \a button to the button group, with the given \a id.

    \sa removeButton() buttons()
*/

/*!
    \fn void QButtonGroup::removeButton(QAbstractButton *button);

    Removes the given \a button from the button group.

    \sa addButton() buttons()
*/

/*!
    \fn QList<QAbstractButton*> QButtonGroup::buttons() const

    Returns the list of this groups's buttons. This may be empty.

    \sa addButton(), removeButton()
*/

/*!
    \fn QAbstractButton *QButtonGroup::checkedButton() const;

    Returns the button group's checked button, or 0 if no buttons are
    checked.

    \sa buttonClicked()
*/

/*!
\fn QAbstractButton *QButtonGroup::button(int id) const;

Returns the button with the specified \a id, or 0 if no such button
exists.
*/

/*!
\fn void QButtonGroup::setId(QAbstractButton *button, int id) const;

Sets the \a id for the specified \a button.

\sa id()
*/

/*!
\fn int QButtonGroup::id(QAbstractButton *button) const;

Returns the id for the specified \a button, or -1 if no such button
exists.

\sa setId()
*/

/*!
\fn int QButtonGroup::checkedId() const;

Returns the id of the checkedButton(), or -1 if no button is checked.

\sa setId()
*/


/*! \fn void QButtonGroup::insert(QAbstractButton *b)

    Use addButton() instead.
*/

/*! \fn void QButtonGroup::remove(QAbstractButton *b)

    Use removeButton() instead.
*/
