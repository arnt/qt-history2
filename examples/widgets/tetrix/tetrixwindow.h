#ifndef TETRIXWINDOW_H
#define TETRIXWINDOW_H

#include <QFrame>
#include <QWidget>

class QLCDNumber;
class QLabel;
class QPushButton;
class TetrixBoard;

class TetrixWindow : public QWidget
{
    Q_OBJECT

public:
    TetrixWindow();

private:
    void createLabel(const QString &text, int x, int y, int width, int height);

    TetrixBoard *board;
    QLabel *nextPieceLabel;
    QLCDNumber *scoreLcd;
    QLCDNumber *levelLcd;
    QLCDNumber *linesLcd;
    QPushButton *quitButton;
    QPushButton *startButton;
    QPushButton *pauseButton;
};

#endif
