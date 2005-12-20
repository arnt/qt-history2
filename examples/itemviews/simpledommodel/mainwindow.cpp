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

#include <QDomDocument>
#include <QFile>
#include <QtGui>

#include "dommodel.h"
#include "mainwindow.h"

MainWindow::MainWindow() : QMainWindow(), model(0)
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(tr("&Open..."), this, SLOT(openFile()),
                        QKeySequence(tr("Ctrl+O")));
    fileMenu->addAction(tr("E&xit"), this, SLOT(close()),
                        QKeySequence(tr("Ctrl+Q")));

    model = new DomModel(QDomDocument(), this);
    view = new QTreeView(this);
    view->setModel(model);

    setCentralWidget(view);
    setWindowTitle(tr("Simple DOM Model"));
}

void MainWindow::openFile()
{
    QString filePath = QFileDialog::getOpenFileName(this, tr("Open File"),
        xmlPath, tr("XML files (*.xml);;HTML files (*.html);;"
                    "SVG files (*.svg);;User Interface files (*.ui)"));

    if (!filePath.isEmpty()) {
        QFile file(filePath);
        if (file.open(QIODevice::ReadOnly)) {
            QDomDocument document;
            if (document.setContent(&file)) {
                DomModel *newModel = new DomModel(document, this);
                view->setModel(newModel);
                delete model;
                model = newModel;
                xmlPath = filePath;
            }
            file.close();
        }
    }
}
