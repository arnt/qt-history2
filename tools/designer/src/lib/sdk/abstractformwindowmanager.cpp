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

AbstractFormWindowManager::AbstractFormWindowManager(QObject *parent)
    : QObject(parent)
{
}

AbstractFormWindowManager::~AbstractFormWindowManager()
{
}

QAction *AbstractFormWindowManager::actionCut() const
{
    return 0;
}

QAction *AbstractFormWindowManager::actionCopy() const
{
    return 0;
}

QAction *AbstractFormWindowManager::actionPaste() const
{
    return 0;
}

QAction *AbstractFormWindowManager::actionDelete() const
{
    return 0;
}

QAction *AbstractFormWindowManager::actionSelectAll() const
{
    return 0;
}

QAction *AbstractFormWindowManager::actionLower() const
{
    return 0;
}

QAction *AbstractFormWindowManager::actionRaise() const
{
    return 0;
}

QAction *AbstractFormWindowManager::actionHorizontalLayout() const
{
    return 0;
}

QAction *AbstractFormWindowManager::actionVerticalLayout() const
{
    return 0;
}

QAction *AbstractFormWindowManager::actionSplitHorizontal() const
{
    return 0;
}

QAction *AbstractFormWindowManager::actionSplitVertical() const
{
    return 0;
}

QAction *AbstractFormWindowManager::actionGridLayout() const
{
    return 0;
}

QAction *AbstractFormWindowManager::actionBreakLayout() const
{
    return 0;
}

QAction *AbstractFormWindowManager::actionAdjustSize() const
{
    return 0;
}

AbstractFormWindow *AbstractFormWindowManager::activeFormWindow() const
{
    return 0;
}

AbstractFormEditor *AbstractFormWindowManager::core() const
{
    return 0;
}

void AbstractFormWindowManager::addFormWindow(AbstractFormWindow *formWindow)
{
    Q_UNUSED(formWindow);
}

void AbstractFormWindowManager::removeFormWindow(AbstractFormWindow *formWindow)
{
    Q_UNUSED(formWindow);
}

void AbstractFormWindowManager::setActiveFormWindow(AbstractFormWindow *formWindow)
{
    Q_UNUSED(formWindow);
}

int AbstractFormWindowManager::formWindowCount() const
{
    return 0;
}

AbstractFormWindow *AbstractFormWindowManager::formWindow(int index) const
{
    Q_UNUSED(index);
    return 0;
}

AbstractFormWindow *AbstractFormWindowManager::createFormWindow(QWidget *parentWidget, Qt::WFlags flags)
{
    Q_UNUSED(parentWidget);
    Q_UNUSED(flags);
    return 0;
}

QAction *AbstractFormWindowManager::actionUndo() const
{
    return 0;
}

QAction *AbstractFormWindowManager::actionRedo() const
{
    return 0;
}

QAction *AbstractFormWindowManager::actionShowResourceEditor()const
{
    return 0;
}

