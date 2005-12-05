/****************************************************************************
**
** Copyright (C) 2005-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtGui>

#include "mainwindow.h"
#include "findfiledialog.h"

MainWindow::MainWindow()
{
    textViewer = new QTextEdit;
    textViewer->setReadOnly(true);
    setCentralWidget(textViewer);

    createActions();
    createMenus();

    initializeAssistant();

    setWindowTitle(tr("Simple Text Viewer"));
    resize(750, 400);
}

void MainWindow::closeEvent(QCloseEvent *)
{
    if (assistantClient)
        assistantClient->closeAssistant();
}

void MainWindow::about()
{
    QMessageBox::about(this, tr("About Simple Text Viewer"),
                       tr("This example demonstrates how to use\n" \
                          "Qt Assistant as help system for your\n" \
                          "own application."));
}

void MainWindow::assistant()
{
    assistantClient->showPage(QLibraryInfo::location(QLibraryInfo::ExamplesPath) +
                                              QDir::separator() +
                                              "assistant/simpletextviewer/documentation/index.html");
}

void MainWindow::open()
{
    FindFileDialog dialog(textViewer, assistantClient);
    dialog.exec();
}

void MainWindow::createActions()
{
    assistantAct = new QAction(tr("Help Contents"), this);
    assistantAct->setShortcut(tr("F1"));
    connect(assistantAct, SIGNAL(triggered()), this, SLOT(assistant()));

    openAct = new QAction(tr("&Open..."), this);
    openAct->setShortcut(tr("Ctrl+O"));
    connect(openAct, SIGNAL(triggered()), this, SLOT(open()));

    clearAct = new QAction(tr("&Clear"), this);
    clearAct->setShortcut(tr("Ctrl+C"));
    connect(clearAct, SIGNAL(triggered()), textViewer, SLOT(clear()));

    exitAct = new QAction(tr("E&xit"), this);
    exitAct->setShortcut(tr("Ctrl+Q"));
    connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

    aboutAct = new QAction(tr("&About"), this);
    connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

    aboutQtAct = new QAction(tr("About &Qt"), this);
    connect(aboutQtAct, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
}

void MainWindow::createMenus()
{
    fileMenu = new QMenu(tr("&File"), this);
    fileMenu->addAction(openAct);
    fileMenu->addAction(clearAct);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAct);

    helpMenu = new QMenu(tr("&Help"), this);
    helpMenu->addAction(assistantAct);
    helpMenu->addSeparator();
    helpMenu->addAction(aboutAct);
    helpMenu->addAction(aboutQtAct);


    menuBar()->addMenu(fileMenu);
    menuBar()->addMenu(helpMenu);
}

void MainWindow::initializeAssistant()
{
    assistantClient = new QAssistantClient(QLibraryInfo::location(QLibraryInfo::BinariesPath), this);

    QStringList arguments;
    arguments << "-profile" << QString("documentation") + QDir::separator() + QString("simpletextviewer.adp");
    assistantClient->setArguments(arguments);
}
