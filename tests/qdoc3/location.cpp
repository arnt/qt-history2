/*
  location.cpp
*/

#include <qregexp.h>
#include <qtranslator.h>
#include <stdlib.h>

#include "config.h"
#include "location.h"

QT_STATIC_CONST_IMPL Location Location::null;

int Location::tabSize;
QString Location::programName;
QRegExp *Location::spuriousRegExp = 0;


/*!
  Constructs an empty location.
*/
Location::Location()
    : etcetera( FALSE )
{
}

/*!

*/
Location::Location( const QString& fileName )
    : etcetera( FALSE )
{
    push( fileName );
}

void Location::start()
{
    if ( stk.top().lineNo < 1 ) {
	stk.top().lineNo = 1;
	stk.top().columnNo = 1;
    }
}

/*!
  Advances the current location with character \a ch. If \a ch is
  '\\n' or '\\t', the line and column numbers are updated correctly.
*/
void Location::advance( QChar ch )
{
    if ( ch == '\n' ) {
	stk.top().lineNo++;
	stk.top().columnNo = 1;
    } else if ( ch == '\t' ) {
	stk.top().columnNo = 1 + ( (stk.top().columnNo + tabSize - 1) &
				   ~(tabSize - 1) );
    } else {
	stk.top().columnNo++;
    }
}

/*!
  Pushes \a pathAndFileName onto the file position stack. The current
  location becomes (\a pathAndFileName, 1, 1).

  \sa pop()
*/
void Location::push( const QString& pathAndFileName )
{
    StackEntry entry;
    entry.pathAndFileName = pathAndFileName;
    entry.lineNo = INT_MIN;
    entry.columnNo = 1;
    stk.push( entry );
}

/*!
  Pops the top of the internal stack. The current location becomes
  what it was just before the corresponding push().

  \sa push()
*/
void Location::pop()
{
    stk.pop();
}

/*! \fn bool Location::isEmpty() const

  Returns TRUE if there is no file name set yet; returns FALSE
  otherwise. The functions pathAndFileName(), lineNo() and columnNo()
  may only be called on non-empty objects.
*/

/*! \fn const QString& Location::pathAndFileName() const

  Returns the current path and file name.

  \sa lineNo(), columnNo()
*/

/*! \fn QString Location::fileName() const

  ###
*/

QString Location::fileName() const
{
    QString fn = pathAndFileName();
    return fn.mid( fn.findRev('/') + 1 );
}

/*! \fn int Location::lineNo() const

  Returns the current line number.

  \sa pathAndFileName(), columnNo()
*/

/*! \fn int Location::columnNo() const

  Returns the current column number.

  \sa pathAndFileName(), lineNo()
*/

void Location::warning( const QString& message, const QString& details ) const
{
    emitMessage( TRUE, message, details );
}

void Location::error( const QString& message, const QString& details ) const
{
    emitMessage( FALSE, message, details );
}

void Location::fatal( const QString& message, const QString& details ) const
{
    emitMessage( FALSE, message, details );
    information( "Aborting" );
    exit( EXIT_FAILURE );
}

/*!

*/
void Location::initialize( const Config& config )
{
    tabSize = config.getInt( CONFIG_TABSIZE );

    programName = config.programName();

    QRegExp regExp = config.getRegExp( CONFIG_SPURIOUS );
    if ( regExp.isValid() ) {
	spuriousRegExp = new QRegExp( regExp );
    } else {
	config.lastLocation().warning( tr("Invalid regular expression '%1'")
				       .arg(regExp.pattern()) );
    }
}

void Location::terminate()
{
    delete spuriousRegExp;
    spuriousRegExp = 0;
}

void Location::information( const QString& message )
{
    printf( "%s\n", message.latin1() );
    fflush( stdout );
}

void Location::internalError( const QString& hint )
{
    Location::null.fatal( tr("Internal error (%1)").arg(hint),
			  tr("There is a bug in %1. Seek advice from your local"
			     " %2 guru.")
			  .arg(programName).arg(programName) );
}

void Location::emitMessage( bool isWarning, const QString& message,
			    const QString& details ) const
{
    if ( spuriousRegExp != 0 && spuriousRegExp->exactMatch(message) )
	return;

    QString result = message;
    if ( !details.isEmpty() )
	result += "\n[" + details + "]";
    result.replace( QRegExp("\n"), "\n    " );
    if ( isWarning )
	result.prepend( tr("warning: ") );
    result.prepend( toString() );
    printf( "%s\n", result.latin1() );
    fflush( stdout );
}

QString Location::toString() const
{
    QString str;

    if ( isEmpty() ) {
	str = programName;
    } else {
	Location loc2 = *this;
	loc2.setEtc( FALSE );
	loc2.pop();
	if ( !loc2.isEmpty() ) {
	    QString blah = tr( "In file included from " );
	    for ( ;; ) {
		str += blah + loc2.top();
		loc2.pop();
		if ( loc2.isEmpty() )
		    break;
		str += tr( "," ) + "\n";
		blah.fill( ' ' );
	    }
	    str += tr( ":" ) + "\n";
	}
	str += top();
    }
    return str + ": ";
}

QString Location::top() const
{
    QString str = pathAndFileName();
    if ( lineNo() >= 1 ) {
	str += ":" + QString::number( lineNo() );
	if ( columnNo() >= 1 )
	    str += ":" + QString::number( columnNo() );
    }
    if ( etc() )
	str += " (etc.)";
    return str;
}
