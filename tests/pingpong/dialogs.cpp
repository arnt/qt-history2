#include "dialogs.h"

#include <qlayout.h>
#include <qsqlform.h>
#include <qwidget.h>
#include <qlabel.h>
#include <qfont.h>
#include <qpushbutton.h>
#include <qmainwindow.h>
#include <qlistview.h>
#include <qspinbox.h>
#include <qlineedit.h>

//
// TeamPicker editor widget
//
TeamPicker::TeamPicker( QWidget * parent = 0, const char * name = 0 )
    : QComboBox( parent, name )
{
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
// TeamEditor class
//
TeamEditor::TeamEditor( QWidget * parent, const char * name )
    : QWidget( parent, name )
{
    QGridLayout * g = new QGridLayout( this );

    g->setMargin( 5 );
    g->setSpacing( 5 );

    QFont f = font();
    f.setBold( TRUE );
    QLabel * label = new QLabel( "All teams", this );
    label->setFont( f );
    g->addWidget( label, 0, 0 );
    teamTable   = new QSqlTable( this );
    teamCursor.select( teamCursor.index( "name" ) );
    teamTable->setCursor( &teamCursor );
    teamTable->setSorting( TRUE );
    g->addWidget( teamTable, 1, 0 );
    connect( teamTable, SIGNAL( currentChanged( const QSqlRecord * ) ),
	     SLOT( updateTeamMembers( const QSqlRecord * ) ) );

    label = new QLabel( "All players", this );
    label->setFont( f );
    g->addWidget( label, 0, 1 );
    playerTable = new QSqlTable( this );
    playerCursor.select( playerCursor.index( "name" ) );
    playerTable->setCursor( &playerCursor );
    playerTable->setSorting( TRUE );
    g->addWidget( playerTable, 1, 1 );

    player2teamLabel = new QLabel( "Players on ?", this );
    player2teamLabel->setFont( f );
    g->addMultiCellWidget( player2teamLabel, 2, 2, 0, 1 );
    player2teamTable = new QSqlTable( this );
    player2teamTable->setCursor( &player2teamView );
    player2teamTable->setReadOnly( TRUE );
    player2teamTable->setSorting( TRUE );
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

    QSqlRecord t = teamTable->currentFieldSelection();
    updateTeamMembers( &t );
}

void TeamEditor::updateTeamMembers( const QSqlRecord * record )
{
    player2teamView.select( "teamid = " + record->value( "id" ).toString());
    player2teamTable->refresh();
    player2teamLabel->setText( "Players on <i>" +
			       teamCursor.value("name").toString() +
			       "</i>");
}

void TeamEditor::addPlayer()
{
    QSqlRecord currentTeam = teamTable->currentFieldSelection();
    QSqlRecord currentPlayer = playerTable->currentFieldSelection();
    if ( currentPlayer.isEmpty() || currentTeam.isEmpty() )
	return;

    QSqlQuery sql( "select count(*) from player2team where teamid = " +
		   currentTeam.value("id").toString() + " and playerid = " +
		   currentPlayer.value("id").toString() + ";" );

    if( sql.next() && (sql.value(0).toInt() == 0) ){
	QSqlRecord * buf = player2teamCursor.insertBuffer();
	buf->setValue( "teamid", currentTeam.value("id") );
	buf->setValue( "playerid", currentPlayer.value("id") );
	player2teamCursor.insert();
	player2teamTable->refresh();
    }
}

void TeamEditor::removePlayer()
{
    QSqlRecord r = player2teamTable->currentFieldSelection();
    if ( r.isEmpty() )
	return;
    player2teamCursor.setValue( "id", r.value( "id" ) );
    player2teamCursor.select( player2teamCursor.primaryIndex(), player2teamCursor.primaryIndex() );
    if ( player2teamCursor.next() ) {
	player2teamCursor.del();
	player2teamTable->refresh();
    }
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
    matchesLost = new QLabel( "0", this );
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

    connect( teamPicker, SIGNAL( activated( int ) ), SLOT( update() ) );
    update();
}

void Statistics::update()
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
    } else
	matchesLost->setText( "0" );

    // Percentage wins/losses
    QSqlQuery qp( "select count(*) from match where winnerid = " +
		  QString::number( teamId ) + " or loserid = " +
		  QString::number( teamId ) + ";" );
    if( qp.next() )
	numMatches = qp.value(0).toInt();

    if( numMatches > 0 ){
	winPercentage->setText( QString::number( (numWins*100)/numMatches )
				+" %" );
	lossPercentage->setText( QString::number( (numLosses*100)/numMatches )
				 +" %" );
    } else {
	winPercentage->setText( "0 %" );
	lossPercentage->setText( "0 %" );
    }

    // Find out who the team has lost most matches to
    QSqlQuery qwids( "select name from team, match where loserid=" +
		     QString::number( teamId ) + 
		     " and team.id=match.winnerid group by team.name "
		     "order by count(winnerid) desc;" );
    if ( qwids.next() && qwids.value(0).toString().length() )
	hate->setText( qwids.value(0).toString() );
    else
	hate->setText( "None" );

    // Find out who the team has beat most times
    QSqlQuery qlids( "select name from team, match where winnerid=" +
		     QString::number( teamId ) + 
		     " and team.id=match.loserid group by team.name "
		     "order by count(loserid) desc;" );
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
//  HighscoreList class
//
HighscoreList::HighscoreList( QWidget * parent = 0, const char * name = 0 )
    : QWidget( parent, name )
{
    QGridLayout * g = new QGridLayout( this );
    g->setSpacing( 5 );
    g->setMargin( 5 );
    
    list = new QListView( this );
    list->addColumn( "No." );
    list->addColumn( "Team name" );
    list->addColumn( "Number of wins" );
    list->setColumnAlignment( 0, AlignCenter );
    list->setColumnAlignment( 2, AlignCenter );
    list->setColumnAlignment( 3, AlignCenter );
    list->setSorting( -1 );
    list->setAllColumnsShowFocus( TRUE );
    g->addWidget( list, 0, 0 );
    
    update();
}

void HighscoreList::update()
{        
    // Generate a highscore list
    //
    // The 'winners' query will generate a list of the teams that have
    // won at least one game or more, sorted on the number of matches
    // that team has won.
    // The 'losers' query will generate a list of all the teams that have
    // lost at least one game or more, sorted on the number of losses.
    // The highscore list is a combination of these two queries.

    QSqlQuery winners("select name, count(winnerid) from match, team "
		      "where team.id=match.winnerid "
		      "group by name "
		      "order by count(winnerid) asc;" );

    QSqlQuery losers("select name, count(loserid) from match, team "
		     "where team.id=match.loserid "
		     "group by name "
		     "order by count(loserid) asc;" );

    QStringList teamNames;
    
    list->clear();
    while( winners.next() ){
	teamNames.append( winners.value(0).toString() );
    }
    while( losers.next() ){
	if( !teamNames.contains( losers.value(0).toString() ) ){
	    new QListViewItem( list, "", losers.value(0).toString(), "0" );
	}
    }
    winners.first();
    do {
	new QListViewItem( list, "", winners.value(0).toString(), 
			   winners.value(1).toString() );
    } while( winners.next() );
    
    // Generate entry numbers
    int i = 1;
    QListViewItem * item = list->firstChild();
    while( item ){
	item->setText( 0, QString::number( i++ ) );
	item = item->nextSibling();
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

    flabel = new QLabel( buf->displayLabel( "winner" ), w );
    wteam = new TeamPicker( w );
    wteam->setTeamId( buf->value("winnerid").toInt() );
    formLayout->addWidget( flabel, 0, 0 );
    formLayout->addWidget( wteam, 0, 1 );
    form->associate( wteam, buf->field("winnerid") );

    flabel = new QLabel( buf->displayLabel("loser"), w );
    lteam = new TeamPicker( w );
    lteam->setTeamId( buf->value("loserid").toInt() );
    formLayout->addWidget( flabel, 0, 2 );
    formLayout->addWidget( lteam, 0, 3 );
    form->associate( lteam, buf->field("loserid") );

    flabel = new QLabel( buf->displayLabel("winnerwins"), w );
    wins = new QSpinBox( w );
    formLayout->addWidget( flabel, 1, 0 );
    formLayout->addWidget( wins, 1, 1 );
    form->associate( wins, buf->field("winnerwins") );
    connect( wins, SIGNAL( valueChanged(int) ), SLOT( updateSets() ) );

    flabel = new QLabel( buf->displayLabel("loserwins"), w );
    losses = new QSpinBox( w );
    formLayout->addWidget( flabel, 1, 2 );
    formLayout->addWidget( losses, 1, 3 );
    form->associate( losses, buf->field("loserwins") );
    connect( losses, SIGNAL( valueChanged(int) ), SLOT( updateSets() ) );

    flabel = new QLabel( buf->displayLabel("date"), w );
    editor = ef->createEditor( w, buf->value("date") );
    formLayout->addWidget( flabel, 2, 0 );
    formLayout->addWidget( editor, 2, 1 );
    form->associate( editor, buf->field("date") );

    flabel = new QLabel( buf->displayLabel("sets"), w );
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

