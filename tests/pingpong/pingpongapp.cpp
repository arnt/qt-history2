#include "pingpongapp.h"
#include "dialogs.h"
#include "cursors.h"

#include <qlayout.h>
#include <qsqltable.h>
#include <qmessagebox.h>
#include <qmenubar.h>
#include <qtoolbar.h>
#include <qpopupmenu.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qfont.h>
#include <qframe.h>
#include <qapplication.h>
#include <qaction.h>
#include <qlistview.h>
#include <qtabwidget.h>
#include <qgroupbox.h>

//
//  MatchTable class
//
MatchTable::MatchTable( QWidget * parent = 0, const char * name = 0 )
    : QSqlTable( parent, name )
{
}

void MatchTable::sortColumn ( int col, bool ascending, bool wholeRows )
{
    if ( cursor()->field(indexOf(col))->name() == "winner" ) {
	cursor()->select( cursor()->filter(), cursor()->index( "winnerid" )  );
	viewport()->repaint( FALSE );
	return;
    } else if ( cursor()->field(indexOf(col))->name() == "loser" ) {
	cursor()->select( cursor()->filter(), cursor()->index( "loserid" )  );
	viewport()->repaint( FALSE );
	return;
    } else if ( cursor()->field(indexOf(col))->name() == "sets" ) {
	return;
    }

    QSqlTable::sortColumn( col, ascending, wholeRows );
}


//
//  Statistics class
//
Statistics::Statistics( QWidget * parent = 0, const char * name = 0 )
    : QWidget( parent, name )
{
    QGridLayout * g = new QGridLayout( this );
    g->setSpacing( 5 );
    g->setMargin( 5 );
    
    QWidget * baseWidget = new QWidget( this );
    QHBoxLayout * hbl = new QHBoxLayout( baseWidget );
    
    QLabel * label = new QLabel( "Team name:", this );
    g->addWidget( label, 0, 0 );
    teamPicker = new TeamPicker( this );
    hbl->addWidget( teamPicker );
    hbl->addItem( new QSpacerItem( 0, 0, QSizePolicy::Expanding,
				   QSizePolicy::Minimum ) );
    g->addLayout( hbl, 0, 1 );

    label = new QLabel( "Sets won:", this );
    g->addWidget( label, 1, 0 );
    setsWon = new QLabel( "0", this );
    g->addWidget( setsWon, 1, 1 );

    label = new QLabel( "Matches won:", this );
    g->addWidget( label, 2, 0 );
    matchesWon = new QLabel( "0", this );
    g->addWidget( matchesWon, 2, 1 );
    
    label = new QLabel( "Winner percentage:", this );
    g->addWidget( label, 3, 0 );
    winPercentage = new QLabel( "0", this );
    g->addWidget( winPercentage, 3, 1 );

    QFrame * separator = new QFrame( this );
    separator->setFrameStyle( QFrame::HLine | QFrame::Raised );
    g->addMultiCellWidget( separator, 4, 4, 0, 1 );

    label = new QLabel( "Sets lost:", this );
    g->addWidget( label, 5, 0 );
    setsLost = new QLabel( "0", this );
    g->addWidget( setsLost, 5, 1 );

    label = new QLabel( "Matches lost:", this );
    g->addWidget( label, 6, 0 );
    matchesLost = new QLabel( "0 %", this );
    g->addWidget( matchesLost, 6, 1 );

    label = new QLabel( "Loser percentage:", this );
    g->addWidget( label, 7, 0 );
    lossPercentage = new QLabel( "0 %", this );
    g->addWidget( lossPercentage, 7, 1 );

    separator = new QFrame( this );
    separator->setFrameStyle( QFrame::HLine | QFrame::Raised );
    g->addMultiCellWidget( separator, 8, 8, 0, 1 );
    
    label = new QLabel( "Total sets played:", this );
    g->addWidget( label, 9, 0 );
    totalSets = new QLabel( "0", this );
    g->addWidget( totalSets, 9, 1 );

    label = new QLabel( "Total matches played:", this );
    g->addWidget( label, 10, 0 );
    totalMatches = new QLabel( "0", this );
    g->addWidget( totalMatches, 10, 1 );

    label = new QLabel( "Usually loses against:", this );
    g->addWidget( label, 11, 0 );
    hate = new QLabel( "None", this );
    g->addWidget( hate, 11, 1 );
    
    label = new QLabel( "Usually beats:", this );
    g->addWidget( label, 12, 0 );
    love = new QLabel( "None", this );
    g->addWidget( love, 12, 1 );

    separator = new QFrame( this );
    separator->setFrameStyle( QFrame::HLine | QFrame::Raised );
    g->addMultiCellWidget( separator, 13, 13, 0, 1 );

    label = new QLabel( "Current top team:", this );
    g->addWidget( label, 14, 0 );
    topTeam = new QLabel( "None", this );
    g->addWidget( topTeam, 14, 1 );

    g->addItem( new QSpacerItem( 0, 0, QSizePolicy::Expanding,
				 QSizePolicy::Expanding ) );

    connect( teamPicker, SIGNAL( activated( int ) ), SLOT( updateStats() ) );
    updateStats();
}

void Statistics::updateStats()
{
    int numMatches = 0;
    int numWins = 0;
    int numLosses = 0;
    int teamId = teamPicker->teamId();
    int numSets = 0;
    int sets = 0;
    QString str;
    
    // Sets won
    QSqlQuery qsww( "select winnerwins from match where winnerid = " + 
		 QString::number( teamId ) + ";" );
    while( qsww.next() ){
	sets += qsww.value(0).toInt();
    } 
    QSqlQuery qslw( "select loserwins from match where loserid = " + 
		 QString::number( teamId ) + ";" );
    while( qslw.next() ){
	sets += qslw.value(0).toInt();
    } 
    setsWon->setText( QString::number( sets ) );
    numSets = sets;

    // Matches won
    QSqlQuery qw( "select count(*) from match where winnerid = " + 
		 QString::number( teamId ) + ";" );
    if( qw.next() ){
	numWins = qw.value(0).toInt();
	matchesWon->setText( qw.value(0).toString() );
    } else
	matchesWon->setText( "0" );

    // Sets lost
    sets = 0;
    QSqlQuery qsl( "select winnerwins from match where loserid = " + 
		    QString::number( teamId ) + ";" );
    while( qsl.next() ){
	sets += qsl.value(0).toInt();
    } 
    setsLost->setText( QString::number( sets ) );
    numSets += sets;

    // Matches lost
    QSqlQuery ql( "select count(*) from match where loserid = " + 
		 QString::number( teamId ) + ";" );
    if( ql.next() ){
	numLosses = ql.value(0).toInt();
	matchesLost->setText( ql.value(0).toString() );
    }else
	matchesLost->setText( "0" );

    // Percentage wins/losses
    QSqlQuery qp( "select count(*) from match where winnerid = " + 
		  QString::number( teamId ) + " or loserid = " + 
		  QString::number( teamId ) + ";" );
    if( qp.next() )
	numMatches = qp.value(0).toInt();

    if( numMatches > 0 ){
	winPercentage->setText( QString::number( (numWins*100)/numMatches ) + " %" );
	lossPercentage->setText( QString::number( (numLosses*100)/numMatches ) + " %" );
    } else {
	winPercentage->setText( "0 %" );
	lossPercentage->setText( "0 %" );
    }

    // Find out who the team has lost most matches to
    QSqlQuery qwids( "select winnerid from match where loserid = " +
		    QString::number( teamId ) + ";" );
    int lostCount = 0;
    int lostId = -1;
    while( qwids.next() ){
	QSqlQuery qtmp( "select count(*) from match where loserid = " +
			QString::number( teamId ) + " and winnerid = " +
			qwids.value(0).toString() + ";" );
	if( qtmp.next() ){
	    if( qtmp.value(0).toInt() >= lostCount ){
		lostCount = qtmp.value(0).toInt();
		lostId = qwids.value(0).toInt();
	    }
	}
    }
    str = "None";
    if( lostId >= 0 ){
	QSqlQuery qtmp( "select name from team where id = " +
			QString::number( lostId ) + ";" );
	if( qtmp.next() )
	    str = qtmp.value(0).toString();

    }
    hate->setText( str );

    // Find out who the team has beat most times
    QSqlQuery qlids( "select loserid from match where winnerid = " +
		    QString::number( teamId ) + ";" );
    int winCount = 0;
    int winId = -1;
    while( qlids.next() ){
	QSqlQuery qtmp( "select count(*) from match where winnerid = " +
			QString::number( teamId ) + " and loserid = " +
			qlids.value(0).toString() + ";" );
	if( qtmp.next() ){
	    if( qtmp.value(0).toInt() >= winCount ){
		winCount = qtmp.value(0).toInt();
		winId = qlids.value(0).toInt();
	    }
	}
    }
    str = "None";
    if( winId >= 0 ){
	QSqlQuery qtmp( "select name from team where id = " +
			QString::number( winId ) + ";" );
	if( qtmp.next() )
	    str = qtmp.value(0).toString();
    }
    love->setText( str );
    
    // Total sets
    totalSets->setText( QString::number( numSets ) );

    // Total matches
    totalMatches->setText( QString::number( numMatches ) );
    
    // Find the team with most wins
    winCount = 0;
    winId = -1;
    QSqlQuery qtids( "select id from team;" );
    while( qtids.next() ){
	QSqlQuery qtmp( "select count(*) from match where winnerid = " +
			qtids.value(0).toString() + ";" );
	if( qtmp.next() ){
	    if( qtmp.value(0).toInt() >= winCount ){
		winCount = qtmp.value(0).toInt();
		winId = qtids.value(0).toInt();
	    }
	}
    }

    str = "None";
    if( winId >= 0 ){
	QSqlQuery qtmp( "select name from team where id = " +
			QString::number( winId ) + ";" );
	if( qtmp.next() ) 
	    str = qtmp.value(0).toString();
    }
    topTeam->setText( str );
}

//
//  PingpongApp class
//
PingPongApp::PingPongApp( QWidget * parent, const char * name )
    : QMainWindow( parent, name )
{
    init();
}

void PingPongApp::init()
{
    setCaption( "Skandinavisk Bordtennis Forbund (SBF) - Ligatabell" );
    QPixmap icon( "pingpong.xpm" );
    setIcon( icon );

    // Menus
    QPopupMenu * menu = new QPopupMenu( this );
    menu->insertItem( "Edit &Teams", this, SLOT( editTeams() ), CTRL+Key_T );
    menu->insertSeparator();
    menu->insertItem( "&Quit", qApp, SLOT( quit() ), CTRL+Key_Q );
    menuBar()->insertItem( "&File", menu );

    // Toolbar
    QToolBar * toolbar = new QToolBar( this );
    insertResultAc = new QAction( "Insert new result", QPixmap( "new.png" ),
				  QString::null, 0, this, 0 );
    connect( insertResultAc, SIGNAL( activated() ), SLOT( insertMatch() ) );
    insertResultAc->addTo( toolbar );
    updateResultAc = new QAction( "Update result", QPixmap( "edit.png" ), 
				  QString::null, 0, this, 0 );
    connect( updateResultAc, SIGNAL( activated() ), SLOT( updateMatch() ) );
    updateResultAc->addTo( toolbar );
    deleteResultAc = new QAction( "Delete result", QPixmap( "delete.png" ),
				  QString::null, 0, this, 0 );
    connect( deleteResultAc, SIGNAL( activated() ), SLOT( deleteMatch() ) );
    deleteResultAc->addTo( toolbar );

    // Layout the central widget

    tab = new QTabWidget( this );    
    connect( tab, SIGNAL( currentChanged( QWidget * ) ), 
	     SLOT( updateIcons( QWidget * ) ) );
    
    matchTable = new MatchTable( tab );
    teamEditor = new TeamEditorWidget( tab );
    statWidget = new Statistics( tab );

    tab->addTab( matchTable, "Matches" );
    tab->addTab( statWidget, "Statistics" );
    tab->addTab( teamEditor, "Team editor" );
    
    setCentralWidget( tab );
    resize( 700, 400 );

    // Setup the initial match table
    matchCr.select( matchCr.index( "date" ) );
    matchTable->setConfirmEdits( TRUE );
    matchTable->setConfirmCancels( TRUE );
    matchTable->setCursor( &matchCr, FALSE );
    matchTable->addColumn( matchCr.field( "date" ) );
    matchTable->addColumn( matchCr.field( "winner" ) );
    matchTable->addColumn( matchCr.field( "winnerwins" ) );
    matchTable->addColumn( matchCr.field( "loser" ) );
    matchTable->addColumn( matchCr.field( "loserwins" ) );
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

void PingPongApp::updateIcons( QWidget * w )
{
    if( w == matchTable ){ 
	insertResultAc->setEnabled( TRUE );
	updateResultAc->setEnabled( TRUE );
	deleteResultAc->setEnabled( TRUE );
    } else {
	insertResultAc->setEnabled( FALSE );
	updateResultAc->setEnabled( FALSE );
	deleteResultAc->setEnabled( FALSE );
    }
}

void PingPongApp::editTeams()
{
    if( tab->currentPage() != teamEditor ){
	tab->showPage( teamEditor );
    }
}
