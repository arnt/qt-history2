#include "dialogs.h"

#include <qlayout.h>
#include <qsqlform.h>
#include <qwidget.h>
#include <qlabel.h>
#include <qfont.h>
#include <qpushbutton.h>
#include <qmainwindow.h>

//
// TeamPicker editor widget
//
TeamPicker::TeamPicker( QWidget * parent = 0, const char * name = 0 )
    : QComboBox( parent, name )
{
    //    setFocusPolicy( StrongFocus );
    QSqlCursor team( "team" );
    team.select( team.index("name") );
    int idx = 0;
    while( team.next() ) {
	insertItem( team.value("name").toString(), idx );
	index2Id[idx] = team.value("id").toInt();
	idx++;
    }
}

int TeamPicker::teamId() const
{
    return index2Id[ currentItem() ];
}

void TeamPicker::setTeamId( int id )
{
    QMap<int,int>::Iterator it;
    for( it = index2Id.begin(); it != index2Id.end(); ++it ) {
	if ( it.data() == id ) {
	    setCurrentItem( it.key() );
	    break;
	}
    }
}

//
//  MatchDialog class
//

MatchDialog::MatchDialog( QSqlRecord* buf, Mode mode, QWidget * parent,
			  const char * name )
    : QDialog( parent, name, TRUE ),
      matchRecord( buf ),
      mMode( mode )
{
    QWidget *     w = new QWidget( this );
    QVBoxLayout * g = new QVBoxLayout( this );
    QHBoxLayout * h = new QHBoxLayout;

    QString op;
    switch ( mMode ) {
    case Insert:
	setCaption( "Insert Match Result" );
	op = "&Insert";
	break;
    case Update:
	setCaption( "Update Match Result" );
	op = "&Update";
	break;
    case Delete:
	setCaption( "Delete Match Result" );
	op = "&Delete";
	break;
    }

    // Lay out the editor widgets manually
    QSqlEditorFactory * ef = QSqlEditorFactory::defaultFactory();
    QWidget * editor;
    QLabel * flabel;
    QGridLayout * formLayout = new QGridLayout( w );

    formLayout->setSpacing( 5 );
    formLayout->setMargin( 5 );

    form = new QSqlForm( this, "matchform" );

    QSqlPropertyMap* pm = new QSqlPropertyMap();
    pm->insert( "TeamPicker", "teamid" );
    form->installPropertyMap( pm );

    flabel = new QLabel( buf->field("winner")->displayLabel(), w );
    wteam = new TeamPicker( w );
    wteam->setTeamId( buf->value("winnerid").toInt() );
    formLayout->addWidget( flabel, 0, 0 );
    formLayout->addWidget( wteam, 0, 1 );
    form->associate( wteam, buf->field("winnerid") );

    flabel = new QLabel( buf->field("loser")->displayLabel(), w );
    lteam = new TeamPicker( w );
    lteam->setTeamId( buf->value("loserid").toInt() );
    formLayout->addWidget( flabel, 0, 2 );
    formLayout->addWidget( lteam, 0, 3 );
    form->associate( lteam, buf->field("loserid") );

    flabel = new QLabel( buf->field("winnerwins")->displayLabel(), w );
    wins = new QSpinBox( w );
    formLayout->addWidget( flabel, 1, 0 );
    formLayout->addWidget( wins, 1, 1 );
    form->associate( wins, buf->field("winnerwins") );
    connect( wins, SIGNAL( valueChanged(int) ),
	     SLOT( updateSets() ) );

    flabel = new QLabel( buf->field("loserwins")->displayLabel(), w );
    losses = new QSpinBox( w );
    formLayout->addWidget( flabel, 1, 2 );
    formLayout->addWidget( losses, 1, 3 );
    form->associate( losses, buf->field("loserwins") );
    connect( losses, SIGNAL( valueChanged(int) ),
	     SLOT( updateSets() ) );

    flabel = new QLabel( buf->field("date")->displayLabel(), w );
    editor = ef->createEditor( w, buf->value("date") );
    formLayout->addWidget( flabel, 2, 0 );
    formLayout->addWidget( editor, 2, 1 );
    form->associate( editor, buf->field("date") );

    flabel = new QLabel( buf->field("sets")->displayLabel(), w );
    sets = new QLineEdit( w );
    formLayout->addWidget( flabel, 2, 2 );
    formLayout->addWidget( sets, 2, 3 );
    sets->setEnabled( FALSE );
    form->readRecord();

    g->setMargin( 3 );

    QLabel * label = new QLabel( caption(), this );
    QFont f = font();
    f.setBold( TRUE );
    label->setFont( f );
    g->addWidget( label );

    h->addItem( new QSpacerItem( 0, 0, QSizePolicy::Expanding,
				 QSizePolicy::Minimum ) );

    QPushButton * button = new QPushButton( op, this );
    button->setDefault( TRUE );
    connect( button, SIGNAL( clicked() ), SLOT( execute() ) );
    h->addWidget( button );

    button = new QPushButton( "&Close", this );
    connect( button, SIGNAL( clicked() ), SLOT( close() ) );
    h->addWidget( button );

    if ( mMode == Delete )
	w->setEnabled( FALSE );
    updateSets();
    g->addWidget( w );
    g->addLayout( h );
}

void MatchDialog::close()
{
    reject();
}

void MatchDialog::execute()
{
    form->writeRecord();
    accept();
}

void MatchDialog::updateSets()
{
    matchRecord->setValue( "sets", wins->value() + losses->value() );
    sets->setText( matchRecord->value( "sets" ).toString() );
}



//
// EditTeamsDialog class
//
EditTeamsDialog::EditTeamsDialog( QWidget * parent, const char * name )
    : QDialog( parent, name, TRUE )
{
    setCaption( "Team Editor" );

    QGridLayout * g = new QGridLayout( this );

    g->setMargin( 5 );
    g->setSpacing( 5 );

    QFont f = font();
    f.setBold( TRUE );
    QLabel * label = new QLabel( "All teams", this );
    label->setFont( f );
    g->addWidget( label, 0, 0 );
    teamTable   = new QSqlTable( this );
    teamTable->setCursor( &teamCursor );
    g->addWidget( teamTable, 1, 0 );
    connect( teamTable, SIGNAL( currentChanged( const QSqlRecord * ) ),
	     SLOT( updateTeamMembers( const QSqlRecord * ) ) );

    label = new QLabel( "All players", this );
    label->setFont( f );
    g->addWidget( label, 0, 1 );
    playerTable = new QSqlTable( this );
    playerTable->setCursor( &playerCursor );
    g->addWidget( playerTable, 1, 1 );

    player2teamLabel = new QLabel( "Players on ?", this );
    player2teamLabel->setFont( f );
    g->addMultiCellWidget( player2teamLabel, 2, 2, 0, 1 );
    player2teamTable = new QSqlTable( this );
    player2teamTable->setCursor( &player2teamCursor );
    player2teamTable->setReadOnly( TRUE );
    g->addWidget( player2teamTable, 3, 0 );
    QFrame * buttonFrame = new QFrame( this );
    QVBoxLayout * v = new QVBoxLayout( buttonFrame );

    QPushButton * button = new QPushButton( "<< &Add",
					    buttonFrame );
    connect( button, SIGNAL( clicked() ), SLOT( addPlayer() ) );
    v->addWidget( button );

    button = new QPushButton( ">> &Remove", buttonFrame );
    connect( button, SIGNAL( clicked() ), SLOT( removePlayer() ) );
    v->addWidget( button );
    v->addItem( new QSpacerItem( 0, 0, QSizePolicy::Expanding,
				 QSizePolicy::Expanding ) );
    g->addWidget( buttonFrame, 3, 1 );

    buttonFrame = new QFrame( this );
    QHBoxLayout * h = new QHBoxLayout( buttonFrame );
    h->addItem( new QSpacerItem( 0, 0, QSizePolicy::Expanding,
				 QSizePolicy::Minimum ) );

    button = new QPushButton( "&Close", buttonFrame );
    button->setDefault( TRUE );
    connect( button, SIGNAL( clicked() ), SLOT( close() ) );
    h->addWidget( button );
    h->addItem( new QSpacerItem( 0, 0, QSizePolicy::Expanding,
				 QSizePolicy::Minimum ) );

    g->addMultiCellWidget( buttonFrame, 4, 4, 0, 1 );
}

void EditTeamsDialog::updateTeamMembers( const QSqlRecord * record )
{
    player2teamCursor.select( "teamid = " + record->value( "id" ).toString());
    player2teamTable->refresh();
    player2teamLabel->setText( "Players on " +
			       teamCursor.value("name").toString() );
}

void EditTeamsDialog::addPlayer()
{
    QSqlQuery sql( "select count(*) from player2team where teamid = " +
		   teamCursor.value("id").toString() + " and playerid = " +
		   playerCursor.value("id").toString() + ";" );

    if( sql.next() && (sql.value(0).toInt() == 0) ){
	QSqlRecord * buf = player2teamCursor.insertBuffer();
	buf->setValue( "teamid", teamCursor.value("id") );
	buf->setValue( "playerid", playerCursor.value("id") );
	player2teamCursor.insert();
	player2teamTable->refresh();
    }
}

void EditTeamsDialog::removePlayer()
{
    // ### Review this!
    player2teamCursor.del();
    player2teamTable->refresh();
}
