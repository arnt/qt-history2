/****************************************************************************
**
** Implementation of QButtonGroup class.
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



/*!
    \class QButtonGroup qbuttongroup.h
    \brief The QButtonGroup widget organizes QButton widgets in a group.

    \ingroup organizers
    \ingroup geomanagement
    \ingroup appearance
    \mainclass

    An \l exclusive button group switches off all toggle buttons
    except the one that was clicked. A button group is, by default,
    exclusive. The buttons in a button group are usually toggle
    QPushButton's, \l{QCheckBox}es (normally for non-exclusive button
    groups), or \l{QRadioButton}s.

    A button is added to the group with addButton(). It can be removed
    from the group with removeButton(). If the group is exclusive, the
    currently checked button is available as checkedButton(), and if a
    button is checked the buttonChecked() signal is emitted. The
    number of buttons in the group is returned by count().

    \inlineimage qbttngrp-m.png Screenshot in Motif style
    \inlineimage qbttngrp-w.png Screenshot in Windows style

    \sa QPushButton, QCheckBox, QRadioButton
*/

/*!
    \fn QButtonGroup::QButtonGroup(QObject *parent)

    Constructs a new, empty, button group with the given \a parent.

    \sa addButton() setExclusive()
*/

/*!
    \fn QButtonGroup::~QButtonGroup()

    Destroys the button group and all its child widgets.
*/

/*!
    \property QButtonGroup::exclusive
    \brief whether the button group is exclusive

    If this property is true, then the buttons in the group are
    toggled, and to untoggle a button you must click on another button
    in the group. The default value is true.
*/

/*!
    \fn void QButtonGroup::buttonChecked(QAbstractButton *button)

    This signal is emitted when the given \a button is checked.

    \sa checkedButton()
*/


/*!
    \fn void QButtonGroup::addButton(QAbstractButton *button);

    Adds the given \a button to the button group.

    \sa removeButton() count()
*/

/*!
    \fn void QButtonGroup::removeButton(QAbstractButton *button);

    Removes the given \a button from the button group.

    \sa addButton() count()
*/

/*!
    \fn QAbstractButton *QButtonGroup::checkedButton() const;

    Returns the button group's checked button, or 0 if there isn't
    one.

    \sa buttonChecked()
*/

