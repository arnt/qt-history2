/*
  walkthrough.cpp
*/

#include <qfile.h>
#include <qfileinfo.h>
#include <qstring.h>
#include <qtextstream.h>

#include <stdlib.h>

#include "config.h"
#include "messages.h"
#include "walkthrough.h"

static QString classPart( const QString& link )
{
    int k = link.find( QChar('#') );
    if ( k == -1 )
	return link;
    else
	return link.left( k );
}

/*
  Transforms 'int x = 3 + 4' into 'int x=3+4'. A white space is kept
  between 'int' and 'x' because it is meaningful in C++.
*/
static QString killWhiteSpace( const QString& s )
{
    QString t;
    enum { Normal, MetAlnum, MetSpace } state = Normal;

    for ( int i = 0; i < (int) s.length(); i++ ) {
	if ( s[i].isSpace() ) {
	    if ( state == MetAlnum )
		state = MetSpace;
	} else {
	    if ( s[i].isLetterOrNumber() ) {
		if ( state == Normal ) {
		    state = MetAlnum;
		} else {
		    if ( state == MetSpace )
			t += QChar( ' ' );
		    state = Normal;
		}
	    } else {
		state = Normal;
	    }
	    t += s[i];
	}
    }
    return t;
}

// skip trailing blank line
static QString fix( const QString& s )
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
    start( TRUE, TRUE, fileName, resolver, LinkMap(), LinkMap() );
}

QString Walkthrough::includePass2( const QString& fileName,
				   const Resolver *resolver,
				   const LinkMap& includeLinkMap,
				   const LinkMap& walkthroughLinkMap )
{
    return start( TRUE, FALSE, fileName, resolver, includeLinkMap,
		  walkthroughLinkMap );
}

void Walkthrough::startPass1( const QString& fileName,
			      const Resolver *resolver )
{
    start( FALSE, TRUE, fileName, resolver, LinkMap(), LinkMap() );
}

void Walkthrough::startPass2( const QString& fileName,
			      const Resolver *resolver,
			      const LinkMap& walkthroughLinkMap )
{
    start( FALSE, FALSE, fileName, resolver, LinkMap(), walkthroughLinkMap );
}

QString Walkthrough::printline( const QString& substr, const Location& docLoc )
{
    int lineNo0 = walkloc.lineNum();
    QString t = fix( xline(substr, docLoc, QString("printline")) );
    incrementScores( FALSE, lineNo0, 1001 );
    return t;
}

QString Walkthrough::printto( const QString& substr, const Location& docLoc )
{
    int lineNo0 = walkloc.lineNum();
    QString t = fix( xto(substr, docLoc, QString("printto")) );
    for ( int i = lineNo0; i < walkloc.lineNum(); i++ )
	incrementScores( FALSE, i,
			 484 + 989 / (walkloc.lineNum() - lineNo0 + 1) );
    return t;
}

QString Walkthrough::printuntil( const QString& substr, const Location& docLoc )
{
    int lineNo0 = walkloc.lineNum();
    QString t = fix( xuntil(substr, docLoc, QString("printuntil")) );
    for ( int i = lineNo0; i < walkloc.lineNum(); i++ )
	incrementScores( FALSE, i,
			 484 + 989 / (walkloc.lineNum() - lineNo0 + 1) );
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

void Walkthrough::addANames( QString *text, const LinkMap& exampleLinkMap )
{
    if ( exampleLinkMap.isEmpty() )
	return;

    int lineNo = 1;
    int k = 0;
    LinkMap::ConstIterator links = exampleLinkMap.begin();
    while ( links != exampleLinkMap.end() ) {
	while ( links.key() > lineNo ) {
	    lineNo++;
	    k = text->find( QChar('\n'), k ) + 1;
	    if ( k == 0 ) // shouldn't happen
		return;
	}

	/*
	  This condition should always be met, in theory. If it isn't,
	  and if we don't test it, the results are desastrous.
	*/
	if ( k < (int) text->length() && (*text)[k] != QChar('\n') ) {
	    StringSet::ConstIterator link = (*links).begin();

	    while ( link != (*links).end() ) {
		text->insert( k, QString("<a name=\"%1\"></a>").arg(*link) );
		++link;
	    }
	}
	++links;
    }
}

QString Walkthrough::start( bool include, bool firstPass,
			    const QString& fileName, const Resolver *resolver,
			    const LinkMap& includeLinkMap,
			    const LinkMap& walkthroughLinkMap )
{
    static QRegExp trailingSpacesPlusNL( QString("[ \t]+\n") );
    static QRegExp endOfLine( QString("\n(?!\n)") );
    static QRegExp manyNLs( QString("\n\n+") );
    static QRegExp aname( QString("<a name=[^>]*></a>") );

    fname = fileName;
    QString filePath = config->findDepth( fileName, config->exampleDirList() );

    if ( filePath.isEmpty() ) {
	QString ext = fileName.mid( fileName.findRev(QChar('.')) );

	if ( ext == QString(".cpp") || ext == QString(".h") ) {
	    /*
	      The .cpp or .h file does not exist. Maybe it's a
	      uic-generated file? Look for a .ui file and generate the
	      .cpp or .h file.
	    */
	    QString uiFileName = fileName;
	    uiFileName.truncate( uiFileName.findRev(QChar('.')) );
	    uiFileName.append( QString(".ui") );

	    QString uiFilePath = config->findDepth( uiFileName,
						    config->exampleDirList() );

	    if ( !uiFilePath.isEmpty() ) {
		QString cmd( "uic %1 -o %2" );

		filePath = uiFilePath.left( uiFilePath.length() - 3 ) + ext +
			   QString( ".temp" );
		if ( ext == QString(".cpp") ) {
		    QString hFileName = fileName.left( fileName.length() - 4 ) +
					QString( ".h" );
		    cmd.append( QString(" -impl %1").arg(hFileName) );
		}

		if ( system(cmd.arg(uiFilePath).arg(filePath).latin1()) != 0 ) {
		    message( 1, "Problems with generation of '%s'",
			     filePath.latin1() );
		    filePath = QString::null;
		}
	    }
	}
    }

    if ( justIncluded && filePath == fpath ) {
	/*
	  It's already started. This happens with '\include' followed
	  by '\quotefile'. Restarting would break local links.
	*/
	justIncluded = FALSE;
	return includeText;
    }

    fpath = filePath;
    shutUp = firstPass;

    if ( filePath.isEmpty() ) {
	if ( !shutUp )
	    message( 1, "Cannot find example file '%s'", fileName.latin1() );
	return QString::null;
    }

    QFile f( filePath );
    if ( !f.open(IO_ReadOnly) ) {
	if ( !shutUp )
	    message( 1, "Cannot open example file '%s'", filePath.latin1() );
	return QString::null;
    }

    QTextStream t( &f );
    QString code = t.read();
    f.close();
    if ( code.isEmpty() ) {
	if ( !shutUp )
	    message( 2, "Example file '%s' empty", filePath.latin1() );
	return QString::null;
    }

    code.replace( trailingSpacesPlusNL, "\n" );

    occMap.clear();
    classOccCounts.clear();
    totalOccCount = 0;

    scores.clear();
    if ( firstPass ) {
	occMap = occurrenceMap( code, resolver, QFileInfo(f).dirPath() );
	OccurrenceMap::ConstIterator occsOnLine = occMap.begin();
	while ( occsOnLine != occMap.end() ) {
	    StringSet::ConstIterator occ = (*occsOnLine).begin();
	    while ( occ != (*occsOnLine).end() ) {
		QString base = classPart( *occ );
		QMap<QString, int>::Iterator count =
			classOccCounts.find( base );
		if ( count == classOccCounts.end() )
		    classOccCounts.insert( base, 1 );
		else
		    (*count)++;
		totalOccCount++;
		++occ;
	    }
	    ++occsOnLine;
	}

	if ( include ) {
	    int numLines = code.count( QChar('\n') );
	    for ( int i = 0; i < numLines; i++ )
		incrementScores( TRUE, i, 484 );
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
      Local '<a name="...">'s are nice, but not in '\quotefile's.
    */
    if ( include && !firstPass )
	walkthroughText.replace( aname, QString::null );

    if ( !firstPass ) {
	if ( include )
	    addANames( &includeText, includeLinkMap );
	addANames( &walkthroughText, walkthroughLinkMap );
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
	    (*p).replace( manyNLs, "\n" );
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
    QString subs = killWhiteSpace( substr );
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
    } else if ( killWhiteSpace(plainlines.first()).find(subs) == -1 ) {
	if ( !shutUp ) {
	    warning( 2, docLoc, "Command '\\%s %s' failed at line %d of '%s'",
		     command.latin1(), subs.latin1(), walkloc.lineNum(),
		     walkloc.shortFilePath().latin1() );
	    warning( 2, walkloc,
		     "Text '%s' does not contain '%s' (see line %d of '%s')",
		     killWhiteSpace(plainlines.first()).latin1(),
		     subs.latin1(),
		     docLoc.lineNum(), docLoc.shortFilePath().latin1() );
	    shutUp = TRUE;
	}
    } else {
	s = getNextLine( docLoc );
    }
    return s;
}

QString Walkthrough::xto( const QString& substr, const Location& docLoc,
			  const QString& command )
{
    QString subs = killWhiteSpace( substr );
    QString s;

    if ( walkloc.filePath().isEmpty() ) {
	if ( !shutUp )
	    warning( 2, docLoc, "Command '\\%s %s' ignored", command.latin1(),
		     subs.latin1() );
	return s;
    }

    while ( !plainlines.isEmpty() ) {
	if ( killWhiteSpace(plainlines.first()).find(subs) != -1 )
	    return s;
	s += getNextLine( docLoc );
    }
    if ( !shutUp && substr != QString("EOF") ) {
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
    return s;
}

QString Walkthrough::getNextLine( const Location& docLoc )
{
    QString s;

    if ( !shutUp && justIncluded ) {
	warning( 2, docLoc,
		 "Command '\\quotefile' now needed after '\\include'" );
	shutUp = TRUE;
    }

    if ( !plainlines.isEmpty() ) {
	s = fancylines.first();

	int n = plainlines.first().count( '\n' ) + 1;
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
	int proportionOfSameClass =
		( classOccCounts[classPart(*occ)] << 10 ) / totalOccCount;
	int alpha = ( proportionOfSameClass + 1 ) >> 1;

	int netContribution = ( alpha * contribution ) >> 10;
	scores[*occ].addContribution( include, lineNo, netContribution );
	++occ;
    }
}
