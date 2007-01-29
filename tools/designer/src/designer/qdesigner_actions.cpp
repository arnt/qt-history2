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

#include "qdesigner_actions.h"
#include "qdesigner.h"
#include "qdesigner_workbench.h"
#include "qdesigner_formwindow.h"
#include "qdesigner_settings.h"
#include "newform.h"
#include "versiondialog.h"
#include "oublietteview.h"
#include "saveformastemplate.h"
#include "plugindialog.h"
#include "formwindowsettings.h"

#include <pluginmanager_p.h>
#include <qdesigner_formbuilder_p.h>
// sdk
#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/QDesignerFormWindowInterface>
#include <QtDesigner/QDesignerLanguageExtension>
#include <QtDesigner/QDesignerWidgetDataBaseInterface>
#include <QtDesigner/QDesignerMetaDataBaseInterface>
#include <QtDesigner/QDesignerFormWindowManagerInterface>
#include <QtDesigner/QDesignerFormWindowCursorInterface>
#include <QtDesigner/QDesignerPropertySheetExtension>
#include <QtDesigner/QDesignerPropertyEditorInterface>
#include <QtDesigner/QDesignerFormEditorPluginInterface>
#include <QtDesigner/QExtensionManager>

#include <QtAssistant/QAssistantClient>

#include <QtGui/QAction>
#include <QtGui/QStyleFactory>
#include <QtGui/QAction>
#include <QtGui/QActionGroup>
#include <QtGui/QCloseEvent>
#include <QtGui/QFileDialog>
#include <QtGui/QMenu>
#include <QtGui/QMessageBox>
#include <QtGui/QIcon>
#include <QtGui/QMdiSubWindow>

#include <QtCore/QLibraryInfo>
#include <QtCore/QBuffer>
#include <QtCore/QPluginLoader>
#include <QtCore/qdebug.h>
#include <QtCore/QTimer>
#include <QtXml/QDomDocument>

#include <QtGui/QDesktopWidget>
#include <QtCore/QMetaObject>
#include <QtGui/QStatusBar>

static QString getFileExtension(QDesignerFormEditorInterface *core)
{
    QDesignerLanguageExtension *lang
        = qt_extension<QDesignerLanguageExtension *>(core->extensionManager(), core);
    if (lang)
        return lang->uiExtension();
    return QLatin1String("ui");
}

QDesignerActions::QDesignerActions(QDesignerWorkbench *workbench)
    : QObject(workbench),
      m_workbench(workbench), m_assistantClient(0), m_openDirectory(QString()),
      m_saveDirectory(QString())
{
    Q_ASSERT(m_workbench != 0);

    m_core = m_workbench->core();
    Q_ASSERT(m_core != 0);

    QDesignerFormWindowManagerInterface *formWindowManager = m_core->formWindowManager();
    Q_ASSERT(formWindowManager != 0);

    QDesignerSettings settings;
    m_fileActions = new QActionGroup(this);
    m_fileActions->setExclusive(false);

    m_recentFilesActions = new QActionGroup(this);
    m_recentFilesActions->setExclusive(false);

    m_editActions = new QActionGroup(this);
    m_editActions->setExclusive(false);

    m_formActions = new QActionGroup(this);
    m_formActions->setExclusive(false);

    m_windowActions = new QActionGroup(this);
    m_windowActions->setExclusive(false);

    m_toolActions = new QActionGroup(this);
    m_toolActions->setExclusive(true);

    m_helpActions = new QActionGroup(this);
    m_helpActions->setExclusive(false);


//
// file actions
//
    m_newFormAction = new QAction(tr("&New Form..."), this);
    m_newFormAction->setShortcut(tr("CTRL+N"));
    connect(m_newFormAction, SIGNAL(triggered()), this, SLOT(createForm()));
    m_fileActions->addAction(m_newFormAction);

    m_openFormAction = new QAction(tr("&Open Form..."), this);
    m_openFormAction->setShortcut(tr("CTRL+O"));
    connect(m_openFormAction, SIGNAL(triggered()), this, SLOT(openForm()));
    m_fileActions->addAction(m_openFormAction);

    QAction *act;
    // Need to insert this into the QAction.
    for (int i = 0; i < MaxRecentFiles; ++i) {
        act = new QAction(this);
        act->setVisible(false);
        connect(act, SIGNAL(triggered()), this, SLOT(openRecentForm()));
        m_recentFilesActions->addAction(act);
    }
    updateRecentFileActions();

    act = new QAction(this);
    act->setSeparator(true);
    m_recentFilesActions->addAction(act);

    act = new QAction(tr("Clear &Menu"), this);
    act->setObjectName(QLatin1String("__qt_action_clear_menu_"));
    connect(act, SIGNAL(triggered()), this, SLOT(clearRecentFiles()));
    m_recentFilesActions->addAction(act);

    QAction *sep = new QAction(this);
    sep->setSeparator(true);
    m_fileActions->addAction(sep);

    m_saveFormAction = new QAction(tr("&Save Form"), this);
    m_saveFormAction->setShortcut(tr("CTRL+S"));
    connect(m_saveFormAction, SIGNAL(triggered()), this, SLOT(saveForm()));
    m_fileActions->addAction(m_saveFormAction);

    m_saveFormAsAction = new QAction(tr("Save Form &As..."), this);
    connect(m_saveFormAsAction, SIGNAL(triggered()), this, SLOT(saveFormAs()));
    m_fileActions->addAction(m_saveFormAsAction);

    m_saveAllFormsAction = new QAction(tr("Save A&ll Forms"), this);
    m_saveAllFormsAction->setShortcut(tr("CTRL+SHIFT+S"));
    connect(m_saveAllFormsAction, SIGNAL(triggered()), this, SLOT(saveAllForms()));
    m_fileActions->addAction(m_saveAllFormsAction);

    m_saveFormAsTemplateAction = new QAction(tr("Save Form As &Template..."), this);
    connect(m_saveFormAsTemplateAction, SIGNAL(triggered()), this, SLOT(saveFormAsTemplate()));
    m_fileActions->addAction(m_saveFormAsTemplateAction);

    sep = new QAction(this);
    sep->setSeparator(true);
    m_fileActions->addAction(sep);

    m_closeFormAction = new QAction(tr("&Close Form"), this);
    m_closeFormAction->setShortcut(tr("CTRL+W"));
    connect(m_closeFormAction, SIGNAL(triggered()), this, SLOT(closeForm()));
    m_fileActions->addAction(m_closeFormAction);

    sep = new QAction(this);
    sep->setSeparator(true);
    m_fileActions->addAction(sep);

    m_quitAction = new QAction(tr("&Quit"), this);
    m_quitAction->setShortcut(tr("CTRL+Q"));
    connect(m_quitAction, SIGNAL(triggered()), this, SLOT(shutdown()));
    m_fileActions->addAction(m_quitAction);

//
// edit actions
//
    m_undoAction = formWindowManager->actionUndo();
    m_undoAction->setShortcut(tr("CTRL+Z"));
    m_editActions->addAction(m_undoAction);

    m_redoAction = formWindowManager->actionRedo();
    m_redoAction->setShortcut(tr("CTRL+SHIFT+Z"));
    m_editActions->addAction(m_redoAction);

    sep = new QAction(this);
    sep->setSeparator(true);
    m_editActions->addAction(sep);

    m_cutAction = formWindowManager->actionCut();
    m_editActions->addAction(m_cutAction);

    m_copyAction = formWindowManager->actionCopy();
    m_editActions->addAction(m_copyAction);

    m_pasteAction = formWindowManager->actionPaste();
    m_editActions->addAction(m_pasteAction);

    m_deleteAction = formWindowManager->actionDelete();
    m_editActions->addAction(m_deleteAction);

    m_selectAllAction = formWindowManager->actionSelectAll();
    m_editActions->addAction(m_selectAllAction);

    sep = new QAction(this);
    sep->setSeparator(true);
    m_editActions->addAction(sep);

    m_sendToBackAction = formWindowManager->actionLower();
    m_editActions->addAction(m_sendToBackAction);

    m_bringToFrontAction = formWindowManager->actionRaise();
    m_editActions->addAction(m_bringToFrontAction);

//
// edit mode actions
//

    m_editWidgetsAction = new QAction(tr("Edit Widgets"), this);
    m_editWidgetsAction->setCheckable(true);
    m_editWidgetsAction->setShortcut(tr("F3"));
    m_editWidgetsAction->setIcon(QIcon(m_core->resourceLocation() + QLatin1String("/widgettool.png")));
    connect(formWindowManager, SIGNAL(activeFormWindowChanged(QDesignerFormWindowInterface*)),
                this, SLOT(activeFormWindowChanged(QDesignerFormWindowInterface*)));
    connect(m_editWidgetsAction, SIGNAL(triggered()), this, SLOT(editWidgetsSlot()));
    m_toolActions->addAction(m_editWidgetsAction);
    m_editWidgetsAction->setChecked(true);
    m_editWidgetsAction->setEnabled(false);
    QList<QObject*> builtinPlugins = QPluginLoader::staticInstances();
    builtinPlugins += m_core->pluginManager()->instances();
    foreach (QObject *plugin, builtinPlugins) {
        if (QDesignerFormEditorPluginInterface *formEditorPlugin = qobject_cast<QDesignerFormEditorPluginInterface*>(plugin)) {
            if (QAction *action = formEditorPlugin->action()) {
                m_toolActions->addAction(action);
                action->setCheckable(true);
            }
        }
    }

    m_uiMode = new QActionGroup(this);
    m_uiMode->setExclusive(true);

    m_sdiAction = m_uiMode->addAction(tr("Multiple Top-Level Windows"));
    m_sdiAction->setCheckable(true);

    m_dockedMdiAction = m_uiMode->addAction(tr("Docked Window"));
    m_dockedMdiAction->setCheckable(true);

    switch (settings.uiMode()) {
        default: Q_ASSERT(0); break;

        case QDesignerWorkbench::TopLevelMode:
            m_sdiAction->setChecked(true);
            break;
        case QDesignerWorkbench::DockedMode:
            m_dockedMdiAction->setChecked(true);
            break;
    }

    connect(m_uiMode, SIGNAL(triggered(QAction*)), this, SLOT(updateUIMode(QAction*)));

//
// form actions
//
    m_layoutHorizontallyAction = formWindowManager->actionHorizontalLayout();
    m_formActions->addAction(m_layoutHorizontallyAction);

    m_layoutVerticallyAction = formWindowManager->actionVerticalLayout();
    m_formActions->addAction(m_layoutVerticallyAction);

    m_layoutHorizontallyInSplitterAction = formWindowManager->actionSplitHorizontal();
    m_formActions->addAction(m_layoutHorizontallyInSplitterAction);

    m_layoutVerticallyInSplitterAction = formWindowManager->actionSplitVertical();
    m_formActions->addAction(m_layoutVerticallyInSplitterAction);

    m_layoutGridAction = formWindowManager->actionGridLayout();
    m_formActions->addAction(m_layoutGridAction);

    m_breakLayoutAction = formWindowManager->actionBreakLayout();
    m_formActions->addAction(m_breakLayoutAction);

    m_adjustSizeAction = formWindowManager->actionAdjustSize();
    m_formActions->addAction(m_adjustSizeAction);

    sep = new QAction(this);
    sep->setSeparator(true);

    m_formActions->addAction(sep);

    m_previewFormAction = new QAction(tr("&Preview"), this);
    m_previewFormAction->setShortcut(tr("CTRL+R"));
    connect(m_previewFormAction, SIGNAL(triggered()), this, SLOT(previewFormLater()));
    m_formActions->addAction(m_previewFormAction);

    m_styleActions = new QActionGroup(this);
    m_styleActions->setExclusive(true);
    connect(m_styleActions, SIGNAL(triggered(QAction*)), this, SLOT(previewForm(QAction*)));

    QAction *sep2 = new QAction(this);
    sep2->setSeparator(true);
    m_formActions->addAction(sep2);

    m_formSettings = new QAction(tr("Form &Settings..."), this);
    m_formSettings->setEnabled(false);
    connect(m_formSettings, SIGNAL(triggered()), this, SLOT(showFormSettings()));
    m_formActions->addAction(m_formSettings);

    QStringList availableStyleList = QStyleFactory::keys();
    foreach (QString style, availableStyleList) {
        QAction *a = new QAction(this);
        a->setText(tr("%1 Style").arg(style));
        a->setObjectName(QLatin1String("__qt_action_style_") + style);

        m_styleActions->addAction(a);
    }

//
// window actions
//
    m_minimizeAction = new QAction(tr("&Minimize"), this);
    m_minimizeAction->setEnabled(false);
    m_minimizeAction->setCheckable(true);
    m_minimizeAction->setShortcut(tr("CTRL+M"));
    connect(m_minimizeAction, SIGNAL(triggered()), m_workbench, SLOT(toggleFormMinimizationState()));
    m_windowActions->addAction(m_minimizeAction);

    sep = new QAction(this);
    sep->setSeparator(true);
    m_windowActions->addAction(sep);

    m_bringToFrontAction = new QAction(tr("Bring All to Front"), this);
    connect(m_bringToFrontAction, SIGNAL(triggered()), m_workbench, SLOT(bringAllToFront()));
    m_windowActions->addAction(m_bringToFrontAction);

//
// Help actions
//

    m_mainHelpAction = new QAction(tr("Qt Designer &Help"), this);
    connect(m_mainHelpAction, SIGNAL(triggered()), this, SLOT(showDesignerHelp()));
    m_mainHelpAction->setShortcut(Qt::CTRL + Qt::Key_Question);
    m_helpActions->addAction(m_mainHelpAction);

    sep = new QAction(this);
    sep->setSeparator(true);
    m_helpActions->addAction(sep);

    m_widgetHelp = new QAction(tr("Current Widget Help"), this);
    m_widgetHelp->setShortcut(Qt::Key_F1);
    connect(m_widgetHelp, SIGNAL(triggered()), this, SLOT(showWidgetSpecificHelp()));
    m_helpActions->addAction(m_widgetHelp);

    sep = new QAction(this);
    sep->setSeparator(true);
    m_helpActions->addAction(sep);

    m_whatsNewAction = new QAction(tr("What's New in Qt Designer?"), this);
    connect(m_whatsNewAction, SIGNAL(triggered()), this, SLOT(showWhatsNew()));
    m_helpActions->addAction(m_whatsNewAction);

    sep = new QAction(this);
    sep->setSeparator(true);
    m_helpActions->addAction(sep);
    m_aboutPluginsAction = new QAction(tr("About Plugins"), this);
    m_aboutPluginsAction->setMenuRole(QAction::ApplicationSpecificRole);
    connect(m_aboutPluginsAction, SIGNAL(triggered()), this, SLOT(aboutPlugins()));
    m_helpActions->addAction(m_aboutPluginsAction);

    m_aboutDesignerAction = new QAction(tr("About Qt Designer"), this);
    connect(m_aboutDesignerAction, SIGNAL(triggered()), this, SLOT(aboutDesigner()));
    m_helpActions->addAction(m_aboutDesignerAction);

    m_aboutQtAction = new QAction(tr("About Qt"), this);
    connect(m_aboutQtAction, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
    m_helpActions->addAction(m_aboutQtAction);
//
// connections
//
    fixActionContext();
    activeFormWindowChanged(core()->formWindowManager()->activeFormWindow());

    m_backupTimer = new QTimer(this);
    m_backupTimer->start(180000); // 3min
    connect(m_backupTimer, SIGNAL(timeout()), this, SLOT(backupForms()));
}

QDesignerActions::~QDesignerActions()
{
}

QActionGroup *QDesignerActions::toolActions() const
{ return m_toolActions; }

QDesignerWorkbench *QDesignerActions::workbench() const
{ return m_workbench; }

QDesignerFormEditorInterface *QDesignerActions::core() const
{ return m_core; }

QActionGroup *QDesignerActions::fileActions() const
{ return m_fileActions; }

QActionGroup *QDesignerActions::editActions() const
{ return m_editActions; }

QActionGroup *QDesignerActions::formActions() const
{ return m_formActions; }

QActionGroup *QDesignerActions::windowActions() const
{ return m_windowActions; }

QActionGroup *QDesignerActions::helpActions() const
{ return m_helpActions; }

QActionGroup *QDesignerActions::styleActions() const
{ return m_styleActions; }

QAction *QDesignerActions::newFormAction() const
{ return m_newFormAction; }

QAction *QDesignerActions::openFormAction() const
{ return m_openFormAction; }

QAction *QDesignerActions::saveFormAction() const
{ return m_saveFormAction; }

QAction *QDesignerActions::saveFormAsAction() const
{ return m_saveFormAsAction; }

QAction *QDesignerActions::saveAllFormsAction() const
{ return m_saveAllFormsAction; }

QAction *QDesignerActions::saveFormAsTemplateAction() const
{ return m_saveFormAsTemplateAction; }

QAction *QDesignerActions::closeFormAction() const
{ return m_closeFormAction; }

QAction *QDesignerActions::quitAction() const
{ return m_quitAction; }

QAction *QDesignerActions::undoAction() const
{ return m_undoAction; }

QAction *QDesignerActions::redoAction() const
{ return m_redoAction; }

QAction *QDesignerActions::cutAction() const
{ return m_cutAction; }

QAction *QDesignerActions::copyAction() const
{ return m_copyAction; }

QAction *QDesignerActions::pasteAction() const
{ return m_pasteAction; }

QAction *QDesignerActions::selectAllAction() const
{ return m_selectAllAction; }

QAction *QDesignerActions::deleteAction() const
{ return m_deleteAction; }

QAction *QDesignerActions::sendToBackAction() const
{ return m_sendToBackAction; }

QAction *QDesignerActions::bringToFrontAction() const
{ return m_bringToFrontAction; }

QAction *QDesignerActions::mainHelpAction() const
{ return m_mainHelpAction; }

QAction *QDesignerActions::whatsNewAction() const
{ return m_whatsNewAction; }

QAction *QDesignerActions::aboutQtAction() const
{ return m_aboutQtAction; }

QAction *QDesignerActions::aboutDesignerAction() const
{ return m_aboutDesignerAction; }

QAction *QDesignerActions::layoutHorizontallyAction() const
{ return m_layoutHorizontallyAction; }

QAction *QDesignerActions::layoutVerticallyAction() const
{ return m_layoutVerticallyAction; }

QAction *QDesignerActions::layoutHorizontallyInSplitterAction() const
{ return m_layoutHorizontallyInSplitterAction; }

QAction *QDesignerActions::layoutVerticallyInSplitterAction() const
{ return m_layoutVerticallyInSplitterAction; }

QAction *QDesignerActions::layoutGridAction() const
{ return m_layoutGridAction; }

QAction *QDesignerActions::breakLayoutAction() const
{ return m_breakLayoutAction; }

QAction *QDesignerActions::adjustSizeAction() const
{ return m_adjustSizeAction; }

QAction *QDesignerActions::previewFormAction() const
{ return m_previewFormAction; }

QAction *QDesignerActions::formSettings() const
{ return m_formSettings; }

void QDesignerActions::editWidgetsSlot()
{
    QDesignerFormWindowManagerInterface *formWindowManager = core()->formWindowManager();
    for (int i=0; i<formWindowManager->formWindowCount(); ++i) {
        QDesignerFormWindowInterface *formWindow = formWindowManager->formWindow(i);
        formWindow->editWidgets();
    }
}

void QDesignerActions::createForm()
{
    showNewFormDialog(QString());
}

void QDesignerActions::showNewFormDialog(const QString &fileName)
{
    NewForm *dlg = new NewForm(workbench(), workbench()->core()->topLevel(), fileName);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->setAttribute(Qt::WA_ShowModal);

    dlg->setGeometry(fixDialogRect(dlg->rect()));
    dlg->exec();
}


bool QDesignerActions::openForm()
{
    const QString extension = getFileExtension(core());
    const QStringList fileNames = QFileDialog::getOpenFileNames(core()->topLevel(), tr("Open Form"),
        m_openDirectory, tr("Designer UI files (*.%1);;All Files (*)").arg(extension), 0, QFileDialog::DontUseSheet);

    if (fileNames.isEmpty())
        return false;

    bool atLeastOne = false;
    foreach (QString fileName, fileNames) {
        if (readInForm(fileName) && !atLeastOne)
            atLeastOne = true;
    }

    return atLeastOne;
}

bool QDesignerActions::saveFormAs(QDesignerFormWindowInterface *fw)
{
    QString extension = getFileExtension(core());

    QString dir = fw->fileName();
    if (dir.isEmpty()) {
        if (!m_saveDirectory.isEmpty() || !m_openDirectory.isEmpty()) {
            dir = m_saveDirectory.isEmpty() ? m_openDirectory + QLatin1String("/untitled.") :
                  m_saveDirectory + QLatin1String("/untitled.") ;
        } else {
            dir = QDir::current().absolutePath() + QLatin1String("/untitled.");
        }
        dir += extension;
    }

    QString saveFile;
    while (1) {
        saveFile = QFileDialog::getSaveFileName(fw, tr("Save form as"),
                dir,
                tr("Designer UI files (*.%1);;All Files (*)").arg(extension), 0, QFileDialog::DontConfirmOverwrite);
        if (saveFile.isEmpty())
            return false;

        const QFileInfo fInfo(saveFile);
        if (fInfo.suffix().isEmpty() && !fInfo.fileName().endsWith(QLatin1Char('.')))
            saveFile.append(QLatin1Char('.')).append(extension);

        const QFileInfo fi(saveFile);
        if (!fi.exists())
            break;

        if (QMessageBox::warning(fw, tr("Save"), tr("%1 already exists.\nDo you want to replace it?")
                    .arg(fi.fileName()), QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
            break;

        dir = saveFile;
    }

    fw->setFileName(saveFile);
    return writeOutForm(fw, saveFile);
}

void QDesignerActions::saveForm()
{
    if (QDesignerFormWindowInterface *fw = core()->formWindowManager()->activeFormWindow()) {
        if (saveForm(fw))
            showStatusBarMessage(tr("Form %1 successful saved...").arg(fw->fileName()));
    }
}

void QDesignerActions::saveAllForms()
{
    QString fileNames;
    QDesignerFormWindowManagerInterface *formWindowManager = core()->formWindowManager();
    const int totalWindows = formWindowManager->formWindowCount();
    for (int i = 0; i < totalWindows; ++i) {
        QDesignerFormWindowInterface *fw = formWindowManager->formWindow(i);
        if (fw && fw->isDirty()) {
            formWindowManager->setActiveFormWindow(fw);
            if (saveForm(fw))
                fileNames.append(QFileInfo(fw->fileName()).fileName() + QString(QLatin1String(", ")));
            else
                break;
        }
    }

    if (!fileNames.isEmpty()) {
        fileNames.resize(fileNames.length() -2);
        showStatusBarMessage(tr("Form %1 successful saved...").arg(fileNames));
    }
}

bool QDesignerActions::saveForm(QDesignerFormWindowInterface *fw)
{
    bool ret;
    if (fw->fileName().isEmpty())
        ret = saveFormAs(fw);
    else
        ret =  writeOutForm(fw, fw->fileName());
    return ret;
}

void QDesignerActions::closeForm()
{
    if (QDesignerFormWindowInterface *fw = core()->formWindowManager()->activeFormWindow())
        if (QWidget *parent = fw->parentWidget()) {
            if (QMdiSubWindow *mdiSubWindow = qobject_cast<QMdiSubWindow *>(parent->parentWidget())) {
                mdiSubWindow->close();
            } else {
                parent->close();
            }
        }
}

void QDesignerActions::saveFormAs()
{
    if (QDesignerFormWindowInterface *fw = core()->formWindowManager()->activeFormWindow()) {
        if (saveFormAs(fw))
            showStatusBarMessage(tr("Form %1 successful saved...").arg(fw->fileName()));

    }
}

void QDesignerActions::saveFormAsTemplate()
{
    if (QDesignerFormWindowInterface *fw = core()->formWindowManager()->activeFormWindow()) {
        SaveFormAsTemplate dlg(fw, fw->window());
        dlg.exec();
    }
}

void QDesignerActions::notImplementedYet()
{
    QMessageBox::information(core()->topLevel(), tr("Designer"), tr("Feature not implemented yet!"));
}

void QDesignerActions::updateUIMode(QAction *act)
{
    QDesignerWorkbench::UIMode mode = QDesignerWorkbench::TopLevelMode;
    if (act == m_dockedMdiAction)
        mode = QDesignerWorkbench::DockedMode;

    QDesignerSettings settings;
    settings.setUIMode(mode);

    m_workbench->setUIMode(QDesignerWorkbench::UIMode(settings.uiMode()));
}

void QDesignerActions::previewFormLater(QAction *action)
{
    qRegisterMetaType<QAction*>("QAction*");
    QMetaObject::invokeMethod(this, "previewForm", Qt::QueuedConnection,
                                Q_ARG(QAction*, action));
}

void QDesignerActions::previewForm(QAction *action)
{
    if (QDesignerFormWindowInterface *fw = core()->formWindowManager()->activeFormWindow()) {
        qdesigner_internal::QDesignerFormBuilder builder(core());
        builder.setWorkingDirectory(fw->absoluteDir());

        QByteArray bytes = fw->contents().toUtf8();
        QBuffer buffer(&bytes);

        QWidget *widget = builder.load(&buffer, 0);
        Q_ASSERT(widget);

        widget->setParent(fw->window(), (widget->windowType() == Qt::Window) ?
                                         Qt::Window | Qt::WindowMaximizeButtonHint :
                                         Qt::Dialog);
#ifndef Q_WS_MAC
        widget->setWindowModality(Qt::ApplicationModal);
#endif
        widget->setAttribute(Qt::WA_DeleteOnClose, true);
        widget->move(fw->window()->mapToGlobal(QPoint(0, 0)) + QPoint(10, 10));

        if (action != 0 && action->objectName().startsWith(QLatin1String("__qt_action_style_"))) {
            const QString styleName = action->objectName().mid(QString::fromUtf8("__qt_action_style_").count());
            if (QStyle *style = QStyleFactory::create(styleName)) {
                style->setParent(widget);
                widget->setStyle(style);
                if (style->metaObject()->className() != QApplication::style()->metaObject()->className())
                    widget->setPalette(style->standardPalette());

                const QList<QWidget*> lst = qFindChildren<QWidget*>(widget);
                foreach (QWidget *w, lst)
                    w->setStyle(style);
            }
        }

        widget->setWindowTitle(tr("%1 - [Preview]").arg(widget->windowTitle()));
        widget->installEventFilter(this);
        widget->show();
    }
}

void QDesignerActions::fixActionContext()
{
    QList<QAction*> actions;
    actions += m_fileActions->actions();
    actions += m_editActions->actions();
    actions += m_toolActions->actions();
    actions += m_formActions->actions();
    actions += m_windowActions->actions();
    actions += m_helpActions->actions();

    foreach (QAction *a, actions) {
        a->setShortcutContext(Qt::ApplicationShortcut);
    }
}

bool QDesignerActions::readInForm(const QString &fileName)
{
    QString fn = fileName;

    // First make sure that we don't have this one open already.
    QDesignerFormWindowManagerInterface *formWindowManager = core()->formWindowManager();
    const int totalWindows = formWindowManager->formWindowCount();
    for (int i = 0; i < totalWindows; ++i) {
        QDesignerFormWindowInterface *w = formWindowManager->formWindow(i);
        if (w->fileName() == fn) {
            w->raise();
            formWindowManager->setActiveFormWindow(w);
            addRecentFile(fn);
            return true;
        }
    }

    // Otherwise load it.
    do {
        QString errorMessage;
        if (workbench()->openForm(fn, &errorMessage)) {
            addRecentFile(fn);
            m_openDirectory = QFileInfo(fn).absolutePath();
            return true;
        } else {
            // prompt to reload
            QMessageBox box(QMessageBox::Warning, tr("Read error"),
                            tr("%1\nDo you want to update the file location or generate a new form?").arg(errorMessage),
                            QMessageBox::Cancel, core()->topLevel());
            
            QPushButton *updateButton = box.addButton(tr("&Update"), QMessageBox::ActionRole);
            QPushButton *newButton    = box.addButton(tr("&New Form"), QMessageBox::ActionRole);
            box.exec();
            if (box.clickedButton() == box.button(QMessageBox::Cancel))
                return false;
            
            if (box.clickedButton() == updateButton) {
                const QString extension = getFileExtension(core());
                fn = QFileDialog::getOpenFileName(core()->topLevel(),
                                                  tr("Open Form"), m_openDirectory,
                                                  tr("Designer UI files (*.%1);;All Files (*)").arg(extension), 0, QFileDialog::DontUseSheet);

                if (fn.isEmpty())
                    return false;
            } else if (box.clickedButton() == newButton) {
                // If the file does not exist, but its directory, is valid, open the template with the editor file name set to it.
                // (called from command line).
                QString newFormFileName;
                const  QFileInfo fInfo(fn);
                if (!fInfo.exists()) {
                    // Normalize file name
                    const QString directory = fInfo.absolutePath();
                    if (QDir(directory).exists()) {
                        newFormFileName = directory;
                        newFormFileName  += QLatin1Char('/');
                        newFormFileName  += fInfo.fileName();
                    }
                }
                showNewFormDialog(newFormFileName);
                return false;
            }
        }
    } while (true);
    return true;
}

static QString createBackup(const QString &fileName)
{
    const QString suffix = QLatin1String(".bak");
    QString backupFile = fileName + suffix;
    QFileInfo fi(backupFile);
    int i = 0;
    while (fi.exists()) {
        backupFile = fileName + suffix + QString::number(++i);
        fi.setFile(backupFile);
    }

    if (QFile::copy(fileName, backupFile))
        return backupFile;
    return QString();
}

static void removeBackup(const QString &backupFile)
{
    if (!backupFile.isEmpty())
        QFile::remove(backupFile);
}

bool QDesignerActions::writeOutForm(QDesignerFormWindowInterface *fw, const QString &saveFile)
{
    Q_ASSERT(fw && !saveFile.isEmpty());

    QString backupFile;
    QFileInfo fi(saveFile);
    if (fi.exists())
        backupFile = createBackup(saveFile);

    const QByteArray utf8Array = fw->contents().toUtf8();
    m_workbench->updateBackup(fw);

    QFile f(saveFile);
    while (!f.open(QFile::WriteOnly)) {
        QMessageBox box(QMessageBox::Warning,
                        tr("Save Form?"),
                        tr("Could not open file"),
                        QMessageBox::NoButton, fw);

        box.setWindowModality(Qt::WindowModal);
        box.setInformativeText(tr("The file, %1, could not be opened"
                               "\nReason: %2"
                               "\nWould you like to retry or change your file?")
                                .arg(f.fileName()).arg(f.errorString()));
        QPushButton *retryButton = box.addButton(tr("Retry"), QMessageBox::AcceptRole);
        retryButton->setDefault(true);
        QPushButton *switchButton = box.addButton(tr("Select New File"), QMessageBox::AcceptRole);
        QPushButton *cancelButton = box.addButton(QMessageBox::Cancel);
        box.exec();

        if (box.clickedButton() == cancelButton) {
            removeBackup(backupFile);
            return false;
        } else if (box.clickedButton() == switchButton) {
            QString extension = getFileExtension(core());
            const QString fileName = QFileDialog::getSaveFileName(fw, tr("Save form as"),
                                                                  QDir::current().absolutePath(),
                                                                  QLatin1String("*.") + extension);
            if (fileName.isEmpty()) {
                removeBackup(backupFile);
                return false;
            }
            if (f.fileName() != fileName) {
                removeBackup(backupFile);
                fi.setFile(fileName);
                backupFile = QString();
                if (fi.exists())
                    backupFile = createBackup(fileName);
            }
            f.setFileName(fileName);
            fw->setFileName(fileName);
        }
        // loop back around...
    }
    while (f.write(utf8Array, utf8Array.size()) != utf8Array.size()) {
        QMessageBox box(QMessageBox::Warning, tr("Save Form?"),
                        tr("Could not write file"),
                        QMessageBox::NoButton, fw);
        box.setWindowModality(Qt::WindowModal);
        box.setInformativeText(tr("It was not possible to write the entire file, %1, to disk."
                                "\nReason:%2\nWould you like to retry?")
                                .arg(f.fileName()).arg(f.errorString()));
        QPushButton *retryButton = box.addButton(tr("Retry"), QMessageBox::AcceptRole);
        retryButton->setDefault(true);
        QPushButton *noRetry = box.addButton(tr("Retry"), QMessageBox::RejectRole);
        box.exec();
        if (box.clickedButton() == retryButton)
            f.resize(0);
        else if (box.clickedButton() == noRetry)
            return false;
    }
    f.close();
    removeBackup(backupFile);
    addRecentFile(saveFile);
    m_saveDirectory = QFileInfo(f).absolutePath();

    fw->setDirty(false);
    fw->parentWidget()->setWindowModified(false);
    return true;
}

void QDesignerActions::shutdown()
{

    // Follow the idea from the Mac, i.e. send the Application a close event
    // and if it's accepted, quit.
    QCloseEvent ev;
    QApplication::sendEvent(qDesigner, &ev);
    if (ev.isAccepted())
        qDesigner->quit();
}

void QDesignerActions::activeFormWindowChanged(QDesignerFormWindowInterface *formWindow)
{
    const bool enable = formWindow != 0;

    m_saveFormAction->setEnabled(enable);
    m_saveFormAsAction->setEnabled(enable);
    m_saveAllFormsAction->setEnabled(enable);
    m_saveFormAsTemplateAction->setEnabled(enable);
    m_closeFormAction->setEnabled(enable);

    m_editWidgetsAction->setEnabled(enable);
    m_formSettings->setEnabled(enable);

    m_previewFormAction->setEnabled(enable);
    m_styleActions->setEnabled(enable);
}

void QDesignerActions::updateRecentFileActions()
{
    QDesignerSettings settings;
    QStringList files = settings.recentFilesList();
    const int originalSize = files.size();
    int numRecentFiles = qMin(files.size(), int(MaxRecentFiles));
    const QList<QAction *> recentFilesActs = m_recentFilesActions->actions();

    for (int i = 0; i < numRecentFiles; ++i) {
        const QFileInfo fi(files[i]);
        // If the file doesn't exist anymore, just remove it from the list so
        // people don't get confused.
        if (!fi.exists()) {
            files.removeAt(i);
            --i;
            numRecentFiles = qMin(files.size(), int(MaxRecentFiles));
            continue;
        }
        const QString text = fi.fileName();
        recentFilesActs[i]->setText(text);
        recentFilesActs[i]->setIconText(files[i]);
        recentFilesActs[i]->setVisible(true);
    }

    for (int j = numRecentFiles; j < MaxRecentFiles; ++j)
        recentFilesActs[j]->setVisible(false);

    // If there's been a change, right it back
    if (originalSize != files.size())
        settings.setRecentFilesList(files);
}

void QDesignerActions::openRecentForm()
{
    if (const QAction *action = qobject_cast<const QAction *>(sender())) {
        if (!readInForm(action->iconText()))
            updateRecentFileActions(); // File doesn't exist, remove it from settings
    }
}

void QDesignerActions::clearRecentFiles()
{
    QDesignerSettings settings;
    settings.setRecentFilesList(QStringList());
    updateRecentFileActions();
}

QActionGroup *QDesignerActions::recentFilesActions() const
{
    return m_recentFilesActions;
}

void QDesignerActions::addRecentFile(const QString &fileName)
{
    QDesignerSettings settings;
    QStringList files = settings.recentFilesList();
    files.removeAll(fileName);
    files.prepend(fileName);
    while (files.size() > MaxRecentFiles)
        files.removeLast();

    settings.setRecentFilesList(files);
    updateRecentFileActions();
}


QAction *QDesignerActions::minimizeAction() const
{
    return m_minimizeAction;
}

QAction *QDesignerActions::bringAllToFront() const
{
    return m_bringToFrontAction;
}

void QDesignerActions::showDesignerHelp()
{
    showHelp(QLatin1String("designer-manual.html"));
}

void QDesignerActions::showWhatsNew()
{
    showHelp(QLatin1String("qt4-designer.html"));
}

void QDesignerActions::showHelp(const QString &url)
{
    if (!m_assistantClient)
        m_assistantClient
            = new QAssistantClient(QLibraryInfo::location(QLibraryInfo::BinariesPath), this);

    QString filePath = QLibraryInfo::location(QLibraryInfo::DocumentationPath)
                                + QLatin1String("/html/") + url;

    QString cleanFilePath;
    const int index = filePath.lastIndexOf(QLatin1Char('#'));
    if (index != -1)
        cleanFilePath = filePath.left(index);
    else
        cleanFilePath = filePath;

    if (!QFile::exists(cleanFilePath)) {
        filePath = QLibraryInfo::location(QLibraryInfo::DocumentationPath)
                                + QLatin1String("/html/designer-manual.html");
    }

    m_assistantClient->showPage(filePath);
}

void QDesignerActions::aboutDesigner()
{
    VersionDialog mb(core()->topLevel());
    mb.setWindowTitle(tr("About Qt Designer"));
    if (mb.exec()) {
        OublietteView *oubliette = new OublietteView;
        oubliette->setAttribute(Qt::WA_DeleteOnClose);
        oubliette->setMinimumSize(800, 600);
        oubliette->show();
    }
}

QActionGroup *QDesignerActions::uiMode() const
{
    return m_uiMode;
}


QAction *QDesignerActions::editWidgets() const
{
    return m_editWidgetsAction;
}

void QDesignerActions::showWidgetSpecificHelp()
{
    QDesignerFormWindowInterface *fw = core()->formWindowManager()->activeFormWindow();
    if (!fw) {
        showDesignerHelp();
        return;
    }

    QString className;
    const QString currentPropertyName = core()->propertyEditor()->currentPropertyName();
    if (!currentPropertyName.isEmpty()) {
        QDesignerPropertySheetExtension *ps
            = qt_extension<QDesignerPropertySheetExtension *>(core()->extensionManager(),
                                                            core()->propertyEditor()->object());
        if (!ps)
            ps = qt_extension<QDesignerPropertySheetExtension *>(core()->extensionManager(),
                                                            fw->cursor()->selectedWidget(0));
        Q_ASSERT(ps);
        className = ps->propertyGroup(ps->indexOf(currentPropertyName));
    } else {
        QDesignerWidgetDataBaseInterface *db = core()->widgetDataBase();
        QDesignerWidgetDataBaseItemInterface *dbi = db->item(db->indexOfObject(fw->cursor()->selectedWidget(0), true));
        className = dbi->name();
    }

    // ### generalize using the Widget Data Base
    if (className == QLatin1String("Line"))
        className = QLatin1String("QFrame");
    else if (className == QLatin1String("Spacer"))
        className = QLatin1String("QSpacerItem");
    else if (className == QLatin1String("QLayoutWidget"))
        className = QLatin1String("QLayout");

    QString url = className.toLower();

    // special case
    url += QLatin1String(".html");

    if (!currentPropertyName.isEmpty())
        url += QLatin1Char('#') + currentPropertyName;

    showHelp(url);
}

QAction *QDesignerActions::widgetHelpAction() const
{
    return m_widgetHelp;
}

void QDesignerActions::aboutPlugins()
{
    PluginDialog dlg(core(), core()->topLevel());
    dlg.exec();
}

void QDesignerActions::showFormSettings()
{
    QDesignerFormWindowInterface *formWindow = core()->formWindowManager()->activeFormWindow();
    QDesignerFormWindow *window = m_workbench->findFormWindow(formWindow);

    QExtensionManager *mgr = core()->extensionManager();

    QDialog *settingsDialog = 0;

    if (QDesignerLanguageExtension *lang = qt_extension<QDesignerLanguageExtension*>(mgr, core()))
        settingsDialog = lang->createFormWindowSettingsDialog(formWindow, /*parent=*/ 0);

    if (! settingsDialog)
        settingsDialog = new FormWindowSettings(formWindow);

    if (settingsDialog->exec() && window) {
        formWindow->setDirty(true);
        window->updateChanged();
    }

    delete settingsDialog;
}

bool QDesignerActions::eventFilter(QObject *watched, QEvent *event)
{
    QWidget *w = qobject_cast<QWidget *>(watched);
    if (w && w->isWindow() && event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = (QKeyEvent *)event;
        if (keyEvent && (keyEvent->key() == Qt::Key_Escape
#ifdef Q_WS_MAC
            || (keyEvent->modifiers() == Qt::ControlModifier && keyEvent->key() == Qt::Key_Period)
#endif
            )) {
            w->close();
            return true;
        }
    }
    return QObject::eventFilter(watched, event);
}

void QDesignerActions::backupForms()
{
    const int count = m_workbench->formWindowCount();
    if (!count)
        return;

    const QString backupPath = QDir::convertSeparators(QDir::homePath() + QDir::separator()
                       + QLatin1String(".designer") + QDir::separator() + QLatin1String("backup"));
    const QString backupTmpPath = QDir::convertSeparators(backupPath + QDir::separator() + QLatin1String("tmp"));

    QDir backupDir(backupPath);
    QDir backupTmpDir(backupTmpPath);
    if (!backupDir.exists()) backupDir.mkpath(backupPath);
    if (!backupTmpDir.exists()) backupTmpDir.mkpath(backupTmpPath);

    QStringList tmpFiles;
    QMap<QString, QString> backupMap;
    for (int i = 0; i < count; ++i) {
        QDesignerFormWindow *fw = m_workbench->formWindow(i);
        QDesignerFormWindowInterface *fwi = fw->editor();

        QString formBackupName;
        QTextStream(&formBackupName) << backupPath << QDir::separator()
                                     << QLatin1String("backup") << i << QLatin1String(".bak");

        QString fwn = QDir::convertSeparators(fwi->fileName());
        if (fwn.isEmpty())
            fwn = fw->windowTitle();

        backupMap.insert(fwn, formBackupName);

        QFile file(formBackupName.replace(backupPath, backupTmpPath));
        if (file.open(QFile::WriteOnly)){
            const QByteArray utf8Array = fixResourceFileBackupPath(fwi, backupDir).toUtf8();
            if (file.write(utf8Array, utf8Array.size()) != utf8Array.size()) {
                backupMap.remove(fwn);
                qDebug() << "Could not write backup file:" << file.fileName();
            } else
                tmpFiles.append(formBackupName);

            file.close();
        }
    }

    if(!tmpFiles.isEmpty()) {
        const QStringList backupFiles = backupDir.entryList(QDir::Files);
        if(!backupFiles.isEmpty()) {
            QStringListIterator it(backupFiles);
            while (it.hasNext())
                backupDir.remove(it.next());
        }

        QStringListIterator it(tmpFiles);
        while (it.hasNext()) {
            QString name = it.next();
            QFile file(name);
            file.copy(name.replace(backupTmpPath, backupPath));
            file.remove();
        }

        QDesignerSettings().setBackup(backupMap);
    }
}

QString QDesignerActions::fixResourceFileBackupPath(QDesignerFormWindowInterface *fwi, const QDir& backupDir)
{
    const QString content = fwi->contents();
    QDomDocument domDoc(QLatin1String("backup"));
    if(!domDoc.setContent(content))
        return content;

    const QDomNodeList list = domDoc.elementsByTagName(QLatin1String("resources"));
    if (list.isEmpty())
        return content;

    for (int i = 0; i < list.count(); i++) {
        const QDomNode node = list.at(i);
        if (!node.isNull()) {
            const QDomElement element = node.toElement();
            if(!element.isNull() && element.tagName() == QLatin1String("resources")) {
                QDomNode childNode = element.firstChild();
                while (!childNode.isNull()) {
                    QDomElement childElement = childNode.toElement();
                    if(!childElement.isNull() && childElement.tagName() == QLatin1String("include")) {
                        const QString attr = childElement.attribute(QLatin1String("location"));
                        const QString path = fwi->absoluteDir().absoluteFilePath(attr);
                        childElement.setAttribute(QLatin1String("location"), backupDir.relativeFilePath(path));
                    }
                    childNode = childNode.nextSibling();
                }
            }
        }
    }

    return domDoc.toString();
}

QRect QDesignerActions::fixDialogRect(const QRect &rect) const
{
    QRect frameGeometry;
    const QRect availableGeometry = QApplication::desktop()->availableGeometry(core()->topLevel());

    if (workbench()->mode() == QDesignerWorkbench::DockedMode) {
        frameGeometry = core()->topLevel()->frameGeometry();
    } else
        frameGeometry = availableGeometry;

    QRect dlgRect = rect;
    dlgRect.moveCenter(frameGeometry.center());

    // make sure that parts of the dialog are not outside of screen
    dlgRect.moveBottom(qMin(dlgRect.bottom(), availableGeometry.bottom()));
    dlgRect.moveRight(qMin(dlgRect.right(), availableGeometry.right()));
    dlgRect.moveLeft(qMax(dlgRect.left(), availableGeometry.left()));
    dlgRect.moveTop(qMax(dlgRect.top(), availableGeometry.top()));

    return dlgRect;
}

void QDesignerActions::showStatusBarMessage(const QString &message) const
{
    if (workbench()->mode() == QDesignerWorkbench::DockedMode) {
        QStatusBar *bar = qDesigner->mainWindow()->statusBar();
        if (bar && !bar->isHidden())
            bar->showMessage(message, 3000);
    }
}
