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

#ifndef SIGNALSLOTEDITOR_PLUGIN_H
#define SIGNALSLOTEDITOR_PLUGIN_H

#include "signalsloteditor_global.h"

#include <abstractformeditorplugin.h>

#include <QtCore/QPointer>
#include <QtCore/QHash>

class SignalSlotEditorTool;
class AbstractFormWindow;

class QT_SIGNALSLOTEDITOR_EXPORT SignalSlotEditorPlugin: public QObject, public AbstractFormEditorPlugin
{
    Q_OBJECT
    Q_INTERFACES(AbstractFormEditorPlugin)
public:
    SignalSlotEditorPlugin();
    virtual ~SignalSlotEditorPlugin();

    virtual bool isInitialized() const;
    virtual void initialize(AbstractFormEditor *core);
    virtual QAction *action() const;

    virtual AbstractFormEditor *core() const;

private slots:
    void addFormWindow(AbstractFormWindow *formWindow);
    void removeFormWindow(AbstractFormWindow *formWindow);

private:
    QPointer<AbstractFormEditor> m_core;
    QHash<AbstractFormWindow*, SignalSlotEditorTool*> m_tools;
    bool m_initialized;
    QAction *m_action;
};

#endif // SIGNALSLOTEDITOR_PLUGIN_H
