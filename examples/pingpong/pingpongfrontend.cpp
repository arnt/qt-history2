#include "pingpongfrontend.h"
#include "dialogs.h"
#include "cursors.h"
#include "widgets.h"

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
//  PingpongFrontEnd class
//
PingpongFrontEnd::PingpongFrontEnd( QWidget * parent, const char * name )
    : QMainWindow( parent, name )
{
    init();
}

void PingpongFrontEnd::init()
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
    teamEditor = new TeamEditor( tab );
    statistics = new Statistics( tab );
    highscore  = new HighscoreList( tab );

    tab->addTab( matchTable, "Matches" );
    tab->addTab( statistics, "Statistics" );
    tab->addTab( highscore, "Hall of Fame" );
    tab->addTab( teamEditor, "Team editor" );

    connect( matchTable, SIGNAL( cursorChanged( QSqlCursor::Mode ) ),
	     statistics, SLOT( update() ) );
    
    connect( matchTable, SIGNAL( cursorChanged( QSqlCursor::Mode ) ),
	     highscore, SLOT( update() ) );

    setCentralWidget( tab );
    resize( 700, 400 );

    // Set up the initial match table
    matchView.select( matchView.index( "date" ) );
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

void PingpongFrontEnd::insertMatch()
{
     MatchDialog dlg( matchCursor.insertBuffer(), MatchDialog::Insert, this );
     if( dlg.exec() == QDialog::Accepted ){
 	matchCursor.insert();
 	matchTable->refresh();
	highscore->update();
	statistics->update();
     }
}

void PingpongFrontEnd::updateMatch()
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
	    highscore->update();
	    statistics->update();
	}
    }
}

void PingpongFrontEnd::deleteMatch()
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
	    highscore->update();
	    statistics->update();
	}
    }
}

void PingpongFrontEnd::updateIcons( QWidget * w )
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

void PingpongFrontEnd::editTeams()
{
    if( tab->currentPage() != teamEditor ){
	tab->showPage( teamEditor );
    }
}
