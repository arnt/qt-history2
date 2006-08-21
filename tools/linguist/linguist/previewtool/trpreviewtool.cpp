/****************************************************************************
**
** Copyright (C) 2006-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QFileDialog>
#include <QLabel>
#include <QMessageBox>
#include <QWhatsThis>
#include <QtCore/QIODevice>
#include <QtCore/QPointer>
#include <QtGui/QStandardItemModel>

#include "trpreviewtool.h"
#include "qtwindowlistmenu.h"
#include "messagestreeview.h"

Q_DECLARE_METATYPE(QPointer<FormHolder>)

TrPreviewTool::TrPreviewTool(QWidget *parent, Qt::WindowFlags flags)
    : QMainWindow(parent, flags), currentTr(0)
{
    setAttribute(Qt::WA_DeleteOnClose, true);
    QString x = tr("File");
    ui.setupUi(this);
    workspace = new QWorkspace(this);
    setCentralWidget(workspace);
    QtWindowListMenu* wlm = new QtWindowListMenu(workspace,ui.menuBar);
    wlm->addTo(tr("Windows"), ui.menuBar, 2);
    trCombo = new QComboBox(ui.mainToolBar);
    trCombo->setEditable(false);
    trCombo->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLength);
    trCombo->setMinimumContentsLength(16);
    trCombo->addItem(tr("<No Translation>"),QString());
    ui.mainToolBar->insertWidget(ui.actionOpenForm, trCombo);
    ui.mainToolBar->insertSeparator(ui.actionOpenForm);
    QAction* actionWhatsThis = QWhatsThis::createAction(this);
    ui.mainToolBar->addAction(actionWhatsThis);
    ui.menuHelp->insertAction(ui.actionAbout, actionWhatsThis);

    connect(trCombo,SIGNAL(currentIndexChanged(int)),this,SLOT(translationSelected(int)));
    connect(ui.actionOpenForm, SIGNAL(triggered()), this, SLOT(openForm()));
    connect(ui.actionLoadTranslation, SIGNAL(triggered()), this, SLOT(loadTranslation()));
    connect(ui.actionReloadTranslations, SIGNAL(triggered()), this, SLOT(reloadTranslations()));
    connect(ui.actionAbout, SIGNAL(triggered()), this, SLOT(showAboutBox()));
    connect(ui.actionAbout_Qt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
    connect(ui.actionClose, SIGNAL(triggered()), this, SLOT(close()));

    ui.menuViewViews->addAction(ui.dwForms->toggleViewAction());

    m_uiFilesModel = new QStandardItemModel(0, 1, this);
    ui.viewForms->setModel(m_uiFilesModel);
    ui.viewForms->setAlternatingRowColors(true);
    QPalette pal = palette();
    pal.setColor(QPalette::AlternateBase, TREEVIEW_ODD_COLOR);
    ui.viewForms->setPalette(pal);

}

void TrPreviewTool::on_viewForms_doubleClicked(const QModelIndex &index)
{
    QString path = m_uiFilesModel->data(index, Qt::ToolTipRole).toString();
    QVariant var = m_uiFilesModel->data(index, Qt::UserRole);
    QPointer<FormHolder> holderPtr = qVariantValue<QPointer<FormHolder> >(var);
    if (holderPtr.isNull()) {
        holderPtr = createFormFromFile(path);
        qVariantSetValue(var, holderPtr);
        m_uiFilesModel->setData(index, var, Qt::UserRole);
    }
    
    holderPtr->show();
    holderPtr->activateWindow();
    holderPtr->setFocus(Qt::OtherFocusReason);
}

TrPreviewTool::~TrPreviewTool()
{
}

void TrPreviewTool::cascade()
{
    if (workspace) workspace->cascade();
}

bool TrPreviewTool::addFormFile(const QString &path) 
{
    int row = m_uiFilesModel->rowCount();
    bool ok = m_uiFilesModel->insertRows(row, 1);
    if (ok) {
        QModelIndex idx = m_uiFilesModel->index(row, 0);
        m_uiFilesModel->setData(idx, path, Qt::ToolTipRole);
        m_uiFilesModel->setData(idx, QFileInfo(path).fileName());
    }
    return ok;
}

FormHolder* TrPreviewTool::createFormFromFile(const QString& path)
{
    static QStringList formFileList;

    FormHolder* formHolder = new FormHolder(workspace);
    if(!formHolder->loadFormFile(path)) {
	    delete formHolder;
	    return 0;
    }

    if(!formFileList.contains(path))
	    formFileList.append(path);
    QString ft = tr("Preview Form ");
    ft += QString::number(formFileList.indexOf(path) + 1);
    if(!formHolder->windowTitle().isEmpty())
	    ft += QLatin1String(" [") + formHolder->windowTitle() + QLatin1Char(']');
    formHolder->setWindowTitle(ft);

    workspace->addWindow(formHolder);
    return formHolder;
}

void TrPreviewTool::openForm()
{
    static QString initDir;
    QStringList pathList = QFileDialog::getOpenFileNames(this,
							 tr("Open Forms"),
							 initDir,
							 tr("User interface form files (*.ui);;All files (*.*)"));
    if(pathList.count())
	initDir = QFileInfo(pathList.first()).absolutePath();
    else
	return;

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    QHash<QString,FormHolder*> windowDict;
    foreach(QWidget* window,workspace->windowList()) {
	FormHolder* holder = qobject_cast<FormHolder*>(window);
	if(holder)
	    windowDict.insert(holder->formFilePath(),holder);
    }
    QString noGoodPaths;
    foreach(QString path,pathList) {
	if(windowDict.contains(path)) {
	    // Already open
	    workspace->setActiveWindow(windowDict.value(path));
	}
	else {
	    FormHolder* formHolder = createFormFromFile(path);
	    if(!formHolder) {
		noGoodPaths += QDir::toNativeSeparators(path) + QLatin1Char('\n');
	    }
	    else {
		formHolder->show();
	    }
	}
    }

    QApplication::restoreOverrideCursor();

    if(!noGoodPaths.isEmpty())
	showWarning(tr("Could not load form file(s):\n") + noGoodPaths);
}


void TrPreviewTool::recreateForms()
{
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    foreach(QWidget* window,workspace->windowList()) {
	    FormHolder* holder = qobject_cast<FormHolder*>(window);
	    if(holder)
	        holder->retranslate();
    }
    QApplication::restoreOverrideCursor();
}


void TrPreviewTool::translationSelected(int idx)
{
    QTranslator* newTr = trDict.value(trCombo->itemData(idx).toString());
    trCombo->setCurrentIndex(idx);	// If we're called programmatically
    // currentTr out of sync during resulting language change events; fix here if necessary
    if(currentTr)
	    QApplication::removeTranslator(currentTr);
    if(newTr)
	    QApplication::installTranslator(newTr);
    currentTr = newTr;
    recreateForms();
}

bool TrPreviewTool::loadTranslation(const QString &path, const QString &displayName)
{
    Q_ASSERT(!path.isEmpty());
    QString fn = QFileInfo(path).canonicalFilePath();
    QTranslator* newTr = new QTranslator(this);
    if (!trDict.contains(path)) {
        if (newTr->load(path)) {
	        trDict.insert(path, newTr);
            QString trName = displayName.isEmpty() ? QFileInfo(path).fileName() : displayName;
            int idx = trCombo->findText(trName);
            if (idx != -1)
                trName += QString::fromAscii("(%1)").arg(idx);  // Uniqify!
	        trCombo->addItem(trName, path);
	        trCombo->setCurrentIndex(trCombo->count() - 1);
        } else {
            return false;
        }
    }else {
        //already loaded: make active
        int idx = trCombo->findData(path);
        if(idx >= 0)			// Should always be true
	        translationSelected(idx);
    }
    return true;
}

bool TrPreviewTool::addTranslator(QTranslator *translator, const QString &path, const QString &displayName)
{
    if (!trDict.contains(path)) {
        trDict.insert(path, translator);
        QString trName = displayName;
        int idx = trCombo->findText(trName);
        if (idx != -1)
            trName += QString::fromAscii("(%1)").arg(idx);  // Uniqify!
        trCombo->addItem(trName, path);
        trCombo->setCurrentIndex(trCombo->count() - 1);
    } else {
        int idx = trCombo->findData(path);
        if(idx >= 0)			// Should always be true
	        translationSelected(idx);
    }
    return true;
}

bool TrPreviewTool::addTranslator(QTranslator *translator, const QString &displayName)
{
    Q_ASSERT(translator);
    QString path;
    path.sprintf("#:%p", translator);   // the "path" here is a just the string value of the pointer, 
                                        // which is always unique, and always start with '#:'.
    return addTranslator(translator, path, displayName);
}

void TrPreviewTool::loadTranslation()
{
    //### Handle .ts files as well
    static QString initDir;
    QString path = QFileDialog::getOpenFileName(this,
						tr("Load Translation"),
						initDir,
						tr("Translation files (*.qm);;All files (*.*)"));
    if(!path.isEmpty()) {
	    initDir = QFileInfo(path).absolutePath();
        if (!loadTranslation(path)) {
	        showWarning(tr("Could not load translation file:\n") + QDir::toNativeSeparators(path));
        }
    }
}

void TrPreviewTool::reloadTranslations()
{
    QString path;
    QString noGoodPaths;
    QList<QTranslator*> oldTrs;			
    foreach(path,trDict.keys()) {
        if (!path.startsWith("#:")) {
	        QTranslator* newTr = new QTranslator(this); // ### check if we can just reload on the old translator object instead 
	        if(newTr->load(path)) {
	            oldTrs.append(trDict.value(path));
	            trDict.insert(path, newTr);
	        }
	        else {
	            noGoodPaths += QDir::toNativeSeparators(path) + QLatin1Char('\n');
	        }
        }
    }
    if(!noGoodPaths.isEmpty())
	    showWarning(tr("Could not reload translation file(s):\n") + noGoodPaths);
    // Refresh
    translationSelected(trCombo->currentIndex());
    // Clean up now when we are sure it's not in use any longer
    foreach(QTranslator* oldTr,oldTrs) {
	    delete oldTr;
    }
}

void TrPreviewTool::showWarning(const QString& warning)
{
    QMessageBox::warning(this, tr("Qt Translation Preview Tool: Warning"),
			 warning,
			 QMessageBox::Ok,
			 QMessageBox::NoButton,
			 QMessageBox::NoButton);
}


void TrPreviewTool::showAboutBox()
{
    QFile f(QString::fromUtf8(":/about.html"));
    f.open(QIODevice::ReadOnly);
    QString aboutText = QString::fromUtf8(f.readAll());

    QMessageBox::about(this, tr("About ") + windowTitle(), aboutText);
}

/**
 * Uninstall the translator if the window was deactivated (i.e. moved to linguist itself) 
 * in order to avoid that linguist uses those translations.
 */
bool TrPreviewTool::event(QEvent *e)
{
    if(currentTr) {
        if (e->type() == QEvent::WindowActivate) {
            QApplication::installTranslator(currentTr);    
            return true;
        } else if (e->type() == QEvent::WindowDeactivate) {
            QApplication::removeTranslator(currentTr);
            return true;
        }
    }
    return false;
}

QUiLoader* FormHolder::uiLoader = 0;

FormHolder::FormHolder(QWidget* parent, Qt::WindowFlags flags)
    : QWidget(parent,flags), form(0)
{
    setAttribute(Qt::WA_DeleteOnClose);
    if(!uiLoader)
	uiLoader = new QUiLoader;
    layout = new QHBoxLayout(this);
    layout->setMargin(0);
    setLayout(layout);
}

QString FormHolder::formFilePath()
{
    return formPath;
}

bool FormHolder::loadFormFile(const QString& path)
{
    formPath = path;
    QFile file(path);
    if(!file.open(QIODevice::ReadOnly))
	return false;

    QWidget* newForm = uiLoader->load(&file,this);
    if (!newForm)
	return false;
    delete form;
    form = newForm;
    form->setWindowFlags(Qt::Widget);
    layout->addWidget(form);
    setWindowTitle(form->windowTitle());
    return true;
}

void FormHolder::retranslate()
{
    loadFormFile(formPath);
}
