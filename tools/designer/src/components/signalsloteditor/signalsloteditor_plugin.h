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

#include <QtDesigner/QtDesigner>

#include <QtCore/QPointer>
#include <QtCore/QHash>

class SignalSlotEditorTool;
class QDesignerFormWindowInterface;

class QT_SIGNALSLOTEDITOR_EXPORT SignalSlotEditorPlugin: public QObject, public QDesignerFormEditorPluginInterface
{
    Q_OBJECT
    Q_INTERFACES(QDesignerFormEditorPluginInterface)
public:
    SignalSlotEditorPlugin();
    virtual ~SignalSlotEditorPlugin();

    virtual bool isInitialized() const;
    virtual void initialize(QDesignerFormEditorInterface *core);
    virtual QAction *action() const;

    virtual QDesignerFormEditorInterface *core() const;

public slots:
    void activeFormWindowChanged(QDesignerFormWindowInterface *formWindow);

private slots:
    void addFormWindow(QDesignerFormWindowInterface *formWindow);
    void removeFormWindow(QDesignerFormWindowInterface *formWindow);

private:
    QPointer<QDesignerFormEditorInterface> m_core;
    QHash<QDesignerFormWindowInterface*, SignalSlotEditorTool*> m_tools;
    bool m_initialized;
    QAction *m_action;
};

#endif // SIGNALSLOTEDITOR_PLUGIN_H
