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
    : stk( 0 ), stkTop( &stkBottom ), stkDepth( 0 ), etcetera( FALSE )
{
}

/*!

*/
Location::Location( const QString& fileName )
    : stk( 0 ), stkTop( &stkBottom ), stkDepth( 0 ), etcetera( FALSE )
{
    push( fileName );
}

Location::Location( const Location& other )
    : stk( 0 ), stkTop( &stkBottom ), stkDepth( 0 ), etcetera( FALSE )
{
    *this = other;
}

Location& Location::operator=( const Location& other )
{
    QValueStack<StackEntry> *oldStk = stk;

    stkBottom = other.stkBottom;
    if ( other.stk == 0 ) {
	stk = 0;
	stkTop = &stkBottom;
    } else {
	stk = new QValueStack<StackEntry>( *other.stk );
	stkTop = &stk->top();
    }
    stkDepth = other.stkDepth;
    etcetera = other.etcetera;
    delete oldStk;
    return *this;
}

void Location::start()
{
    if ( stkTop->lineNo < 1 ) {
	stkTop->lineNo = 1;
	stkTop->columnNo = 1;
    }
}

/*!
  Advances the current location with character \a ch. If \a ch is
  '\\n' or '\\t', the line and column numbers are updated correctly.
*/
void Location::advance( QChar ch )
{
    if ( ch == '\n' ) {
	stkTop->lineNo++;
	stkTop->columnNo = 1;
    } else if ( ch == '\t' ) {
	stkTop->columnNo = 1 + tabSize * ( stkTop->columnNo + tabSize - 1 )
			       / tabSize;
    } else {
	stkTop->columnNo++;
    }
}

/*!
  Pushes \a filePath onto the file position stack. The current
  location becomes (\a filePath, 1, 1).

  \sa pop()
*/
void Location::push( const QString& filePath )
{
    if ( stkDepth++ >= 1 ) {
	if ( stk == 0 )
	    stk = new QValueStack<StackEntry>;
	stk->push( StackEntry() );
	stkTop = &stk->top();
    }

    stkTop->filePath = filePath;
    stkTop->lineNo = INT_MIN;
    stkTop->columnNo = 1;
}

/*!
  Pops the top of the internal stack. The current location becomes
  what it was just before the corresponding push().

  \sa push()
*/
void Location::pop()
{
    if ( --stkDepth == 0 ) {
	stkBottom = StackEntry();
    } else {
	stk->pop();
	if ( stk->isEmpty() ) {
	    delete stk;
	    stk = 0;
	    stkTop = &stkBottom;
	} else {
	    stkTop = &stk->top();
	}
    }
}

/*! \fn bool Location::isEmpty() const

  Returns TRUE if there is no file name set yet; returns FALSE
  otherwise. The functions filePath(), lineNo() and columnNo()
  may only be called on non-empty objects.
*/

/*! \fn const QString& Location::filePath() const

  Returns the current path and file name.

  \sa lineNo(), columnNo()
*/

/*!
  ###
*/
QString Location::fileName() const
{
    QString fp = filePath();
    return fp.mid( fp.findRev('/') + 1 );
}

/*! \fn int Location::lineNo() const

  Returns the current line number.

  \sa filePath(), columnNo()
*/

/*! \fn int Location::columnNo() const

  Returns the current column number.

  \sa filePath(), lineNo()
*/

void Location::warning( const QString& message, const QString& details ) const
{
    emitMessage( Warning, message, details );
}

void Location::error( const QString& message, const QString& details ) const
{
    emitMessage( Error, message, details );
}

void Location::fatal( const QString& message, const QString& details ) const
{
    emitMessage( Error, message, details );
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

void Location::emitMessage( MessageType type, const QString& message,
			    const QString& details ) const
{
    if ( type == Warning && spuriousRegExp != 0
	 && spuriousRegExp->exactMatch(message) )
	return;

    QString result = message;
    if ( !details.isEmpty() )
	result += "\n[" + details + "]";
    result.replace( QRegExp("\n"), "\n    " );
    if ( type == Warning )
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
    QString str = filePath();
    if ( lineNo() >= 1 ) {
	str += ":" + QString::number( lineNo() );
	if ( columnNo() >= 1 )
	    str += ":" + QString::number( columnNo() );
    }
    if ( etc() )
	str += " (etc.)";
    return str;
}
