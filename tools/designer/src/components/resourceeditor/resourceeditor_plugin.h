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

#ifndef RESOURCEEDITOR_PLUGIN_H
#define RESOURCEEDITOR_PLUGIN_H

#include "resourceeditor_global.h"

#include <abstractformeditorplugin.h>

#include <QtCore/QPointer>
#include <QtCore/QHash>

class ResourceEditorTool;
class AbstractFormWindow;
class QAction;

class QT_RESOURCEEDITOR_EXPORT ResourceEditorPlugin: public QObject, public AbstractFormEditorPlugin
{
    Q_OBJECT
    Q_INTERFACES(AbstractFormEditorPlugin)
public:
    ResourceEditorPlugin();
    virtual ~ResourceEditorPlugin();

    virtual bool isInitialized() const;
    virtual void initialize(AbstractFormEditor *core);
    QAction *action() const;
    
    virtual AbstractFormEditor *core() const;

public slots:
    void activeFormWindowChanged(AbstractFormWindow *formWindow);

private slots:
    void addFormWindow(AbstractFormWindow *formWindow);
    void removeFormWindow(AbstractFormWindow *formWindow);

private:
    QPointer<AbstractFormEditor> m_core;
    QHash<AbstractFormWindow*, ResourceEditorTool*> m_tools;
    bool m_initialized;
    QAction *m_action;
};

#endif // RESOURCEEDITOR_PLUGIN_H
