/*
  messages.cpp
*/

#include <qregexp.h>
#include <qtranslator.h>
#include <stdlib.h>

#include "config.h"
#include "messages.h"

QString Messages::programName;
QRegExp *Messages::spuriousRegExp = 0;

void Messages::initialize( const Config& config )
{
    programName = config.programName();

    QRegExp regExp = config.getRegExp( CONFIG_SPURIOUS );
    if ( regExp.isValid() ) {
	spuriousRegExp = new QRegExp( regExp );
    } else {
	Messages::warning( config.location(CONFIG_SPURIOUS),
			   Qdoc::tr("Invalid regular expression '%1'")
			   .arg(regExp.pattern()) );
    }
}

void Messages::terminate()
{
    delete spuriousRegExp;
    spuriousRegExp = 0;
}

/*!

*/
void Messages::information( const QString& message )
{
    printf( "%s\n", message.latin1() );
    fflush( stdout );
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
    emitMessage( FALSE, location, "Aborting", "" );
    exit( EXIT_FAILURE );
}

void Messages::internalError( const QString& hint )
{
    fatal( Location::null, Qdoc::tr("Internal error (%1)").arg(hint),
	   Qdoc::tr("There is a bug in %1. Seek advice from your local %2"
		    " guru.")
	   .arg(programName).arg(programName) );
}

void Messages::emitMessage( bool isWarning, const Location& location,
			    const QString& message, const QString& details )
{
    if ( spuriousRegExp != 0 && spuriousRegExp->exactMatch(message) )
	return;

    QString result = message;
    if ( !details.isEmpty() )
	result += "\n[" + details + "]";
    result.replace( QRegExp("\n"), "\n    " );
    if ( isWarning )
	result.prepend( Qdoc::tr("warning: ") );
    result.prepend( toString(location) );
    printf( "%s\n", result.latin1() );
    fflush( stdout );
}

QString Messages::toString( const Location& location )
{
    QString str;

    if ( location.isEmpty() ) {
	str = programName;
    } else {
	Location loc2 = location;
	loc2.pop();
	if ( !loc2.isEmpty() ) {
	    QString blah = Qdoc::tr( "In file included from " );
	    for ( ;; ) {
		str += blah + top( loc2 );
		loc2.pop();
		if ( loc2.isEmpty() )
		    break;
		str += Qdoc::tr( "," ) + "\n";
		blah.fill( ' ' );
	    }
	    str += Qdoc::tr( ":" ) + "\n";
	}
	str += top( location );
    }
    return str + ": ";
}

QString Messages::top( const Location& location )
{
    QString str = location.pathAndFileName();
    if ( location.lineNo() >= 1 )
	str += ":" + QString::number( location.lineNo() );
    return str;
}
