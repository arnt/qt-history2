#include "pingpongapp.h"
#include "dialogs.h"

#include <qlayout.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qsqltable.h>
#include <qmessagebox.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qwidget.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qfont.h>
#include <qframe.h>
#include <qsplitter.h>
#include <qapplication.h>
#include <qfiledialog.h>

MatchCursor::MatchCursor()
    : QSqlCursor( "match" )
{
    teamCr = new QSqlCursor( "team" );

    field("winnerid")->setVisible( FALSE );
    field("loserid")->setVisible( FALSE );

    field("losses")->setDisplayLabel( "Losses" );
    field("wins")->setDisplayLabel( "Wins" );
    field("date")->setDisplayLabel( "Date" );
    field("sets")->setDisplayLabel( "Sets" );
    
    // add lookup field

    QSqlField loser("loser", QVariant::String );
    loser.setDisplayLabel("Loser");
    loser.setCalculated( TRUE );
    prepend( loser );    

    QSqlField winner("winner", QVariant::String );
    winner.setDisplayLabel("Winner");
    winner.setCalculated( TRUE );
    prepend( winner );
}

QVariant MatchCursor::calculateField( uint fieldNumber )
{    
    qDebug("here? %s, %s", (const char *)field(0)->value().toString(), 
       (const char *)field(1)->value().toString());
    
    if( field( fieldNumber )->name() == "winner" ){ // Winner team
	teamCr->setValue( "id", field("winnerid")->value() );
    } else if( field( fieldNumber )->name() == "loser" ){ // Looser team
	teamCr->setValue( "id", field("loserid")->value() );
    }
    teamCr->select( teamCr->primaryIndex(), teamCr->primaryIndex() );
    if( teamCr->next() )
	return teamCr->value( "name" );
    else
	return QVariant( QString::null );
}

PingPongApp::PingPongApp( QWidget * parent, const char * name )
    : QMainWindow( parent, name )
{
    init();
}

void PingPongApp::init()
{
    setCaption( "PingPong Statistics" );

    // Setup menus
    QPopupMenu * menu = new QPopupMenu( this );
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

    matchTable = new QSqlTable( f1 );
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

    //
    // Set up the initial tables
    //
    matchCr.select( matchCr.primaryIndex() );

    // match table
    matchTable->setConfirmEdits( TRUE );
    matchTable->setConfirmCancels( TRUE );
    matchTable->setCursor( &matchCr );
}

void PingPongApp::insertMatch()
{
     QSqlCursor * cr = matchTable->cursor();

     GenericDialog dlg( cr->insertBuffer(), GenericDialog::Insert, this );
     if( dlg.exec() == QDialog::Accepted ){
 	cr->insert();
 	matchTable->refresh();
     }
}

void PingPongApp::updateMatch()
{
     QSqlCursor * cr = matchTable->cursor();

     UpdateMatchDialog dlg( cr->updateBuffer(), this );
     if( dlg.exec() == QDialog::Accepted ){
 	cr->update();
 	matchTable->refresh();
     }
}

void PingPongApp::deleteMatch()
{
     QSqlCursor * cr = matchTable->cursor();

     GenericDialog dlg( cr->updateBuffer(), GenericDialog::Delete, this );
     if( dlg.exec() == QDialog::Accepted ){
 	cr->del();
 	matchTable->refresh();
     }
}
