/*
  quoter.cpp
*/

#include <qregexp.h>

#include "quoter.h"

Quoter::Quoter()
    : silent( FALSE ), splitPoint( "\n(?!\n|$)" ), manyEndls( "\n\n+" )
{
}

void Quoter::reset()
{
    silent = FALSE;
    plainLines.clear();
    markedLines.clear();
    codeLocation = Location::null;
}

void Quoter::quoteFromFile( const QString& userFriendlyFilePath,
			    const QString& plainCode,
			    const QString& markedCode )
{
    silent = FALSE;

    /*
      Split the source code into logical lines. Empty lines are
      treated specially. Before:

	  p->alpha();
	  p->beta();

	  p->gamma();


	  p->delta();

      After:

	  p->alpha();
	  p->beta();\n
	  p->gamma();\n\n
	  p->delta();

      Newlines are preserved because they affect codeLocation.
    */
    plainLines = plainCode.split(splitPoint);
    markedLines = markedCode.split(splitPoint);

    /*
      Squeeze blanks (cat -s).
    */
    QStringList::Iterator m = markedLines.begin();
    while ( m != markedLines.end() ) {
	(*m).replace( manyEndls, "\n" );
	(*m).append( "\n" );
	++m;
    }
    codeLocation = Location( userFriendlyFilePath );
    codeLocation.start();
}

QString Quoter::quoteLine( const Location& docLocation, const QString& command,
			   const QString& pattern )
{
    if ( plainLines.isEmpty() ) {
	failedAtEnd( docLocation, command );
	return "";
    }

    if ( pattern.isEmpty() ) {
	docLocation.warning( tr("Missing pattern after '\\%1'").arg(command) );
	return "";
    }

    if ( match(docLocation, pattern, plainLines.first()) ) {
	return getLine();
    } else {
	if ( !silent ) {
	    docLocation.warning( tr("Command '\\%1' failed").arg(command) );
	    codeLocation.warning( tr("Pattern '%1' didn't match here")
				  .arg(pattern) );
	    silent = TRUE;
	}
	return "";
    }
}

QString Quoter::quoteTo( const Location& docLocation, const QString& command,
			 const QString& pattern )
{
    QString t;

    if ( pattern.isEmpty() ) {
	while ( !plainLines.isEmpty() )
	    t += getLine();
    } else {
	while ( !plainLines.isEmpty() ) {
	    if ( match(docLocation, pattern, plainLines.first()) )
		return t;
	    t += getLine();
	}
	failedAtEnd( docLocation, command );
    }
    return t;
}

QString Quoter::quoteUntil( const Location& docLocation, const QString& command,
			    const QString& pattern )
{
    QString t = quoteTo( docLocation, command, pattern );
    t += getLine();
    return t;
}

QString Quoter::getLine()
{
    if ( plainLines.isEmpty() )
	return "";

    QString t = markedLines.first();
    int n = t.contains( '\n' );
    for ( int i = 0; i < n; i++ )
	codeLocation.advance( '\n' );
    plainLines.remove( plainLines.begin() );
    markedLines.remove( markedLines.begin() );
    return t;
}

bool Quoter::match( const Location& docLocation, const QString& pattern,
		    const QString& line )
{
    QString str = line;
    while ( str.endsWith("\n") )
	str.truncate( str.length() - 1 );

    if ( pattern.startsWith("/") && pattern.endsWith("/") &&
	 pattern.length() > 2 ) {
	QRegExp rx( pattern.mid(1, pattern.length() - 2) );
	if ( !silent && !rx.isValid() ) {
	    docLocation.warning( tr("Invalid regular expression '%1'")
				 .arg(rx.pattern()) );
	    silent = TRUE;
	}
	return str.indexOf( rx ) != -1 || trimWhiteSpace( str ).indexOf( rx ) != -1;
    } else {
	return trimWhiteSpace( str ).indexOf( trimWhiteSpace(pattern) ) != -1;
    }
}

void Quoter::failedAtEnd( const Location& docLocation, const QString& command )
{
    if (!silent && !command.isEmpty()) {
	if ( codeLocation.filePath().isEmpty() ) {
	    docLocation.warning( tr("Unexpected '\\%1'").arg(command) );
	} else {
	    docLocation.warning( tr("Command '\\%1' failed at end of file '%2'")
				 .arg(command).arg(codeLocation.filePath()) );
	}
	silent = TRUE;
    }
}

QString Quoter::fix( const QString& str )
{
    QString t = str;
    while ( t.endsWith("\n\n") )
	t.truncate( t.length() - 1 );
    return t;
}

/*
  Transforms 'int x = 3 + 4' into 'int x=3+4'. A white space is kept
  between 'int' and 'x' because it is meaningful in C++.
*/
QString Quoter::trimWhiteSpace( const QString& str )
{
    enum { Normal, MetAlnum, MetSpace } state = Normal;
    QString t;

    for ( int i = 0; i < (int) str.length(); i++ ) {
	if ( str[i].isSpace() ) {
	    if ( state == MetAlnum )
		state = MetSpace;
	} else {
	    if ( str[i].isLetterOrNumber() ) {
		if ( state == Normal ) {
		    state = MetAlnum;
		} else {
		    if ( state == MetSpace )
			t += ' ';
		    state = Normal;
		}
	    } else {
		state = Normal;
	    }
	    t += str[i];
	}
    }
    return t;
}
