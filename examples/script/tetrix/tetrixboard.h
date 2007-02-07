/****************************************************************************
**
** Copyright (C) 2007-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef TETRIXBOARD_H
#define TETRIXBOARD_H

#include <QTimer>
#include <QFrame>
#include <QPointer>

class QLabel;

class TetrixBoard : public QFrame
{
    Q_OBJECT
    Q_PROPERTY(QObject* timer READ getTimer)
    Q_PROPERTY(QWidget* nextPieceLabel WRITE setNextPieceLabel)

public:
    TetrixBoard(QWidget *parent = 0);

    void setNextPieceLabel(QWidget *label);
    void setBoardWidth(int width);
    void setBoardHeight(int height);
    QSize minimumSizeHint() const;

    QObject *getTimer();

signals:
    void scoreChanged(int score);
    void levelChanged(int level);
    void linesRemovedChanged(int numLines);

#if !defined(Q_MOC_RUN)
private: // can only be emitted by TetrixBoard
#endif
    void keyPressed(int key);
    void paintRequested(QPainter *painter);
    void paintNextPieceRequested(QPainter *painter);

protected:
    void paintEvent(QPaintEvent *event);
    void keyPressEvent(QKeyEvent *event);

private slots:
    void showNextPiece(int width, int height);
    void drawPauseScreen(QPainter *painter);
    void drawSquare(QPainter *painter, int x, int y, int shape);

private:
    enum { BoardWidth = 10, BoardHeight = 22 };

    int squareWidth() { return contentsRect().width() / BoardWidth; }
    int squareHeight() { return contentsRect().height() / BoardHeight; }

    QTimer *timer;
    QPointer<QLabel> nextPieceLabel;
};

#endif
