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

#include "abstractformwindowmanager.h"

/*!
    \class QDesignerFormWindowManagerInterface
    \inmodule QtDesigner
*/

/*!
    Constructs an interface with the given \a parent for the form window
    manager.
*/
QDesignerFormWindowManagerInterface::QDesignerFormWindowManagerInterface(QObject *parent)
    : QObject(parent)
{
}

/*!
    Destroys the interface for the form window manager.
*/
QDesignerFormWindowManagerInterface::~QDesignerFormWindowManagerInterface()
{
}

/*!
*/
QAction *QDesignerFormWindowManagerInterface::actionCut() const
{
    return 0;
}

/*!
*/
QAction *QDesignerFormWindowManagerInterface::actionCopy() const
{
    return 0;
}

/*!
*/
QAction *QDesignerFormWindowManagerInterface::actionPaste() const
{
    return 0;
}

/*!
*/
QAction *QDesignerFormWindowManagerInterface::actionDelete() const
{
    return 0;
}

/*!
*/
QAction *QDesignerFormWindowManagerInterface::actionSelectAll() const
{
    return 0;
}

/*!
*/
QAction *QDesignerFormWindowManagerInterface::actionLower() const
{
    return 0;
}

/*!
*/
QAction *QDesignerFormWindowManagerInterface::actionRaise() const
{
    return 0;
}

/*!
*/
QAction *QDesignerFormWindowManagerInterface::actionHorizontalLayout() const
{
    return 0;
}

/*!
*/
QAction *QDesignerFormWindowManagerInterface::actionVerticalLayout() const
{
    return 0;
}

/*!
*/
QAction *QDesignerFormWindowManagerInterface::actionSplitHorizontal() const
{
    return 0;
}

/*!
*/
QAction *QDesignerFormWindowManagerInterface::actionSplitVertical() const
{
    return 0;
}

/*!
*/
QAction *QDesignerFormWindowManagerInterface::actionGridLayout() const
{
    return 0;
}

/*!
*/
QAction *QDesignerFormWindowManagerInterface::actionBreakLayout() const
{
    return 0;
}

/*!
*/
QAction *QDesignerFormWindowManagerInterface::actionAdjustSize() const
{
    return 0;
}

/*!
*/
QDesignerFormWindowInterface *QDesignerFormWindowManagerInterface::activeFormWindow() const
{
    return 0;
}

/*!
*/
QDesignerFormEditorInterface *QDesignerFormWindowManagerInterface::core() const
{
    return 0;
}

/*!
*/
void QDesignerFormWindowManagerInterface::addFormWindow(QDesignerFormWindowInterface *formWindow)
{
    Q_UNUSED(formWindow);
}

/*!
*/
void QDesignerFormWindowManagerInterface::removeFormWindow(QDesignerFormWindowInterface *formWindow)
{
    Q_UNUSED(formWindow);
}

/*!
*/
void QDesignerFormWindowManagerInterface::setActiveFormWindow(QDesignerFormWindowInterface *formWindow)
{
    Q_UNUSED(formWindow);
}

/*!
*/
int QDesignerFormWindowManagerInterface::formWindowCount() const
{
    return 0;
}

/*!
*/
QDesignerFormWindowInterface *QDesignerFormWindowManagerInterface::formWindow(int index) const
{
    Q_UNUSED(index);
    return 0;
}

/*!
*/
QDesignerFormWindowInterface *QDesignerFormWindowManagerInterface::createFormWindow(QWidget *parentWidget, Qt::WindowFlags flags)
{
    Q_UNUSED(parentWidget);
    Q_UNUSED(flags);
    return 0;
}

/*!
*/
QAction *QDesignerFormWindowManagerInterface::actionUndo() const
{
    return 0;
}

/*!
*/
QAction *QDesignerFormWindowManagerInterface::actionRedo() const
{
    return 0;
}

/*!
    \fn void QDesignerFormWindowManagerInterface::formWindowAdded(QDesignerFormWindowInterface *formWindow)

    This signal is emitted ...
*/

/*!
    \fn void QDesignerFormWindowManagerInterface::formWindowRemoved(QDesignerFormWindowInterface *formWindow)

    This signal is emitted ...
*/

/*!
    \fn void QDesignerFormWindowManagerInterface::activeFormWindowChanged(QDesignerFormWindowInterface *formWindow)

    This signal is emitted ...
*/
