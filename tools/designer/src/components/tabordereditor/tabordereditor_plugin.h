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

#ifndef TABORDEREDITOR_PLUGIN_H
#define TABORDEREDITOR_PLUGIN_H

#include "tabordereditor_global.h"

#include <abstractformeditorplugin.h>

#include <QtCore/QPointer>
#include <QtCore/QHash>

class TabOrderEditorTool;
class AbstractFormWindow;
class QAction;

class QT_TABORDEREDITOR_EXPORT TabOrderEditorPlugin: public QObject, public AbstractFormEditorPlugin
{
    Q_OBJECT
    Q_INTERFACES(AbstractFormEditorPlugin)
public:
    TabOrderEditorPlugin();
    virtual ~TabOrderEditorPlugin();

    virtual bool isInitialized() const;
    virtual void initialize(AbstractFormEditor *core);
    QAction *action() const;

    virtual AbstractFormEditor *core() const;

private slots:
    void addFormWindow(AbstractFormWindow *formWindow);
    void removeFormWindow(AbstractFormWindow *formWindow);

private:
    QPointer<AbstractFormEditor> m_core;
    QHash<AbstractFormWindow*, TabOrderEditorTool*> m_tools;
    bool m_initialized;
    QAction *m_action;
};

#endif // TABORDEREDITOR_PLUGIN_H
