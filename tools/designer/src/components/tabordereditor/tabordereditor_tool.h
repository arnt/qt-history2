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

#ifndef TABORDEREDITOR_TOOL_H
#define TABORDEREDITOR_TOOL_H

#include "tabordereditor_global.h"

#include <QtCore/QPointer>

#include <abstractformwindowtool.h>

class AbstractFormEditor;
class AbstractFormWindow;
class TabOrderEditor;
class QAction;

class QT_TABORDEREDITOR_EXPORT TabOrderEditorTool: public AbstractFormWindowTool
{
    Q_OBJECT
public:
    TabOrderEditorTool(AbstractFormWindow *formWindow, QObject *parent = 0);
    virtual ~TabOrderEditorTool();

    virtual AbstractFormEditor *core() const;
    virtual AbstractFormWindow *formWindow() const;

    virtual QWidget *editor() const;
    virtual QAction *action() const;

    virtual void activated();
    virtual void deactivated();

    virtual bool handleEvent(QWidget *widget, QWidget *managedWidget, QEvent *event);

private:
    AbstractFormWindow *m_formWindow;
    mutable QPointer<TabOrderEditor> m_editor;
    QAction *m_action;
};

#endif // TABORDEREDITOR_TOOL_H
