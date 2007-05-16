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

#include "script_p.h"

/*!
    \class QDesignerScriptExtension
    \brief The QDesignerScriptExtension class allows you to generate a
    per-widget \l{QtScript} {Qt Script} snippet to be executed while
    building the form.

    \internal
    \inmodule QtDesigner
    \since 4.3

    On saving the form, the extension is queried for a script snippet
    to be associated with the widget while saving the \c .ui file.
    This script is then run after creating the widget by \l uic or
    QUiLoader.

    As opposed to \l QDesignerCustomWidgetInterface::codeTemplate(),
    it allows for applying an internal state of the widget
    that can be manipulated using \QD.

    Such a state might for example be the contents of a custom item view widget,
    for which an editor is provided by the QDesignerTaskMenuExtension.

    While saving the form, the state is serialized as a QVariantMap of
    \QD-supported properties, which is stored in the \c .ui file. This is
    handled by data() and setData().

    For item view contents, there might be for example a key that determines
    the number of items and other keys that contain the actual items following
    a naming scheme (\c numItems, \c item1, \c item2, ...).

    On saving, script() is invoked, which should return a script snippet that
    applies the state to the widget while building the form.

    \sa {Creating Custom Widgets for Qt Designer#Using Qt Script to Aid in Building Forms}{Creating Custom Widgets for Qt Designer}, QtScript
*/

/*!
    Destroys the extension.
*/

QDesignerScriptExtension::~QDesignerScriptExtension()
{
}

/*!
    \fn virtual QString QDesignerScriptExtension::script() const

    Returns a script snippet to be associated with the widget.
*/

/*!
    \fn virtual QVariantMap QDesignerScriptExtension::data() const

    Returns a map of variants describing the internal state to be
    stored in the  \c .ui file.
*/

/*!
    \fn virtual void QDesignerScriptExtension::setData(const QVariantMap &data)

    Applies the internal state stored in \a data to the widget while loading a form.
*/
