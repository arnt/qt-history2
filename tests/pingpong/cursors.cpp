#include "cursors.h"

#include <qdatetime.h>

MatchCursor::MatchCursor()
    : QSqlCursor( "match" )
{
    teamCr = new QSqlCursor( "team" );

    field("winnerid")->setVisible( FALSE );
    field("loserid")->setVisible( FALSE );

    field("loserwins")->setDisplayLabel( "Loser Wins" );
    field("winnerwins")->setDisplayLabel( "Winner Wins" );
    field("date")->setDisplayLabel( "Date" );

    // add lookup field
    QSqlField loser("loser", QVariant::String );
    loser.setDisplayLabel("Loser");
    append( loser );

    QSqlField winner("winner", QVariant::String );
    winner.setDisplayLabel("Winner");
    append( winner );
    
    QSqlField sets("sets", QVariant::Int );
    sets.setDisplayLabel("Sets");
    append( sets );

    setCalculated( loser.name(), TRUE );
    setCalculated( winner.name(), TRUE );
    setCalculated( sets.name(), TRUE );    
}

QVariant MatchCursor::calculateField( const QString& name )
{
    if ( name == "sets" ) 
	return QVariant( field("winnerwins")->value().toInt() + field("loserwins")->value().toInt() );
	
    if( name == "winner" )
	teamCr->setValue( "id", field("winnerid")->value() );
    else if( name == "loser" )
	teamCr->setValue( "id", field("loserid")->value() );
    else
	return QVariant( QString::null );

    teamCr->select( teamCr->primaryIndex(), teamCr->primaryIndex() );
    if( teamCr->next() )
	return teamCr->value( "name" );
    return QVariant( QString::null );
}

void MatchCursor::primeInsert( QSqlRecord* buf )
{
    QSqlQuery q;
    q.exec( "select nextval( 'matchid_sequence' );" );
    if ( q.next() )
	buf->setValue( "id", q.value(0) );
    buf->setValue( "date", QDate::currentDate() );
}

PlayerCursor::PlayerCursor()
    : QSqlCursor( "player" )
{
    field("name")->setDisplayLabel( "Player name" );
}

void PlayerCursor::primeInsert( QSqlRecord* buf )
{
    QSqlQuery q;
    q.exec( "select nextval( 'playerid_sequence' );" );
    if ( q.next() )
	buf->setValue( "id", q.value(0) );
}


Player2TeamCursor::Player2TeamCursor()
    : QSqlCursor( "player2team" )
{
    field("playerid")->setVisible( FALSE );
    field("teamid")->setVisible( FALSE );

    QSqlField f( "playername", QVariant::String );
    f.setDisplayLabel( "Player name" );
    append( f );
    setCalculated( f.name(), TRUE );
}

QVariant Player2TeamCursor::calculateField( const QString & name )
{
    if( name == "playername" ){
	QSqlQuery sql( "select name from player where id = " +
		       field("playerid")->value().toString() + ";");
	if( sql.next() ){
	    return sql.value( 0 );
	}
    }
    return QVariant( QString::null );
}

void Player2TeamCursor::primeInsert( QSqlRecord* buf )
{
    QSqlQuery q;
    q.exec( "select nextval( 'player2teamid_sequence' );" );
    if ( q.next() )
	buf->setValue( "id", q.value(0) );
}


TeamCursor::TeamCursor()
    : QSqlCursor( "team" )
{
    field("name")->setDisplayLabel( "Team name" );
}

void TeamCursor::primeInsert( QSqlRecord* buf )
{
    QSqlQuery q;
    q.exec( "select nextval( 'teamid_sequence' );" );
    if ( q.next() )
	buf->setValue( "id", q.value(0) );
}
