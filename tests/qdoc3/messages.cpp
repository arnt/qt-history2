/*
  messages.cpp
*/

#include <qregexp.h>
#include <qtranslator.h>
#include <stdlib.h>

#include "messages.h"

/*!

*/
void Messages::information( const QString& message )
{
    fprintf( stderr, "%s\n", message.latin1() );
}

/*!

*/
void Messages::warning( const Location& location, const QString& message,
			const QString& details )
{
    emitMessage( TRUE, location, message, details );
}

/*!

*/
void Messages::error( const Location& location, const QString& message,
		      const QString& details )
{
    emitMessage( FALSE, location, message, details );
}

/*!

*/
void Messages::fatal( const Location& location, const QString& message,
		      const QString& details )
{
    emitMessage( FALSE, location, message, details );
    exit( EXIT_FAILURE );
}

void Messages::internalError( const QString& hint )
{
    fatal( Location::null, qdoc::tr("Internal error (%1)").arg(hint),
	   qdoc::tr("There is a bug in qdoc. Seek advice from your local qdoc"
		    " guru.") );
}

void Messages::emitMessage( bool isWarning, const Location& location,
			    const QString& message, const QString& details )
{
    QString result = message;
    if ( !details.isEmpty() )
	result += "\n(" + details + ")";
    result.replace( QRegExp("\n"), "\n    " );
    if ( isWarning )
	result.prepend( qdoc::tr("warning: ") );
    result.prepend( toString(location) );
    fprintf( stderr, "%s\n", result.latin1() );
}

QString Messages::toString( const Location& location )
{
    QString str;

    if ( location.isEmpty() ) {
	str = qdoc::tr( "qdoc" );
    } else {
	Location loc2 = location;
	loc2.pop();
	if ( !loc2.isEmpty() ) {
	    QString blah = qdoc::tr( "In file included from " );
	    for ( ;; ) {
		str += blah + top( loc2 );
		loc2.pop();
		if ( loc2.isEmpty() )
		    break;
		str += qdoc::tr( "," ) + "\n";
		blah.fill( ' ' );
	    }
	    str += qdoc::tr( ":" ) + "\n";
	}
	str += top( location );
    }
    return str + ": ";
}

QString Messages::top( const Location& location )
{
    QString str = location.pathAndFileName();
    if ( location.lineNo() > 0 )
	str += ":" + QString::number( location.lineNo() );
    return str;
}
