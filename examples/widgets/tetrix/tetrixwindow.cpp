#include <QtGui>

#include "tetrixboard.h"
#include "tetrixwindow.h"

TetrixWindow::TetrixWindow()
{
    board = new TetrixBoard(this);
    board->setFocus();

    nextPieceLabel = new QLabel(this);
    nextPieceLabel->setFrameStyle(QFrame::Box | QFrame::Raised);
    nextPieceLabel->setAlignment(Qt::AlignCenter);
    board->setNextPieceLabel(nextPieceLabel);

    scoreLcd = new QLCDNumber(5, this);
    levelLcd = new QLCDNumber(2, this);
    linesLcd = new QLCDNumber(5, this);

    quitButton  = new QPushButton(tr("&Quit"),this);
    quitButton->setFocusPolicy(Qt::NoFocus);
    startButton = new QPushButton(tr("&New Game"),this);
    startButton->setFocusPolicy(Qt::NoFocus);
    pauseButton = new QPushButton(tr("&Pause"),this);
    pauseButton->setFocusPolicy(Qt::NoFocus);

    connect(startButton, SIGNAL(clicked()), board, SLOT(start()));
    connect(quitButton , SIGNAL(clicked()), qApp, SLOT(quit()));
    connect(pauseButton, SIGNAL(clicked()), board, SLOT(pause()));
    connect(board, SIGNAL(scoreChanged(int)), scoreLcd, SLOT(display(int)));
    connect(board, SIGNAL(levelChanged(int)), levelLcd, SLOT(display(int)));
    connect(board, SIGNAL(linesRemovedChanged(int)),
            linesLcd, SLOT(display(int)));

    board->setGeometry(150, 20, 152, 332);
    nextPieceLabel->setGeometry(50, 40, 78, 94);
    scoreLcd->setGeometry(330, 40, 178, 93);
    levelLcd->setGeometry(50, 160, 78, 93);
    linesLcd->setGeometry(330, 160, 178, 93);
    startButton->setGeometry(46, 288, 90, 30);
    quitButton->setGeometry(370, 265, 90, 30);
    pauseButton->setGeometry(370, 310, 90, 30);

    createLabel(tr("NEXT"), 50, 10, 78, 30);
    createLabel(tr("SCORE"), 330, 10, 178, 30);
    createLabel(tr("LEVEL"), 50, 130, 78, 30);
    createLabel(tr("LINES REMOVED"), 330, 130, 178, 30);

    setWindowTitle(tr("Tetrix"));
    resize(550, 370);
}

void TetrixWindow::createLabel(const QString &text, int x, int y, int width,
                               int height)
{
    QLabel *label = new QLabel(text, this);
    label->setGeometry(x, y, width, height);
    label->setAlignment(Qt::AlignCenter);
}
