/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of an example program for the ActiveQt integration.
** EDITIONS: UNKNOWN
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/


#ifndef QTETRIX_H
#define QTETRIX_H

#include "qtetraxb.h"
#include <qframe.h>
#include <qlcdnumber.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qpainter.h>

class ShowNextPiece : public QFrame
{
    Q_OBJECT
    friend class QTetrix;
public:
    ShowNextPiece( QWidget *parent=0, const char *name=0  );
public slots:
    void drawNextSquare( int x, int y,QColor *color );
signals:
    void update();
private:
    void paintEvent( QPaintEvent * );
    void resizeEvent( QResizeEvent * );
    
    int      blockWidth,blockHeight;
    int      xOffset,yOffset;
};


class QTetrax : public QWidget
{
    Q_OBJECT
    Q_PROPERTY( int score READ score WRITE setScore )
public:
    QTetrax( QWidget *parent=0, const char *name=0 );
    
    int score() const { return showScore->intValue(); }

public slots:
    void startGame() { board->startGame(); }
    void quit();
    void setScore( int score ) { showScore->display( score ); }

signals:
    void gameOver();

private:
    void keyPressEvent( QKeyEvent *e ) { board->keyPressEvent(e); }

    QTetrixBoard  *board;
    ShowNextPiece *showNext;
#ifndef QT_NO_LCDNUMBER
    QLCDNumber    *showScore;
    QLCDNumber    *showLevel;
    QLCDNumber    *showLines;
#else
    QLabel    *showScore;
    QLabel    *showLevel;
    QLabel    *showLines;
#endif
    QPushButton   *quitButton;
    QPushButton   *startButton;
    QPushButton   *pauseButton;
};

void drawTetrixButton( QPainter *, int x, int y, int w, int h,
		       const QColor *color, QWidget *widg);


#endif
