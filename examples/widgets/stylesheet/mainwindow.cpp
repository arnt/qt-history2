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

#include <QtGui>
#include <QtUiTools>
#include "mainwindow.h"

MainWindow::MainWindow() : mw(0), styleEditorDialog(0)
{
    // create the style editor dialog 
    styleEditorDialog = new QDialog;
    styleEditorDialog->setModal(false);

    sse.setupUi(styleEditorDialog);

    sse.styleCombo->clear();
    sse.styleCombo->addItems(QStyleFactory::keys());
    QString styleName = qApp->style()->metaObject()->className();
    styleName = styleName.mid(1 /*q*/ , styleName.length() - 6 /*style*/);
    sse.styleCombo->setCurrentIndex(sse.styleCombo->findData(styleName, Qt::CaseInsensitive));
    QObject::connect(sse.styleCombo, SIGNAL(activated(const QString&)),
                     this, SLOT(setStyle(const QString&)));

    QObject::connect(sse.previewButton, SIGNAL(clicked()),
                     this, SLOT(previewStyleSheet()));

    QObject::connect(sse.styleSheetCombo, SIGNAL(activated(const QString&)),
                     this, SLOT(loadStyleSheet(const QString&)));

    // load the default
    sse.styleSheetCombo->setCurrentIndex(sse.styleSheetCombo->findText("coffee"));
    loadStyleSheet(sse.styleSheetCombo->currentText());
}

void MainWindow::editStyle()
{
    styleEditorDialog->show();
    styleEditorDialog->raise();
    styleEditorDialog->activateWindow();
}

void MainWindow::loadStyleSheet(const QString& qss)
{
    QString layout("boringdefault");
    QFile styleSheetFile(":/qss/" + qss + ".qss");
    styleSheetFile.open(QFile::ReadOnly);
    QString styleSheet = styleSheetFile.readAll();
    styleSheetFile.close();

    // load a specific layout, if requested by the stylesheet
    QRegExp re("layout\\((\\w*)\\)");
    if (re.indexIn(styleSheet) != -1)
        layout = re.cap(1);

    loadLayout(layout);

    sse.styleTextEdit->setPlainText(styleSheet);
    qApp->setStyleSheet(styleSheet);

    mw->setWindowTitle(tr("Style Sheet Example (%1)").arg(qss));
    mw->show();
}

void MainWindow::loadLayout(const QString& layout)
{
    if (layout == currentLayout)
        return;

    currentLayout = layout;
    QMainWindow *oldmw = mw;
    QUiLoader loader;
    QFile file(":/layouts/" + layout + ".ui");
    file.open(QFile::ReadOnly);
    mw = qobject_cast<QMainWindow *>(loader.load(&file, 0));
    Q_ASSERT(mw);
    file.close();

    mw->statusBar()->setSizeGripEnabled(true);
    mw->statusBar()->addWidget(new QLabel(tr("Ready.")));

    // set the nameLabel as mandatory - dynamic property
    QLabel *nameLabel = qFindChild<QLabel *>(mw, "nameLabel");
    nameLabel->setProperty("class", "mandatory QLabel");

    // turn on auto completion
    QComboBox *nameCombo = qFindChild<QComboBox *>(mw, "nameCombo");
    nameCombo->completer()->setCompletionMode(QCompleter::PopupCompletion);

    QAction *exitAction = qFindChild<QAction *>(mw, "exitAction");
    QObject::connect(exitAction, SIGNAL(triggered()), qApp, SLOT(quit()));

    QAction *aboutAction = qFindChild<QAction *>(mw, "aboutAction");
    QObject::connect(aboutAction, SIGNAL(triggered()), this, SLOT(about()));
    
    QAction *aboutQtAction = qFindChild<QAction *>(mw, "aboutQtAction");
    QObject::connect(aboutQtAction, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

    QAction *editStyleAction = qFindChild<QAction *>(mw, "editStyleAction");
    QObject::connect(editStyleAction, SIGNAL(triggered()), this, SLOT(editStyle()));

    if (oldmw) {
        mw->move(oldmw->frameGeometry().topLeft());
        mw->setWindowFlags(oldmw->windowFlags());
    }

    if (oldmw) {
        oldmw->hide();
        delete oldmw;
    }

}

void MainWindow::previewStyleSheet()
{
    qApp->setStyleSheet(sse.styleTextEdit->toPlainText());
}

void MainWindow::setStyle(const QString &s)
{
    qApp->setStyle(s);
}

void MainWindow::about()
{
    QMessageBox::about(mw, tr("About Styled Widget"),
        tr("The <b>Styled Widget</b> example shows how widgets can be styled "
           "according to a set of rules in a stylesheet. Edit the default "
           "stylesheet in the text editor and click <u>U</u>pdate to see the "
           "results of your changes."));
}

