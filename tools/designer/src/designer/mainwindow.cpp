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

#include "mainwindow.h"

// views
#include "newformdialog.h"
#include "propertyeditorview.h"
#include "objectinspectorview.h"

// components
#include <qdesigner_formbuilder.h>
#include <formeditor.h>
#include <objectinspector.h>
#include <widgetbox.h>
#include <specialeditor.h>
#include <specialeditorsupport.h>

// sdk
#include <abstractformwindow.h>
#include <abstractformwindowcursor.h>
#include <abstractformwindowmanager.h>
#include <abstractmetadatabase.h>
#include <abstractwidgetdatabase.h>
#include <abstractwidgetfactory.h>
#include <abstractpropertyeditor.h>
#include <propertysheet.h>
#include <qextensionmanager.h>

#include <QtCore/QBuffer>
#include <QtCore/QEvent>
#include <QtCore/QSettings>
#include <QtCore/QTimer>
#include <QtGui/QActionGroup>
#include <QtGui/QApplication>
#include <QtGui/QCloseEvent>
#include <QtGui/QDesktopWidget>
#include <QtGui/QFileDialog>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QMessageBox>
#include <QtGui/QPixmap>
#include <QtGui/QStatusBar>
#include <QtGui/QToolBar>
#include <QtGui/QVBoxWidget>
#include <QtGui/QVariant>

#define IDE_NO_DEBUGVIEWS

MainWindow::MainWindow()
    : QMainWindow(0),
      m_closeForm(true), m_settingsSaved(false),
      m_newFormDialog(0), m_preferenceDialog(0), m_actionWindowList(0), m_actionWindowSeparator(0)
{
    invisibleParent = new QWidget(0);
    setWindowTitle(tr("Qt Designer"));
    setupFormEditor();
    setupWidgetBox();
    setupMenuBar();
    setupToolBar();

    connect(core->propertyEditor(), SIGNAL(propertyChanged(const QString&, const QVariant&)),
            this, SLOT(propertyChanged(const QString&, const QVariant&)));

#ifndef IDE_NO_DEBUGVIEWS
    QWidget *dbShell = new QVBoxWidget(this, Qt::WType_Dialog);
    dbShell->setWindowTitle(tr("Widget DB"));
    WidgetDataBaseView *dbView = new WidgetDataBaseView(dbShell);
    dbView->setWidgetDataBase(core->widgetDataBase());
    connect(core->widgetDataBase(), SIGNAL(changed()), dbView, SLOT(refresh()));
    dbShell->show();

    QWidget *mdbShell = new QVBoxWidget(this, Qt::WType_Dialog);
    mdbShell->setWindowTitle(tr("Meta DB"));
    MetaDataBaseView *mdbView = new MetaDataBaseView(mdbShell);
    mdbView->setMetaDataBase(core->metaDataBase());
    connect(core->metaDataBase(), SIGNAL(changed()), mdbView, SLOT(refresh()));
    mdbShell->show();
#endif
    readSettings();

    statusBar()->show();
}

MainWindow::~MainWindow()
{
    // the widgetbox has to go before formwindowmanager, 'cause of the scratchpad.
    delete core->widgetBox();
    core->setWidgetBox(0);

    delete invisibleParent;
}

void MainWindow::setupFormEditor()
{
    core = new FormEditor(this);
    m_formWindowManager = core->formWindowManager();
    core->setTopLevel(this);
    connect(m_formWindowManager, SIGNAL(activeFormWindowChanged(AbstractFormWindow*)),
        this, SLOT(windowChanged()));
    connect(m_formWindowManager, SIGNAL(formWindowClosing(AbstractFormWindow *, bool *)),
            this, SLOT(handleClose(AbstractFormWindow *, bool *)));

    new PropertyEditorView(core, invisibleParent);
    new ObjectInspectorView(core, invisibleParent);
    // new SpecialEditorSupport(core); ### disabled for now
}

void MainWindow::handleClose(AbstractFormWindow *fw, bool *accept)
{
    m_closeForm = true;
    if (fw->isDirty()) {
        fw->raise();
        QMessageBox box(tr("Save Form?"),
                tr("Do you want to save the changes you made to \"%1\" before closing?")
                    .arg(fw->fileName().isEmpty() ? fw->windowTitle() : fw->fileName()),
                QMessageBox::Information,
                QMessageBox::Yes | QMessageBox::Default, QMessageBox::No,
                QMessageBox::Cancel | QMessageBox::Escape, fw, Qt::WMacSheet);
        box.setButtonText(QMessageBox::Yes, fw->fileName().isEmpty() ? tr("Save...") : tr("Save"));
        box.setButtonText(QMessageBox::No, tr("Don't Save"));
        switch (box.exec()) {
        case QMessageBox::Yes:
            *accept = saveForm(fw);
            break;
        case QMessageBox::No:
            fw->setDirty(false); // Not really necessary, but stops problems if we get close again.
            *accept = true;
            break;
        case QMessageBox::Cancel:
            *accept = false;
            break;
        }
        m_closeForm = *accept;
    }

    //if (m_formWindowManager->formWindowCount() == 1 && m_closeForm)
    //    QTimer::singleShot(200, this, SLOT(newForm()));  // Use timer in case we are quitting.
}

void MainWindow::setupWidgetBox()
{
    WidgetBox *wb = new WidgetBox(core, WidgetBox::TreeMode, this);
    core->setWidgetBox(wb);
    setCentralWidget(wb);
}

void MainWindow::enableFormActions(bool enable)
{
    m_actionSave->setEnabled(enable);
    m_actionSaveAs->setEnabled(enable);
    m_actionClose->setEnabled(enable);
    m_actionPreviewForm->setEnabled(enable);
    m_actionMaximize->setEnabled(enable);
    m_actionMinimize->setEnabled(enable);
    m_showGrid->setEnabled(enable);
    m_widgetEditMode->setEnabled(enable);
    m_connectionEditMode->setEnabled(enable);
    m_tabOrderEditMode->setEnabled(enable);
}

void MainWindow::windowChanged()
{
    if (AbstractFormWindow *fw = m_formWindowManager->activeFormWindow()) {
        fw->setActiveWindow();
        enableFormActions(true);
        m_showGrid->setChecked(fw->hasFeature(AbstractFormWindow::GridFeature));
        m_readOnly->setChecked(!fw->hasFeature(AbstractFormWindow::EditFeature));
        m_widgetEditMode->setChecked(fw->editMode() == AbstractFormWindow::WidgetEditMode);
        m_connectionEditMode->setChecked(fw->editMode() == AbstractFormWindow::ConnectionEditMode);
        m_tabOrderEditMode->setChecked(fw->editMode() == AbstractFormWindow::TabOrderEditMode);
    } else {
        //### re-enable when the bug in QAbstractItemView::reset() is fixed
        if (AbstractObjectInspector *objectInspector = core->objectInspector())
            objectInspector->setFormWindow(0);

        if (AbstractPropertyEditor *propertyEditor = core->propertyEditor())
            propertyEditor->setObject(0);

        enableFormActions(false);
    }
}

void MainWindow::showGrid(bool b)
{
    if (AbstractFormWindow *fw = m_formWindowManager->activeFormWindow()) {
        AbstractFormWindow::Feature f = fw->features();
        if (!b)
            f &= ~AbstractFormWindow::GridFeature;
        else
            f |= AbstractFormWindow::GridFeature;
        fw->setFeatures(f);
    }
}

void MainWindow::readOnly(bool b)
{
    if (AbstractFormWindow *fw = m_formWindowManager->activeFormWindow()) {
        AbstractFormWindow::Feature f = fw->features();
        if (b)
            f &= ~AbstractFormWindow::EditFeature;
        else
            f |= AbstractFormWindow::EditFeature;
        fw->setFeatures(f);
    }
}

void MainWindow::selectionChanged()
{
    if (AbstractFormWindow *fw = m_formWindowManager->activeFormWindow()) {
        QWidget *sel = fw->cursor()->selectedWidget(0);

        if (AbstractObjectInspector *objectInspector = core->objectInspector())
            objectInspector->setFormWindow(fw);

        if (AbstractPropertyEditor *propertyEditor = core->propertyEditor()) {
            propertyEditor->setObject(sel);
        }

        enableFormActions(true);
    } else {
        if (AbstractObjectInspector *objectInspector = core->objectInspector())
            objectInspector->setFormWindow(0);

        if (AbstractPropertyEditor *propertyEditor = core->propertyEditor())
            propertyEditor->setObject(0);

        enableFormActions(false);
    }
}

void MainWindow::propertyChanged(const QString &name, const QVariant &value)
{
    if (AbstractFormWindow *fw = m_formWindowManager->activeFormWindow()) {

        fw->cursor()->setProperty(name, value);

        if (name == QLatin1String("windowTitle")) {
            QString filename = fw->fileName().isEmpty()
                ? QString::fromLatin1("Untitled")
                : fw->fileName();

            fw->setWindowTitle(QString::fromLatin1("%1 - (%2)")
                    .arg(value.toString())
                    .arg(filename));

        } else if (name == QLatin1String("geometry")) {
            fw->setGeometry(value.toRect());
        }

        if (core->objectInspector())
            core->objectInspector()->setFormWindow(fw);
    }
}


void MainWindow::setupMenuBar()
{
#ifndef Q_WS_MAC
    QMenuBar *mb = menuBar();
#else
    QMenuBar *mb = new QMenuBar(0);
#endif
    QMenu *menu = mb->addMenu(tr("&File"));
    QAction *act = menu->addAction(tr("&New Form..."), this,
                                        SLOT(newForm()), Qt::CTRL + Qt::Key_N);
    act->setShortcutContext(Qt::ShortcutOnApplication);
    act = menu->addAction(tr("&Open Form..."), this, SLOT(openForm()),
                                        Qt::CTRL + Qt::Key_O);
    act->setShortcutContext(Qt::ShortcutOnApplication);
    QMenu *recentFilesMenu = menu->addMenu(tr("&Recent Forms"));
    for (int i = 0; i < MaxRecentFiles; ++i) {
        recentFilesActs[i] = new QAction(this);
        recentFilesActs[i]->setVisible(false);
        connect(recentFilesActs[i], SIGNAL(triggered()), this, SLOT(openRecentForm()));
        recentFilesMenu->addAction(recentFilesActs[i]);
    }
    updateRecentFileActions();
    recentFilesMenu->addSeparator();
    act = recentFilesMenu->addAction(tr("Clear &Menu"), this, SLOT(clearRecentFiles()));
    act->setShortcutContext(Qt::ShortcutOnApplication);
    menu->addSeparator();
    m_actionSave = menu->addAction(tr("&Save Form"), this, SLOT(saveForm()), Qt::CTRL + Qt::Key_S);
    m_actionSave->setShortcutContext(Qt::ShortcutOnApplication);
    m_actionSaveAs = menu->addAction(tr("Save Form &As..."), this, SLOT(saveFormAs()),
                                     Qt::CTRL | Qt::SHIFT + Qt::Key_S);
    m_actionSaveAs->setShortcutContext(Qt::ShortcutOnApplication);
    menu->addSeparator();
    m_actionClose = menu->addAction(tr("&Close Form"), this, SLOT(closeForm()),
                                    Qt::CTRL + Qt::Key_W);
    m_actionClose->setShortcutContext(Qt::ShortcutOnApplication);
#ifndef Q_WS_MAC
    menu->addSeparator();
    menu->addAction(tr("&Quit"), this, SLOT(close()), Qt::CTRL + Qt::Key_Q);
#endif

    menu = mb->addMenu(tr("&Edit"));
    act = m_formWindowManager->actionUndo();
    act->setShortcut(Qt::CTRL + Qt::Key_Z);
    act->setShortcutContext(Qt::ShortcutOnApplication);
    menu->addAction(act);
    act = m_formWindowManager->actionRedo();
    act->setShortcut(Qt::CTRL | Qt::SHIFT + Qt::Key_Z);
    act->setShortcutContext(Qt::ShortcutOnApplication);
    menu->addAction(act);
    menu->addSeparator();
    act = m_formWindowManager->actionCut();
    act->setShortcut(Qt::CTRL + Qt::Key_X);
    act->setShortcutContext(Qt::ShortcutOnApplication);
    menu->addAction(act);
    act = m_formWindowManager->actionCopy();
    act->setShortcut(Qt::CTRL + Qt::Key_C);
    act->setShortcutContext(Qt::ShortcutOnApplication);
    menu->addAction(act);
    act = m_formWindowManager->actionPaste();
    act->setShortcut(Qt::CTRL + Qt::Key_V);
    act->setShortcutContext(Qt::ShortcutOnApplication);
    menu->addAction(act);
    act = m_formWindowManager->actionDelete();
    act->setShortcut(Qt::Key_Delete);
    act->setShortcutContext(Qt::ShortcutOnApplication);
    menu->addAction(act);
    menu->addAction(m_formWindowManager->actionLower());
    menu->addAction(m_formWindowManager->actionRaise());
    act = m_formWindowManager->actionSelectAll();
    act->setShortcut(Qt::CTRL + Qt::Key_A);
    act->setShortcutContext(Qt::ShortcutOnApplication);
    menu->addAction(act);
    act = new QAction(tr("Preferences..."), this);
    act->setEnabled(true);
    connect(act, SIGNAL(triggered()), this, SLOT(showPreferenceDialog()));
#ifndef Q_WS_MAC
    menu->addSeparator();
#endif
    menu->addAction(act);


    menu = mb->addMenu(tr("F&orm"));
    m_formWindowManager->actionHorizontalLayout()
        ->setShortcutContext(Qt::ShortcutOnApplication);
    m_formWindowManager->actionVerticalLayout()
        ->setShortcutContext(Qt::ShortcutOnApplication);
    m_formWindowManager->actionSplitHorizontal()
        ->setShortcutContext(Qt::ShortcutOnApplication);
    m_formWindowManager->actionSplitVertical()
        ->setShortcutContext(Qt::ShortcutOnApplication);
    m_formWindowManager->actionGridLayout()
        ->setShortcutContext(Qt::ShortcutOnApplication);
    m_formWindowManager->actionBreakLayout()
        ->setShortcutContext(Qt::ShortcutOnApplication);
    m_formWindowManager->actionAdjustSize()
        ->setShortcutContext(Qt::ShortcutOnApplication);
    menu->addAction(m_formWindowManager->actionHorizontalLayout());
    menu->addAction(m_formWindowManager->actionVerticalLayout());
    menu->addAction(m_formWindowManager->actionSplitHorizontal());
    menu->addAction(m_formWindowManager->actionSplitVertical());
    menu->addAction(m_formWindowManager->actionGridLayout());
    menu->addAction(m_formWindowManager->actionBreakLayout());
    menu->addAction(m_formWindowManager->actionAdjustSize());
    menu->addSeparator();
    m_actionPreviewForm = menu->addAction(tr("&Preview"), this, SLOT(previewForm()),
                                          Qt::CTRL + Qt::Key_R);
    m_actionPreviewForm->setShortcutContext(Qt::ShortcutOnApplication);
    menu->addSeparator();
    m_showGrid = menu->addAction(tr("Show Grid"));
    m_showGrid->setCheckable(true);
    m_showGrid->setShortcutContext(Qt::ShortcutOnApplication);
    connect(m_showGrid, SIGNAL(checked(bool)), this, SLOT(showGrid(bool)));

    menu->addSeparator();
    m_widgetEditMode = menu->addAction(tr("Edit Widgets"));
    m_widgetEditMode->setCheckable(true);
    m_widgetEditMode->setShortcutContext(Qt::ShortcutOnApplication);
    m_connectionEditMode = menu->addAction(tr("Edit Connections"));
    m_connectionEditMode->setCheckable(true);
    m_connectionEditMode->setShortcutContext(Qt::ShortcutOnApplication);
    m_tabOrderEditMode = menu->addAction(tr("Edit Tab order"));
    m_tabOrderEditMode->setCheckable(true);
    m_tabOrderEditMode->setShortcutContext(Qt::ShortcutOnApplication);
    QActionGroup *editModeGrp = new QActionGroup(this);
    editModeGrp->setExclusive(true);
    editModeGrp->addAction(m_widgetEditMode);
    editModeGrp->addAction(m_connectionEditMode);
    editModeGrp->addAction(m_tabOrderEditMode);
    connect(editModeGrp, SIGNAL(triggered(QAction*)), this, SLOT(editMode(QAction*)));

    m_readOnly = menu->addAction(tr("Read-Only"));
    m_readOnly->setCheckable(true);
    m_readOnly->setVisible(false);
    m_readOnly->setShortcutContext(Qt::ShortcutOnApplication);
    connect(m_readOnly, SIGNAL(checked(bool)), this, SLOT(readOnly(bool)));

    menu = mb->addMenu(tr("&Tools"));

    m_actionPE = menu->addAction(tr("&Property Editor"));
    m_actionPE->setShortcut(Qt::CTRL + Qt::Key_I);
    m_actionPE->setShortcutContext(Qt::ShortcutOnApplication);
    m_actionPE->setCheckable(true);

    PropertyEditorView *tmpPE = qt_cast<PropertyEditorView *>(core->propertyEditor()->topLevelWidget());
    Q_ASSERT(tmpPE);
    connect(m_actionPE, SIGNAL(checked(bool)), this, SLOT(showPropertyEditor(bool)));
    connect(tmpPE, SIGNAL(visibilityChanged(bool)), m_actionPE, SLOT(setChecked(bool)));

    m_actionOI = menu->addAction(tr("&Object Inspector"));
    m_actionOI->setCheckable(true);
    m_actionOI->setShortcutContext(Qt::ShortcutOnApplication);
    ObjectInspectorView *tmpOI = qt_cast<ObjectInspectorView *>(core->objectInspector()->topLevelWidget());
    Q_ASSERT(tmpOI);
    connect(m_actionOI, SIGNAL(checked(bool)), this, SLOT(showObjectInspector(bool)));
    connect(tmpOI, SIGNAL(visibilityChanged(bool)), m_actionOI, SLOT(setChecked(bool)));

    m_menuWindow = mb->addMenu(tr("&Window"));
    m_actionMinimize = m_menuWindow->addAction(tr("&Minimize"), this, SLOT(minimizeForm()),
                                               Qt::CTRL + Qt::Key_M);
#ifdef Q_WS_MAC
    m_actionMaximize = m_menuWindow->addAction(tr("Zoom"), this, SLOT(maximizeForm()));
#else
    m_actionMaximize = m_menuWindow->addAction(tr("M&aximize"), this, SLOT(maximizeForm()));
#endif
    m_actionMinimize->setShortcutContext(Qt::ShortcutOnApplication);

    menu = mb->addMenu(tr("&Help"));
    act = menu->addAction(tr("Designer Help"), this, SLOT(showDesignerHelp()),
#ifdef Q_WS_MAC
                    Qt::CTRL + Qt::Key_Question
#else
                    Qt::Key_F1
#endif
                   );
    act->setShortcutContext(Qt::ShortcutOnApplication);
    menu->addSeparator();
    act = menu->addAction(tr("What's New in Designer?"), this, SLOT(showTheNewStuff()));
    act->setShortcutContext(Qt::ShortcutOnApplication);
#ifndef Q_WS_MAC // No need to show this on the mac since it is merged away.
    menu->addSeparator();
#endif
    act = menu->addAction(tr("About Designer"), this, SLOT(aboutDesigner()));
    act->setShortcutContext(Qt::ShortcutOnApplication);
    act = menu->addAction(tr("About Qt"), qApp, SLOT(aboutQt()));
    act->setShortcutContext(Qt::ShortcutOnApplication);

    enableFormActions(false);
}

void MainWindow::setupToolBar()
{
    QToolBar *formToolbar = new QToolBar(this);
    addToolBar(formToolbar);
    formToolbar->setMovable(false);
    formToolbar->addAction(m_formWindowManager->actionHorizontalLayout());
    formToolbar->addAction(m_formWindowManager->actionVerticalLayout());
    formToolbar->addAction(m_formWindowManager->actionSplitHorizontal());
    formToolbar->addAction(m_formWindowManager->actionSplitVertical());
    formToolbar->addAction(m_formWindowManager->actionGridLayout());
    formToolbar->addAction(m_formWindowManager->actionBreakLayout());
    formToolbar->addAction(m_formWindowManager->actionAdjustSize());
}

void MainWindow::editMode(QAction *action)
{
    if (AbstractFormWindow *fw = m_formWindowManager->activeFormWindow()) {
        if (action == m_widgetEditMode)
            fw->setEditMode(AbstractFormWindow::WidgetEditMode);
        else if (action == m_connectionEditMode)
            fw->setEditMode(AbstractFormWindow::ConnectionEditMode);
        else if (action == m_tabOrderEditMode)
            fw->setEditMode(AbstractFormWindow::TabOrderEditMode);
        else
            Q_ASSERT(0);
    }
}

void MainWindow::newForm()
{
    if (!m_newFormDialog) {
        m_newFormDialog = new NewFormDialog(this);
        connect(m_newFormDialog, SIGNAL(needOpen()), this, SLOT(openForm()));
        connect(m_newFormDialog, SIGNAL(itemPicked(const QString &)),
                this, SLOT(newForm(const QString &)));
    }
    m_newFormDialog->show();
    m_newFormDialog->raise();
    m_newFormDialog->setFocus();
    m_newFormDialog->setActiveWindow();
}

void MainWindow::newForm(const QString &widgetClass)
{
    int maxUntitled = 0;
    int totalWindows = m_formWindowManager->formWindowCount();
    // This will cause some problems with i18n, but for now I need the string to be "static"
    QRegExp rx("Untitled( (\\d+))*");
    for (int i = 0; i < totalWindows; ++i) {
        if (rx.exactMatch(m_formWindowManager->formWindow(i)->windowTitle())) {
            if (maxUntitled == 0)
                ++maxUntitled;
            if (rx.numCaptures() > 1)
                maxUntitled = qMax(rx.cap(2).toInt(), maxUntitled);
        }
    }

    QString newTitle = "Untitled";
    if (maxUntitled)
        newTitle += " " + QString::number(maxUntitled + 1);
    AbstractWidgetFactory *f = core->widgetFactory();
    AbstractFormWindow *fw = m_formWindowManager->createFormWindow(invisibleParent, Qt::WType_TopLevel);
    setupFormWindow(fw);

    fw->setAttribute(Qt::WA_DeleteOnClose);
    fw->setWindowTitle(newTitle);
    QWidget *w = f->createWidget(widgetClass, fw);
    w->setObjectName("Form");
    fw->setMainContainer(w);
    fw->show();
    m_formWindowManager->setActiveFormWindow(fw);
    updateWindowMenu();
    connect(m_formWindowManager, SIGNAL(formWindowRemoved(AbstractFormWindow*)),
                this, SLOT(updateWindowMenu()));
}

void MainWindow::saveForm()
{
    if (AbstractFormWindow *fw = m_formWindowManager->activeFormWindow()) {
        if (saveForm(fw)) {
            fw->setDirty(false);
        }
    }
}

bool MainWindow::saveForm(AbstractFormWindow *fw)
{
    if (fw->fileName().isEmpty())
        return saveFormAs(fw);
    else
        return writeOutForm(fw, fw->fileName());
}

void MainWindow::saveFormAs()
{
    if (AbstractFormWindow *fw = m_formWindowManager->activeFormWindow())
        saveFormAs(fw);
}

bool MainWindow::saveFormAs(AbstractFormWindow *fw)
{
    QString fileName = fw->fileName().isEmpty() ? QDir::home().absolutePath()
                                                  + QLatin1String("/untitled.ui") : fw->fileName();
    QString saveFile = QFileDialog::getSaveFileName(fw, tr("Save form as"),
                                                    fileName,
                                                    tr("Designer UI files (*.ui)"));
    if (saveFile.isEmpty())
        return false;
    fw->setFileName(saveFile);
    return writeOutForm(fw, saveFile);
}

void MainWindow::openForm()
{
    QString fileName = QFileDialog::getOpenFileName(0, tr("Open Form"), QString(),
                                                    tr("Designer UI files (*.ui)"));
    if (!fileName.isEmpty())
        readInForm(fileName);
}

bool MainWindow::readInForm(const QString &fileName)
{
    // First make sure that we don't have this one open already.
    int totalWindows = m_formWindowManager->formWindowCount();
    for (int i = 0; i < totalWindows; ++i) {
        AbstractFormWindow *w = m_formWindowManager->formWindow(i);
        if (w->fileName() == fileName) {
            w->raise();
            m_formWindowManager->setActiveFormWindow(w);
            addRecentFile(fileName);
            return true;
        }
    }

    // Otherwise load it.
    QFile f(fileName);
    if (!f.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, tr("Read Error"), tr("Couldn't open file: %1\nReason: %2")
                            .arg(f.fileName()).arg(f.errorString()));
        return false;
    }
    AbstractFormWindow *fw = m_formWindowManager->createFormWindow(this, Qt::WType_TopLevel);
    setupFormWindow(fw);

    fw->setAttribute(Qt::WA_DeleteOnClose);
    fw->setContents(&f);
    fw->setFileName(fileName);
    fw->setWindowTitle(QString("%1 - (%2)").arg(fw->mainContainer()->windowTitle()).arg(fileName));
    if (m_newFormDialog)
        m_newFormDialog->close();
    fw->show();
    m_formWindowManager->setActiveFormWindow(fw);
    addRecentFile(fileName);
    return true;
}

bool MainWindow::writeOutForm(AbstractFormWindow *fw, const QString &saveFile)
{
    Q_ASSERT(fw && !saveFile.isEmpty());
    QFile f(saveFile);
    while (!f.open(QIODevice::WriteOnly)) {
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
                                                    QDir::home().absolutePath(), QString("*.ui"));
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
    addRecentFile(saveFile);
    return true;
}

void MainWindow::closeForm()
{
    if (AbstractFormWindow *fw = m_formWindowManager->activeFormWindow()) {
        fw->close();
    }
}

void MainWindow::closeEvent(QCloseEvent *ev)
{
    QList<AbstractFormWindow *> dirtyForms;
    int totalWindows = m_formWindowManager->formWindowCount();
    for (int i = 0; i < totalWindows; ++i) {
        AbstractFormWindow *w = m_formWindowManager->formWindow(i);
        if (w->isDirty())
            dirtyForms << w;
    }

    ev->accept();

    if (dirtyForms.size()) {
        if (dirtyForms.size() == 1) {
            dirtyForms.at(0)->close();
            if (!m_closeForm) {
                ev->ignore();
                return;
            }
        } else {
            QMessageBox box(tr("Save Forms?"),
                    tr("There are %1 forms with unsaved changes."
                        " Do you want to review these changes before quitting?")
                    .arg(dirtyForms.size()),
                    QMessageBox::Warning,
                    QMessageBox::Yes | QMessageBox::Default, QMessageBox::No,
                    QMessageBox::Cancel | QMessageBox::Escape, this);
            box.setButtonText(QMessageBox::Yes, tr("Review Changes"));
            box.setButtonText(QMessageBox::No, tr("Discard Changes"));
            switch (box.exec()) {
                case QMessageBox::Cancel:
                    ev->ignore();
                    return;

                case QMessageBox::Yes: {
                    foreach (AbstractFormWindow *fw, dirtyForms) {
                        fw->show();
                        fw->raise();
                        fw->close();
                        if (!m_closeForm) {
                            ev->ignore();
                            return;
                        }
                    }
                } break;

                case QMessageBox::No: {
                    foreach (AbstractFormWindow *fw, dirtyForms)
                        fw->setDirty(false);
                } break;
            }
        }
    }

    totalWindows = m_formWindowManager->formWindowCount();
    for (int i = 0; i < totalWindows; ++i)
        m_formWindowManager->formWindow(i)->close();

    saveSettings();
    QApplication::instance()->quit();
}

void MainWindow::changeEvent(QEvent *ev)
{
    switch (ev->type()) {
    case QEvent::ActivationChange:
        if (isActiveWindow()) {
            core->propertyEditor()->topLevelWidget()->raise();
            core->objectInspector()->topLevelWidget()->raise();
        }
        break;

    default:
        break;
    }

    QMainWindow::changeEvent(ev);
}

void MainWindow::previewForm()
{
    if (AbstractFormWindow *fw = m_formWindowManager->activeFormWindow()) {
        QByteArray contents = fw->contents().utf8();
        QBuffer stream(&contents);

        QDesignerFormBuilder formBuilder(core);
        QWidget *shell = new QVBoxWidget(this, Qt::WType_TopLevel);
        shell->setAttribute(Qt::WA_DeleteOnClose, true);
        QWidget *w = formBuilder.load(&stream, shell);
        if (QDialog *dlg = qt_cast<QDialog *>(w)) {
            dlg->setAttribute(Qt::WA_DeleteOnClose, true);
            connect(dlg, SIGNAL(destroyed()), shell, SLOT(close()));
        }
        shell->setWindowTitle(tr("Preview - %1").arg(w->windowTitle()));
        shell->resize(w->size());
        shell->show();
    }
}

static void readSizeSettings(const QSettings &settings, const QString &key, QWidget *w,
                             const QRect &defaultRect)
{
    QRect g = settings.value(key + "/geometry", defaultRect).toRect();
    // Resolution Change, lost a screen, someone playing around in your settings?
    // No problem, we'll assume that the default is always OK.
    if (g.intersect(QApplication::desktop()->availableGeometry()).isEmpty())
        g = defaultRect;
    w->resize(g.size());
    w->move(g.topLeft());
    if (settings.value(key + "/visible", true).toBool())
        w->show();
}

void MainWindow::readSettings()
{
    QSettings settings;

    QDesktopWidget *dw = QApplication::desktop();
    QRect availG = dw->availableGeometry(dw->primaryScreen());

    // All this work so that the defaults look okay.

    QString settingsString = QString::fromUtf8("widgetbox");
    QWidget *w = core->widgetBox()->topLevelWidget();
    QSize sz = w->sizeHint();
    sz.setHeight(qMin(sz.height() + 300, availG.height()));
    sz.setWidth(qMin(sz.width() + 50, availG.width()));
    QRect defaultRect(availG.topLeft(), sz);
    readSizeSettings(settings, settingsString, w, defaultRect);

    settingsString = QString::fromUtf8("objectinspector");
    w = core->objectInspector()->topLevelWidget();
    defaultRect.translate(availG.topRight().x() - (w->sizeHint().width() + 20), 0);
    defaultRect.setSize(w->sizeHint());
    readSizeSettings(settings, settingsString, w, defaultRect);

    settingsString = QString::fromUtf8("propertyeditor");
    defaultRect.translate(0,  w->sizeHint().height() + 20);
    w = core->propertyEditor()->topLevelWidget();
    defaultRect.setSize(w->sizeHint());
    readSizeSettings(settings, settingsString, w, defaultRect);
}

void MainWindow::saveSettings()
{
    if (m_settingsSaved)
        return;

    QSettings settings;

    QMap<QString, QWidget*> m;
    m.insert("propertyeditor", core->propertyEditor()->topLevelWidget());
    m.insert("objectinspector", core->objectInspector()->topLevelWidget());
    m.insert("widgetbox", core->widgetBox()->topLevelWidget());

    QMapIterator<QString, QWidget*> it(m);
    while (it.hasNext()) {
        it.next();
        QRect geom(it.value()->x(), it.value()->y(), it.value()->width(), it.value()->height());
        settings.setValue(it.key() + "/screen", QApplication::desktop()->screenNumber(it.value()));
        settings.setValue(it.key() + "/geometry", geom);
        settings.setValue(it.key() + "/visible", it.value()->isVisible());
    }
    m_settingsSaved = true;
}

void MainWindow::setupFormWindow(AbstractFormWindow *fw)
{
    fw->addAction(m_formWindowManager->actionUndo());
    fw->addAction(m_formWindowManager->actionRedo());
    fw->addAction(m_formWindowManager->actionCut());
    fw->addAction(m_formWindowManager->actionCopy());
    fw->addAction(m_formWindowManager->actionPaste());
    fw->addAction(m_formWindowManager->actionDelete());

    fw->addAction(m_actionPreviewForm);
    // ### more

    connect(fw, SIGNAL(selectionChanged()), this, SLOT(selectionChanged()));
    connect(fw, SIGNAL(activated(QWidget *)), this, SLOT(onActivated(QWidget *)));
}

void MainWindow::openRecentForm()
{
    QAction *action = qt_cast<QAction *>(sender());
    if (action)
        if (!readInForm(action->iconText())) {
            // This file doesn't seem to exist, just update the settings so it isn't listed anymore.
            updateRecentFileActions();
        }
}

void MainWindow::clearRecentFiles()
{
    QSettings settings;
    settings.setValue("recentFilesList", QStringList());
    updateRecentFileActions();
}

void MainWindow::updateRecentFileActions()
{
    QSettings settings;
    QStringList files = settings.value("recentFilesList").toStringList();
    int originalSize = files.size();
    int numRecentFiles = qMin(files.size(), (int)MaxRecentFiles);

    for (int i = 0; i < numRecentFiles; ++i) {
        QFileInfo fi(files[i]);
        // If the file doesn't exist anymore, just remove it from the list so
        // people can't get confused.
        if (!fi.exists()) {
            files.removeAt(i);
            --i;
            numRecentFiles = qMin(files.size(), (int)MaxRecentFiles);
            continue;
        }
        QString text = QFileInfo(files[i]).fileName();
        recentFilesActs[i]->setText(text);
        recentFilesActs[i]->setIconText(files[i]);
        recentFilesActs[i]->setVisible(true);
    }
    for (int j = numRecentFiles; j < MaxRecentFiles; ++j)
        recentFilesActs[j]->setVisible(false);

    // Since we could have modified the list write it back.
    if (originalSize != files.size())
        settings.setValue("recentFilesList", files);

}

void MainWindow::addRecentFile(const QString &fileName)
{
    QSettings settings;
    QStringList files = settings.value("recentFilesList").toStringList();
    files.removeAll(fileName);
    files.prepend(fileName);
    while (files.size() > MaxRecentFiles)
        files.removeLast();

    settings.setValue("recentFilesList", files);
    updateRecentFileActions();
}

void MainWindow::showPropertyEditor(bool checked)
{
    if (QWidget *topLevel = core->propertyEditor()->topLevelWidget()) {
        if (checked) {
            topLevel->raise();
            topLevel->show();
            selectionChanged();
        } else {
            topLevel->hide();
        }
    }
}

void MainWindow::showObjectInspector(bool checked)
{
    if (QWidget *topLevel = core->objectInspector()->topLevelWidget()) {
        if (checked) {
            topLevel->raise();
            topLevel->show();
            selectionChanged();
        } else {
            topLevel->hide();
        }
    }
}

void MainWindow::minimizeForm()
{
    if (AbstractFormWindow *fw = m_formWindowManager->activeFormWindow())
        fw->showMinimized();
}

void MainWindow::maximizeForm()
{
    if (AbstractFormWindow *fw = m_formWindowManager->activeFormWindow())
        fw->showMaximized();
}

void MainWindow::updateWindowMenu()
{
    // A bit brain dead, but it works.
    if (!m_actionWindowList) {
        m_actionWindowList = new QActionGroup(this);
        m_actionWindowList->setExclusive(true);
        connect(m_actionWindowList, SIGNAL(triggered(QAction *)),
                this, SLOT(activateFormWindow(QAction *)));
    }

    qDeleteAll(m_showWindowActions.keys());
    m_showWindowActions.clear();

    int totalWindows = m_formWindowManager->formWindowCount();
    if (totalWindows && !m_actionWindowSeparator) {
        m_actionWindowSeparator = m_menuWindow->addSeparator();
    } else if (totalWindows == 0) {
        delete m_actionWindowSeparator;
        m_actionWindowSeparator = 0;
    }

    for (int i = 0; i < totalWindows; ++i) {
        AbstractFormWindow *fw = m_formWindowManager->formWindow(i);
        QAction *a = m_actionWindowList->addAction(fw->windowTitle());
        a->setCheckable(true);
        a->setChecked(m_formWindowManager->activeFormWindow() == m_formWindowManager->formWindow(i));
        m_menuWindow->addAction(a);
        m_showWindowActions.insert(a, fw);
    }
}

void MainWindow::activateFormWindow(QAction *action)
{
    if (AbstractFormWindow *fw = m_showWindowActions.value(action)) {
        fw->setWindowState(fw->windowState() & ~Qt::WindowMinimized);
        fw->raise();
        m_formWindowManager->setActiveFormWindow(fw);
    }
}

void MainWindow::onActivated(QWidget *w)
{
    if (ISpecialEditor *se = qt_extension<ISpecialEditor*>(core->extensionManager(), w)) {
        QWidget *editor = se->createEditor(AbstractFormWindow::findFormWindow(w));
        if (QDialog *dlg = qt_cast<QDialog *>(editor))
            dlg->exec();
        else
            editor->show();
    }
}

void MainWindow::showDesignerHelp()
{
    QMessageBox::warning(0, tr("No Help For You"), tr("Please write a manual for me."));
}

void MainWindow::showTheNewStuff()
{
    QMessageBox::warning(0, tr("What's New in Designer"),
                         tr("I should point to a page that explains"
                            " what is new in this version of Designer."));
}

void MainWindow::aboutDesigner()
{
    QString text = tr("<h3>%1</h3>"
                      "<br/><br/>Version: %2"
                      "<br/>Qt Designer is a graphical user interface designer "
                      "for Qt applications.<br/><br/>"
                      "<b>### Insert license information here ###</b>")
                      .arg(tr("Qt Designer")).arg("4.0");
    QMessageBox mb;
    mb.setWindowTitle(tr("Qt Designer"));
    mb.setText(text);
    mb.setIconPixmap(QPixmap(":/images/designer.png"));
    mb.exec();
}

void MainWindow::showPreferenceDialog()
{
    if (!m_preferenceDialog) {
        m_preferenceDialog = new PreferenceDialog(this);
        m_preferenceDialog->setAttribute(Qt::WA_DeleteOnClose, true);
    }
    m_preferenceDialog->show();
    m_preferenceDialog->raise();
}
