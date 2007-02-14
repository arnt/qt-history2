/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qdesigner_workbench.h"
#include "qdesigner.h"
#include "preferences.h"
#include "qdesigner_actions.h"
#include "qdesigner_toolwindow.h"
#include "qdesigner_formwindow.h"
#include "qdesigner_settings.h"
#include "qdesigner_widgetbox.h"
#include "qdesigner_propertyeditor.h"
#include "qdesigner_objectinspector.h"
#include "qdesigner_signalsloteditor.h"
#include "qdesigner_actioneditor.h"
#include "qdesigner_resourceeditor.h"

#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/QDesignerFormWindowInterface>
#include <QtDesigner/QDesignerFormWindowManagerInterface>
#include <QtDesigner/QDesignerFormEditorPluginInterface>
#include <QtDesigner/QDesignerWidgetBoxInterface>
#include <QtDesigner/QDesignerMetaDataBaseInterface>

#include <QtDesigner/QDesignerComponents>
#include <QtDesigner/private/qdesigner_integration_p.h>
#include <QtDesigner/private/pluginmanager_p.h>

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QUrl>
#include <QtCore/QPluginLoader>
#include <QtCore/qdebug.h>

#include <QtGui/QActionGroup>
#include <QtGui/QCloseEvent>
#include <QtGui/QDesktopWidget>
#include <QtGui/QDockWidget>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QMessageBox>
#include <QtGui/QPushButton>
#include <QtGui/QToolBar>
#include <QtGui/QMdiArea>
#include <QtGui/QMdiSubWindow>

static QMdiSubWindow *mdiSubWindowOf(const QWidget *w) 
{
    QMdiSubWindow *rc = qobject_cast<QMdiSubWindow *>(w->parentWidget());
    Q_ASSERT(rc);
    return rc;
}

static QDockWidget *dockWidgetOf(const QWidget *w)
{
    for (QWidget *parentWidget = w->parentWidget(); parentWidget ; parentWidget = parentWidget->parentWidget()) {
        if (QDockWidget *dw = qobject_cast<QDockWidget *>(parentWidget)) {
            return dw;
        }
    }
    Q_ASSERT("Dock widget not found");
    return 0;
}

QDesignerWorkbench::Position::Position(const QMdiSubWindow *mdiSubWindow, const QPoint &mdiAreaOffset) :
    m_minimized(mdiSubWindow->isShaded()),
    m_position(mdiSubWindow->pos() + mdiAreaOffset)
{
}

QDesignerWorkbench::Position::Position(const QDockWidget *dockWidget) :
    m_minimized(dockWidget->isMinimized()),
    m_position(dockWidget->pos())
{
}

QDesignerWorkbench::Position::Position(const QWidget *topLevelWindow, const QPoint &desktopTopLeft)
{
    const QWidget *window =topLevelWindow->window ();
    Q_ASSERT(window);
    m_minimized = window->isMinimized();
    m_position = window->pos() - desktopTopLeft;
}


void QDesignerWorkbench::Position::applyTo(QMdiSubWindow *mdiSubWindow,
                                           const QPoint &mdiAreaOffset) const
{
    // QMdiSubWindow attempts to resize its children to sizeHint() when switching user interface modes.
    // Restore old size
    const QPoint mdiAreaPos =  QPoint(qMax(0, m_position.x() - mdiAreaOffset.x()),
                                      qMax(0, m_position.y() - mdiAreaOffset.y()));
    mdiSubWindow->move(mdiAreaPos);
    const QSize decorationSize = mdiSubWindow->size() - mdiSubWindow->contentsRect().size();
    mdiSubWindow->resize(mdiSubWindow->widget()->size() + decorationSize);    
    mdiSubWindow->show();
    if (m_minimized) {
        mdiSubWindow->showShaded();
    }
}

void QDesignerWorkbench::Position::applyTo(QWidget *topLevelWindow, const QPoint &desktopTopLeft) const
{
    QWidget *window = topLevelWindow->window ();
    const QPoint newPos = m_position + desktopTopLeft;
    window->move(newPos);
    if ( m_minimized) {
        topLevelWindow->showMinimized();
    } else {
        topLevelWindow->show();
    }
}

void QDesignerWorkbench::Position::applyTo(QDockWidget *dockWidget) const
{
    dockWidget->widget()->setVisible(true);
    dockWidget->setVisible(!m_minimized);
}


// -------- QDesignerWorkbench

QDesignerWorkbench::QDesignerWorkbench()
    : m_mode(NeutralMode), m_mdiArea(0)
{
    m_initializing = true;
    initialize();
    
    applyPreferences(QDesignerSettings().preferences());

    m_initializing = false;
}

QDesignerWorkbench::~QDesignerWorkbench()
{
    QDesignerSettings settings;
    settings.clearBackup();

    if (m_mode == DockedMode) {
        Q_ASSERT(m_mdiArea != 0);
        QMainWindow *mw = qobject_cast<QMainWindow*>(m_mdiArea->window());
        Q_ASSERT(mw != 0);

        settings.setMainWindowState(mw->saveState(2));
    }

    while (!m_toolWindows.isEmpty())
        delete m_toolWindows.takeLast();
}

void QDesignerWorkbench::saveGeometries() 
{
    m_Positions.clear();
    switch (m_mode) {
    case NeutralMode:
        break;
    case TopLevelMode: {
        const QPoint desktopOffset = QApplication::desktop()->availableGeometry().topLeft();
        foreach (QDesignerToolWindow *tw, m_toolWindows) 
            m_Positions.insert(tw, Position(tw, desktopOffset));
        foreach (QDesignerFormWindow *fw, m_formWindows) {
            m_Positions.insert(fw,  Position(fw, desktopOffset));
        }
    }
        break;
    case DockedMode: {
        const QPoint mdiAreaOffset = m_mdiArea->pos();
        foreach (QDesignerToolWindow *tw, m_toolWindows) {
            m_Positions.insert(tw, Position(dockWidgetOf(tw)));
        }
        foreach (QDesignerFormWindow *fw, m_formWindows) {
            m_Positions.insert(fw, Position(mdiSubWindowOf(fw), mdiAreaOffset));
        }
    }
        break;
    }
}

UIMode QDesignerWorkbench::mode() const
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
    m_actionManager->minimizeAction()->setChecked(false);
    connect(formWindow, SIGNAL(minimizationStateChanged(QDesignerFormWindowInterface *, bool)),
            this, SLOT(minimizationStateChanged(QDesignerFormWindowInterface *, bool)));

    m_actionManager->editWidgets()->trigger();
}

void QDesignerWorkbench::initialize()
{
    QDesignerSettings settings;
    m_core = QDesignerComponents::createFormEditor(this);

    (void) QDesignerComponents::createTaskMenu(core(), this);

    initializeCorePlugins();
    QDesignerComponents::initializePlugins(core());

    m_toolActions = new QActionGroup(this);
    m_toolActions->setExclusive(false);

    m_windowActions = new QActionGroup(this);
    m_windowActions->setExclusive(true);
    connect(m_windowActions, SIGNAL(triggered(QAction*)), this, SLOT(formWindowActionTriggered(QAction*)));

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
    m_editMenu->addAction(m_actionManager->preferencesAction());

    m_formMenu = m_globalMenuBar->addMenu(tr("F&orm"));
    foreach (QAction *action, m_actionManager->formActions()->actions()) {
        m_formMenu->addAction(action);
    }

    QMenu *previewSubMenu = new QMenu(tr("Preview in"), m_formMenu);
    m_formMenu->insertMenu(m_actionManager->previewFormAction(), previewSubMenu);

    foreach (QAction *action, m_actionManager->styleActions()->actions()) {
        previewSubMenu->addAction(action);
    }

    m_toolMenu = m_globalMenuBar->addMenu(tr("&Tools"));

    m_toolMenu->addSeparator();

    m_windowMenu = m_globalMenuBar->addMenu(tr("&Window"));
    foreach (QAction *action, m_actionManager->windowActions()->actions()) {
        m_windowMenu->addAction(action);
    }

    m_helpMenu = m_globalMenuBar->addMenu(tr("&Help"));
    foreach (QAction *action, m_actionManager->helpActions()->actions()) {
        m_helpMenu->addAction(action);
    }

    QDesignerToolWindow *tw = new QDesignerWidgetBox(this);
    tw->setObjectName(QLatin1String("qt_designer_widgetbox"));
    addToolWindow(tw);
    tw = new QDesignerObjectInspector(this);
    tw->setObjectName(QLatin1String("qt_designer_objectinspector"));
    addToolWindow(tw);
    tw = new QDesignerPropertyEditor(this);
    tw->setObjectName(QLatin1String("qt_designer_propertyeditor"));
    addToolWindow(tw);
    tw = new QDesignerSignalSlotEditor(this);
    tw->setObjectName(QLatin1String("qt_designer_signalsloteditor"));
    addToolWindow(tw);
    tw = new QDesignerResourceEditor(this);
    tw->setObjectName(QLatin1String("qt_designer_resourceeditor"));
    addToolWindow(tw);
    tw = new QDesignerActionEditor(this);
    tw->setObjectName(QLatin1String("qt_designer_actioneditor"));
    addToolWindow(tw);

    m_integration = new qdesigner_internal::QDesignerIntegration(core(), this);

    // create the toolbars
    m_editToolBar = new QToolBar;
    m_editToolBar->setObjectName(QLatin1String("editToolBar"));
    m_editToolBar->setWindowTitle(tr("Edit"));
    foreach (QAction *action, m_actionManager->editActions()->actions()) {
        if (action->icon().isNull() == false)
            m_editToolBar->addAction(action);
    }

    m_toolToolBar = new QToolBar;
    m_toolToolBar->setObjectName(QLatin1String("toolsToolBar"));
    m_toolToolBar->setWindowTitle(tr("Tools"));
    foreach (QAction *action, m_actionManager->toolActions()->actions()) {
        if (action->icon().isNull() == false)
            m_toolToolBar->addAction(action);
    }

    m_formToolBar = new QToolBar;
    m_formToolBar->setObjectName(QLatin1String("formToolBar"));
    m_formToolBar->setWindowTitle(tr("Form"));
    foreach (QAction *action, m_actionManager->formActions()->actions()) {
        if (action->icon().isNull() == false)
            m_formToolBar->addAction(action);
    }

    QMenu *toolbarMenu = m_toolMenu->addMenu(tr("Toolbars"));
    toolbarMenu->addAction(m_editToolBar->toggleViewAction());
    toolbarMenu->addAction(m_toolToolBar->toggleViewAction());
    toolbarMenu->addAction(m_formToolBar->toggleViewAction());

    emit initialized();

    connect(m_core->formWindowManager(), SIGNAL(activeFormWindowChanged(QDesignerFormWindowInterface*)),
                this, SLOT(updateWindowMenu(QDesignerFormWindowInterface*)));
}

Qt::WindowFlags QDesignerWorkbench::magicalWindowFlags(const QWidget *widgetForFlags) const
{
    switch (m_mode) {
        case TopLevelMode: {
#ifdef Q_WS_MAC
            if (qobject_cast<const QDesignerToolWindow *>(widgetForFlags))
                return Qt::Tool;
#else
            Q_UNUSED(widgetForFlags);
#endif
            return Qt::Window;
        }
        case DockedMode:
            Q_ASSERT(m_mdiArea != 0);
            return Qt::Window | Qt::WindowShadeButtonHint | Qt::WindowSystemMenuHint | Qt::WindowTitleHint;
        case NeutralMode:
            return Qt::Window;
        default:
            Q_ASSERT(0);
            return 0;
    }
}

QWidget *QDesignerWorkbench::magicalParent() const
{
    switch (m_mode) {
        case TopLevelMode:
            return 0;
        case DockedMode:
            Q_ASSERT(m_mdiArea != 0);
            return m_mdiArea;
        case NeutralMode:
            return 0;
        default:
            Q_ASSERT(0);
            return 0;
    }
}

void QDesignerWorkbench::switchToNeutralMode()
{
    if (m_mode == NeutralMode) {
        return;
    } else if (m_mode == DockedMode) {
        Q_ASSERT(m_mdiArea != 0);
        QMainWindow *mw = qobject_cast<QMainWindow*>(m_mdiArea->window());
        QDesignerSettings settings;
        settings.setMainWindowState(mw->saveState(2));
    }

    saveGeometries();

    m_mode = NeutralMode;

    foreach (QDesignerToolWindow *tw, m_toolWindows) {
        tw->setSaveSettingsOnClose(false);
        tw->setParent(0);
    }

    foreach (QDesignerFormWindow *fw, m_formWindows) {
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

    if (m_mdiArea)
        delete m_mdiArea->parentWidget();

    m_mdiArea = 0;
}

void QDesignerWorkbench::switchToDockedMode()
{
    bool wasTopLevel = (m_mode == TopLevelMode);
    if (m_mode == DockedMode)
        return;

    switchToNeutralMode();
    m_mode = DockedMode;

    QDesignerToolWindow *widgetBoxWrapper = 0;
    if (0 != (widgetBoxWrapper = findToolWindow(core()->widgetBox()))) {
        widgetBoxWrapper->action()->setVisible(true);
        widgetBoxWrapper->setWindowTitle(tr("Widget Box"));
    }

    Q_ASSERT(m_mdiArea == 0);

    QDesignerSettings settings;
    QDesignerToolWindow *mw = new QDesignerToolWindow(this); // Just to have a copy of
    mw->setSaveSettingsOnClose(true);
    mw->setObjectName(QLatin1String("MDIWindow"));
    mw->setWindowTitle(tr("Qt Designer"));
    m_mdiArea = new QMdiArea(mw);
    m_mdiArea->setAcceptDrops(true);
    m_mdiArea->installEventFilter(this);
    m_mdiArea->setScrollBarsEnabled(true);
    connect(m_mdiArea, SIGNAL(subWindowActivated(QMdiSubWindow*)),
            this, SLOT(activateMdiAreaChildWindow(QMdiSubWindow*)));
    mw->setCentralWidget(m_mdiArea);
    m_core->setTopLevel(mw);
    (void) mw->statusBar();
    if (m_Positions.isEmpty()) {
        settings.setGeometryFor(mw, qApp->desktop()->availableGeometry(0));
    } else {
        if (QDesignerToolWindow *widgetBox = findToolWindow(core()->widgetBox())) {
            const PositionMap::const_iterator pit = m_Positions.constFind(widgetBox);
            const QPoint pos = pit != m_Positions.constEnd() ? pit->position() : QPoint(0, 0);
            mw->move(pos);
        }
        mw->setWindowState(mw->windowState() | Qt::WindowMaximized);
    }

#ifndef Q_WS_MAC
    mw->setMenuBar(m_globalMenuBar);
    m_globalMenuBar->show();
#endif
    mw->addToolBar(m_editToolBar);
    mw->addToolBar(m_toolToolBar);
    mw->addToolBar(m_formToolBar);
    m_editToolBar->show();
    m_toolToolBar->show();
    m_formToolBar->show();

    qDesigner->setMainWindow(mw);

    foreach (QDesignerToolWindow *tw, m_toolWindows) {
        if (wasTopLevel)
            settings.saveGeometryFor(tw);
        QDockWidget *dockWidget = magicalDockWidget(tw);
        if (dockWidget == 0) {
            dockWidget = new QDockWidget(mw);
            dockWidget->setObjectName(tw->objectName() + QLatin1String("_dock"));
            dockWidget->setWindowTitle(tw->windowTitle());
            mw->addDockWidget(tw->dockWidgetAreaHint(), dockWidget);
        }

        dockWidget->setWidget(tw);
        const PositionMap::const_iterator pit = m_Positions.constFind(tw);
        if (pit != m_Positions.constEnd()) pit->applyTo(dockWidget);
    }

    mw->restoreState(settings.mainWindowState(), 2);
    
    foreach (QDesignerFormWindow *fw, m_formWindows) {
        QMdiSubWindow *w = m_mdiArea->addSubWindow(fw, magicalWindowFlags(fw));
        w->setMinimumSize(QSize(0, 0));
        w->hide();
    }
    m_actionManager->setBringAllToFrontVisibility(false);
    mw->show();
    // Trigger adjustMDIFormPositions() delayed as viewport size is not yet known.

    if (!m_initializing)
        QMetaObject::invokeMethod(this, "adjustMDIFormPositions", Qt::QueuedConnection);
}

void QDesignerWorkbench::adjustMDIFormPositions()
{
    const QPoint mdiAreaOffset =  m_mdiArea->pos();

    foreach (QDesignerFormWindow *fw, m_formWindows) {
        const PositionMap::const_iterator pit = m_Positions.constFind(fw);
        if (pit != m_Positions.constEnd()) pit->applyTo(mdiSubWindowOf(fw), mdiAreaOffset);
    }
}

bool QDesignerWorkbench::eventFilter(QObject *object, QEvent *event)
{
    if (object == m_mdiArea) {
        if (event->type() == QEvent::DragEnter) {
            QDragEnterEvent *e = static_cast<QDragEnterEvent*>(event);
            if (e->mimeData()->hasFormat(QLatin1String("text/uri-list"))) {
                e->acceptProposedAction();
                return true;
            }
        } else if (event->type() == QEvent::Drop) {
            QDropEvent *e = static_cast<QDropEvent*>(event);
            if (!e->mimeData()->hasFormat(QLatin1String("text/uri-list")))
                return false;
            foreach (QUrl url, e->mimeData()->urls()) {
                if (!url.toLocalFile().isEmpty())
                    readInForm(url.toLocalFile());
            }
            e->acceptProposedAction();
            return true;
        }
    }
    return false;
}

void QDesignerWorkbench::switchToTopLevelMode()
{
    if (m_mode == TopLevelMode)
        return;

    // make sure that the widgetbox is visible if it is different from neutral.
    if (m_mode != NeutralMode) {
        if (QDesignerToolWindow *widgetbox_tool = findToolWindow(core()->widgetBox())) {
            if (!widgetbox_tool->action()->isChecked())
                widgetbox_tool->action()->trigger();
        }
    }

    switchToNeutralMode();
    const QPoint desktopOffset = QApplication::desktop()->availableGeometry().topLeft();
    m_mode = TopLevelMode;

    // The widget box is special, it gets the menubar and gets to be the main widget.

    QDesignerToolWindow *widgetBoxWrapper = 0;
    if (0 != (widgetBoxWrapper = findToolWindow(core()->widgetBox()))) {
        m_core->setTopLevel(widgetBoxWrapper);
#ifndef Q_WS_MAC
        widgetBoxWrapper->setMenuBar(m_globalMenuBar);
        widgetBoxWrapper->action()->setVisible(false);
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
    bool found_visible_window = false;
    foreach (QDesignerToolWindow *tw, m_toolWindows) {
        tw->setParent(magicalParent(), magicalWindowFlags(tw));
        settings.setGeometryFor(tw, tw->geometryHint());
        tw->action()->setChecked(tw->isVisible());
        found_visible_window |= tw->isVisible();
    }

    if (!m_toolWindows.isEmpty() && !found_visible_window)
        m_toolWindows.first()->show();

    m_actionManager->setBringAllToFrontVisibility(true);

    foreach (QDesignerFormWindow *fw, m_formWindows) {
        fw->setParent(magicalParent(), magicalWindowFlags(fw));
        const PositionMap::const_iterator pit = m_Positions.constFind(fw);
        if (pit != m_Positions.constEnd()) pit->applyTo(fw, desktopOffset);
    }
}

QDesignerFormWindow *QDesignerWorkbench::createFormWindow()
{
    QDesignerFormWindow *formWindow = new QDesignerFormWindow(/*formWindow=*/ 0, this);

    if (m_mdiArea) {
        QMdiSubWindow *newMdiSubWindow = m_mdiArea->addSubWindow(formWindow, magicalWindowFlags(formWindow));
        newMdiSubWindow->setMinimumSize(QSize(0, 0));
        m_mdiArea->setActiveSubWindow(newMdiSubWindow);
    } else {
        const QRect formWindowGeometryHint = formWindow->geometryHint();
        formWindow->setAttribute(Qt::WA_DeleteOnClose, true);
        formWindow->setParent(magicalParent(), magicalWindowFlags(formWindow));
        formWindow->resize(formWindowGeometryHint.size());
        formWindow->move(availableGeometry().center() - formWindowGeometryHint.center());
    }

    addFormWindow(formWindow);
    return formWindow;
}

QDesignerFormWindowManagerInterface *QDesignerWorkbench::formWindowManager() const
{
    return m_core->formWindowManager();
}

QDesignerFormEditorInterface *QDesignerWorkbench::core() const
{
    return m_core;
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
    if (m_mdiArea)
        return m_mdiArea->geometry();

    QDesktopWidget *desktop = qDesigner->desktop();

    QWidget *window = findToolWindow(core()->widgetBox());
    if (window)
        return desktop->availableGeometry(desktop->screenNumber(window));
    return desktop->availableGeometry(0);
}

int QDesignerWorkbench::marginHint() const
{    return 20;
}

void QDesignerWorkbench::activateMdiAreaChildWindow(QMdiSubWindow *subWindow)
{
    if (subWindow) {
        QWidget *widget = subWindow->widget();
        if (QDesignerFormWindow *fw = qobject_cast<QDesignerFormWindow*>(widget)) {
            core()->formWindowManager()->setActiveFormWindow(fw->editor());
            m_mdiArea->setActiveSubWindow(subWindow);
        }
    }
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
    updateBackup(formWindow->editor());
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
    QList<QObject*> plugins = QPluginLoader::staticInstances();
    plugins += core()->pluginManager()->instances();

    foreach (QObject *plugin, plugins) {
        if (QDesignerFormEditorPluginInterface *formEditorPlugin = qobject_cast<QDesignerFormEditorPluginInterface*>(plugin)) {
            if (!formEditorPlugin->isInitialized())
                formEditorPlugin->initialize(core());
        }
    }
}

void QDesignerWorkbench::saveSettings() const
{
    QDesignerSettings settings;
    if (m_mode == DockedMode) {
        if (qFindChild<QMdiArea *>(qDesigner->mainWindow())) {
            settings.saveGeometryFor(qDesigner->mainWindow());
            settings.setValue(qDesigner->mainWindow()->objectName() + QLatin1String("/visible"), false);
        }
    } else {
        foreach (QDesignerToolWindow *tw, m_toolWindows) {
            settings.saveGeometryFor(tw);
        }
    }
}

bool QDesignerWorkbench::readInForm(const QString &fileName) const
{
    return m_actionManager->readInForm(fileName);
}

bool QDesignerWorkbench::writeOutForm(QDesignerFormWindowInterface *formWindow, const QString &fileName) const
{
    return m_actionManager->writeOutForm(formWindow, fileName);
}

bool QDesignerWorkbench::saveForm(QDesignerFormWindowInterface *frm)
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
        if (formWindow->editor() == widget)
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
            QMessageBox box(QMessageBox::Warning, tr("Save Forms?"),
                    tr("There are %1 forms with unsaved changes."
                        " Do you want to review these changes before quitting?")
                    .arg(dirtyForms.size()),
                    QMessageBox::Cancel | QMessageBox::Discard | QMessageBox::Save);
            box.setInformativeText(tr("If you don't review your documents, all your changes will be lost."));
            box.button(QMessageBox::Discard)->setText(tr("Discard Changes"));
            QPushButton *save = static_cast<QPushButton *>(box.button(QMessageBox::Save));
            save->setText(tr("Review Changes"));
            box.setDefaultButton(save);
            switch (box.exec()) {
            case QMessageBox::Cancel:
                return false;
            case QMessageBox::Save:
               foreach (QDesignerFormWindow *fw, dirtyForms) {
                   fw->show();
                   fw->raise();
                   if (!fw->close()) {
                       return false;
                   }
               }
               break;
            case QMessageBox::Discard:
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
    switch (mode) {
        case TopLevelMode:
            switchToTopLevelMode();
            break;
        case DockedMode:
            switchToDockedMode();
            break;

        default: Q_ASSERT(0);
    }
}

void QDesignerWorkbench::updateWindowMenu(QDesignerFormWindowInterface *fwi)
{
    if (!fwi)
        return;
    QDesignerFormWindow *fw = qobject_cast<QDesignerFormWindow *>(fwi->parentWidget());
    if (!fw)
        return;
    
    fw->action()->setChecked(true);    
    m_actionManager->minimizeAction()->setChecked(isFormWindowMinimized(fw));    
}

void QDesignerWorkbench::formWindowActionTriggered(QAction *a)
{
    QDesignerFormWindow *fw = qobject_cast<QDesignerFormWindow *>(a->parentWidget());
    Q_ASSERT(fw);
    
    if (isFormWindowMinimized(fw))
        setFormWindowMinimized(fw, false);
    
    if (m_mode == DockedMode) {
        if (QMdiSubWindow *subWindow = qobject_cast<QMdiSubWindow *>(fw->parent())) {
            m_mdiArea->setActiveSubWindow(subWindow);
        }
    } else {
        fw->activateWindow();
        fw->raise();
    }
}

void QDesignerWorkbench::showToolBars()
{
    m_toolToolBar->show();
    m_formToolBar->show();
    m_editToolBar->show();
}

void QDesignerWorkbench::closeAllToolWindows()
{
    foreach (QDesignerToolWindow *tw, m_toolWindows)
        tw->hide();
}

QDockWidget *QDesignerWorkbench::magicalDockWidget(QWidget *widget) const
{
    if (!m_mdiArea)
        return 0;

    QDockWidget *dockWidget = qFindChild<QDockWidget*>(m_mdiArea->window(), widget->objectName() + QLatin1String("_dock"));
    return dockWidget;
}

bool QDesignerWorkbench::readInBackup()
{
    QMap<QString, QString> backupFileMap;
    backupFileMap = QDesignerSettings().backup();
    if(!backupFileMap.isEmpty()) {
        if (QMessageBox::question(0, tr("Backup Information"),
                                    tr("Designer was not correctly terminated during your last session."
                                       "There are existing Backup files, do you want to load them?"),
                                    tr("&Yes"), tr("&No"), QString(), 0, 1) == 0)
        {
            QMapIterator<QString, QString> it(backupFileMap);
            while(it.hasNext()) {
                it.next();

                QString fileName = it.key();
                fileName.replace(QLatin1String("[*]"), QString());

                if(m_actionManager->readInForm(it.value())) {
                    formWindowManager()->activeFormWindow()->setFileName(fileName);
                }
            }
            return true;
        }
    }

    return false;
}

void QDesignerWorkbench::updateBackup(QDesignerFormWindowInterface* fwi)
{
    QString fwn = QDir::convertSeparators(fwi->fileName());
    if (fwn.isEmpty())
        fwn = fwi->parentWidget()->windowTitle();

    QMap<QString, QString> map = QDesignerSettings().backup();
    map.remove(fwn);
    QDesignerSettings().setBackup(map);
}

namespace {
    void raiseWindow(QWidget *w) {
        if (w->isMinimized())
            w->setWindowState(w->windowState() & ~Qt::WindowMinimized);
        w->raise();
    }
}

void QDesignerWorkbench::bringAllToFront()
{
    if (m_mode !=  TopLevelMode)
        return;
    foreach(QDesignerToolWindow *tw, m_toolWindows)
        raiseWindow(tw);
    foreach(QDesignerFormWindow *dfw, m_formWindows)
        raiseWindow(dfw);
}

// Resize a form window taking MDI decorations into account
void QDesignerWorkbench::resizeForm(QDesignerFormWindow *fw, const QWidget *mainContainer) const
{
    const QSize containerSize = mainContainer->size();
    const QSize containerMinimumSize = mainContainer->minimumSize();
    const QSize containerMaximumSize = mainContainer->maximumSize();
    if (m_mode != DockedMode) {
        fw->resize(containerSize);
        fw->setMinimumSize(containerMinimumSize);
        fw->setMaximumSize(containerMaximumSize);
        return;
    }
    // get decorations and resize MDI
    QMdiSubWindow *mdiSubWindow = qobject_cast<QMdiSubWindow *>(fw->parent());
    Q_ASSERT(mdiSubWindow);
    const QSize decorationSize = mdiSubWindow->geometry().size() - mdiSubWindow->contentsRect().size(); // TODO new API
    mdiSubWindow->resize(containerSize + decorationSize);
    mdiSubWindow->setMinimumSize(containerMinimumSize + decorationSize);

    if (containerMaximumSize == QSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX)) {
        mdiSubWindow->setMaximumSize(containerMaximumSize);
    } else {
        mdiSubWindow->setMaximumSize(containerMaximumSize + decorationSize);
    }
}


// Load a form or return 0 and do cleanup. file name and editor file
// name in case of loading a template file.

QDesignerFormWindow * QDesignerWorkbench::loadForm(const QString &fileName,
                                                   bool *uic3Converted,
                                                   QString *errorMessage)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly)) {
        *errorMessage = tr("The file <b>%1</b> could not be opened.").arg(file.fileName());
        return 0;
    }
    // Create a form
     QDesignerFormWindowManagerInterface *formWindowManager = m_core->formWindowManager();

    QDesignerFormWindow *formWindow = createFormWindow();
    QDesignerFormWindowInterface *editor = formWindow->editor();
    Q_ASSERT(editor);

    
    // Temporarily set the file name. It is needed when converting a UIC 3 file.
    // In this case, the file name will we be cleared on return to force a save box.
    editor->setFileName(fileName);
    editor->setContents(&file);
    if (!editor->mainContainer()) {
        removeFormWindow(formWindow);
        formWindowManager->removeFormWindow(editor);
        m_core->metaDataBase()->remove(editor);
        *errorMessage = tr("The file <b>%1</b> is not a valid Designer ui file.").arg(file.fileName());
        return 0;
    }
    *uic3Converted = editor->fileName().isEmpty();
    editor->setDirty(false);
    resizeForm(formWindow, editor->mainContainer());
    formWindowManager->setActiveFormWindow(editor);
    return formWindow;
}


QDesignerFormWindow * QDesignerWorkbench::openForm(const QString &fileName, QString *errorMessage)
{
    bool uic3Converted;
    QDesignerFormWindow *rc =loadForm(fileName, &uic3Converted, errorMessage);
    if (!rc)
        return 0;

    rc->updateWindowTitle(fileName);
    if (!uic3Converted)
        rc->editor()->setFileName(fileName);

    rc->show();
    return rc;
}

QDesignerFormWindow * QDesignerWorkbench::openTemplate(const QString &templateFileName,
                                                       const QString &editorFileName,
                                                       const QString &title,
                                                       QString *errorMessage)
{
    bool uic3Converted;
    QDesignerFormWindow *rc =loadForm(templateFileName, &uic3Converted, errorMessage);
    if (!rc)
        return 0;

    rc->setWindowTitle(title);
    if (!uic3Converted)
        rc->editor()->setFileName(editorFileName);
    rc->show();
    return rc;
}

void QDesignerWorkbench::minimizationStateChanged(QDesignerFormWindowInterface *formWindow, bool minimized)
{
    // refresh the minimize action state
    if (core()->formWindowManager()->activeFormWindow() == formWindow) {
        m_actionManager->minimizeAction()->setChecked(minimized);
    }
}


void QDesignerWorkbench::toggleFormMinimizationState()
{
    QDesignerFormWindowInterface *fwi = core()->formWindowManager()->activeFormWindow();
    if (!fwi || m_mode == NeutralMode)
        return;
    QDesignerFormWindow *fw = qobject_cast<QDesignerFormWindow *>(fwi->parentWidget());
    Q_ASSERT(fw);
    setFormWindowMinimized(fw, !isFormWindowMinimized(fw));
}

bool QDesignerWorkbench::isFormWindowMinimized(const QDesignerFormWindow *fw)
{
    switch (m_mode) {
    case DockedMode:
        return mdiSubWindowOf(fw)->isShaded();
    case TopLevelMode: 
        return fw->window()->isMinimized();
    default:
        break;
    }
    return fw->isMinimized();
}

void QDesignerWorkbench::setFormWindowMinimized(QDesignerFormWindow *fw, bool minimized)
{
    switch (m_mode) {
    case DockedMode: {
        QMdiSubWindow *mdiSubWindow = mdiSubWindowOf(fw);
        if (minimized) {
            mdiSubWindow->showShaded();
        } else {
            mdiSubWindow->setWindowState(mdiSubWindow->windowState() & ~Qt::WindowMinimized);
        }
    }
        break;
    case TopLevelMode:        {
        QWidget *window = fw->window();
        if (window->isMinimized()) {
            window->setWindowState(window->windowState() & ~Qt::WindowMinimized);
        } else {
            window->showMinimized();
        }
    }
        break;
    default:
        break;
    }
}
 
void QDesignerWorkbench::applyPreferences(const Preferences &preferences)
{    
    if (preferences.m_uiMode != mode())
        setUIMode(preferences.m_uiMode);
        
    if (preferences.m_font != qApp->font())
        qApp->setFont(preferences.m_font);
}
