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
#include "qdesigner_mainwindow.h"
#include "qdesigner_workbench.h"
#include "qdesigner_formwindow.h"
#include "preferencedialog.h"
#include "newform.h"

// sdk
#include <abstractformeditor.h>
#include <abstractformwindow.h>
#include <abstractformwindowmanager.h>
#include <qtundo.h>

#include <QtGui/QAction>
#include <QtGui/QActionGroup>
#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>

QDesignerActions::QDesignerActions(QDesignerMainWindow *mainWindow)
    : QObject(mainWindow),
      m_mainWindow(mainWindow)
{
    Q_ASSERT(m_mainWindow != 0);

    m_workbench = m_mainWindow->workbench();
    Q_ASSERT(m_workbench != 0);

    m_core = m_mainWindow->core();
    Q_ASSERT(m_core != 0);

    AbstractFormWindowManager *formWindowManager = m_core->formWindowManager();
    Q_ASSERT(formWindowManager != 0);

    m_fileActions = new QActionGroup(this);
    m_fileActions->setExclusive(false);

    m_editActions = new QActionGroup(this);
    m_editActions->setExclusive(false);

    m_editModeActions = new QActionGroup(this);
    m_editModeActions->setExclusive(true);

    m_formActions = new QActionGroup(this);
    m_formActions->setExclusive(false);

    m_windowActions = new QActionGroup(this);
    m_windowActions->setExclusive(false);

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

    m_fileActions->addSeparator();

    m_saveFormAction = new QAction(tr("&Save Form"), this);
    m_saveFormAction->setShortcut(tr("CTRL+S"));
    connect(m_saveFormAction, SIGNAL(triggered()), this, SLOT(saveForm()));
    m_fileActions->addAction(m_saveFormAction);

    m_saveFormAsAction = new QAction(tr("Save Form &As..."), this);
    connect(m_saveFormAsAction, SIGNAL(triggered()), this, SLOT(saveFormAs()));
    m_fileActions->addAction(m_saveFormAsAction);

    m_fileActions->addSeparator();

    m_closeFormAction = new QAction(tr("&Close Form"), this);
    m_closeFormAction->setShortcut(tr("CTRL+W"));
    connect(m_closeFormAction, SIGNAL(triggered()), this, SLOT(notImplementedYet()));
    m_fileActions->addAction(m_closeFormAction);

    m_fileActions->addSeparator();

    m_quitAction = new QAction(tr("&Quit"), this);
    connect(m_quitAction, SIGNAL(triggered()),
            qDesigner, SLOT(quit()));
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

    m_editActions->addSeparator();

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

    m_editActions->addSeparator();

    m_sendToBackAction = formWindowManager->actionLower();
    m_editActions->addAction(m_sendToBackAction);

    m_bringToFrontAction = formWindowManager->actionRaise();
    m_editActions->addAction(m_bringToFrontAction);

    m_preferencesSeparator = new QAction(this);
    m_preferencesSeparator->setSeparator(true);
    m_editActions->addAction(m_preferencesSeparator);

    m_preferences = new QAction(tr("Preferences"), this);
    connect(m_preferences, SIGNAL(triggered()), this, SLOT(editPreferences()));
    m_editActions->addAction(m_preferences);

//
// edit mode actions
//
    m_editWidgets = new QAction(tr("Edit Widgets"), this);
    m_editWidgets->setShortcut(tr("F2"));
    m_editWidgets->setCheckable(true);
    m_editModeActions->addAction(m_editWidgets);

    m_editConnections = new QAction(tr("Edit Connections"), this);
    m_editConnections->setShortcut(tr("F3"));
    m_editConnections->setCheckable(true);
    m_editModeActions->addAction(m_editConnections);

    m_editTabOrders= new QAction(tr("Edit Tab Orders"), this);
    m_editTabOrders->setShortcut(tr("F4"));
    m_editTabOrders->setCheckable(true);
    m_editModeActions->addAction(m_editTabOrders);

    m_editBuddies = new QAction(tr("Edit Buddies"), this);
    m_editBuddies->setShortcut(tr("F5"));
    m_editBuddies->setCheckable(true);
    m_editModeActions->addAction(m_editBuddies);

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

    m_formActions->addSeparator();

    m_previewFormAction = new QAction(tr("&Preview"), this);
    m_previewFormAction->setShortcut(tr("CTRL+R"));
    connect(m_previewFormAction, SIGNAL(triggered()), this, SLOT(notImplementedYet()));
    m_formActions->addAction(m_previewFormAction);

//
// window actions
//
    m_showWorkbenchAction = new QAction(tr("Show &Workbench"), this);
    m_showWorkbenchAction->setCheckable(true);
    connect(m_showWorkbenchAction, SIGNAL(checked(bool)), this, SLOT(setWorkbenchVisible(bool)));
    m_windowActions->addAction(m_showWorkbenchAction);

//
// connections
//
    connect(editModeActions(), SIGNAL(triggered(QAction*)),
            this, SLOT(updateEditMode(QAction*)));
}

QDesignerActions::~QDesignerActions()
{
}

QDesignerMainWindow *QDesignerActions::mainWindow() const
{ return m_mainWindow; }

QDesignerWorkbench *QDesignerActions::workbench() const
{ return m_workbench; }

AbstractFormEditor *QDesignerActions::core() const
{ return m_core; }

QActionGroup *QDesignerActions::fileActions() const
{ return m_fileActions; }

QActionGroup *QDesignerActions::editActions() const
{ return m_editActions; }

QActionGroup *QDesignerActions::editModeActions() const
{ return m_editModeActions; }

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

QAction *QDesignerActions::preferencesSeparator() const
{ return m_preferencesSeparator; }

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

QAction *QDesignerActions::editWidgets() const
{ return m_editWidgets; }

QAction *QDesignerActions::editConnections() const
{ return m_editConnections; }

QAction *QDesignerActions::editBuddies() const
{ return m_editBuddies; }

QAction *QDesignerActions::editTabOrders() const
{ return m_editTabOrders; }

QAction *QDesignerActions::showWorkbenchAction() const
{ return m_showWorkbenchAction; }

void QDesignerActions::updateEditMode(QAction *action)
{
    Q_ASSERT(m_editModeActions->actions().contains(action) == true);

    AbstractFormWindowManager *formWindowManager = core()->formWindowManager();
    AbstractFormWindow::EditMode mode = AbstractFormWindow::WidgetEditMode;

    if (action == m_editWidgets)
        mode = AbstractFormWindow::WidgetEditMode;
    else if (action == m_editConnections)
        mode = AbstractFormWindow::ConnectionEditMode;
    else if (action == m_editTabOrders)
        mode = AbstractFormWindow::TabOrderEditMode;
    else if (action == m_editBuddies)
        mode = AbstractFormWindow::BuddyEditMode;
    else
        Q_ASSERT(0);

    for (int i=0; i<formWindowManager->formWindowCount(); ++i) {
        AbstractFormWindow *formWindow = formWindowManager->formWindow(i);
        formWindow->setEditMode(mode);
    }
}

void QDesignerActions::setWorkbenchVisible(bool visible)
{
    if (visible) {
        workbench()->switchToWorkspaceMode();
    } else {
        workbench()->switchToTopLevelMode();
    }
}

void QDesignerActions::createForm()
{
    NewForm dlg(workbench(), core()->topLevel());
    dlg.exec();
}

void QDesignerActions::openForm()
{
    QString fileName = QFileDialog::getOpenFileName(core()->topLevel(),
            tr("Open Form"), QString(),
            tr("Designer UI files (*.ui)"));

    if (fileName.isEmpty() == false) {
        readInForm(fileName);
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
        editor->setContents(&f);
        editor->setFileName(fileName);
        formWindowManager->setActiveFormWindow(editor);
    }
    formWindow->show();

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
                        QMessageBox::Cancel | QMessageBox::Escape, fw, Qt::WMacSheet);
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
                        fw, Qt::WMacSheet);
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
    fw->setDirty(false);
    return true;
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
    bool breakLoop = false;
    QFileInfo fi(saveFile);
    while (fi.exists() && !breakLoop) {
        QFileInfo fi2(fi.absoluteFilePath());
        QMessageBox box(tr("File already exists"),
                        tr("<b>%1 already exits.<br>Do you want to replace it?</b><br>"
                                "A file with the same name already exists in %2."
                                " Replacing it will overwrite its current contents.")
                                .arg(fi.baseName()).arg(fi2.baseName()),
                        QMessageBox::Information,
                        QMessageBox::Yes, QMessageBox::No| QMessageBox::Default,
                        QMessageBox::Cancel | QMessageBox::Escape, fw, Qt::WMacSheet);
        box.setButtonText(QMessageBox::Yes, tr("Overwrite file"));
        box.setButtonText(QMessageBox::No, tr("Choose another name"));
        switch (box.exec()) {
            default: break;
            case QMessageBox::Cancel:
                return false;
            case QMessageBox::Yes:
                breakLoop = true;
                break;
            case QMessageBox::No:
                saveFile = QFileDialog::getSaveFileName(fw, tr("Save form as"),
                        fileName,
                        tr("Designer UI files (*.ui)"));
                fi.setFile(saveFile);
                break;
        }
    }
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

void QDesignerActions::saveFormAs()
{
    if (AbstractFormWindow *fw = core()->formWindowManager()->activeFormWindow())
        saveFormAs(fw);
}

void QDesignerActions::notImplementedYet()
{
    QMessageBox::information(core()->topLevel(), tr("Designer"), tr("Feature not implemented yet!"));
}

void QDesignerActions::editPreferences()
{
    if (!m_preferenceDialog) {
        m_preferenceDialog = new PreferenceDialog(core(), core()->topLevel());
        m_preferenceDialog->setAttribute(Qt::WA_DeleteOnClose, true);
    }
    m_preferenceDialog->show();
    m_preferenceDialog->raise();
}


