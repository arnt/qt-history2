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

#ifndef UNDOMANAGER_H
#define UNDOMANAGER_H

#include "formeditor.h"

#include <abstractundomanager.h>
#include <qtundo.h>

class FormWindow;
class FormEditor;

namespace MergeMe { // ###

struct AbstractCommand;
struct AbstractCommandGroup;

struct FormWindowCommand;
struct ResizeCommand;
struct MoveCommand;
struct AddStackedWidgetPageCommand;
struct DeleteStackedWidgetPageCommand;
struct AddToolBoxPageCommand;
struct DeleteToolBoxPageCommand;
struct LayoutHorizontalCommand;
struct LayoutHorizontalSplitCommand;
struct LayoutVerticalCommand;
struct LayoutVerticalSplitCommand;
struct LayoutGridCommand;
struct BreakLayoutCommand;
struct DeleteCommand;
struct PasteCommand;
struct LowerCommand;
struct RaiseCommand;
struct AddTabPageCommand;
struct DeleteTabPageCommand;
struct MoveTabPageCommand;
struct TabOrderCommand;
struct SetPropertyCommand;


} // namespace MergeMe

class UndoManager: public AbstractUndoManager
{
    Q_OBJECT
public:
    UndoManager(FormEditor *core, QObject *parent);
    virtual ~UndoManager();

    virtual AbstractFormEditor *core() const
    { return m_core; }

private:
    FormEditor *m_core;
};

namespace MergeMe { // ###

struct AbstractCommand
{
    virtual ~AbstractCommand() {}

    virtual AbstractCommand *parent() const = 0;

    virtual void undo() = 0;
    virtual void redo() = 0;

    virtual QString description() const = 0;
    virtual bool canMerge() const = 0;
    virtual bool mergeMeWith(AbstractCommand *command) = 0;

    virtual AbstractCommandGroup *toCommandGroup() const
    { return 0; }

    virtual FormWindowCommand *toFormWindowCommand() const
    { return 0; }

    virtual ResizeCommand *toResizeCommand() const
    { return 0; }

    virtual MoveCommand *toMoveCommand() const
    { return 0; }

    virtual AddStackedWidgetPageCommand *toAddStackedWidgetPageCommand() const
    { return 0; }

    virtual DeleteStackedWidgetPageCommand *toDeleteStackedWidgetPageCommand() const
    { return 0; }

    virtual AddToolBoxPageCommand *toAddToolBoxPageCommand() const
    { return 0; }

    virtual DeleteToolBoxPageCommand *toDeleteToolBoxPageCommand() const
    { return 0; }

    virtual LayoutHorizontalCommand *toLayoutHorizontalCommand() const
    { return 0; }

    virtual LayoutHorizontalSplitCommand *toLayoutHorizontalSplitCommand() const
    { return 0; }

    virtual LayoutVerticalCommand *toLayoutVerticalCommand() const
    { return 0; }

    virtual LayoutVerticalSplitCommand *toLayoutVerticalSplitCommand() const
    { return 0; }

    virtual LayoutGridCommand *toLayoutGridCommand() const
    { return 0; }

    virtual BreakLayoutCommand *toBreakLayoutCommand() const
    { return 0; }

    virtual DeleteCommand *toDeleteCommand() const
    { return 0; }

    virtual PasteCommand *toPasteCommand() const
    { return 0; }

    virtual LowerCommand *toLowerCommand() const
    { return 0; }

    virtual RaiseCommand *toRaiseCommand() const
    { return 0; }

    virtual AddTabPageCommand *toAddTabPageCommand() const
    { return 0; }

    virtual DeleteTabPageCommand *toDeleteTabPageCommand() const
    { return 0; }

    virtual MoveTabPageCommand *toMoveTabPageCommand() const
    { return 0; }

    virtual TabOrderCommand *toTabOrderCommand() const
    { return 0; }

    virtual SetPropertyCommand *toSetPropertyCommand() const
    { return 0; }
};

struct AbstractCommandGroup: public AbstractCommand
{
    virtual AbstractCommandGroup *toCommandGroup() const
    { return const_cast<AbstractCommandGroup *>(this); }
};


class QtCommandProxy: public QtCommand
{
    Q_OBJECT
public:
    inline QtCommandProxy()
        : m_command(0) {}

    inline QtCommandProxy(AbstractCommand *command)
        : m_command(0)
    { setCommand(command); }

    inline AbstractCommand *command() const
    { return m_command; }

    inline void setCommand(AbstractCommand *command)
    {
        Q_ASSERT(command);
        m_command = command;

        setDescription(m_command->description());
        setCanMerge(m_command->canMerge());
    }

    virtual void undo() const
    { Q_ASSERT(m_command); m_command->undo(); }

    virtual void redo() const
    { Q_ASSERT(m_command); m_command->redo(); }

protected:
    virtual bool mergeMeWith(QtCommand *other)
    {
        Q_ASSERT(m_command);
        if (QtCommandProxy *o = qt_cast<QtCommandProxy *>(other))
            return m_command->mergeMeWith(o->command());

        return false;
    }

private:
    AbstractCommand *m_command;
};

struct FormWindowCommand: public AbstractCommand
{
    inline FormWindowCommand()
    { Q_ASSERT(0); } // ### remove me

    inline FormWindowCommand(const QString description, FormWindow *formWindow, AbstractCommand *parent = 0)
        : m_description(description),
          m_formWindow(formWindow),
          m_parent(parent)
    { Q_ASSERT(m_formWindow); }

    inline FormWindow *formWindow() const
    { return m_formWindow; }

    virtual FormWindowCommand *toFormWindowCommand() const
    { return const_cast<FormWindowCommand *>(this); }

    virtual AbstractCommand *parent() const
    { return m_parent; }

    virtual QString description() const
    { return m_description; }

    virtual bool canMerge() const
    { return false; }

    virtual bool mergeMeWith(AbstractCommand *command)
    { Q_UNUSED(command); return false; }

private:
    QString m_description;
    FormWindow *m_formWindow;
    AbstractCommand *m_parent;
};


struct ResizeCommand: public FormWindowCommand
{
    ResizeCommand(const QString description, FormWindow *formWindow, AbstractCommand *parent = 0);

    inline QWidget *widget() const
    { return m_widget; }

    inline void setWidget(QWidget *widget)
    { m_widget = widget; }

    inline QSize oldSize() const
    { return m_oldSize; }

    inline void setOldSize(const QSize &oldSize)
    { m_oldSize = oldSize; }

    inline QSize newSize() const
    { return m_newSize; }

    inline void setNewSize(const QSize &newSize)
    { m_newSize = newSize; }

    virtual void undo();
    virtual void redo();

    virtual ResizeCommand *toResizeCommand() const
    { return const_cast<ResizeCommand *>(this); }

private:
    QWidget *m_widget;
    QSize m_oldSize;
    QSize m_newSize;
};

struct MoveCommand: public FormWindowCommand
{
    virtual MoveCommand *toMoveCommand() const
    { return const_cast<MoveCommand *>(this); }
};

struct DeleteCommand: public FormWindowCommand
{
    virtual DeleteCommand *toDeleteCommand() const
    { return const_cast<DeleteCommand *>(this); }
};

struct PasteCommand: public FormWindowCommand
{
    virtual PasteCommand *toPasteCommand() const
    { return const_cast<PasteCommand *>(this); }
};

struct LowerCommand: public FormWindowCommand
{
    virtual LowerCommand *toLowerCommand() const
    { return const_cast<LowerCommand *>(this); }
};

struct RaiseCommand: public FormWindowCommand
{
    virtual RaiseCommand *toRaiseCommand() const
    { return const_cast<RaiseCommand *>(this); }
};

struct SetPropertyCommand: public FormWindowCommand
{
    virtual SetPropertyCommand *toSetPropertyCommand() const
    { return const_cast<SetPropertyCommand *>(this); }
};

struct TabOrderCommand: public FormWindowCommand
{
    virtual TabOrderCommand *toTabOrderCommand() const
    { return const_cast<TabOrderCommand *>(this); }
};

struct LayoutHorizontalCommand: public FormWindowCommand
{
    virtual LayoutHorizontalCommand *toLayoutHorizontalCommand() const
    { return const_cast<LayoutHorizontalCommand *>(this); }
};

struct LayoutHorizontalSplitCommand: public FormWindowCommand
{
    virtual LayoutHorizontalSplitCommand *toLayoutHorizontalSplitCommand() const
    { return const_cast<LayoutHorizontalSplitCommand *>(this); }
};

struct LayoutVerticalCommand: public FormWindowCommand
{
    virtual LayoutVerticalCommand *toLayoutVerticalCommand() const
    { return const_cast<LayoutVerticalCommand *>(this); }
};

struct LayoutVerticalSplitCommand: public FormWindowCommand
{
    virtual LayoutVerticalSplitCommand *toLayoutVerticalSplitCommand() const
    { return const_cast<LayoutVerticalSplitCommand *>(this); }
};

struct LayoutGridCommand: public FormWindowCommand
{
    virtual LayoutGridCommand *toLayoutGridCommand() const
    { return const_cast<LayoutGridCommand *>(this); }
};

struct BreakLayoutCommand: public FormWindowCommand
{
    virtual BreakLayoutCommand *toBreakLayoutCommand() const
    { return const_cast<BreakLayoutCommand *>(this); }
};

struct AddTabPageCommand: public FormWindowCommand
{
    virtual AddTabPageCommand *toAddTabPageCommand() const
    { return const_cast<AddTabPageCommand *>(this); }
};

struct DeleteTabPageCommand: public FormWindowCommand
{
    virtual DeleteTabPageCommand *toDeleteTabPageCommand() const
    { return const_cast<DeleteTabPageCommand *>(this); }
};

struct MoveTabPageCommand: public FormWindowCommand
{
    virtual MoveTabPageCommand *toMoveTabPageCommand() const
    { return const_cast<MoveTabPageCommand *>(this); }
};

struct AddStackedWidgetPageCommand: public FormWindowCommand
{
    virtual AddStackedWidgetPageCommand *toAddStackedWidgetPageCommand() const
    { return const_cast<AddStackedWidgetPageCommand *>(this); }
};

struct DeleteStackedWidgetPageCommand: public FormWindowCommand
{
    virtual DeleteStackedWidgetPageCommand *toDeleteStackedWidgetPageCommand() const
    { return const_cast<DeleteStackedWidgetPageCommand *>(this); }
};

struct AddToolBoxPageCommand: public FormWindowCommand
{
    virtual AddToolBoxPageCommand *toAddToolBoxPageCommand() const
    { return const_cast<AddToolBoxPageCommand *>(this); }
};

struct DeleteToolBoxPageCommand: public FormWindowCommand
{
    virtual DeleteToolBoxPageCommand *toDeleteToolBoxPageCommand() const
    { return const_cast<DeleteToolBoxPageCommand *>(this); }
};

} // namespace MergeMe

#endif // UNDOMANAGER_H
