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

#include "abstractactioneditor.h"

/*!
    \class QDesignerActionEditorInterface
    \brief The QDesignerActionEditorInterface class provides an interface that is used to
    control Qt Designer's action editor component.
    \inmodule QtDesigner
*/

/*!
*/
QDesignerActionEditorInterface::QDesignerActionEditorInterface(QWidget *parent, Qt::WindowFlags flags)
    : QWidget(parent, flags)
{
}

/*!
*/
QDesignerActionEditorInterface::~QDesignerActionEditorInterface()
{
}

/*!
*/
QDesignerFormEditorInterface *QDesignerActionEditorInterface::core() const
{
    return 0;
}

/*!
    \fn virtual void QDesignerActionEditorInterface::setFormWindow(QDesignerFormWindowInterface *formWindow) = 0
*/
