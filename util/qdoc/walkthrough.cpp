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

static QString stripTrailingBlankLine( const QString& s )
{
    if ( s.right(2) == QString("\n\n") )
	return s.left( s.length() - 1 );
    else
	return s;
}

static ScoreMap scoreMapForOccurrenceMap( const OccurrenceMap& occMap )
{
    ScoreMap scores;
    OccurrenceMap::ConstIterator occsOnLine = occMap.begin();
    while ( occsOnLine != occMap.end() ) {
	StringSet::ConstIterator occ = (*occsOnLine).begin();
	while ( occ != (*occsOnLine).end() ) {
	    ScoreMap::Iterator sc = scores.find( *occ );
	    if ( sc == scores.end() )
		scores.insert( *occ, LineScore(occsOnLine.key(), 1) );
	    else
		scores.replace( *occ,
				LineScore((*sc).line(), (*sc).score() + 1) );
	    ++occ;
	}
	++occsOnLine;
    }
    return scores;
}

void Walkthrough::includePass1( const QString& fileName,
				const Resolver *resolver )
{
    QString t = start( TRUE, TRUE, fileName, resolver );
}

QString Walkthrough::includePass2( const QString& fileName,
				   const Resolver *resolver )
{
    QString t = start( TRUE, FALSE, fileName, resolver );
    return t;
}

void Walkthrough::startPass1( const QString& fileName,
			      const Resolver *resolver )
{
    start( FALSE, TRUE, fileName, resolver );
}

void Walkthrough::startPass2( const QString& fileName,
			      const Resolver *resolver )
{
    start( FALSE, FALSE, fileName, resolver );
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

QString Walkthrough::start( bool include, bool firstPass,
			    const QString& fileName, const Resolver *resolver )
{
    static QRegExp trailingSpacesPlusNL( QString("[ \t]+\n") );
    static QRegExp endOfLine( QString("\n(?!\n)") );
    static QRegExp manyNLs( QString("\n+") );
    static QRegExp aname( QString("<a name=[^>]*></a>") );

    QString filePath = config->findDepth( fileName, config->exampleDirList() );
    if ( justIncluded && filePath == fpath ) {
	/*
	  It's already started. This happens with \include followed
	  by \walkthrough. If we restarted again, we would loose the
	  local links.
	*/
	justIncluded = FALSE;
	return fancyText;
    }
    fpath = filePath;

    shutUp = !firstPass;

    if ( filePath.isEmpty() ) {
	if ( !shutUp )
	    warning( 1, "Cannot find example file '%s'", filePath.latin1() );
	return QString::null;
    }

    QFile f( config->findDepth(filePath, config->exampleDirList()) );
    if ( !f.open(IO_ReadOnly) ) {
	if ( !shutUp )
	    warning( 1, "Cannot open example file '%s'", filePath.latin1() );
	return QString::null;
    }

    QTextStream t( &f );
    QString code = t.read();
    f.close();
    if ( code.isEmpty() ) {
	if ( !shutUp )
	    warning( 2, "Example file '%s' empty", filePath.latin1() );
	return QString::null;
    }

    code.replace( trailingSpacesPlusNL, QChar('\n') );

    if ( firstPass ) {
	occMap = occurrenceMap( code, resolver, QFileInfo(f).dirPath() );
	scores = scoreMapForOccurrenceMap( occMap );
	fancyText = code;
    } else {
	fancyText = processCodeHtml( code, resolver, QFileInfo(f).dirPath(),
				     include );
    }
    justIncluded = include;

    /*
      Split the source code into logical lines. Empty lines are
      handled specially. If the source code is

	  p->alpha();
	  p->beta();

	  p->gamma();


	  p->delta();

      the plainlines list will contain four items:

	  p->alpha();
	  p->beta();\n
	  p->gamma();\n\n
	  p->delta();

      The '\n' are important for walkloc.
    */
    plainlines = QStringList::split( endOfLine, code, TRUE );

    QString walkthroughText = fancyText;

    /*
      Local links are nice, but not twice in the same HTML page (once in the
      \include and once in the walkthrough).
    */
    if ( include && !firstPass )
	walkthroughText.replace( aname, QString::null );

    fancylines = QStringList::split( endOfLine, walkthroughText, TRUE );

    /*
      Add a 4-space indent to walkthrough code, so that it stands out from the
      explanations, and squeeze blanks (cat -s).
    */
    QStringList::Iterator p = fancylines.begin();
    while ( p != fancylines.end() ) {
	(*p).prepend( QString("    ") );
	(*p).replace( manyNLs, QChar('\n') );
	(*p).append( QChar('\n') );
	++p;
    }

    walkloc = Location( filePath );
    return fancyText;
}

QString Walkthrough::xline( const QString& substr, const Location& docLoc,
			    const QString& command )
{
    QString subs = substr.simplifyWhiteSpace();
    QString s;

    if ( walkloc.filePath().isEmpty() ) {
	if ( !shutUp )
	    warning( 2, docLoc, "Command '\\%s %s' ignored", command.latin1(),
		     subs.latin1() );
	return s;
    }

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
	    warning( 2, walkloc,
		     "Text '%s' does not contain '%s' (see line %d of '%s')",
		     plainlines.first().simplifyWhiteSpace().latin1(),
		     subs.latin1(),
		     docLoc.lineNum(), docLoc.shortFilePath().latin1() );
	    shutUp = TRUE;
	}
    } else {
	s = getNextLine( docLoc );
	shutUp = FALSE;
    }
    return stripTrailingBlankLine( s );
}

QString Walkthrough::xto( const QString& substr, const Location& docLoc,
			  const QString& command )
{
    QString subs = substr.simplifyWhiteSpace();
    QString s;

    if ( walkloc.filePath().isEmpty() ) {
	if ( !shutUp )
	    warning( 2, docLoc, "Command '\\%s %s' ignored", command.latin1(),
		     subs.latin1() );
	return s;
    }

    while ( !plainlines.isEmpty() ) {
	if ( plainlines.first().simplifyWhiteSpace().find(subs) != -1 )
	    return stripTrailingBlankLine( s );
	s += getNextLine( docLoc );
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
    s += getNextLine( docLoc );
    return stripTrailingBlankLine( s );
}

QString Walkthrough::getNextLine( const Location& docLoc )
{
    QString s;

    if ( !shutUp && justIncluded ) {
	warning( 2, docLoc,
		 "Command '\\walkthrough' now needed after '\\include'" );
	shutUp = TRUE;
    }

    if ( !plainlines.isEmpty() ) {
	s = fancylines.first();

	int n = plainlines.first().contains( '\n' ) + 1;
	for ( int i = 0; i < n; i++ )
	    walkloc.advance( '\n' );

	plainlines.remove( plainlines.begin() );
	fancylines.remove( fancylines.begin() );
    }
    return stripTrailingBlankLine( s );
}
