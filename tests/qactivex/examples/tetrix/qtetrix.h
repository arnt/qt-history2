/****************************************************************************
** $Id: $
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#ifndef QTETRIX_H
#define QTETRIX_H

#include "qtetrixb.h"
#include <qframe.h>
#include <qlcdnumber.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qpainter.h>

#include <qactiveqt.h>

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


class QTetrix : public QWidget, public QActiveQt
{
    Q_OBJECT
public:
    QTetrix( QWidget *parent=0, const char *name=0 );
    void startGame() { board->startGame(); }

public slots:
    void gameOver();
    void quit();
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

QT_ACTIVEX( QTetrix, "{852558AD-CBD6-4f07-844C-D1E8983CD6FC}", 
		     "{2F5D0068-772C-4d1e-BCD2-D3F6BC7FD315}", 
		     "{769F4820-9F28-490f-BA50-5545BD381DCB}",
		     "{5753B1A8-53B9-4abe-8690-6F14EC5CA8D0}",
		     "{DE2F7CE3-CFA7-4938-A9FC-867E2FEB63BA}" )


void drawTetrixButton( QPainter *, int x, int y, int w, int h,
		       const QColor *color, QWidget *widg);


#endif
