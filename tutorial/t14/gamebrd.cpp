/****************************************************************************
** Implementation of GameBoard class, Qt tutorial 14
*****************************************************************************/

#include "gamebrd.h"

#include <qfont.h>
#include <qapp.h>
#include <qlabel.h>
#include <qaccel.h>
#include <qkeycode.h>
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
    quit->setGeometry( 10, 10, 75, 30 );
    quit->setFont( QFont( "Times", 18, QFont::Bold ) );
    connect( quit, SIGNAL(clicked()), qApp, SLOT(quitApp()) );

    angle  = new LCDRange( "ANGLE", this, "angle" );
    angle->setRange( 5, 70 );
    angle->setGeometry( 10, 45, 75, 130 );

    force  = new LCDRange( "FORCE", this, "force" );
    force->setRange( 10, 50 );
    force->setGeometry( 10, 180, 75, 130 );

    QFrame *frame = new QFrame( this, "cannonFrame" );
    frame->setGeometry( angle->x() + angle->width() + 5, 45, 400, 300 );
    frame->setFrameStyle( QFrame::WinPanel | QFrame::Sunken );

    cannon = new CannonField( this, "canonfield" );
    cannon->setGeometry( frame->x() + 2, frame->y() + 2,
			 frame->width() - 4, frame->height() - 4 );
    cannon->setBackgroundColor( QColor( 250, 250, 200) );
    cannon->raise();

    connect( angle, SIGNAL(valueChanged(int)), cannon, SLOT(setAngle(int)) );
    connect( force, SIGNAL(valueChanged(int)), cannon, SLOT(setForce(int)) );
    connect( cannon, SIGNAL(angleChanged(int)), angle, SLOT(setValue(int)) );
    connect( cannon, SIGNAL(forceChanged(int)), force, SLOT(setValue(int)) );
    connect( cannon, SIGNAL(hit()),SLOT(hit()) );
    connect( cannon, SIGNAL(missed()),SLOT(missed()) );

    angle->setValue( 45 );
    force->setValue( 25 );

    shoot = new QPushButton( "Shoot", this, "shoot" );
    shoot->setGeometry( 10, 315, 75, 30 );
    shoot->setFont( QFont( "Times", 18, QFont::Bold ) );
    connect( shoot, SIGNAL(clicked()), SLOT(fire()) );

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

    QAccel *accel = new QAccel( this );
    accel->connectItem( accel->insertItem( Key_Space), this, SLOT(fire()) );
    accel->connectItem( accel->insertItem( Key_Q), qApp, SLOT(quitApp()) );

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
