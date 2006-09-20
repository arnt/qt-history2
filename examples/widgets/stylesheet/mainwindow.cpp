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

#include "mainwindow.h"
#include "stylesheeteditor.h"

MainWindow::MainWindow()
{
    ui.setupUi(this);

    ui.nameLabel->setProperty("class", "mandatory QLabel");

    styleSheetEditor = new StyleSheetEditor(this);

    statusBar()->addWidget(new QLabel(tr("Ready")));

    connect(ui.exitAction, SIGNAL(triggered()), qApp, SLOT(quit()));
    connect(ui.aboutQtAction, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
}

void MainWindow::on_editStyleAction_triggered()
{
    styleSheetEditor->show();
    styleSheetEditor->activateWindow();
}

void MainWindow::on_aboutAction_triggered()
{
    QMessageBox::about(this, tr("About Style sheet"),
        tr("The <b>Style Sheet</b> example shows how widgets can be styled "
           "using <a href=\"http://doc.trolltech.com/4.2/stylesheet.html\">Qt "
           "Style Sheets</a>. Click <b>File|Edit Style Sheet</b> to pop up the "
           "style editor, and either choose an existing style sheet or design "
           "your own."));
}
