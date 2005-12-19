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

MainWindow::MainWindow()
    : QMainWindow()
{
    area = new SvgWindow;

    QMenu *fileMenu = new QMenu(tr("&File"), this);
    QAction *openAction = fileMenu->addAction(tr("&Open..."));
    openAction->setShortcut(QKeySequence(tr("Ctrl+O")));
    QAction *quitAction = fileMenu->addAction(tr("E&xit"));
    quitAction->setShortcut(QKeySequence(tr("Ctrl+Q")));

    menuBar()->addMenu(fileMenu);

    QMenu *rendererMenu = new QMenu(tr("&Renderer"), this);
    nativeAction = rendererMenu->addAction(tr("&Native"));
    nativeAction->setCheckable(true);
    nativeAction->setChecked(true);
    #ifndef QT_NO_OPENGL
    glAction = rendererMenu->addAction(tr("&OpenGL"));
    glAction->setCheckable(true);
    #endif
    imageAction = rendererMenu->addAction(tr("&Image"));
    imageAction->setCheckable(true);

    QActionGroup *rendererGroup = new QActionGroup(this);
    rendererGroup->addAction(nativeAction);
    #ifndef QT_NO_OPENGL
    rendererGroup->addAction(glAction);
    #endif
    rendererGroup->addAction(imageAction);

    menuBar()->addMenu(rendererMenu);

    connect(openAction, SIGNAL(triggered()), this, SLOT(openFile()));
    connect(quitAction, SIGNAL(triggered()), qApp, SLOT(quit()));
    connect(rendererGroup, SIGNAL(triggered(QAction *)),
            this, SLOT(setRenderer(QAction *)));

    setCentralWidget(area);
    setWindowTitle(tr("SVG Viewer"));
}

void MainWindow::openFile(const QString &path)
{
    QString fileName;
    if (path.isNull())
        fileName = QFileDialog::getOpenFileName(this, tr("Open SVG File"),
                                                currentPath, "*.svg");
    else
        fileName = path;

    if (!fileName.isEmpty()) {
        area->openFile(fileName);
        if (!fileName.startsWith(":/")) {
            currentPath = fileName;
            setWindowTitle(tr("%1 - SVGViewer").arg(currentPath));
        }
    }
}

void MainWindow::setRenderer(QAction *action)
{
    if (action == nativeAction)
        area->setRenderer(SvgWindow::Native);
    #ifndef QT_NO_OPENGL
    else if (action == glAction)
        area->setRenderer(SvgWindow::OpenGL);
    #endif
    else if (action == imageAction)
        area->setRenderer(SvgWindow::Image);
}
