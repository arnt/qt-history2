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
#include <qstringlist.h>

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
    QSqlQuery qsww( "select sum(winnerwins) from match where winnerid = " +
		 QString::number( teamId ) + ";" );
    if( qsww.next() )
	sets = qsww.value(0).toInt();
    QSqlQuery qslw( "select sum(loserwins) from match where loserid = " +
		 QString::number( teamId ) + ";" );
    if( qslw.next() )
	sets += qslw.value(0).toInt();
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
    QSqlQuery qsl( "select sum(winnerwins) from match where loserid = " +
		    QString::number( teamId ) + ";" );
    if( qsl.next() )
	sets = qsl.value(0).toInt();
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
    QSqlQuery qwids( "select name from team, match where loserid=" + QString::number( teamId ) + " and team.id=match.winnerid group by team.name order by count(winnerid) desc;" );
    if ( qwids.next() && qwids.value(0).toString().length() )
	hate->setText( qwids.value(0).toString() );
    else
	hate->setText( "None" );

    // Find out who the team has beat most times
    QSqlQuery qlids( "select name from team, match where winnerid=" + QString::number( teamId ) + " and team.id=match.loserid group by team.name order by count(loserid) desc;" );
    if ( qlids.next() && qlids.value(0).toString().length() )
	love->setText( qlids.value(0).toString() );
    else
	love->setText( "None" );

    // Total sets
    totalSets->setText( QString::number( numSets ) );

    // Total matches
    totalMatches->setText( QString::number( numMatches ) );
}


//
//  HallOfFame class
//
HallOfFame::HallOfFame( QWidget * parent = 0, const char * name = 0 )
    : QWidget( parent, name )
{
    QGridLayout * g = new QGridLayout( this );
    g->setSpacing( 5 );
    g->setMargin( 5 );
    
    scorelist = new QListView( this );
    scorelist->addColumn( "Team name" );
    scorelist->addColumn( "Number of wins" );
    scorelist->setSorting( -1 );
    g->addWidget( scorelist, 0, 0 );
    
    QStringList teamNames;
    
    // Generate a highscore table - the hacky way
    QSqlQuery highscore("select name, count(winnerid) from match, team "
		       "where team.id=match.winnerid "
		       "group by name "
		       "order by count(winnerid) asc;" );
    while( highscore.next() ){
	teamNames.append( highscore.value(0).toString() );
	qDebug(  highscore.value(0).toString() );
    }
    QSqlQuery lowscore("select name, count(loserid) from match, team "
		       "where team.id=match.loserid "
		       "group by name "
		       "order by count(loserid) asc;" );
    while( lowscore.next() ){
	if( !teamNames.contains( lowscore.value(0).toString() ) ){
	    new QListViewItem( scorelist, lowscore.value(0).toString(), "0");
	    qDebug(  lowscore.value(0).toString() );
	}
    }    

    highscore.first();
    do {
	new QListViewItem( scorelist, highscore.value(0).toString(), 
			   highscore.value(1).toString() );
	teamNames.append( highscore.value(0).toString() );
	qDebug(  highscore.value(0).toString() );
    } while( highscore.next() );
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

    matchTable = new QSqlTable( tab );
    teamEditor = new TeamEditorWidget( tab );
    statWidget = new Statistics( tab );
    hallOfFameWidget = new HallOfFame( tab );

    tab->addTab( matchTable, "Matches" );
    tab->addTab( statWidget, "Statistics" );
    tab->addTab( hallOfFameWidget, "Hall of Fame" );
    tab->addTab( teamEditor, "Team editor" );

    setCentralWidget( tab );
    resize( 700, 400 );

    // Setup the initial match table
    matchView.select( matchView.index( "date" ) );
    matchTable->setConfirmEdits( TRUE );
    matchTable->setConfirmCancels( TRUE );
    matchTable->setCursor( &matchView, FALSE );
    matchTable->addColumn( matchView.field( "date" ) );
    matchTable->addColumn( matchView.field( "winner" ) );
    matchTable->addColumn( matchView.field( "winnerwins" ) );
    matchTable->addColumn( matchView.field( "loser" ) );
    matchTable->addColumn( matchView.field( "loserwins" ) );
    matchTable->addColumn( matchView.field( "sets" ) );
    matchTable->setSorting( TRUE );
    matchTable->setReadOnly( TRUE );
}

void PingPongApp::insertMatch()
{
     MatchDialog dlg( matchCursor.insertBuffer(), MatchDialog::Insert, this );
     if( dlg.exec() == QDialog::Accepted ){
 	matchCursor.insert();
 	matchTable->refresh();
     }
}

void PingPongApp::updateMatch()
{
    QSqlRecord r = matchTable->currentFieldSelection();
    if ( !r.count() )
	return;
    matchCursor.setValue( "id", r.value( "id" ) );
    matchCursor.select( matchCursor.primaryIndex(), matchCursor.primaryIndex() );
    if ( matchCursor.next() ) {
	MatchDialog dlg( matchCursor.updateBuffer(), MatchDialog::Update, this );
	if( dlg.exec() == QDialog::Accepted ){
	    matchCursor.update();
	    matchTable->refresh();
	}
    }
}

void PingPongApp::deleteMatch()
{
    QSqlRecord r = matchTable->currentFieldSelection();
    if ( !r.count() )
	return;
    
    matchCursor.setValue( "id", r.value( "id" ) );
    matchCursor.select( matchCursor.primaryIndex(), matchCursor.primaryIndex() );
    if ( matchCursor.next() ) {
	MatchDialog dlg( matchCursor.updateBuffer(), MatchDialog::Delete, this );
	if( dlg.exec() == QDialog::Accepted ){
	    matchCursor.del();
	    matchTable->refresh();
	}
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
