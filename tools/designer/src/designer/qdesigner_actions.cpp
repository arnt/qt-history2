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
#include "qdesigner_actions.h"
#include "qdesigner_workbench.h"
#include "qdesigner_formwindow.h"
#include "qdesigner_settings.h"
#include "qdesigner_settingsdialog.h"
#include "newform.h"
#include "saveformastemplate.h"

// sdk
#include <abstractformeditor.h>
#include <abstractformwindow.h>
#include <abstractformwindowmanager.h>
#include <abstractformeditorplugin.h>
#include <qdesigner_formbuilder.h>
#include <qtundo.h>

#include <QtCore/QBuffer>

#include <QtGui/QAction>
#include <QtGui/QActionGroup>
#include <QtGui/QCloseEvent>
#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>
#include <QtGui/QIcon>

#include <QtCore/QPluginLoader>
#include <QtCore/qdebug.h>

QDesignerActions::QDesignerActions(QDesignerWorkbench *workbench)
    : QObject(workbench),
      m_workbench(workbench)
{
    Q_ASSERT(m_workbench != 0);

    m_core = m_workbench->core();
    Q_ASSERT(m_core != 0);

    AbstractFormWindowManager *formWindowManager = m_core->formWindowManager();
    Q_ASSERT(formWindowManager != 0);

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


    act = new QAction(tr("Clear &Menu"), this);
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
    connect(m_quitAction, SIGNAL(triggered()),
            this, SLOT(shutdown()));
    m_fileActions->addAction(m_quitAction);

//
// edit actions
//
    m_undoAction = QtUndoManager::manager()->createUndoAction(this);
    m_undoAction->setShortcut(tr("CTRL+Z"));
    m_editActions->addAction(m_undoAction);

    m_redoAction = QtUndoManager::manager()->createRedoAction(this);
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
    m_editWidgetsAction->setShortcut(tr("F2"));
    m_editWidgetsAction->setIcon(QIcon(m_core->resourceLocation() + QLatin1String("/widgettool.png")));
    connect(formWindowManager, SIGNAL(activeFormWindowChanged(AbstractFormWindow*)),
                this, SLOT(activeFormWindowChanged(AbstractFormWindow *)));
    connect(m_editWidgetsAction, SIGNAL(triggered()), this, SLOT(editWidgets()));
    m_toolActions->addAction(m_editWidgetsAction);
    m_editWidgetsAction->setChecked(true);
    m_editWidgetsAction->setEnabled(false);
    QList<QObject*> builtinPlugins = QPluginLoader::staticInstances();
    foreach (QObject *plugin, builtinPlugins) {
        if (AbstractFormEditorPlugin *formEditorPlugin = qobject_cast<AbstractFormEditorPlugin*>(plugin)) {
            m_toolActions->addAction(formEditorPlugin->action());
            formEditorPlugin->action()->setCheckable(true);
        }
    }

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
    connect(m_previewFormAction, SIGNAL(triggered()), this, SLOT(previewForm()));
    m_formActions->addAction(m_previewFormAction);

//
// tools actions
//
    m_preferences = new QAction(tr("Options..."), this);
    connect(m_preferences, SIGNAL(triggered()), this, SLOT(editPreferences()));

//
// window actions
//
    m_minimizeAction = new QAction(tr("&Minimize"), this);
    m_minimizeAction->setShortcut(tr("CTRL+M"));
    connect(m_minimizeAction, SIGNAL(triggered()), this, SLOT(minimizeForm()));
    m_windowActions->addAction(m_minimizeAction);

    m_zoomAction = new QAction(
#ifdef Q_WS_MAC
                               tr("Zoom"),
#else
                               tr("Maximize"),
#endif
                               this);
    connect(m_zoomAction, SIGNAL(triggered()), this, SLOT(zoomForm()));
    m_windowActions->addAction(m_zoomAction);

    sep = new QAction(this);
    sep->setSeparator(true);
    m_windowActions->addAction(sep);

    m_bringToFrontAction = new QAction(tr("Bring All to Front"), this);
    connect(m_bringToFrontAction, SIGNAL(triggered()), this, SLOT(bringAllToFront()));
    m_windowActions->addAction(m_bringToFrontAction);

//
// connections
//
    fixActionContext();
    activeFormWindowChanged(core()->formWindowManager()->activeFormWindow());
}

QDesignerActions::~QDesignerActions()
{
}

QActionGroup *QDesignerActions::toolActions() const
{ return m_toolActions; }

QDesignerWorkbench *QDesignerActions::workbench() const
{ return m_workbench; }

AbstractFormEditor *QDesignerActions::core() const
{ return m_core; }

QActionGroup *QDesignerActions::fileActions() const
{ return m_fileActions; }

QActionGroup *QDesignerActions::editActions() const
{ return m_editActions; }

QActionGroup *QDesignerActions::formActions() const
{ return m_formActions; }

QActionGroup *QDesignerActions::windowActions() const
{ return m_windowActions; }

QAction *QDesignerActions::newFormAction() const
{ return m_newFormAction; }

QAction *QDesignerActions::openFormAction() const
{ return m_openFormAction; }

QAction *QDesignerActions::saveFormAction() const
{ return m_saveFormAction; }

QAction *QDesignerActions::saveFormAsAction() const
{ return m_saveFormAsAction; }

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

QAction *QDesignerActions::preferences() const
{ return m_preferences; }

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

void QDesignerActions::editWidgets()
{
    AbstractFormWindowManager *formWindowManager = core()->formWindowManager();
    for (int i=0; i<formWindowManager->formWindowCount(); ++i) {
        AbstractFormWindow *formWindow = formWindowManager->formWindow(i);
        formWindow->editWidgets();
    }
}

void QDesignerActions::createForm()
{
    NewForm dlg(workbench(), 0);
    dlg.exec();
}

void QDesignerActions::openForm()
{
    QString fileName = QFileDialog::getOpenFileName(
#ifdef Q_WS_MAC
            0,
#else
            core()->topLevel(),
#endif
            tr("Open Form"), QString(),
            tr("Designer UI files (*.ui)"));

    if (fileName.isEmpty() == false) {
        readInForm(fileName);
    }
}

bool QDesignerActions::saveFormAs(AbstractFormWindow *fw)
{
    QString fileName = fw->fileName().isEmpty() ? QDir::current().absolutePath()
            + QLatin1String("/untitled.ui") : fw->fileName();
    QString saveFile = QFileDialog::getSaveFileName(fw, tr("Save form as"),
            fileName,
            tr("Designer UI files (*.ui)"));
    if (saveFile.isEmpty())
        return false;
    fw->setFileName(saveFile);
    return writeOutForm(fw, saveFile);
}

void QDesignerActions::saveForm()
{
    if (AbstractFormWindow *fw = core()->formWindowManager()->activeFormWindow())
        saveForm(fw);
}

bool QDesignerActions::saveForm(AbstractFormWindow *fw)
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
    if (AbstractFormWindow *fw = core()->formWindowManager()->activeFormWindow())
        fw->window()->close();
}

void QDesignerActions::saveFormAs()
{
    if (AbstractFormWindow *fw = core()->formWindowManager()->activeFormWindow())
        saveFormAs(fw);
}

void QDesignerActions::saveFormAsTemplate()
{
    if (AbstractFormWindow *fw = core()->formWindowManager()->activeFormWindow()) {
        SaveFormAsTemplate dlg(fw);
        dlg.exec();
    }
}

void QDesignerActions::notImplementedYet()
{
    QMessageBox::information(core()->topLevel(), tr("Designer"), tr("Feature not implemented yet!"));
}

void QDesignerActions::editPreferences()
{
    QDesignerSettingsDialog *dlg = new QDesignerSettingsDialog(workbench(), core()->topLevel());
    dlg->show();
}

void QDesignerActions::handlePreferenceChange()
{
    m_workbench->setUIMode(QDesignerWorkbench::UIMode(QDesignerSettings().uiMode()));
}

void QDesignerActions::previewForm()
{
    if (AbstractFormWindow *fw = core()->formWindowManager()->activeFormWindow()) {
        QDialog *fakeTopLevel = new QDialog(fw);
        QHBoxLayout *layout = new QHBoxLayout(fakeTopLevel);
        layout->setMargin(0);
        fakeTopLevel->hide();

        QDesignerFormBuilder builder(core());

        QByteArray bytes = fw->contents().toUtf8();
        QBuffer buffer(&bytes);

        QWidget *widget = builder.load(&buffer, fakeTopLevel);
        Q_ASSERT(widget);

        QSize size = widget->size();

        widget->setParent(fakeTopLevel, 0);
        layout->addWidget(widget);

        fakeTopLevel->resize(size);
        fakeTopLevel->setWindowTitle(tr("%1 - [Preview]").arg(widget->windowTitle()));
        fakeTopLevel->exec();

        delete fakeTopLevel;
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

    foreach (QAction *a, actions) {
        a->setShortcutContext(Qt::ApplicationShortcut);
    }
}

bool QDesignerActions::readInForm(const QString &fileName)
{
    // First make sure that we don't have this one open already.
    AbstractFormWindowManager *formWindowManager = core()->formWindowManager();
    int totalWindows = formWindowManager->formWindowCount();
    for (int i = 0; i < totalWindows; ++i) {
        AbstractFormWindow *w = formWindowManager->formWindow(i);
        if (w->fileName() == fileName) {
            w->raise();
            formWindowManager->setActiveFormWindow(w);
            addRecentFile(fileName);
            return true;
        }
    }

    // Otherwise load it.
    QFile f(fileName);
    if (!f.open(QFile::ReadOnly)) {
        QMessageBox::warning(core()->topLevel(), tr("Read Error"), tr("Couldn't open file: %1\nReason: %2")
                .arg(f.fileName()).arg(f.errorString()));
        return false;
    }


    QDesignerFormWindow *formWindow = workbench()->createFormWindow();
    if (AbstractFormWindow *editor = formWindow->editor()) {
        editor->setFileName(fileName);
        editor->setContents(&f);
        Q_ASSERT(editor->mainContainer() != 0);
        formWindow->resize(editor->mainContainer()->size());
        formWindowManager->setActiveFormWindow(editor);
    }
    formWindow->show();
    addRecentFile(fileName);
    return true;
}

bool QDesignerActions::writeOutForm(AbstractFormWindow *fw, const QString &saveFile)
{
    Q_ASSERT(fw && !saveFile.isEmpty());
    QFile f(saveFile);
    while (!f.open(QFile::WriteOnly)) {
        QMessageBox box(tr("Save Form?"),
                        tr("Could not open file: %1"
                                "\nReason: %2"
                                "\nWould you like to retry or change your file?")
                                .arg(f.fileName()).arg(f.errorString()),
                        QMessageBox::Warning,
                        QMessageBox::Yes | QMessageBox::Default, QMessageBox::No,
                        QMessageBox::Cancel | QMessageBox::Escape, fw, Qt::Sheet);
        box.setButtonText(QMessageBox::Yes, tr("Retry"));
        box.setButtonText(QMessageBox::No, tr("Select New File"));
        switch(box.exec()) {
            case QMessageBox::Yes:
                break;
                case QMessageBox::No: {
                    QString fileName = QFileDialog::getSaveFileName(fw, tr("Save form as"),
                            QDir::current().absolutePath(), QString("*.ui"));
                    if (fileName.isEmpty())
                        return false;
                    f.setFileName(fileName);
                    fw->setFileName(fileName);
                    break; }
            case QMessageBox::Cancel:
                return false;
        }
    }
    QByteArray utf8Array = fw->contents().toUtf8();
    while (f.write(utf8Array, utf8Array.size()) != utf8Array.size()) {
        QMessageBox box(tr("Save Form?"),
                        tr("Could not write file: %1\nReason:%2\nWould you like to retry?")
                                .arg(f.fileName()).arg(f.errorString()),
                        QMessageBox::Warning,
                        QMessageBox::Yes | QMessageBox::Default, QMessageBox::No, 0,
                        fw, Qt::Sheet);
        box.setButtonText(QMessageBox::Yes, tr("Retry"));
        box.setButtonText(QMessageBox::No, tr("Don't Retry"));
        switch(box.exec()) {
            case QMessageBox::Yes:
                f.resize(0);
                break;
            case QMessageBox::No:
                return false;
        }
    }
    addRecentFile(saveFile);
    fw->setDirty(false);
    fw->window()->setWindowModified(false);
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

void QDesignerActions::activeFormWindowChanged(AbstractFormWindow *formWindow)
{
    bool enable = formWindow != 0;

    m_saveFormAction->setEnabled(enable);
    m_saveFormAsAction->setEnabled(enable);
    m_saveFormAsTemplateAction->setEnabled(enable);
    m_closeFormAction->setEnabled(enable);

    m_editWidgetsAction->setEnabled(enable);

    m_previewFormAction->setEnabled(enable);
}

void QDesignerActions::updateRecentFileActions()
{
    QDesignerSettings settings;
    QStringList files = settings.recentFilesList();
    int originalSize = files.size();
    int numRecentFiles = qMin(files.size(), int(MaxRecentFiles));
    QList<QAction *> recentFilesActs = m_recentFilesActions->actions();

    for (int i = 0; i < numRecentFiles; ++i) {
        QFileInfo fi(files[i]);
        // If the file doesn't exist anymore, just remove it from the list so
        // people don't get confused.
        if (!fi.exists()) {
            files.removeAt(i);
            --i;
            numRecentFiles = qMin(files.size(), int(MaxRecentFiles));
            continue;
        }
        QString text = fi.fileName();
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
    if (QAction *action = qobject_cast<QAction *>(sender())) {
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

void QDesignerActions::minimizeForm()
{
    if (AbstractFormWindow *fw = core()->formWindowManager()->activeFormWindow())
        fw->window()->showMinimized();
}

void QDesignerActions::zoomForm()
{
    if (AbstractFormWindow *fw = core()->formWindowManager()->activeFormWindow())
        fw->window()->showMaximized();
}

void QDesignerActions::bringAllToFront()
{
    int i;
    for (i = 0; i < m_workbench->formWindowCount(); ++i)
        m_workbench->formWindow(i)->raise();

    for (i = 0; i < m_workbench->toolWindowCount(); ++i)
        m_workbench->toolWindow(i)->raise();
}

QAction *QDesignerActions::minimizeAction() const
{
    return m_minimizeAction;
}

QAction *QDesignerActions::zoomAction() const
{
    return m_zoomAction;
}

QAction *QDesignerActions::bringAllToFront() const
{
    return m_bringToFrontAction;
}
