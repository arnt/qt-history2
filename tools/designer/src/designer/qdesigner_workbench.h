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

class QDesigner;
class QDesignerActions;
class QDesignerToolWindow;
class QDesignerFormWindow;
class QDesignerIntegration;

class QAction;
class QActionGroup;
class QMenu;
class QMenuBar;
class QVariant;
class QToolBar;
class Q3Workspace;
class QCloseEvent;

class AbstractFormEditor;
class AbstractFormWindow;
class AbstractFormWindowManager;

class TaskMenuComponent;

class QDesignerWorkbench: public QObject
{
    Q_OBJECT
public:
    enum UIMode
    {
        NeutralMode,
        TopLevelMode,
        WorkspaceMode
        //### more (i.e. TabMode)
    };

public:
    QDesignerWorkbench();
    virtual ~QDesignerWorkbench();

    UIMode mode() const;

    AbstractFormEditor *core() const;

    QDesignerToolWindow *findToolWindow(QWidget *widget) const;
    QDesignerFormWindow *findFormWindow(QWidget *widget) const;

    QDesignerFormWindow *createFormWindow();

    int toolWindowCount() const;
    QDesignerToolWindow *toolWindow(int index) const;

    int formWindowCount() const;
    QDesignerFormWindow *formWindow(int index) const;

    QDesignerActions *actionManager() const;

    QActionGroup *modeActionGroup() const;

    QRect availableGeometry() const;
    int marginHint() const;

    void saveSettings() const;

    bool readInForm(const QString &fileName) const;
    bool writeOutForm(AbstractFormWindow *formWindow, const QString &fileName) const;
    bool saveForm(AbstractFormWindow *fw);
    bool handleClose();

signals:
    void modeChanged(UIMode mode);
    void initialized();

public slots:
    void addToolWindow(QDesignerToolWindow *toolWindow);
    void addFormWindow(QDesignerFormWindow *formWindow);
    void removeToolWindow(QDesignerToolWindow *toolWindow);
    void removeFormWindow(QDesignerFormWindow *formWindow);
    void setUIMode(UIMode mode);
    void setUseBigIcons(bool superSizeMe);

// ### private slots:
    void switchToNeutralMode();
    void switchToWorkspaceMode();
    void switchToTopLevelMode();

    void initializeCorePlugins();

private slots:
    void initialize();
    void activateWorkspaceChildWindow(QWidget *widget);
    void updateWorkbench(AbstractFormWindow *formWindow, const QString &name, const QVariant &value);
    void updateWindowMenu(AbstractFormWindow *fw);
    void formWindowActionTriggered(QAction *a);

private:
    QWidget *magicalParent() const;
    AbstractFormWindowManager *formWindowManager() const;
    void changeBringToFrontVisiblity(bool visible);
    void changeToolBarIconSize(bool big);


private:
    AbstractFormEditor *m_core;
    QDesignerIntegration *m_integration;

    QDesignerActions *m_actionManager;
    QActionGroup *m_toolActions;
    QActionGroup *m_windowActions;

    QMenu *m_fileMenu;
    QMenu *m_editMenu;
    QMenu *m_formMenu;
    QMenu *m_toolMenu;
    QMenu *m_windowMenu;
    QMenu *m_helpMenu;

    QMenuBar *m_globalMenuBar;
    QToolBar *m_toolToolBar;
    QToolBar *m_formToolBar;
    QToolBar *m_editToolBar;

    QActionGroup *m_modeActionGroup;
    QAction *m_topLevelModeAction;
    QAction *m_workspaceModeAction;

    UIMode m_mode;

    QList<QDesignerToolWindow*> m_toolWindows;
    QList<QDesignerFormWindow*> m_formWindows;

    Q3Workspace *m_workspace;
    QHash<QWidget*, bool> m_visibilities;
    QHash<QWidget*, QRect> m_geometries;

    class ToolWindowExtra {};
    class FormWindowExtra {};

    QHash<QDesignerToolWindow*, ToolWindowExtra> m_toolWindowExtras;
    QHash<QDesignerFormWindow*, FormWindowExtra> m_formWindowExtras;

    TaskMenuComponent *m_taskMenuComponent;
};

#endif // QDESIGNER_WORKBENCH_H
