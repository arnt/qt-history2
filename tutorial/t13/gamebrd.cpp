/****************************************************************************
** Implementation of GameBoard class, Qt tutorial 13
*****************************************************************************/

#include "gamebrd.h"

#include <qfont.h>
#include <qapp.h>
#include <qlabel.h>
#include <qpushbt.h>
#include <qlcdnum.h>

#include "lcdrange.h"
#include "cannon.h"

GameBoard::GameBoard( QWidget *parent, const char *name )
        : QWidget( parent, name )
{
    setMinimumSize( 500, 355 );
    setMaximumSize( 500, 355 );

    quit = new QPushButton( "Quit", this, "quit" );
    quit->setFont( QFont( "Times", 18, QFont::Bold ) );
    connect( quit, SIGNAL(clicked()), qApp, SLOT(quitApp()) );

    angle  = new LCDRange( "ANGLE", this, "angle" );
    angle->setRange( 5, 70 );

    force  = new LCDRange( "FORCE", this, "force" );
    force->setRange( 10, 50 );

    cannon = new CannonField( this, "canonfield" );
    cannon->setBackgroundColor( QColor( 250, 250, 200) );

    connect( angle, SIGNAL(valueChanged(int)), cannon, SLOT(setAngle(int)) );
    connect( force, SIGNAL(valueChanged(int)), cannon, SLOT(setForce(int)) );
    connect( cannon, SIGNAL(hit()),SLOT(hit()) );
    connect( cannon, SIGNAL(missed()),SLOT(missed()) );

    angle->setValue( 45 );
    force->setValue( 25 );

    shoot = new QPushButton( "Shoot", this, "shoot" );
    shoot->setFont( QFont( "Times", 18, QFont::Bold ) );
    connect( shoot, SIGNAL(clicked()), SLOT(fire()) );

    reStart = new QPushButton( "New Game", this, "newgame" );
    reStart->setFont( QFont( "Times", 18, QFont::Bold ) );
    connect( reStart, SIGNAL(clicked()), SLOT(newGame()) );

    hits  = new QLCDNumber( 2, this, "hits" );

    QLabel *hitsL  = new QLabel( "HITS", this, "hitsLabel" );

    shotsLeft  = new QLCDNumber( 2, this, "shotsleft" );

    QLabel *shotsLeftL  = new QLabel( "SHOTS LEFT", this, "shotsleftLabel" );

    quit->setGeometry( 10, 10, 75, 30 );
    angle->setGeometry( 10, 45, 75, 130 );
    force->setGeometry( 10, 180, 75, 130 );
    cannon->setGeometry( angle->x() + angle->width() + 5, 45, 400, 300 );
    shoot->setGeometry( 10, 315, 75, 30 );
    reStart->setGeometry( 380, 10, 110, 30 );
    hits->setGeometry( 130, 10, 40, 30 );
    hitsL->setGeometry( hits->x() + hits->width() + 5, 10, 60, 30 );
    shotsLeft->setGeometry( 240, 10, 40, 30 );
    shotsLeftL->setGeometry( shotsLeft->x() + shotsLeft->width() + 5, 10, 
                             60, 30);

    newGame();
}


void GameBoard::fire()
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
