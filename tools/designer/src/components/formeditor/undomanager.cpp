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

#include "undomanager.h"
#include "formwindow.h"

// ---- UndoManager ----
UndoManager::UndoManager(FormEditor *core, QObject *parent)
    : AbstractUndoManager(parent),
      m_core(core)
{
}

UndoManager::~UndoManager()
{
}

namespace MergeMe {

// ---- ResizeCommand ----
ResizeCommand::ResizeCommand(const QString description, FormWindow *formWindow, AbstractCommand *parent)
    : FormWindowCommand(description, formWindow, parent),
      m_widget(0)
{
}

void ResizeCommand::undo()
{
    qWarning("ResizeCommand::undo() -- not implemented yet!");
    Q_ASSERT(m_widget);
    m_widget->resize(oldSize());
}

void ResizeCommand::redo()
{
    qWarning("ResizeCommand::redo() -- not implemented yet!");
    Q_ASSERT(m_widget);
    m_widget->resize(newSize());
}

} // namespace MergeMe
