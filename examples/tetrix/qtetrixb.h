/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QTETRIXB_H
#define QTETRIXB_H

#include "gtetrix.h"
#include <qframe.h>

class QTimer;

class QTetrixBoard : public QFrame, public GenericTetrix
{
    Q_OBJECT
public:
    QTetrixBoard( QWidget *parent=0, const char *name=0 );

    void      gameOver();
    void      startGame(int gameType = 0,int fillRandomLines = 0);

public slots:
    void      timeout();
    void      updateNext()	{ GenericTetrix::updateNext(); }
    void      key(QKeyEvent *e) { keyPressEvent(e); }
    void      start()		{ startGame(); }
    void      pause();

signals:
    void      gameOverSignal();
    void      drawNextSquareSignal(int x,int y,QColor *color1);
    void      updateRemovedSignal(int noOfLines);
    void      updateScoreSignal(int score);
    void      updateLevelSignal(int level);

public:       // until we have keyboard focus, should be protected
    void      keyPressEvent( QKeyEvent * );

private:
    void      drawContents( QPainter * );
    void      resizeEvent( QResizeEvent * );
    void      drawSquare(int x,int y,int value);
    void      drawNextSquare(int x,int y,int value);
    void      updateRemoved(int noOfLines);
    void      updateScore(int newScore);
    void      updateLevel(int newLlevel);
    void      pieceDropped(int dropHeight);
    void      updateTimeoutTime();

    QTimer   *timer;

    int       xOffset,yOffset;
    int       blockWidth,blockHeight;
    int       timeoutTime;
    bool      noGame;
    bool      isPaused;
    bool      waitingAfterLine;

    QColor    colors[7];
    QPainter *paint;
    QWidget *paint_widget;
};

#endif
