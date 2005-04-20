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
#include "signalsloteditor.h"

#include <QtCore/QPointer>

#include <QtDesigner/abstractformwindowtool.h>

class QDesignerFormEditorInterface;
class QDesignerFormWindowInterface;
class SignalSlotEditor;
class QAction;

class QT_SIGNALSLOTEDITOR_EXPORT SignalSlotEditorTool: public QDesignerFormWindowToolInterface
{
    Q_OBJECT
public:
    SignalSlotEditorTool(QDesignerFormWindowInterface *formWindow, QObject *parent = 0);
    virtual ~SignalSlotEditorTool();

    virtual QDesignerFormEditorInterface *core() const;
    virtual QDesignerFormWindowInterface *formWindow() const;

    virtual QWidget *editor() const;

    QAction *action() const;

    virtual void activated();
    virtual void deactivated();

    virtual bool handleEvent(QWidget *widget, QWidget *managedWidget, QEvent *event);

    virtual void saveToDom(DomUI *ui, QWidget *mainContainer);
    virtual void loadFromDom(DomUI *ui, QWidget *mainContainer);

private:
    QDesignerFormWindowInterface *m_formWindow;
    mutable QPointer<qdesigner::components::signalsloteditor::SignalSlotEditor> m_editor;
    QAction *m_action;
};

#endif // SIGNALSLOTEDITOR_TOOL_H
