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

#include "abstractobjectinspector.h"

/*!
    \class QDesignerObjectInspectorInterface
    \brief The QDesignerObjectInspectorInterface class provides an interface that is used to
    control Qt Designer's object inspector component.
    \inmodule QtDesigner
*/

/*!
*/
QDesignerObjectInspectorInterface::QDesignerObjectInspectorInterface(QWidget *parent, Qt::WindowFlags flags)
    : QWidget(parent, flags)
{
}

/*!
*/
QDesignerObjectInspectorInterface::~QDesignerObjectInspectorInterface()
{
}

/*!
*/
QDesignerFormEditorInterface *QDesignerObjectInspectorInterface::core() const
{
    return 0;
}

/*!
    \fn virtual void QDesignerObjectInspectorInterface::setFormWindow(QDesignerFormWindowInterface *formWindow) = 0
*/
