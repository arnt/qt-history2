/****************************************************************************
** Implementation of GameBoard class, Qt tutorial 13
**
** Copyright (C) 1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include <qfont.h>
#include <qapp.h>
#include <qlabel.h>

#include "gamebrd.h"


GameBoard::GameBoard( QWidget *parent=0, const char *name=0 )
        : QWidget( parent, name )
{
    quit = new QPushButton( "Quit", this, "quit" );
    quit->setGeometry( 10, 10, 75, 30 );
    quit->setFont( QFont( "Times", 18, QFont::Bold ) );
    connect( quit, SIGNAL(clicked()), qApp, SLOT(quitApp()) );

    angle  = new LCDRange( "ANGLE", this, "angle" );
    angle->setRange( 5, 70 );
    angle->setGeometry( 10, 45, 75, 130 );

    force  = new LCDRange( "FORCE", this, "force" );
    force->setRange( 10, 50 );
    force->setGeometry( 10, 180, 75, 130 );

    cannon = new CannonField( this, "canonfield" );
    cannon->setGeometry( angle->x() + angle->width() + 5, 45, 400, 300 );
    cannon->setBackgroundColor( QColor( 250, 250, 200) );

    connect( angle, SIGNAL(valueChanged(int)), cannon, SLOT(setAngle(int)) );
    connect( force, SIGNAL(valueChanged(int)), cannon, SLOT(setForce(int)) );
    connect( cannon, SIGNAL(hit()),SLOT(hit()) );
    connect( cannon, SIGNAL(missed()),SLOT(missed()) );

    angle->setValue( 45 );
    force->setValue( 25 );

    shoot = new QPushButton( "Shoot", this, "shoot" );
    shoot->setGeometry( 10, 315, 75, 30 );
    shoot->setFont( QFont( "Times", 18, QFont::Bold ) );
    connect( shoot, SIGNAL(clicked()), SLOT(shootClicked()) );

    reStart = new QPushButton( "New Game", this, "newgame" );
    reStart->setGeometry( 380, 10, 110, 30 );
    reStart->setFont( QFont( "Times", 18, QFont::Bold ) );
    connect( reStart, SIGNAL(clicked()), SLOT(newGame()) );

    hits  = new QLCDNumber( 2, this, "hits" );
    hits->setGeometry( 130, 10, 40, 30 );

    QLabel *hitsL  = new QLabel( "HITS", this, "hitsLabel" );
    hitsL->setGeometry( hits->x() + hits->width() + 5, 10, 60, 30 );

    shotsLeft  = new QLCDNumber( 2, this, "shotsleft" );
    shotsLeft->setGeometry( 240, 10, 40, 30 );

    QLabel *shotsLeftL  = new QLabel( "SHOTS LEFT", this, "shotsleftLabel" );
    shotsLeftL->setGeometry( shotsLeft->x() + shotsLeft->width() + 5, 10, 
                             60, 30);

    newGame();
}

void GameBoard::shootClicked()
{
    if ( cannon->gameOver() || cannon->isShooting() )
	return;
    shotsLeft->display( shotsLeft->longValue() - 1 );
    cannon->shoot();
}

void GameBoard::hit()
{
    hits->display( hits->longValue() + 1 );
    if ( shotsLeft->longValue() == 0 )
	cannon->setGameOver();
}
void GameBoard::missed()
{
    if ( shotsLeft->longValue() == 0 )
	cannon->setGameOver();
}

void GameBoard::newGame()
{
    shotsLeft->display( 15 );
    hits->display( 0 );
    cannon->restartGame();
}
