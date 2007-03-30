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

#include "script.h"

/*!
    \class QDesignerScriptExtension
    \brief The QDesignerScriptExtension is queried for a script
           to be associated with the widget while saving the \c .ui file.
           This script is then run after creating the widget by \l uic or
           the form builder classes.

    \inmodule QtDesigner
    \since 4.3

           As opposed to \l QDesignerCustomWidgetInterface::codeTemplate(),
           it allows for applying an internal state
           created for example by a task menu
           extension implementation.
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
