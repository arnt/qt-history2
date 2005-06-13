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

#include "abstractformwindowcursor.h"

/*!
    \class QDesignerFormWindowCursorInterface
    \brief The QDesignerFormWindowCursorInterface class provides an interface to the form window's
    text cursor.
    \inmodule QtDesigner
*/

/*!
    \enum QDesignerFormWindowCursorInterface::MoveOperation

    This enum describes the types of text cursor operation that can occur in a form window.

    \value NoMove The cursor does not move.
    \value Start  Moves the cursor to the start of the focus chain.
    \value End    Moves the cursor to the end of the focus chain.
    \value Next   Moves the cursor to the next widget in the focus chain.
    \value Prev   Moves the cursor to the previous widget in the focus chain.
    \value Left   The cursor moves to the left.
    \value Right  The cursor moves to the right.
    \value Up     The cursor moves upwards.
    \value Down   The cursor moves downwards.
*/

/*!
    \enum QDesignerFormWindowCursorInterface::MoveMode

    This enum describes the different modes that are used when the text cursor moves.

    \value MoveAnchor The anchor moves with the cursor to its new location.
    \value KeepAnchor The anchor remains at the cursor's old location.
*/

/*!
    Returns true if the specified \a widget is selected; otherwise returns false.*/
bool QDesignerFormWindowCursorInterface::isWidgetSelected(QWidget *widget) const
{
    for (int index=0; index<selectedWidgetCount(); ++index) {
        if (selectedWidget(index) == widget)
            return true;
    }

    return false;
}

/*!
    \fn virtual QDesignerFormWindowCursorInterface::~QDesignerFormWindowCursorInterface()

    Destroys the cursor interface.
*/

/*!
    \fn virtual QDesignerFormWindowInterface *QDesignerFormWindowCursorInterface::formWindow() const = 0

    Returns the interface to the form window associated with this interface.
*/

/*!
    \fn virtual bool QDesignerFormWindowCursorInterface::movePosition(MoveOperation operation, MoveMode mode) = 0

    Performs the given \a operation on the cursor using the specified \a mode, and returns true
    if it completed successfully; otherwise returns false.
*/

/*!
    \fn virtual int QDesignerFormWindowCursorInterface::position() const = 0

    Returns the cursor position.

    \sa setPosition()
*/

/*!
    \fn virtual void QDesignerFormWindowCursorInterface::setPosition(int position, MoveMode mode = MoveAnchor) = 0

    Sets the position of the cursor to the given \a position using the \a mode to specify
    how it is moved there.

    \sa position()
*/

/*!
    \fn virtual QWidget *QDesignerFormWindowCursorInterface::current() const = 0

    Returns the current widget in the form.

    \sa selectedWidget()
*/

/*!
    \fn virtual int QDesignerFormWindowCursorInterface::widgetCount() const = 0

    Returns the number of widgets in the form window.
*/

/*!
    \fn virtual QWidget *QDesignerFormWindowCursorInterface::widget(int index) const = 0

    Returns the widget with the given \a index in the list of widgets on the form.

    \sa selectedWidget()
*/

/*!
    \fn virtual bool QDesignerFormWindowCursorInterface::hasSelection() const = 0

    Returns true if the form window contains a selection; otherwise returns false.
*/

/*!
    \fn virtual int QDesignerFormWindowCursorInterface::selectedWidgetCount() const = 0

    Returns the number of selected widgets in the form window.
*/

/*!
    \fn virtual QWidget *QDesignerFormWindowCursorInterface::selectedWidget(int index) const = 0

    Returns the widget with the given \a index in the list of selected widgets.

    \sa widget()
*/

/*!
    \fn virtual void QDesignerFormWindowCursorInterface::setProperty(const QString &name, const QVariant &value) = 0

    Sets the property with the given \a name in the current widget to the specified \a value.

    \sa setWidgetProperty(), resetWidgetProperty()
*/

/*!
    \fn virtual void QDesignerFormWindowCursorInterface::setWidgetProperty(QWidget *widget, const QString &name, const QVariant &value) = 0

    Sets the property with the given \a name in the \a widget to the specified \a value.

    \sa resetWidgetProperty(), setProperty()
*/

/*!
    \fn virtual void QDesignerFormWindowCursorInterface::resetWidgetProperty(QWidget *widget, const QString &name) = 0

    Resets the property with the given \a name in the specified \a widget to its default value.

    \sa setProperty()
*/
