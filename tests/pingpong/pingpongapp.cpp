#include "pingpongapp.h"
#include "dialogs.h"
#include "cursors.h"

#include <qlayout.h>
#include <qsqltable.h>
#include <qmessagebox.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qfont.h>
#include <qframe.h>
#include <qapplication.h>

MatchTable::MatchTable( QWidget * parent = 0, const char * name = 0 )
    : QSqlTable( parent, name )
{
}

void MatchTable::sortColumn ( int col, bool ascending,
			      bool wholeRows )
{
    if ( cursor()->field(indexOf(col))->name() == "winner" ) {
	cursor()->select( cursor()->filter(), cursor()->index( "winnerid" )  );
	viewport()->repaint( FALSE );
	return;
    } else if ( cursor()->field(indexOf(col))->name() == "loser" ) {
	cursor()->select( cursor()->filter(), cursor()->index( "loserid" )  );
	viewport()->repaint( FALSE );	
	return;
    }
    QSqlTable::sortColumn( col, ascending, wholeRows );
}

PingPongApp::PingPongApp( QWidget * parent, const char * name )
    : QMainWindow( parent, name )
{
    init();
}

void PingPongApp::init()
{
    setCaption( "National Pingpong Association (NPA) League Table" );
    QPixmap ppicon( "pingpong.xpm" );
    setIcon( ppicon );

    // Setup menus
    QPopupMenu * menu = new QPopupMenu( this );
    menu->insertItem( "Edit &Teams", this, SLOT( editTeams() ), CTRL+Key_T );
    menu->insertSeparator();
    menu->insertItem( "&Quit", qApp, SLOT( quit() ), CTRL+Key_Q );
    menuBar()->insertItem( "&File", menu );

    resize( 700, 400 );

    QFrame * f1       = new QFrame( this );
    QVBoxLayout * vb1 = new QVBoxLayout( f1 );

    vb1->setMargin( 5 );
    vb1->setSpacing( 5 );

    //
    // Set up the different widgets
    //
    QFont f = font();
    f.setBold( TRUE );

    QLabel * label = new QLabel( f1 );
    label->setText( "Matches" );
    label->setFont( f );
    QFontMetrics fm = label->fontMetrics();

    vb1->addWidget( label );

    matchTable = new MatchTable( f1 );
    vb1->addWidget( matchTable );

    // insert/update/delete buttons
    QFrame * buttonFrame = new QFrame( f1 );
    QHBoxLayout * chl = new QHBoxLayout( buttonFrame );
    chl->setSpacing( 2 );

    chl->addItem( new QSpacerItem( 0, 0, QSizePolicy::Expanding,
				   QSizePolicy::Minimum ) );

    QPushButton * button = new QPushButton( "U&pdate", buttonFrame );
    chl->addWidget( button );
    connect( button, SIGNAL( clicked() ), this, SLOT( updateMatch() ) );

    button = new QPushButton( "I&nsert", buttonFrame );
    chl->addWidget( button );
    connect( button, SIGNAL( clicked() ), this, SLOT( insertMatch() ) );

    button = new QPushButton( "D&elete", buttonFrame );
    chl->addWidget( button );
    connect( button, SIGNAL( clicked() ), this, SLOT( deleteMatch() ) );

    vb1->addWidget( buttonFrame );

    setCentralWidget( f1 );

    // match table
    matchCr.select( matchCr.index( "date" ) );    
    matchTable->setConfirmEdits( TRUE );
    matchTable->setConfirmCancels( TRUE );
    matchTable->setCursor( &matchCr, FALSE );
    matchTable->addColumn( matchCr.field( "date" ) );
    matchTable->addColumn( matchCr.field( "winner" ) );
    matchTable->addColumn( matchCr.field( "wins" ) );
    matchTable->addColumn( matchCr.field( "loser" ) );
    matchTable->addColumn( matchCr.field( "losses" ) );
    matchTable->addColumn( matchCr.field( "sets" ) );
    matchTable->setSorting( TRUE );
    matchTable->setReadOnly( TRUE );
}

void PingPongApp::insertMatch()
{
     QSqlCursor * cr = matchTable->cursor();

     MatchDialog dlg( cr->insertBuffer(), MatchDialog::Insert, this );
     if( dlg.exec() == QDialog::Accepted ){
 	cr->insert();
 	matchTable->refresh();
     }
}

void PingPongApp::updateMatch()
{
     QSqlCursor * cr = matchTable->cursor();

     MatchDialog dlg( cr->updateBuffer(), MatchDialog::Update, this );
     if( dlg.exec() == QDialog::Accepted ){
 	cr->update();
 	matchTable->refresh();
     }
}

void PingPongApp::deleteMatch()
{
     QSqlCursor * cr = matchTable->cursor();

     MatchDialog dlg( cr->updateBuffer(), MatchDialog::Delete, this );
     if( dlg.exec() == QDialog::Accepted ){
 	cr->del();
 	matchTable->refresh();
     }
}

void PingPongApp::editTeams()
{
    EditTeamsDialog dlg( this );
    dlg.resize( 400, 300 );
    dlg.exec();
}
