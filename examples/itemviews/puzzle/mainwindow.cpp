/****************************************************************************
**
** Copyright (C) 2005-$THISYEAR$ $TROLLTECH$. All rights reserved.
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
#include <stdlib.h>

#include "mainwindow.h"
#include "piecesmodel.h"
#include "puzzlewidget.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupMenus();
    setupWidgets();
    model = new PiecesModel(this);
    piecesList->setModel(model);

    setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    setWindowTitle(tr("Puzzle"));
}

void MainWindow::openImage(const QString &path)
{
    QString fileName = path;

    if (fileName.isNull())
        fileName = QFileDialog::getOpenFileName(this,
            tr("Open Image"), "", "Image Files (*.png *.jpg *.bmp)");

    if (!fileName.isEmpty()) {
        QPixmap newImage;
        if (!newImage.load(fileName)) {
            QMessageBox::warning(this, tr("Open Image"),
                                 tr("The image file could not be loaded."),
                                 QMessageBox::Cancel);
            return;
        }
        puzzleImage = newImage;
        setupPuzzle();
    }
}

void MainWindow::setCompleted()
{
    QMessageBox::information(this, tr("Puzzle Completed"),
        tr("Congratulations! You have completed the puzzle!\n"
           "Click OK to start again."),
        QMessageBox::Ok);

    setupPuzzle();
}

void MainWindow::setupPuzzle()
{
    int size = qMin(puzzleImage.width(), puzzleImage.height());
    puzzleImage = puzzleImage.copy((puzzleImage.width() - size)/2,
        (puzzleImage.height() - size)/2, size, size).scaled(400,
            400, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    qsrand(QCursor::pos().x() ^ QCursor::pos().y());

    model->addPieces(puzzleImage);
    puzzleWidget->clear();
}

void MainWindow::setupMenus()
{
    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));

    QAction *openAction = fileMenu->addAction(tr("&Open..."));
    openAction->setShortcut(QKeySequence(tr("Ctrl+O")));

    QAction *exitAction = fileMenu->addAction(tr("E&xit"));
    exitAction->setShortcut(QKeySequence(tr("Ctrl+Q")));

    QMenu *gameMenu = menuBar()->addMenu(tr("&Game"));

    QAction *restartAction = gameMenu->addAction(tr("&Restart"));

    connect(openAction, SIGNAL(triggered()), this, SLOT(openImage()));
    connect(exitAction, SIGNAL(triggered()), qApp, SLOT(quit()));
    connect(restartAction, SIGNAL(triggered()), this, SLOT(setupPuzzle()));
}

void MainWindow::setupWidgets()
{
    QFrame *frame = new QFrame;
    QHBoxLayout *frameLayout = new QHBoxLayout(frame);

    piecesList = new QListView;
    piecesList->setDragEnabled(true);
//    piecesList->setViewMode(QListView::IconMode);
    piecesList->setIconSize(QSize(60, 60));
    piecesList->setGridSize(QSize(80, 80));
    piecesList->setSpacing(10);
    piecesList->setMovement(QListView::Snap);
    piecesList->setAcceptDrops(true);
    piecesList->setDropIndicatorShown(true);

    PiecesModel *model = new PiecesModel(this);
    piecesList->setModel(model);

    puzzleWidget = new PuzzleWidget;

    connect(puzzleWidget, SIGNAL(puzzleCompleted()),
            this, SLOT(setCompleted()), Qt::QueuedConnection);

    frameLayout->addWidget(piecesList);
    frameLayout->addWidget(puzzleWidget);
    setCentralWidget(frame);
}
