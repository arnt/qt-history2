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
    } else if ( cursor()->field(indexOf(col))->name() == "sets" ) {
	return;
    }

    QSqlTable::sortColumn( col, ascending, wholeRows );
}

Statistics::Statistics( QWidget * parent = 0, const char * name = 0 )
    : QFrame( parent, name )
{
    QVBoxLayout* b = new QVBoxLayout( this );
    list = new QListView( this );
    list->addColumn( " ", 30 );
    list->addColumn( " ", 30 );
    b->addWidget( list );
    refresh();
}

void Statistics::refresh()
{
    list->clear();
    QListViewItem* lvi = new QListViewItem( list, "some text", "more text" );
    list->insertItem( lvi );
}

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
    QToolBar * tbar = new QToolBar( this );
    QAction * a = new QAction( "Insert new result", QPixmap( "new.png" ),
		     QString::null, 0, this, 0 );
    connect( a, SIGNAL( activated() ), SLOT( insertMatch() ) );
    a->addTo( tbar );
    a = new QAction( "Update result", QPixmap( "edit.png" ), QString::null,
		     0, this, 0 );
    connect( a, SIGNAL( activated() ), SLOT( updateMatch() ) );
    a->addTo( tbar );
    a = new QAction( "Delete result", QPixmap( "delete.png" ), QString::null,
		     0, this, 0 );
    connect( a, SIGNAL( activated() ), SLOT( deleteMatch() ) );
    a->addTo( tbar );

    tbar = new QToolBar( this );
    a = new QAction( "Edit teams", QPixmap( "teamedit.xpm" ), QString::null,
		     0, this, 0 );
    connect( a, SIGNAL( activated() ), SLOT( editTeams() ) );
    a->addTo( tbar );

    QFrame * f1       = new QFrame( this );
    QVBoxLayout * vb1 = new QVBoxLayout( f1 );

    vb1->setMargin( 5 );
    vb1->setSpacing( 5 );

    // Layout the central widget
    QFont f = font();
    f.setBold( TRUE );

    QLabel * label = new QLabel( f1 );
    label->setText( "Matches" );
    label->setFont( f );
    QFontMetrics fm = label->fontMetrics();

    vb1->addWidget( label );

    matchTable = new MatchTable( f1 );
    vb1->addWidget( matchTable );

    setCentralWidget( f1 );
    resize( 700, 400 );

    // Setup initial match table
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

void PingPongApp::editTeams()
{
    EditTeamsDialog dlg( this );
    dlg.resize( 400, 300 );
    dlg.exec();
}
