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

#ifndef QDESIGNER_WORKBENCH_H
#define QDESIGNER_WORKBENCH_H

#include <QtCore/QObject>
#include <QtCore/QPointer>
#include <QtCore/QHash>
#include <QtCore/QList>
#include <QtCore/QRect>

class QDesignerMainWindow;
class QDesignerToolWindow;
class QDesignerFormWindow;
class QDesignerIntegration;

class QAction;
class QActionGroup;
class QVariant;
class Q3Workspace;

class AbstractFormEditor;
class AbstractFormWindow;
class AbstractFormWindowManager;

class QDesignerWorkbench: public QObject
{
    Q_OBJECT
public:
    enum Mode
    {
        NeutralMode,
        TopLevelMode,
        WorkspaceMode
        //### more (i.e. TabMode)
    };

public:
    QDesignerWorkbench(QDesignerMainWindow *mainWindow);
    virtual ~QDesignerWorkbench();

    QDesignerMainWindow *mainWindow() const;
    void setMainWindow(QDesignerMainWindow *mainWindow);

    Mode mode() const;

    AbstractFormEditor *core() const;

    QDesignerToolWindow *findToolWindow(QWidget *widget) const;
    QDesignerFormWindow *findFormWindow(QWidget *widget) const;

    QDesignerFormWindow *createFormWindow();

    int toolWindowCount() const;
    QDesignerToolWindow *toolWindow(int index) const;

    int formWindowCount() const;
    QDesignerFormWindow *formWindow(int index) const;

    QActionGroup *modeActionGroup() const;
    QAction *actionForToolWindow(QDesignerToolWindow *toolWindow) const;
    QAction *actionForFormWindow(QDesignerFormWindow *formWindow) const;

    QRect availableGeometry() const;
    int marginHint() const;

signals:
    void toolWindowAdded(QDesignerToolWindow *toolWindow);
    void toolWindowRemoved(QDesignerToolWindow *toolWindow);

    void formWindowAdded(QDesignerFormWindow *formWindow);
    void formWindowRemoved(QDesignerFormWindow *formWindow);

    void modeChanged(Mode mode);
    void initialized();

public slots:
    void addToolWindow(QDesignerToolWindow *toolWindow);
    void addFormWindow(QDesignerFormWindow *formWindow);
    void removeToolWindow(QDesignerToolWindow *toolWindow);
    void removeFormWindow(QDesignerFormWindow *formWindow);

// ### private slots:
    void switchToNeutralMode();
    void switchToWorkspaceMode();
    void switchToTopLevelMode();

private slots:
    void initialize();
    void activateWorkspaceChildWindow(QWidget *widget);
    void updateWorkbench(AbstractFormWindow *formWindow, const QString &name, const QVariant &value);

private:
    QWidget *magicalParent() const;
    AbstractFormWindowManager *formWindowManager() const;

private:
    AbstractFormEditor *m_core;
    QDesignerIntegration *m_integration;
    QActionGroup *m_modeActionGroup;
    QAction *m_topLevelModeAction;
    QAction *m_workspaceModeAction;

    QPointer<QDesignerMainWindow> m_mainWindow;
    Mode m_mode;

    QList<QDesignerToolWindow*> m_toolWindows;
    QList<QDesignerFormWindow*> m_formWindows;

    Q3Workspace *m_workspace;
    QHash<QWidget*, QRect> m_geometries;

    struct ToolWindowExtra {};
    struct FormWindowExtra {};
    QHash<QDesignerToolWindow*, ToolWindowExtra> m_toolWindowExtras;
    QHash<QDesignerFormWindow*, FormWindowExtra> m_formWindowExtras;
};

#endif // QDESIGNER_WORKBENCH_H
