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

#ifndef SIGNALSLOTEDITOR_TOOL_H
#define SIGNALSLOTEDITOR_TOOL_H

#include "signalsloteditor_global.h"

#include <QtCore/QPointer>

#include <abstractformwindowtool.h>

class AbstractFormEditor;
class AbstractFormWindow;
class SignalSlotEditor;
class QAction;

class QT_SIGNALSLOTEDITOR_EXPORT SignalSlotEditorTool: public AbstractFormWindowTool
{
    Q_OBJECT
public:
    SignalSlotEditorTool(AbstractFormWindow *formWindow, QObject *parent = 0);
    virtual ~SignalSlotEditorTool();

    virtual AbstractFormEditor *core() const;
    virtual AbstractFormWindow *formWindow() const;

    virtual QWidget *editor() const;

    QAction *action() const;

    virtual void activated();
    virtual void deactivated();

    virtual bool handleEvent(QWidget *widget, QWidget *managedWidget, QEvent *event);

private:
    AbstractFormWindow *m_formWindow;
    mutable QPointer<SignalSlotEditor> m_editor;
    QAction *m_action;
};

#endif // SIGNALSLOTEDITOR_TOOL_H
