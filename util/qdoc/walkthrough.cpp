/*
  walkthrough.cpp
*/

#include <qfile.h>
#include <qfileinfo.h>
#include <qstring.h>
#include <qtextstream.h>

#include "codeprocessor.h"
#include "config.h"
#include "messages.h"
#include "walkthrough.h"

Walkthrough::Walkthrough( const Walkthrough& w )
    : plainlines( w.plainlines ), processedlines( w.processedlines ),
      walkloc( w.walkloc ), shutUp( w.shutUp )
{
}

Walkthrough& Walkthrough::operator=( const Walkthrough& w )
{
    plainlines = w.plainlines;
    processedlines = w.processedlines;
    walkloc = w.walkloc;
    shutUp = w.shutUp;
    return *this;
}

QString Walkthrough::start( const QString& filePath, const Resolver *resolver )
{
    return start( TRUE, filePath, resolver );
}

void Walkthrough::dontstart( const QString& filePath, const Resolver *resolver )
{
    start( FALSE, filePath, resolver );
}

QString Walkthrough::printline( const QString& substr, const Location& docLoc )
{
    return xline( substr, docLoc, QString("printline") );
}

QString Walkthrough::printto( const QString& substr, const Location& docLoc )
{
    return xto( substr, docLoc, QString("printto") );
}

QString Walkthrough::printuntil( const QString& substr, const Location& docLoc )
{
    return xuntil( substr, docLoc, QString("printuntil") );
}

void Walkthrough::skipline( const QString& substr, const Location& docLoc )
{
    xline( substr, docLoc, QString("skipline") );
}

void Walkthrough::skipto( const QString& substr, const Location& docLoc )
{
    xto( substr, docLoc, QString("skipto") );
}

void Walkthrough::skipuntil( const QString& substr, const Location& docLoc )
{
    xuntil( substr, docLoc, QString("skipuntil") );
}

QString Walkthrough::start( bool localLinks, const QString& fileName,
			    const Resolver *resolver )
{
    static QRegExp *trailingSpaces = 0;
    static QRegExp *manyNLsInARow = 0;
    static QRegExp *aname = 0;

    if ( trailingSpaces == 0 ) {
	trailingSpaces = new QRegExp( QString(" +\n") );
	manyNLsInARow = new QRegExp( QString("\n\n\n+") );
	aname = new QRegExp( QString("<a name=[^>]*></a>") );
    }

    *this = Walkthrough();

    QString filePath = config->findDepth( fileName, config->exampleDirList() );
    if ( filePath.isEmpty() ) {
	warning( 1, "Cannot find example file '%s'", fileName.latin1() );
	return QString::null;
    }

    QFile f( config->findDepth(fileName, config->exampleDirList()) );
    if ( !f.open(IO_ReadOnly) ) {
	warning( 1, "Cannot open example file '%s'", filePath.latin1() );
	return QString::null;
    }

    QTextStream t( &f );
    QString fullText = t.read();
    f.close();
    if ( fullText.isEmpty() ) {
	warning( 2, "Example file '%s' empty", filePath.latin1() );
	return QString::null;
    }

    /*
      Clean a bit.  This has to be done here, not later, because the plain lines
      and the processed lines have to be synchronized.

      We make sure no two empty lines occur in a row, because the old qdoc did
      that.  Maybe the source files should be fixed.
    */
    fullText.replace( *trailingSpaces, QChar('\n') );
    fullText.replace( *manyNLsInARow, QString("\n\n") );

    QString processedText = processCodeHtml( fullText, resolver, localLinks,
					     QFileInfo(f).dirPath() );

    plainlines = QStringList::split( QChar('\n'), fullText, TRUE );

    QString walkthroughText = processedText;

    /*
      Local links are nice, but not twice in the same HTML page (once in the
      \include and once in the walkthrough).
    */
    if ( localLinks )
	walkthroughText.replace( *aname, QString::null );

    processedlines = QStringList::split( QChar('\n'), walkthroughText, TRUE );

    QStringList::Iterator p = processedlines.begin();
    while ( p != processedlines.end() ) {
	(*p).prepend( QString("    ") );
	(*p).append( QChar('\n') );
	++p;
    }

    walkloc = Location( filePath );
    return processedText;
}

QString Walkthrough::xline( const QString& substr, const Location& docLoc,
			    const QString& command )
{
    QString subs = substr.simplifyWhiteSpace();
    QString s;

    if ( walkloc.filePath().isEmpty() ) {
	warning( 2, docLoc, "Command '\\%s %s' ignored", command.latin1(),
		 subs.latin1() );
	return s;
    }

    if ( !subs.isEmpty() )
	skipEmptyLines();

    if ( plainlines.isEmpty() ) {
	if ( !shutUp ) {
	    warning( 2, docLoc, "Command '\\%s %s' failed at end of '%s'",
		     command.latin1(), subs.latin1(),
		     walkloc.shortFilePath().latin1() );
	    shutUp = TRUE;
	}
    } else if ( plainlines.first().simplifyWhiteSpace().find(subs) == -1 ) {
	if ( !shutUp ) {
	    warning( 2, docLoc, "Command '\\%s %s' failed at line %d of '%s'",
		     command.latin1(), subs.latin1(), walkloc.lineNum(),
		     walkloc.shortFilePath().latin1() );
	    shutUp = TRUE;
	}
    } else {
	s = getNextLine();
	shutUp = FALSE;
    }
    return s;
}

QString Walkthrough::xto( const QString& substr, const Location& docLoc,
			  const QString& command ) 
{
    QString subs = substr.simplifyWhiteSpace();
    QString s;

    if ( walkloc.filePath().isEmpty() ) {
	warning( 2, docLoc, "Command '\\%s %s' ignored", command.latin1(),
		 subs.latin1() );
	return s;
    }

    if ( !subs.isEmpty() )
	skipEmptyLines();

    while ( !plainlines.isEmpty() ) {
	if ( plainlines.first().simplifyWhiteSpace().find(subs) != -1 )
	    return s;
	s += getNextLine();
    }
    if ( !shutUp ) {
	warning( 2, docLoc, "Command '\\%s %s' failed at end of '%s'",
		 command.latin1(), subs.latin1(),
		 walkloc.shortFilePath().latin1() );
	shutUp = TRUE;
    }
    return s;
}

QString Walkthrough::xuntil( const QString& substr, const Location& docLoc,
			     const QString& command ) 
{
    QString s = xto( substr, docLoc, command );
    s += getNextLine();
    return s;
}

void Walkthrough::skipEmptyLines()
{
    while ( !plainlines.isEmpty() &&
	    plainlines.first().simplifyWhiteSpace().isEmpty() )
	getNextLine();
}

QString Walkthrough::getNextLine()
{
    QString s;
    if ( !plainlines.isEmpty() ) {
	s = processedlines.first();
	plainlines.remove( plainlines.begin() );
	processedlines.remove( processedlines.begin() );
	walkloc.advance( '\n' );
    }
    return s;
}
