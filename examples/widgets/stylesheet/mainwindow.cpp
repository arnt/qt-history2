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
    loadLayout("boringdefault");
}

void MainWindow::applyStyle()
{
    Q_ASSERT(mw);
    mw->setStyleSheet(sse.styleTextEdit->toPlainText());
}

void MainWindow::editStyle()
{
    if (styleEditorDialog) {
        styleEditorDialog->activateWindow();
        return;
    }
    styleEditorDialog = new QDialog;
    styleEditorDialog->setModal(false);

    sse.setupUi(styleEditorDialog);

    QString styleName = mw->style()->metaObject()->className();
    styleName = styleName.mid(1 /*q*/ , styleName.length() - 6 /*style*/);
    sse.styleCombo->setCurrentIndex(sse.styleCombo->findData(styleName, Qt::CaseInsensitive));
    QObject::connect(sse.styleCombo, SIGNAL(activated(const QString&)),
                     this, SLOT(setStyle(const QString&)));

    QObject::connect(sse.layoutCombo, SIGNAL(activated(const QString&)),
                     this, SLOT(loadLayout(const QString&)));

    QObject::connect(sse.previewButton, SIGNAL(clicked()),
                     this, SLOT(previewStyleSheet()));

    QObject::connect(sse.styleSheetCombo, SIGNAL(activated(const QString&)),
                     this, SLOT(loadEditor(const QString&)));

    loadEditor("boringdefault");

    styleEditorDialog->show();
}

void MainWindow::previewStyleSheet()
{
    mw->setStyleSheet(sse.styleTextEdit->toPlainText());
}

void MainWindow::loadEditor(const QString& qss)
{
    QFile defaultSheet(":/qss/" + qss + ".qss");
    if (defaultSheet.open(QFile::ReadOnly)) {
        sse.styleTextEdit->setPlainText(defaultSheet.readAll());
        defaultSheet.close();
    }
    applyStyle();
}

void MainWindow::loadLayout(const QString& layout)
{
    QMainWindow *oldmw = mw;

    QUiLoader loader;
    QFile file(":/layouts/" + layout + ".ui");
    file.open(QFile::ReadOnly);
    mw = qobject_cast<QMainWindow *>(loader.load(&file, 0));
    Q_ASSERT(mw);
    file.close();

    mw->statusBar()->setSizeGripEnabled(true);

    mw->setWindowTitle(tr("Style Sheet Example"));

    QAction *exitAction = qFindChild<QAction *>(mw, "exitAction");
    QObject::connect(exitAction, SIGNAL(triggered()), qApp, SLOT(quit()));

    QAction *aboutAction = qFindChild<QAction *>(mw, "aboutAction");
    QObject::connect(aboutAction, SIGNAL(triggered()), this, SLOT(about()));
    
    QAction *aboutQtAction = qFindChild<QAction *>(mw, "aboutQtAction");
    QObject::connect(aboutQtAction, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

    QAction *editStyleAction = qFindChild<QAction *>(mw, "editStyleAction");
    QObject::connect(editStyleAction, SIGNAL(triggered()), this, SLOT(editStyle()));

    if (oldmw)
        mw->move(oldmw->frameGeometry().topLeft());

    mw->show();

    if (oldmw) {
        oldmw->hide();
        delete oldmw;
    }

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
