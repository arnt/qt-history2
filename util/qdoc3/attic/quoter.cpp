/*
  quoter.cpp
*/

#include <qregexp.h>

#include "messages.h"
#include "quoter.h"

Quoter::Quoter()
    : verbose( TRUE ), splitPoint( "\n(?!\n|$)" ), manyEndls( "\n\n+" )
{
}

void Quoter::reset()
{
    verbose = TRUE;
    plainLines.clear();
    markedLines.clear();
    codeLocation = Location();
}

void Quoter::quoteFromFile( const QString& filePath, const QString& plainCode,
			    const QString& markedCode )
{
    verbose = TRUE;

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
    plainLines = QStringList::split( splitPoint, plainCode, TRUE );
    markedLines = QStringList::split( splitPoint, markedCode, TRUE );

    /*
      Squeeze blanks (cat -s).
    */
    QStringList::Iterator m = markedLines.begin();
    while ( m != markedLines.end() ) {
	(*m).replace( manyEndls, "\n" );
	(*m).append( "\n" );
	++m;
    }
    codeLocation = Location( filePath );
}

QString Quoter::quoteLine( const Location& docLocation, const QString& command,
			   const QString& pattern )
{
    if ( plainLines.isEmpty() ) {
	failedAtEnd( docLocation, command );
	return "";
    }

    if ( pattern.isEmpty() ) {
	warning( 2, docLocation, "Missing pattern after '\\%s'",
		 command.latin1() );
	return "";
    }

    if ( match(docLocation, pattern, plainLines.first()) ) {
	return getLine();
    } else {
	if ( verbose ) {
	    warning( 2, docLocation, "Command '\\%s' failed",
		     command.latin1() );
	    warning( 2, codeLocation, "(pattern '%s' didn't match here)",
		     pattern.latin1() );
	    verbose = FALSE;
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
	QString t = pattern.mid( 1, pattern.length() - 2 );
	if ( t.contains("(?:^|[^/])/") > 0 )
	    warning( 2, docLocation, "Unescaped '/' in regular expression '%s'",
		     t.latin1() );
	t.replace( QRegExp("\\\\/"), "/" );
	QRegExp rx( t );
	if ( verbose && !rx.isValid() ) {
	    warning( 2, docLocation, "Invalid regular expression '%s'",
		     t.latin1() );
	    verbose = FALSE;
	}
	return str.find( rx ) != -1 || trimWhiteSpace( str ).find( rx ) != -1;
    } else {
	return trimWhiteSpace( str ).find( trimWhiteSpace(pattern) ) != -1;
    }
}

void Quoter::failedAtEnd( const Location& docLocation, const QString& command )
{
    if ( verbose ) {
	if ( codeLocation.filePath().isEmpty() ) {
	    warning( 2, docLocation, "Missing '\\quotefromfile' before '\\%s'",
		     command.latin1() );
	} else {
	    warning( 2, docLocation, "Command '\\%s' failed at end of '%s'",
		     command.latin1(), codeLocation.shortFilePath().latin1() );
	}
	verbose = FALSE;
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
