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

#include "qdesigner.h"
#include "qdesigner_workbench.h"
#include "qdesigner_mainwindow.h"
#include "qdesigner_toolwindow.h"
#include "qdesigner_formwindow.h"
#include "qdesigner_widgetbox.h"
#include "qdesigner_propertyeditor.h"
#include "qdesigner_objectinspector.h"
#include "qdesigner_integration.h"

// components
#include <formeditor/formeditor.h>

// sdk ### remove me
#include <abstractformwindowmanager.h>
#include <abstractformwindow.h>
#include <abstractwidgetbox.h>

#include <Q3Workspace>
#include <QDesktopWidget>
#include <QLabel>
#include <QSignal>
#include <QActionGroup>

#include <qdebug.h>

QDesignerWorkbench::QDesignerWorkbench(QDesignerMainWindow *mainWindow)
    : QObject(mainWindow),
      m_mainWindow(mainWindow),
      m_mode(QDesignerWorkbench::NeutralMode),
      m_workspace(0)
{
    Q_ASSERT(mainWindow);
    initialize();
    // switchToNeutralMode();
}

QDesignerWorkbench::~QDesignerWorkbench()
{
}

QDesignerWorkbench::Mode QDesignerWorkbench::mode() const
{
    return m_mode;
}

void QDesignerWorkbench::addToolWindow(QDesignerToolWindow *toolWindow)
{
    Q_ASSERT(m_toolWindowExtras.contains(toolWindow) == false);
    Q_ASSERT(toolWindow->windowTitle().isEmpty() == false);

    m_toolWindows.append(toolWindow);

    emit toolWindowAdded(toolWindow);
}

void QDesignerWorkbench::addFormWindow(QDesignerFormWindow *formWindow)
{
    Q_ASSERT(m_formWindowExtras.contains(formWindow) == false);
    // ### Q_ASSERT(formWindow->windowTitle().isEmpty() == false);

    m_formWindows.append(formWindow);

    emit formWindowAdded(formWindow);
}

QDesignerToolWindow *QDesignerWorkbench::findToolWindow(QWidget *widget) const
{
    foreach (QDesignerToolWindow *toolWindow, m_toolWindows) {
        if (toolWindow->centralWidget() == widget)
            return toolWindow;
    }

    return 0;
}

QDesignerFormWindow *QDesignerWorkbench::findFormWindow(QWidget *widget) const
{
    foreach (QDesignerFormWindow *formWindow, m_formWindows) {
        if (formWindow->centralWidget() == widget)
            return formWindow;
    }

    return 0;
}

QDesignerToolWindow *QDesignerWorkbench::createToolWindow(QWidget *widget, QWidget *parent, Qt::WFlags flags)
{
    QDesignerToolWindow *toolWindow = new QDesignerToolWindow(this, parent, flags);
    widget->setParent(toolWindow);
    toolWindow->setCentralWidget(widget);
    widget->show();

    toolWindow->setWindowTitle(widget->windowTitle()); // ### temp
    addToolWindow(toolWindow);

    return toolWindow;
}

void QDesignerWorkbench::initialize()
{
    m_core = new FormEditor(this);
    m_core->setTopLevel(mainWindow());

    addToolWindow(new QDesignerWidgetBox(this));
    addToolWindow(new QDesignerObjectInspector(this));
    addToolWindow(new QDesignerPropertyEditor(this));

    m_modeActionGroup = new QActionGroup(this);
    m_modeActionGroup->setExclusive(true);

    QAction *modeAction = 0;

    modeAction = m_modeActionGroup->addAction(tr("&Top Level Mode"));
    connect(modeAction, SIGNAL(triggered()), this, SLOT(switchToTopLevelMode()));

    modeAction = m_modeActionGroup->addAction(tr("&Workspace Mode"));
    connect(modeAction, SIGNAL(triggered()), this, SLOT(switchToWorkspaceMode()));

    m_integration = new QDesignerIntegration(core(), this);
    connect(m_integration, SIGNAL(propertyChanged(AbstractFormWindow*, const QString&, const QVariant& )),
            this, SLOT(updateWorkbench(AbstractFormWindow*, const QString&, const QVariant& )));

    emit initialized();
}

QWidget *QDesignerWorkbench::magicalParent() const
{
    switch (m_mode) {
        case TopLevelMode:
            return 0;
        case WorkspaceMode:
            Q_ASSERT(m_workspace != 0);
            return m_workspace;
        case NeutralMode:
            return 0;
        default:
            Q_ASSERT(0);
            return 0;
    }
}

void QDesignerWorkbench::switchToNeutralMode()
{
     if (m_mode == NeutralMode)
         return;

    m_mode = NeutralMode;

    m_geometries.clear();

    foreach (QDesignerToolWindow *tw, m_toolWindows) {
        if (tw->isVisible()) {
            // use the actual geometry
            QPoint pos = tw->pos();
            if (QWidget *p = tw->parentWidget())
                pos = p->mapToGlobal(pos);

            m_geometries.insert(tw, QRect(pos, tw->size()));
        }

        tw->setParent(0);
    }

    foreach (QDesignerFormWindow *fw, m_formWindows) {
        if (fw->isVisible()) {
            // use the actual geometry
            QPoint pos = fw->pos();
            if (QWidget *p = fw->parentWidget())
                pos = p->mapToGlobal(pos);

            m_geometries.insert(fw, QRect(pos, fw->size()));
        }

        fw->setParent(0);
    }

    mainWindow()->setCentralWidget(0);

    delete m_workspace;
    m_workspace = 0;
}

void QDesignerWorkbench::switchToWorkspaceMode()
{
    if (m_mode == WorkspaceMode)
        return;

    switchToNeutralMode();
    m_mode = WorkspaceMode;

    Q_ASSERT(m_workspace == 0);

    m_workspace = new Q3Workspace(mainWindow());
    connect(m_workspace, SIGNAL(windowActivated(QWidget*)), this, SLOT(activateWorkspaceChildWindow(QWidget* )));
    mainWindow()->setCentralWidget(m_workspace);

    foreach (QDesignerToolWindow *tw, m_toolWindows) {
        tw->setParent(magicalParent());
        tw->setGeometry(m_geometries.value(tw, tw->geometryHint()));
        tw->show();
    }

    foreach (QDesignerFormWindow *fw, m_formWindows) {
        fw->setParent(magicalParent());
        fw->setGeometry(m_geometries.value(fw, fw->geometryHint()));
        fw->show();
    }

    m_workspace->show();
    mainWindow()->showMaximized();
}

void QDesignerWorkbench::switchToTopLevelMode()
{
    if (m_mode == TopLevelMode)
        return;

    switchToNeutralMode();
    m_mode = TopLevelMode;

    QDesignerToolWindow *widgetBoxWrapper = 0;
    if (0 != (widgetBoxWrapper = findToolWindow(core()->widgetBox()))) {
        QRect g = m_geometries.value(widgetBoxWrapper, widgetBoxWrapper->geometryHint());
        widgetBoxWrapper->setParent(mainWindow());
        mainWindow()->setCentralWidget(widgetBoxWrapper);
        mainWindow()->setGeometry(g);
        widgetBoxWrapper->show();
    }

    foreach (QDesignerToolWindow *tw, m_toolWindows) {
        if (tw == widgetBoxWrapper)
            continue;

        tw->setParent(magicalParent(), Qt::WStyle_Tool);
        tw->setGeometry(m_geometries.value(tw, tw->geometryHint()));
        tw->show();
    }

    foreach (QDesignerFormWindow *fw, m_formWindows) {
        fw->setParent(magicalParent(), Qt::WType_TopLevel);
        fw->setGeometry(m_geometries.value(fw, fw->geometryHint()));
        fw->show();
    }

    mainWindow()->showNormal();
}

void QDesignerWorkbench::createFormWindow(const QString &contents)
{
    QDesignerFormWindow *formWindow = new QDesignerFormWindow(this);
    formWindow->editor()->setContents(contents);

    Qt::WFlags flags = 0;

    if (m_mode == QDesignerWorkbench::TopLevelMode)
        flags = Qt::WType_TopLevel;
    formWindow->setParent(magicalParent(), flags);

    formWindow->setAttribute(Qt::WA_DeleteOnClose, true);

    addFormWindow(formWindow);
    formWindow->show();
}

AbstractFormWindowManager *QDesignerWorkbench::formWindowManager() const
{
    return m_core->formWindowManager();
}

AbstractFormEditor *QDesignerWorkbench::core() const
{
    return m_core;
}

QAction *QDesignerWorkbench::actionForToolWindow(QDesignerToolWindow *toolWindow) const
{
    return toolWindow->action();
}

QAction *QDesignerWorkbench::actionForFormWindow(QDesignerFormWindow *formWindow) const
{
    return formWindow->action();
}

QActionGroup *QDesignerWorkbench::modeActionGroup() const
{
    return m_modeActionGroup;
}

int QDesignerWorkbench::toolWindowCount() const
{
    return m_toolWindows.count();
}

QDesignerToolWindow *QDesignerWorkbench::toolWindow(int index) const
{
    return m_toolWindows.at(index);
}

int QDesignerWorkbench::formWindowCount() const
{
    return m_formWindows.count();
}

QDesignerFormWindow *QDesignerWorkbench::formWindow(int index) const
{
    return m_formWindows.at(index);
}

QDesignerMainWindow *QDesignerWorkbench::mainWindow() const
{
    return m_mainWindow;
}

void QDesignerWorkbench::setMainWindow(QDesignerMainWindow *mainWindow)
{
    m_mainWindow = mainWindow;
    core()->setTopLevel(mainWindow);
}

QRect QDesignerWorkbench::availableGeometry() const
{
    if (m_workspace)
        return m_workspace->geometry();

    return qDesigner->desktop()->geometry();
}

int QDesignerWorkbench::marginHint() const
{
    return 20;
}

void QDesignerWorkbench::activateWorkspaceChildWindow(QWidget *widget)
{
    if (QDesignerFormWindow *fw = qt_cast<QDesignerFormWindow*>(widget)) {
        AbstractFormWindow *widget = qt_cast<AbstractFormWindow*>(fw->centralWidget());
        core()->formWindowManager()->setActiveFormWindow(widget);
    }
}

void QDesignerWorkbench::updateWorkbench(AbstractFormWindow *fw, const QString &name, const QVariant &value)
{
    Q_UNUSED(value);

    if (name == QLatin1String("windowTitle")) {
        if (QDesignerFormWindow *window = findFormWindow(fw)) {
            window->setWindowTitle(fw->windowTitle());
        }
    }
}

