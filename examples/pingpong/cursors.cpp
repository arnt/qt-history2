#include "cursors.h"
#include <qdatetime.h>

//
// MatchView
//
MatchView::MatchView()
    : QSqlCursor( "matchview" )
{
    setDisplayLabel( "loserwins", "Loser Wins" );
    setDisplayLabel( "winnerwins", "Winner Wins" );
    setDisplayLabel( "date", "Date" );
    setDisplayLabel( "loser", "Loser" );
    setDisplayLabel( "winner", "Winner" );
    setDisplayLabel( "sets", "Sets" );
}

//
// Player2TeamView
//
Player2TeamView::Player2TeamView()
    : QSqlCursor( "player2teamview" )
{
    setDisplayLabel( "name", "Name" );    
    setVisible( "teamid", FALSE );
    setVisible( "id" , FALSE );
}

//
// MatchCursor
//
MatchCursor::MatchCursor()
    : QSqlCursor( "match" )
{
    setVisible( "winnerid", FALSE );
    setVisible( "loserid", FALSE );

    setDisplayLabel( "loserwins", "Loser Wins" );
    setDisplayLabel( "winnerwins", "Winner Wins" );
    setDisplayLabel( "date", "Date" );
}

void MatchCursor::primeInsert( QSqlRecord* buf )
{
    QSqlQuery query( "select nextval( 'matchid_sequence' );" );
    if ( query.next() )
	buf->setValue( "id", query.value(0) );
    buf->setValue( "date", QDate::currentDate() );
}

//
// PlayerCursor
//
PlayerCursor::PlayerCursor()
    : QSqlCursor( "player" )
{
    setDisplayLabel( "name", "Player name" );
}

void PlayerCursor::primeInsert( QSqlRecord* buf )
{
    QSqlQuery query( "select nextval( 'playerid_sequence' );" );
    if ( query.next() )
	buf->setValue( "id", query.value(0) );
}

//
// Player2TeamCursor
//
Player2TeamCursor::Player2TeamCursor()
    : QSqlCursor( "player2team" )
{
    setVisible( "playerid", FALSE );
    setVisible( "teamid", FALSE );

    QSqlField f( "playername", QVariant::String );
    append( f );
    setDisplayLabel( "playername", "Player name" );
    setCalculated( f.name(), TRUE );
}

QVariant Player2TeamCursor::calculateField( const QString & name )
{
    if( name == "playername" ){
	QSqlQuery query( "select name from player where id = " +
			 field("playerid")->value().toString() + ";");
	if( query.next() ){
	    return query.value( 0 );
	}
    }
    return QVariant( QString::null );
}

void Player2TeamCursor::primeInsert( QSqlRecord* buf )
{
    QSqlQuery query( "select nextval( 'player2teamid_sequence' );" );
    if ( query.next() )
	buf->setValue( "id", query.value(0) );
}

//
// TeamCursor
//
TeamCursor::TeamCursor()
    : QSqlCursor( "team" )
{
    setDisplayLabel( "name", "Team name" );
}

void TeamCursor::primeInsert( QSqlRecord* buf )
{
    QSqlQuery query( "select nextval( 'teamid_sequence' );" );
    if ( query.next() )
	buf->setValue( "id", query.value(0) );
}
