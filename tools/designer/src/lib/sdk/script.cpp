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
    \brief The  QDesignerScriptExtension is queried for a script
           to be associated with the widget while saving the ui-file.
           This script is then run after creating the widget by uic or
           the form builder classes. As opposed
           to the codeTemplate() method of QDesignerCustomWidgetInterface,
           this allows for applying an internal state created by a task menu
           extension implementation.

    \inmodule QtDesigner
*/

/*!
    Destroys the extension.
*/

QDesignerScriptExtension::~QDesignerScriptExtension()
{
}

/*!
    \fn virtual QString QDesignerScriptExtension::script(QWidget *widget) const = 0;

     Returns a script snippet to be associated with the widget.
*/

/*!
    \fn virtual QVariantMap QDesignerScriptExtension::data() = 0

     Returns a map of variants describing the internal state.
*/

/*!
   \fn virtual void QDesignerScriptExtension::setData(const QVariantMap &) = 0;

   Applies the saved internal state to the widget.
*/


