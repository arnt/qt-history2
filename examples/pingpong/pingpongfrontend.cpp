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
    QPixmap icon( "pingpong.png" );
    setIcon( icon );

    // Toolbar
    QToolBar * toolbar = new QToolBar( this );
    insertResultAc = new QAction( "Insert result", QPixmap( "new.png" ),
                                  "&Insert result", CTRL+Key_I, this, 0 );
    connect( insertResultAc, SIGNAL( activated() ), SLOT( insertMatch() ) );
    insertResultAc->addTo( toolbar );

    updateResultAc = new QAction( "Update result", QPixmap( "edit.png" ),
                                  "&Update result", CTRL+Key_U, this, 0 );
    connect( updateResultAc, SIGNAL( activated() ), SLOT( updateMatch() ) );
    updateResultAc->addTo( toolbar );

    deleteResultAc = new QAction( "Delete result", QPixmap( "delete.png" ),
                                  "&Delete result", CTRL+Key_D, this, 0 );
    connect( deleteResultAc, SIGNAL( activated() ), SLOT( deleteMatch() ) );
    deleteResultAc->addTo( toolbar );

    // Menus
    QPopupMenu * menu = new QPopupMenu( this );

    insertResultAc->addTo( menu );
    updateResultAc->addTo( menu );
    deleteResultAc->addTo( menu );
    menu->insertSeparator();
    menu->insertItem( "Edit &Teams", this, SLOT( editTeams() ), CTRL+Key_T );
    menu->insertSeparator();
    menu->insertItem( "&Quit", qApp, SLOT( quit() ), CTRL+Key_Q );
    menuBar()->insertItem( "&File", menu );

    // Layout the central widget
    tab = new QTabWidget( this );
    connect( tab, SIGNAL( currentChanged( QWidget * ) ),
             SLOT( updateIcons( QWidget * ) ) );

    matchBase = new QWidget( tab );
    QGridLayout * matchLayout = new QGridLayout( matchBase );
    matchLayout->setSpacing( 6 );
    matchLayout->setMargin( 11 );
    matchTable = new QSqlTable( matchBase );
    matchLayout->addWidget( matchTable, 0, 0 );

    teamEditor = new TeamEditor( tab );
    statistics = new Statistics( tab );
    highscore  = new HighscoreList( tab );

    tab->addTab( matchBase, "Matches" );
    tab->addTab( statistics, "Statistics" );
    tab->addTab( highscore, "Hall of Fame" );
    tab->addTab( teamEditor, "Team editor" );

    connect( matchTable, SIGNAL( cursorChanged( QSqlCursor::Mode ) ),
             statistics, SLOT( update() ) );
    connect( matchTable, SIGNAL( cursorChanged( QSqlCursor::Mode ) ),
             highscore, SLOT( update() ) );
    setCentralWidget( tab );

    // Set up the initial match table
    matchView.select( matchView.index( "date" ) );
    matchTable->setCursor( &matchView, FALSE);
    matchTable->addColumn( "date" );
    matchTable->addColumn( "winner" );
    matchTable->addColumn( "winnerwins" );
    matchTable->addColumn( "loser" );
    matchTable->addColumn( "loserwins" );
    matchTable->addColumn( "sets" );
    matchTable->setSorting( TRUE );
    matchTable->setReadOnly( TRUE );
    matchTable->refresh();

    teamEditor->refreshTables();
}

void PingpongFrontEnd::insertMatch()
{
     MatchDialog dlg( matchCursor.primeInsert(), MatchDialog::Insert, this );
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
        matchCursor.primeUpdate();
        MatchDialog dlg( matchCursor.editBuffer(), MatchDialog::Update, this );
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
        matchCursor.primeUpdate();
        MatchDialog dlg( matchCursor.editBuffer(), MatchDialog::Delete, this );
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
    if( w == matchBase ){
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
