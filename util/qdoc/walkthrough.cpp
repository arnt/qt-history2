/*
  walkthrough.cpp
*/

#include <qfile.h>
#include <qfileinfo.h>
#include <qstring.h>
#include <qtextstream.h>

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

HighScore& HighScore::operator=( const HighScore& hs )
{
    ininc = hs.ininc;
    ln = hs.ln;
    contri = hs.contri;
    tota = hs.tota;
    return *this;
}

void HighScore::addContribution( bool inInclude, int lineNo, int contribution )
{
    if ( contribution > contri ) {
	ininc = inInclude;
	ln = lineNo;
	contri = contribution;
    }
    tota += contribution;
}

void Walkthrough::includePass1( const QString& fileName,
				const Resolver *resolver )
{
    QString t = start( TRUE, TRUE, fileName, resolver, LinkMap() );
}

QString Walkthrough::includePass2( const QString& fileName,
				   const Resolver *resolver,
				   const LinkMap& exampleLinkMap )
{
    QString t = start( TRUE, FALSE, fileName, resolver, exampleLinkMap );
    return t;
}

void Walkthrough::startPass1( const QString& fileName,
			      const Resolver *resolver )
{
    start( FALSE, TRUE, fileName, resolver, LinkMap() );
}

void Walkthrough::startPass2( const QString& fileName,
			      const Resolver *resolver,
			      const LinkMap& exampleLinkMap )
{
    start( FALSE, FALSE, fileName, resolver, exampleLinkMap );
}

QString Walkthrough::printline( const QString& substr, const Location& docLoc )
{
    int lineNo0 = walkloc.lineNum();
    QString t = xline( substr, docLoc, QString("printline") );
    incrementScores( FALSE, lineNo0, 20 );
    return t;
}

QString Walkthrough::printto( const QString& substr, const Location& docLoc )
{
    int lineNo0 = walkloc.lineNum();
    QString t = xto( substr, docLoc, QString("printto") );
    for ( int i = lineNo0; i < walkloc.lineNum(); i++ )
	incrementScores( FALSE, i, 9 + 20 / (walkloc.lineNum() - lineNo0 + 1) );
    return t;
}

QString Walkthrough::printuntil( const QString& substr, const Location& docLoc )
{
    int lineNo0 = walkloc.lineNum();
    QString t = xuntil( substr, docLoc, QString("printuntil") );
    for ( int i = lineNo0; i < walkloc.lineNum(); i++ )
	incrementScores( FALSE, i, 9 + 20 / (walkloc.lineNum() - lineNo0 + 1) );
    return t;
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
			    const QString& fileName, const Resolver *resolver,
			    const LinkMap& exampleLinkMap )
{
    static QRegExp trailingSpacesPlusNL( QString("[ \t]+\n") );
    static QRegExp endOfLine( QString("\n(?!\n)") );
    static QRegExp manyNLs( QString("\n\n+") );
    static QRegExp aname( QString("<a name=[^>]*></a>") );

    fname = fileName;
    QString filePath = config->findDepth( fileName, config->exampleDirList() );
    if ( justIncluded && filePath == fpath ) {
	/*
	  It's already started. This happens with '\include' followed
	  by '\walkthrough'. Restarting would break local links.
	*/
	justIncluded = FALSE;
	return includeText;
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
	if ( include ) {
	    int numLines = code.contains( QChar('\n') );
	    for ( int i = 0; i < numLines; i++ )
		incrementScores( TRUE, i, 9 );
	}
	includeText = code;
    } else {
	includeText = processCodeHtml( code, resolver, QFileInfo(f).dirPath(),
				       include );
    }
    justIncluded = include;

    /*
      Split the source code into logical lines. Empty lines are
      treated specially. If the source code is

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

    QString walkthroughText = includeText;

    /*
      Local '<a name="...">'s are nice, but not in '\walkthrough's.
    */
    if ( include && !firstPass )
	walkthroughText.replace( aname, QString::null );

    if ( !firstPass ) {
	QString *text = include ? &includeText : &walkthroughText;

	// add '<a name="...">' as specified in the link map
	int lineNo = 1;
	int k = 0;
	LinkMap::ConstIterator links = exampleLinkMap.begin();
	while ( links != exampleLinkMap.end() ) {
	    while ( links.key() > lineNo ) {
		k = text->find( QChar('\n'), k ) + 1;
		lineNo++;
	    }
	    StringSet::ConstIterator link = (*links).begin();
	    while ( link != (*links).end() ) {
		text->insert( k, QString("<a name=\"%1\">").arg(*link) );
		++link;
	    }
	    ++links;
	}
    }

    fancylines = QStringList::split( endOfLine, walkthroughText, TRUE );

    if ( !firstPass ) {
	/*
	  Add a four-space indent to walkthrough code, so that it
	  stands out, and squeeze blanks (cat -s).
	*/
	QStringList::Iterator p = fancylines.begin();
	while ( p != fancylines.end() ) {
	    (*p).prepend( QString("    ") );
	    (*p).replace( manyNLs, QChar('\n') );
	    (*p).append( QChar('\n') );
	    ++p;
	}
    }

    walkloc = Location( filePath );
    return includeText;
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
    return s;
}

void Walkthrough::incrementScores( bool include, int lineNo, int contribution )
{
    StringSet occsOnLine = occMap[lineNo];
    StringSet::ConstIterator occ = occsOnLine.begin();
    while ( occ != occsOnLine.end() ) {
	scores[*occ].addContribution( include, lineNo, contribution );
	++occ;
    }
}
