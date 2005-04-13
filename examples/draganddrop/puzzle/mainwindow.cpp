#include <QtGui>
#include <stdlib.h>

#include "mainwindow.h"
#include "pieceslist.h"
#include "puzzlewidget.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupMenus();
    setupWidgets();

    setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    setWindowTitle(tr("Puzzle"));
}

void MainWindow::openImage(const QString &path)
{
    QString fileName = path;

    if (fileName.isNull())
        fileName = QFileDialog::getOpenFileName(this,
            tr("Open Image"), "", "Image files (*.png *.jpg *.bmp)");

    if (!fileName.isEmpty()) {
        QPixmap newImage;
        if (!newImage.load(fileName)) {
            QMessageBox::warning(this, tr("Open Image"),
                                 tr("The image file could not be loaded."),
                                 QMessageBox::Cancel, QMessageBox::NoButton);
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

    piecesList->clear();

    for (int y = 0; y < 5; ++y) {
        for (int x = 0; x < 5; ++x) {
            QPixmap pieceImage = puzzleImage.copy(x*80, y*80, 80, 80);
            piecesList->addPiece(pieceImage, QPoint(x, y));
        }
    }

    srand(QCursor::pos().x() ^ QCursor::pos().y());

    for (int i = 0; i < piecesList->count(); ++i) {
        if (int(2.0*rand()/(RAND_MAX+1.0)) == 1) {
            QListWidgetItem *item = piecesList->takeItem(i);
            piecesList->insertItem(0, item);
        }
    }

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

    piecesList = new PiecesList;
    puzzleWidget = new PuzzleWidget;

    connect(puzzleWidget, SIGNAL(puzzleCompleted()),
            this, SLOT(setCompleted()), Qt::QueuedConnection);

    frameLayout->addWidget(piecesList);
    frameLayout->addWidget(puzzleWidget);
    setCentralWidget(frame);
}
