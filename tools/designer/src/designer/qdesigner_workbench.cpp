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
#include "qdesigner_actions.h"
#include "qdesigner_toolwindow.h"
#include "qdesigner_formwindow.h"
#include "qdesigner_settings.h"
#include "qdesigner_widgetbox.h"
#include "qdesigner_propertyeditor.h"
#include "qdesigner_objectinspector.h"
#include <qdesigner_integration.h>

// components
#include <formeditor/formeditor.h>
#include <taskmenu/taskmenu_component.h>

#include <abstractformwindow.h>
#include <abstractformeditorplugin.h>
#include <abstractformwindowmanager.h>
#include <abstractwidgetbox.h>

#include <Qt3Support/Q3Workspace>
#include <QtGui/QCloseEvent>
#include <QtGui/QDesktopWidget>
#include <QtGui/QLabel>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QToolBar>
#include <QtGui/QActionGroup>
#include <QtGui/QMessageBox>
#include <QtGui/QHeaderView>
#include <QtCore/QVariant>

#include <QtCore/QPluginLoader>
#include <QtCore/qdebug.h>

QDesignerWorkbench::QDesignerWorkbench()
    : m_mode(QDesignerWorkbench::NeutralMode), m_workspace(0)
{
    initialize();

    setUIMode(UIMode(QDesignerSettings().uiMode()));
}

QDesignerWorkbench::~QDesignerWorkbench()
{
}

QDesignerWorkbench::UIMode QDesignerWorkbench::mode() const
{
    return m_mode;
}

void QDesignerWorkbench::addToolWindow(QDesignerToolWindow *toolWindow)
{
    Q_ASSERT(m_toolWindowExtras.contains(toolWindow) == false);
    Q_ASSERT(toolWindow->windowTitle().isEmpty() == false);

    m_toolWindows.append(toolWindow);

    if (QAction *action = toolWindow->action()) {
        Q_ASSERT(m_toolMenu->actions().isEmpty() == false);

        QList<QAction*> lst = m_toolActions->actions();
        QAction *before = lst.count() ? lst.last() : m_toolMenu->actions().first();
        m_toolMenu->insertAction(before, action);
        m_toolActions->addAction(action);
    }
}

void QDesignerWorkbench::addFormWindow(QDesignerFormWindow *formWindow)
{
    Q_ASSERT(m_formWindowExtras.contains(formWindow) == false);
    // ### Q_ASSERT(formWindow->windowTitle().isEmpty() == false);

    m_formWindows.append(formWindow);
    if (m_windowActions->actions().isEmpty())
        m_windowMenu->addSeparator();

    if (QAction *action = formWindow->action()) {
        m_windowActions->addAction(action);
        m_windowMenu->addAction(action);
        action->setChecked(true);
    }

    m_actionManager->minimizeAction()->setEnabled(true);
}

void QDesignerWorkbench::initialize()
{
    QDesignerSettings settings;
    m_core = new FormEditor(this);

    initializeCorePlugins();

    m_toolActions = new QActionGroup(this);
    m_toolActions->setExclusive(false);

    m_windowActions = new QActionGroup(this);
    m_windowActions->setExclusive(true);

    m_actionManager = new QDesignerActions(this);

    m_globalMenuBar = new QMenuBar;

    m_fileMenu = m_globalMenuBar->addMenu(tr("&File"));
    foreach (QAction *action, m_actionManager->fileActions()->actions()) {
        m_fileMenu->addAction(action);
        if (action->text() == QDesignerActions::tr("&Open Form...")) {
            QMenu *recentFilesMenu = m_fileMenu->addMenu(tr("&Recent Forms"));
            // Pop the "Recent Files" stuff in here.
            foreach(QAction *recentAction, m_actionManager->recentFilesActions()->actions())
                recentFilesMenu->addAction(recentAction);
        }
    }

    m_editMenu = m_globalMenuBar->addMenu(tr("&Edit"));
    foreach (QAction *action, m_actionManager->editActions()->actions()) {
        m_editMenu->addAction(action);
    }

    m_editMenu->addSeparator();

    foreach (QAction *action, m_actionManager->toolActions()->actions()) {
        m_editMenu->addAction(action);
    }

    m_editMenu->addSeparator();
    QMenu *menu = m_editMenu->addMenu(tr("UI &Mode"));
    foreach (QAction *action, m_actionManager->uiMode()->actions())
        menu->addAction(action);


    m_formMenu = m_globalMenuBar->addMenu(tr("F&orm"));
    foreach (QAction *action, m_actionManager->formActions()->actions()) {
        m_formMenu->addAction(action);
    }

    m_toolMenu = m_globalMenuBar->addMenu(tr("&Tool"));

    QAction *bigAction = m_actionManager->useBigIconsAction();
    connect(bigAction, SIGNAL(toggled(bool)), this, SLOT(setUseBigIcons(bool)));
    m_toolMenu->addAction(bigAction);
    m_toolMenu->addSeparator();

    m_windowMenu = m_globalMenuBar->addMenu(tr("&Window"));
    foreach (QAction *action, m_actionManager->windowActions()->actions()) {
        m_windowMenu->addAction(action);
    }

    m_helpMenu = m_globalMenuBar->addMenu(tr("&Help"));
    foreach (QAction *action, m_actionManager->helpActions()->actions()) {
        m_helpMenu->addAction(action);
    }

    addToolWindow(new QDesignerWidgetBox(this));
    addToolWindow(new QDesignerObjectInspector(this));
    addToolWindow(new QDesignerPropertyEditor(this));

    m_modeActionGroup = new QActionGroup(this);
    m_modeActionGroup->setExclusive(true);

    QAction *modeAction = 0;

    modeAction = m_modeActionGroup->addAction(tr("&Top Level Mode"));
    modeAction->setCheckable(true);
    connect(modeAction, SIGNAL(triggered()), this, SLOT(switchToTopLevelMode()));

    modeAction = m_modeActionGroup->addAction(tr("&Workspace Mode"));
    modeAction->setCheckable(true);
    connect(modeAction, SIGNAL(triggered()), this, SLOT(switchToWorkspaceMode()));

    m_integration = new QDesignerIntegration(core(), this);
    connect(m_integration, SIGNAL(propertyChanged(AbstractFormWindow*, const QString&, const QVariant& )),
            this, SLOT(updateWorkbench(AbstractFormWindow*, const QString&, const QVariant& )));

    m_taskMenuComponent = new TaskMenuComponent(core(), this);

    // create the toolbars
    m_editToolBar = new QToolBar;
    m_editToolBar->setWindowTitle(tr("Edit"));
    foreach (QAction *action, m_actionManager->editActions()->actions()) {
        if (action->icon().isNull() == false)
            m_editToolBar->addAction(action);
    }

    m_toolToolBar = new QToolBar;
    m_toolToolBar->setWindowTitle(tr("Tools"));
    foreach (QAction *action, m_actionManager->toolActions()->actions()) {
        if (action->icon().isNull() == false)
            m_toolToolBar->addAction(action);
    }

    m_formToolBar = new QToolBar;
    m_formToolBar->setWindowTitle(tr("Form"));
    foreach (QAction *action, m_actionManager->formActions()->actions()) {
        if (action->icon().isNull() == false)
            m_formToolBar->addAction(action);
    }

    changeToolBarIconSize(settings.useBigIcons());

    m_geometries.clear();


    emit initialized();

    connect(m_core->formWindowManager(), SIGNAL(activeFormWindowChanged(AbstractFormWindow*)),
                this, SLOT(updateWindowMenu(AbstractFormWindow *)));
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

    QPoint desktopOffset;
    if (m_mode == TopLevelMode)
        desktopOffset = QApplication::desktop()->availableGeometry().topLeft();
    m_mode = NeutralMode;

    m_geometries.clear();
    m_visibilities.clear();

    foreach (QDesignerToolWindow *tw, m_toolWindows) {
        m_visibilities.insert(tw, tw->isVisible());
        if (tw->isVisible()) {
            // use the actual geometry
            QPoint pos = tw->window()->pos();
            if (!tw->isWindow())
                if (QWidget *p = tw->parentWidget())
                    pos = p->pos(); // in workspace

            m_geometries.insert(tw, QRect(pos - desktopOffset, tw->size()));
        }
        tw->setSaveSettingsOnClose(false);

        tw->setParent(0);
    }

    foreach (QDesignerFormWindow *fw, m_formWindows) {
        if (fw->isVisible()) {
            // use the actual geometry
            QPoint pos = fw->window()->pos();
            if (!fw->isWindow())
                if (QWidget *p = fw->parentWidget())
                    pos = p->pos(); // in workspace

            m_geometries.insert(fw, QRect(pos - desktopOffset, fw->size()));
        }

        fw->setParent(0);
    }

#ifndef Q_WS_MAC
    m_globalMenuBar->setParent(0);
#endif
    m_editToolBar->setParent(0);
    m_toolToolBar->setParent(0);
    m_formToolBar->setParent(0);
    m_core->setTopLevel(0);
    qDesigner->setMainWindow(0);

    if (m_workspace)
        delete m_workspace->parentWidget();

    m_workspace = 0;
}

void QDesignerWorkbench::switchToWorkspaceMode()
{
    if (m_mode == WorkspaceMode)
        return;

    switchToNeutralMode();
    m_mode = WorkspaceMode;

    QDesignerToolWindow *widgetBoxWrapper = 0;
    if (0 != (widgetBoxWrapper = findToolWindow(core()->widgetBox()))) {
        widgetBoxWrapper->action()->setEnabled(true);
        widgetBoxWrapper->setWindowTitle(tr("Widget Box"));
    }

    Q_ASSERT(m_workspace == 0);

    QDesignerToolWindow *mw = new QDesignerToolWindow(this); // Just to have a copy of
    mw->setSaveSettingsOnClose(true);
    mw->setWindowTitle(tr("Qt Designer"));
    m_workspace = new Q3Workspace(mw);
    connect(m_workspace, SIGNAL(windowActivated(QWidget*)),
            this, SLOT(activateWorkspaceChildWindow(QWidget* )));
    mw->setCentralWidget(m_workspace);
    m_core->setTopLevel(mw);

#ifndef Q_WS_MAC
    mw->setMenuBar(m_globalMenuBar);
#endif
    mw->addToolBar(m_editToolBar);
    mw->addToolBar(m_toolToolBar);
    mw->addToolBar(m_formToolBar);

    qDesigner->setMainWindow(mw);

    QDesignerSettings settings;
    foreach (QDesignerToolWindow *tw, m_toolWindows) {
        tw->setParent(magicalParent(), Qt::Tool | Qt::WindowShadeButtonHint | Qt::WindowSystemMenuHint | Qt::WindowTitleHint);
        if (m_geometries.isEmpty()) {
            settings.setGeometryFor(tw, tw->geometryHint());
            QHeaderView *header = qFindChild<QHeaderView*>(tw);
            if (header != 0)
                settings.setHeaderSizesFor(header);
        } else {
            QRect g = m_geometries.value(tw, tw->geometryHint());
            tw->resize(g.size());
            tw->move(g.topLeft());
            tw->setVisible(m_visibilities.value(tw, true));
        }
    }

    foreach (QDesignerFormWindow *fw, m_formWindows) {
        fw->setParent(magicalParent());
        QRect g = m_geometries.value(fw, fw->geometryHint());
        fw->resize(g.size());
        fw->move(g.topLeft());
        fw->show();
    }
    changeBringToFrontVisiblity(false);

    m_workspace->show();
    mw->showMaximized();
}

void QDesignerWorkbench::changeBringToFrontVisiblity(bool visible)
{
    QAction *btf = m_actionManager->bringToFrontAction();
    QList<QAction *> actionList = m_actionManager->windowActions()->actions();
    btf->setVisible(visible);
    QAction *sep = actionList.at(actionList.indexOf(btf) - 1);
    if (sep->isSeparator())
        sep->setVisible(visible);


}

void QDesignerWorkbench::switchToTopLevelMode()
{
    if (m_mode == TopLevelMode)
        return;

    switchToNeutralMode();
    QPoint desktopOffset = QApplication::desktop()->availableGeometry().topLeft();
    m_mode = TopLevelMode;

    // The widget box is special, it gets the menubar and gets to be the main widget.

    QDesignerToolWindow *widgetBoxWrapper = 0;
    if (0 != (widgetBoxWrapper = findToolWindow(core()->widgetBox()))) {
        m_core->setTopLevel(widgetBoxWrapper);
#ifndef Q_WS_MAC
        widgetBoxWrapper->setMenuBar(m_globalMenuBar);
        widgetBoxWrapper->action()->setEnabled(false);
        widgetBoxWrapper->setSaveSettingsOnClose(true);
        qDesigner->setMainWindow(widgetBoxWrapper);
        widgetBoxWrapper->setWindowTitle(tr("Qt Designer"));
#endif
        widgetBoxWrapper->addToolBar(m_editToolBar);
        widgetBoxWrapper->addToolBar(m_toolToolBar);
        widgetBoxWrapper->addToolBar(m_formToolBar);

        widgetBoxWrapper->insertToolBarBreak(m_formToolBar);
    }

    QDesignerSettings settings;
    foreach (QDesignerToolWindow *tw, m_toolWindows) {
        tw->setParent(magicalParent(), Qt::Window);
        if (m_geometries.isEmpty()) {
            settings.setGeometryFor(tw, tw->geometryHint());
            QHeaderView *header = qFindChild<QHeaderView*>(tw);
            if (header != 0)
                settings.setHeaderSizesFor(header);
        } else {
            QRect g = m_geometries.value(tw, tw->geometryHint());
            tw->resize(g.size());
            tw->move(g.topLeft() + desktopOffset);
            tw->setVisible(m_visibilities.value(tw, true));
        }
    }
    changeBringToFrontVisiblity(true);

    foreach (QDesignerFormWindow *fw, m_formWindows) {
        fw->setParent(magicalParent(), Qt::Window);
        QRect g = m_geometries.value(fw, fw->geometryHint());
        fw->resize(g.size());
        fw->move(g.topLeft() + desktopOffset);
        fw->show();
    }
}

QDesignerFormWindow *QDesignerWorkbench::createFormWindow()
{
    QDesignerFormWindow *formWindow = new QDesignerFormWindow(/*formWindow=*/ 0, this);

    Qt::WFlags flags = 0;

    if (m_mode == QDesignerWorkbench::TopLevelMode)
        flags = Qt::Window;

    formWindow->setParent(magicalParent(), flags);
    formWindow->setAttribute(Qt::WA_DeleteOnClose, true);

    addFormWindow(formWindow);

    QRect g = formWindow->geometryHint();
    formWindow->resize(g.size());
    formWindow->move(availableGeometry().center() - g.center());

    formWindow->show();

    return formWindow;
}

AbstractFormWindowManager *QDesignerWorkbench::formWindowManager() const
{
    return m_core->formWindowManager();
}

AbstractFormEditor *QDesignerWorkbench::core() const
{
    return m_core;
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

QRect QDesignerWorkbench::availableGeometry() const
{
    if (m_workspace)
        return m_workspace->geometry();

    return qDesigner->desktop()->availableGeometry(0);
}

int QDesignerWorkbench::marginHint() const
{
    return 20;
}

void QDesignerWorkbench::activateWorkspaceChildWindow(QWidget *widget)
{
    if (QDesignerFormWindow *fw = qobject_cast<QDesignerFormWindow*>(widget)) {
        core()->formWindowManager()->setActiveFormWindow(fw->editor());
    }
}

void QDesignerWorkbench::updateWorkbench(AbstractFormWindow *fw, const QString &name,
                                         const QVariant &value)
{
    Q_UNUSED(fw);
    Q_UNUSED(name);
    Q_UNUSED(value);
}

void QDesignerWorkbench::removeToolWindow(QDesignerToolWindow *toolWindow)
{
    int index = m_toolWindows.indexOf(toolWindow);
    if (index != -1) {
        m_toolWindows.removeAt(index);
        m_toolWindowExtras.remove(toolWindow);
    }

    if (QAction *action = toolWindow->action()) {
        m_toolActions->removeAction(action);
        m_toolMenu->removeAction(action);
    }
}

void QDesignerWorkbench::removeFormWindow(QDesignerFormWindow *formWindow)
{
    int index = m_formWindows.indexOf(formWindow);
    if (index != -1) {
        m_formWindows.removeAt(index);
        m_formWindowExtras.remove(formWindow);
    }

    if (QAction *action = formWindow->action()) {
        m_windowActions->removeAction(action);
        m_windowMenu->removeAction(action);
    }

    if (formWindowCount() == 0) {
        m_actionManager->minimizeAction()->setEnabled(false);
        QList<QAction *> actions = m_windowMenu->actions();
        for (int i = actions.size() - 1; i >= 0; --i) {
            QAction *act = actions.at(i);
            if (act->isSeparator()) {
                delete act;
                break;
            }
        }
    }
}

void QDesignerWorkbench::initializeCorePlugins()
{
    QList<QObject*> builtinPlugins = QPluginLoader::staticInstances();
    foreach (QObject *plugin, builtinPlugins) {
        if (AbstractFormEditorPlugin *formEditorPlugin = qobject_cast<AbstractFormEditorPlugin*>(plugin)) {
            if (!formEditorPlugin->isInitialized())
                formEditorPlugin->initialize(core());
        }
    }
}

void QDesignerWorkbench::saveSettings() const
{
    QDesignerSettings settings;
    foreach (QDesignerToolWindow *tw, m_toolWindows) {
        settings.saveGeometryFor(tw);
        QHeaderView *header = qFindChild<QHeaderView*>(tw);
        if (header != 0) {
            settings.setHeaderSizesFor(header);
            settings.saveHeaderSizesFor(header);
        }
    }
}

bool QDesignerWorkbench::readInForm(const QString &fileName) const
{
    return m_actionManager->readInForm(fileName);
}

bool QDesignerWorkbench::writeOutForm(AbstractFormWindow *formWindow, const QString &fileName) const
{
    return m_actionManager->writeOutForm(formWindow, fileName);
}

bool QDesignerWorkbench::saveForm(AbstractFormWindow *frm)
{
    return m_actionManager->saveForm(frm);
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

bool QDesignerWorkbench::handleClose()
{
    QList<QDesignerFormWindow *> dirtyForms;
    foreach (QDesignerFormWindow *w, m_formWindows) {
        if (w->editor()->isDirty())
            dirtyForms << w;
    }

    if (dirtyForms.size()) {
        if (dirtyForms.size() == 1) {
            if (!dirtyForms.at(0)->close()) {
                return false;
            }
        } else {
            QMessageBox box(tr("Save Forms?"),
                    tr("There are %1 forms with unsaved changes."
                        " Do you want to review these changes before quitting?")
                    .arg(dirtyForms.size()),
                    QMessageBox::Warning,
                    QMessageBox::Yes | QMessageBox::Default, QMessageBox::No,
                    QMessageBox::Cancel | QMessageBox::Escape, 0);
            box.setButtonText(QMessageBox::Yes, tr("Review Changes"));
            box.setButtonText(QMessageBox::No, tr("Discard Changes"));
            switch (box.exec()) {
            case QMessageBox::Cancel:
                return false;
            case QMessageBox::Yes:
               foreach (QDesignerFormWindow *fw, dirtyForms) {
                   fw->show();
                   fw->raise();
                   if (!fw->close()) {
                       return false;
                   }
               }
               break;
            case QMessageBox::No:
              foreach (QDesignerFormWindow *fw, dirtyForms) {
                  fw->editor()->setDirty(false);
                  fw->setWindowModified(false);
              }
              break;
            }
        }
    }

    foreach (QDesignerFormWindow *fw, m_formWindows)
        fw->close();

    saveSettings();
    return true;
}

QDesignerActions *QDesignerWorkbench::actionManager() const
{
    return m_actionManager;
}

void QDesignerWorkbench::setUIMode(UIMode mode)
{
    Q_ASSERT(mode > NeutralMode && mode <= WorkspaceMode);
    if (mode == WorkspaceMode)
        switchToWorkspaceMode();
    else
        switchToTopLevelMode();
}

void QDesignerWorkbench::updateWindowMenu(AbstractFormWindow *fw)
{
    if (!fw)
        return;
    if (QDesignerFormWindow *dfw = qobject_cast<QDesignerFormWindow *>(fw->parentWidget()))
        dfw->action()->setChecked(true);
}

void QDesignerWorkbench::setUseBigIcons(bool superSizeMe)
{
    QDesignerSettings settings;
    if (settings.useBigIcons() == superSizeMe)
        return;
    settings.setUseBigIcons(superSizeMe);
    changeToolBarIconSize(superSizeMe);
}

void QDesignerWorkbench::changeToolBarIconSize(bool big)
{
    if (big) {
        m_toolToolBar->setIconSize(QSize(32, 32));
        m_formToolBar->setIconSize(QSize(32, 32));
        m_editToolBar->setIconSize(QSize(32, 32));
    } else {
        m_toolToolBar->setIconSize(QSize(16, 16));
        m_formToolBar->setIconSize(QSize(16, 16));
        m_editToolBar->setIconSize(QSize(16, 16));
    }
}
