#include "widgets.h"

#include <qsqltable.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qfont.h>
#include <qpushbutton.h>
#include <qlistview.h>

//
// TeamPicker editor widget
//
TeamPicker::TeamPicker( QWidget * parent, const char * name )
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
    : TeamEditorBase( parent, name )
{
    teamCursor.select( teamCursor.index( "name" ) );
    teamTable->setCursor( &teamCursor, TRUE );
    teamTable->setSorting( TRUE );

    playerCursor.select( playerCursor.index( "name" ) );
    playerTable->setCursor( &playerCursor, TRUE );
    playerTable->setSorting( TRUE );

    player2teamTable->setCursor( &player2teamView, TRUE );
    player2teamTable->setReadOnly( TRUE );
    player2teamTable->setSorting( TRUE );

    QSqlRecord t = teamTable->currentFieldSelection();
    updateTeamMembers( &t );
}

void TeamEditor::refreshTables()
{
    teamTable->refresh();
    playerTable->refresh();
    player2teamTable->refresh();
}

void TeamEditor::updateTeamMembers( const QSqlRecord * record )
{
    player2teamTable->setFilter( "teamid = " +
                                 record->value( "id" ).toString() );
    player2teamTable->refresh();
    player2teamLabel->setText( "Players on <i>" +
                               teamCursor.value("name").toString() + "</i>");
}

void TeamEditor::addPlayer()
{
    QSqlRecord currentTeam   = teamTable->currentFieldSelection();
    QSqlRecord currentPlayer = playerTable->currentFieldSelection();
    if ( currentPlayer.isEmpty() || currentTeam.isEmpty() )
        return;

    QSqlQuery query( "select count(*) from player2team where teamid = " +
                     currentTeam.value("id").toString() + " and playerid = " +
                     currentPlayer.value("id").toString() + ";" );

    if( query.next() && (query.value(0).toInt() == 0) ){
        QSqlRecord * buf = player2teamCursor.primeInsert();
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
Statistics::Statistics( QWidget * parent, const char * name )
    : StatisticsBase( parent, name )
{
    update();
}

void Statistics::update()
{
    int numMatches = 0;
    int numWins    = 0;
    int numLosses  = 0;
    int numSets    = 0;
    int sets       = 0;
    int teamId = teamPicker->teamId();
    QString str;

    // Sets won
    QSqlQuery query( "select sum(winnerwins) from match where winnerid = " +
                     QString::number( teamId ) + ";" );
    if( query.next() )
        sets = query.value(0).toInt();
    // Remember the sets won in matches that has been lost
    query.exec( "select sum(loserwins) from match where loserid = " +
                QString::number( teamId ) + ";" );
    if( query.next() )
        sets += query.value(0).toInt();
    setsWon->setText( QString::number( sets ) );
    numSets = sets;

    // Matches won
    query.exec( "select count(*) from match where winnerid = " +
                QString::number( teamId ) + ";" );
    if( query.next() ){
        numWins = query.value(0).toInt();
        matchesWon->setText( query.value(0).toString() );
    } else
        matchesWon->setText( "0" );

    // Sets lost
    query.exec( "select sum(winnerwins) from match where loserid = " +
                QString::number( teamId ) + ";" );
    if( query.next() )
        sets = query.value(0).toInt();
    // Remember the sets lost in the matches that has been won
    query.exec( "select sum(loserwins) from match where winnerid = " +
                QString::number( teamId ) + ";" );
    if( query.next() )
        sets += query.value(0).toInt();
    setsLost->setText( QString::number( sets ) );
    numSets += sets;

    // Matches lost
    query.exec( "select count(*) from match where loserid = " +
                            QString::number( teamId ) + ";" );
    if( query.next() ){
        numLosses = query.value(0).toInt();
        matchesLost->setText( query.value(0).toString() );
    } else
        matchesLost->setText( "0" );

    // Percentage wins/losses
    query.exec( "select count(*) from match where winnerid = " +
                QString::number( teamId ) + " or loserid = " +
                QString::number( teamId ) + ";" );
    if( query.next() )
        numMatches = query.value(0).toInt();

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
    query.exec( "select name from team, match where loserid=" +
                QString::number( teamId ) +
                " and team.id=match.winnerid group by team.name "
                "order by count(winnerid) desc;" );
    if ( query.next() && query.value(0).toString().length() )
        hate->setText( query.value(0).toString() );
    else
        hate->setText( "None" );

    // Find out who the team has beat most times
    query.exec( "select name from team, match where winnerid=" +
                QString::number( teamId ) +
                " and team.id=match.loserid group by team.name "
                "order by count(loserid) desc;" );
    if ( query.next() && query.value(0).toString().length() )
        love->setText( query.value(0).toString() );
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
HighscoreList::HighscoreList( QWidget * parent, const char * name )
    : QWidget( parent, name )
{
    QGridLayout * g = new QGridLayout( this );
    g->setSpacing( 6 );
    g->setMargin( 11 );

    list = new QListView( this );
    list->addColumn( "No." );
    list->addColumn( "Team name" );
    list->addColumn( "Matches won" );
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


