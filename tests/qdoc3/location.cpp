/*
  location.cpp
*/

#include "config.h"
#include "location.h"

QT_STATIC_CONST_IMPL Location Location::null;

static int tabSize;

/*!
  Constructs an empty location.
*/
Location::Location()
{
}

/*!

*/
Location::Location( const QString& fileName )
{
    push( fileName );
    stk.top().lineNo = INT_MIN;
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
    entry.lineNo = 1;
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

/*!

*/
void Location::initialize( const Config& config )
{
    tabSize = config.getInt( CONFIG_TABSIZE );
}
