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

#ifndef RESOURCEEDITOR_TOOL_H
#define RESOURCEEDITOR_TOOL_H

#include "resourceeditor_global.h"

#include <QtCore/QPointer>

#include <abstractformwindowtool.h>

class AbstractFormEditor;
class AbstractFormWindow;
class ResourceEditor;
class QAction;

class QT_RESOURCEEDITOR_EXPORT ResourceEditorTool: public AbstractFormWindowTool
{
    Q_OBJECT
public:
    ResourceEditorTool(AbstractFormWindow *formWindow, QObject *parent = 0);
    virtual ~ResourceEditorTool();

    virtual AbstractFormEditor *core() const;
    virtual AbstractFormWindow *formWindow() const;

    virtual QWidget *editor() const;
    virtual QAction *action() const;

    virtual void activated();
    virtual void deactivated();

    virtual bool handleEvent(QWidget *widget, QWidget *managedWidget, QEvent *event);

private:
    AbstractFormWindow *m_formWindow;
    mutable QPointer<ResourceEditor> m_editor;
    QAction *m_action;
};

#endif // RESOURCEEDITOR_TOOL_H
