/*
  codeprocessor.cpp
*/

#include <qdir.h>
#include <qfile.h>
#include <qmap.h>
#include <qregexp.h>
#include <qtextstream.h>

#include "codeprocessor.h"
#include "config.h"
#include "metaresolver.h"
#include "stringset.h"

static QString gulbrandsen( "::" );
static QString parenParen( "()" );

static QString untabified( const QString& in )
{
    QString res;
    int col = 0;

    for ( int i = 0; i < (int) in.length(); i++ ) {
	if ( in[i] == QChar('\t') ) {
	    res += QString( "        " + (col & 0x7) );
	    col = ( col + 8 ) & ~0x7;
	} else {
	    res += in[i];
	    if ( in[i] == QChar('\n') )
		col = 0;
	    else
		col++;
	}
    }
    return res;
}

static int *createLineNoAtOffset( const QString& code )
{
    int *lineNoAtOffset = new int[code.length() + 1];
    int lineNo = 1;
    for ( int i = 0; i < (int) code.length(); i++ ) {
	// consider the \n as starting a line (this helps us later)
	if ( code[i] == QChar('\n') )
	    lineNo++;
	lineNoAtOffset[i] = lineNo;
    }
    return lineNoAtOffset;
}

/*
  Derives inheritance information from class definitions.
*/
static QMap<QString, StringSet> inheritanceMap( const QString& t,
						const QString& dirPath )
{
    static QRegExp linclude( QString("#include +\"([^\"]*)\"") );
    static QRegExp xInheritsY( QString(
	    "class +(?:[a-zA-Z_0-9]+[ \n]+)*([a-zA-Z_0-9]+)[ \n]*"
	    ":[ \n]*public[ \n]+([a-zA-Z_0-9]+)") );

    QMap<QString, StringSet> cinherits;
    QString bigt = t;
    int k;

    if ( !dirPath.isEmpty() ) {
	QDir dir( dirPath );
	k = 0;
	while ( (k = linclude.search(t, k)) != -1 ) {
	    QString fn = linclude.cap( 1 );
	    if ( dir.exists(fn) ) {
		QFile f( dir.filePath(fn) );
		if ( f.open(IO_ReadOnly) ) {
		    QTextStream t( &f );
		    bigt += t.read();
		    f.close();
		}
	    }
	    k += linclude.matchedLength();
	}
    }

    k = 0;
    while ( (k = xInheritsY.search(bigt, k)) != -1 ) {
	QString x = xInheritsY.cap( 1 );
	QString y = xInheritsY.cap( 2 );
	cinherits[x].insert( y );
	k += xInheritsY.matchedLength();
    }
    return cinherits;
}

static void insertSomeQtStuffThatCannotHarm( QMap<QString, StringSet> *types )
{
    (*types)[QString("qApp")].insert( QString("QApplication") );
}

/*
  The two following functions have much in common. They don't have to,
  but if it works in one place, it works in the other... mostly. When
  doing changes, strive to keep these in sync.
*/

/*
  Returns a map that give the line numbers of occurrences of
  documented functions.
*/
OccurrenceMap occurrenceMap( const QString& code, const Resolver *res,
			     const QString& dirPath )
{
    static QRegExp yHasTypeX( QString(
	    "(?:[\n:;{(]|const) +([a-zA-Z_][a-zA-Z_0-9]*)(?:<[^;{}]+>)?"
	    "(?: *[*&] *| +)([a-zA-Z_][a-zA-Z_0-9]*)? *[,;()=]") );
    static QRegExp xNewY( QString(
	    "\n *([a-zA-Z_][a-zA-Z_0-9]*) *= *new +([a-zA-Z_0-9]+)") );
    static QRegExp xDotY( QString(
	    "\\b([a-zA-Z_][a-zA-Z_0-9]*) *(?:\\.|->|,[ \n]*S(?:IGNAL|LOT)\\() *"
	    "([a-zA-Z_][a-zA-Z_0-9]*)(?= *\\()") );
    static QRegExp xIsStaticZOfY( QString(
	    "[\n:;{(=] *(([a-zA-Z_0-9]+)::([a-zA-Z_0-9]+))(?= *\\()") );
    static QRegExp globalY( QString(
	    "([\n{()=] *)([a-zA-Z_][a-zA-Z_0-9]*)[ \n]*\\(") );

    OccurrenceMap occMap;

    if ( res == 0 || !config->autoHrefs() )
	return occMap;

    QString t = untabified( code );
    MetaResolver metaRes( res );
    int *lineNoAtOffset = createLineNoAtOffset( t );
    int k;

    QMap<QString, StringSet> cinherits = inheritanceMap( t, dirPath );
    metaRes.setClassInheritanceMap( cinherits );

    /*
      Find reimplementations of documented functions.
    */
    QMap<int, QString> classAtOffset;
    QMap<QString, StringSet>::Iterator c = cinherits.begin();
    while ( c != cinherits.end() ) {
	/*
	  A custom regular expression will find 'Class::memberX()' in
	  a jiffy.
	*/
	QRegExp memberX( c.key() + QString(
		"::([a-zA-Z_0-9]+)[ \n]*\\([^)]*(?:\\([^)]*\\)[^)]*)?\\)[ \n]*"
		"(?:const[ \n]*)?[{:]") );
	k = 0;
	while ( (k = memberX.search(t, k)) != -1 ) {
	    classAtOffset.insert( k, c.key() );
	    QString link = metaRes.resolve( c.key() + gulbrandsen +
					    memberX.cap(1) + parenParen );
	    if ( !link.isEmpty() )
		occMap[lineNoAtOffset[k]].insert( link );
	    k += memberX.matchedLength();
	}
	++c;
    }

    /*
      Look for variable definition or similar and remember the type of
      the variable.
    */
    QMap<QString, StringSet> types;
    k = 0;
    while ( (k = yHasTypeX.search(t, k)) != -1 ) {
	QString x = yHasTypeX.cap( 1 );
	QString y = yHasTypeX.cap( 2 );

	if ( !y.isEmpty() )
	    types[y].insert( x );

	k += yHasTypeX.matchedLength() - 1;
    }

    /*
      Look for 'var = new Class'.
    */
    k = 0;
    while ( (k = xNewY.search(t, k)) != -1 ) {
	QString x = xNewY.cap( 1 );
	QString y = xNewY.cap( 2 );
	types[x].insert( y );
	k += xNewY.matchedLength();
    }

    insertSomeQtStuffThatCannotHarm( &types );

    /*
      Find use of any of

	  var.method()
	  var->method()
	  var, SIGNAL(method())
	  var, SLOT(method()).
    */
    k = 0;
    while ( (k = xDotY.search(t, k)) != -1 ) {
	QString x = xDotY.cap( 1 );

	QStringList::ConstIterator s = types[x].begin();
	while ( s != types[x].end() ) {
	    QString link = metaRes.resolve( *s + gulbrandsen + xDotY.cap(2) +
					    parenParen );
	    if ( !link.isEmpty() ) {
		occMap[lineNoAtOffset[k]].insert( link );
		break;
	    }
	    ++s;
	}
	k += xDotY.matchedLength();
    }

    /*
      Find use of 'Class::method()'.
    */
    k = 0;
    while ( (k = xIsStaticZOfY.search(t, k)) != -1 ) {
	QString link = metaRes.resolve( xIsStaticZOfY.cap(1) + parenParen );
	if ( !link.isEmpty() )
	    occMap[lineNoAtOffset[k]].insert( link );
	k += xIsStaticZOfY.matchedLength();
    }

    /*
      Find use of 'globalFunction()'.
    */
    k = 0;
    while ( (k = globalY.search(t, k)) != -1 ) {
	QString link = metaRes.resolve( globalY.cap(2) + parenParen );
	if ( !link.isEmpty() )
	    occMap[lineNoAtOffset[k]].insert( link );
	k += globalY.matchedLength() - 1;
    }

    delete[] lineNoAtOffset;
    return occMap;
}

/*
  This is a rewrite of the processcode Perl subroutine of old qdoc
  written by Arnt Gulbrandsen. It converts plain C++ code into HTML
  and adds links automatically.
*/
QString processCodeHtml( const QString& code, const Resolver *res,
			 const QString& dirPath, bool localLinks )
{
    /*
      These QRegExps are designed to be fast. If you change them,
      strive to keep them that way.

      A QRegExp such as newClassX is very fast because it starts
      with a literal string ("new"). xIsStaticZOfY is also fast,
      because it starts with a very restrictive character class
      ("[\n:;{(=]").

      The source code is '\n'-free.
    */
    static QRegExp amp( QChar('&') );
    static QRegExp lt( QChar('<') );
    static QRegExp gt( QChar('>') );
    static QRegExp ginclude( QString("#include +&lt;([^&]*)&gt;") );
    static QRegExp yHasTypeX( QString(
	    "(?:[\n:;{(]|const) +([a-zA-Z_][a-zA-Z_0-9]*)"
	    "(?:&lt;[^;{}]+&gt;)?(?: *(?:\\*|&amp;) *| +)"
	    "([a-zA-Z_][a-zA-Z_0-9]*)? *[,;()=]") );
    static QRegExp xNewY( QString(
	    "\n *([a-zA-Z_][a-zA-Z_0-9]*) *= *new +([a-zA-Z_0-9]+)") );
    static QRegExp newClassX( QString("new +([a-zA-Z_][a-zA-Z_0-9]*)") );
    static QRegExp xDotY( QString(
	    "\\b([a-zA-Z_][a-zA-Z_0-9]*)"
	    " *(?:\\.|-&gt;|,[ \n]*S(?:IGNAL|LOT)\\() *"
	    "([a-zA-Z_][a-zA-Z_0-9]*)(?= *\\()") );
    static QRegExp xIsStaticZOfY( QString(
	    "[\n:;{(=] *(([a-zA-Z_0-9]+)::([a-zA-Z_0-9]+))(?= *\\()") );
    static QRegExp classX( QString(
	    ":[ \n]*(?:p(?:ublic|r(?:otected|ivate))[ \n]+)?"
	    "([a-zA-Z_][a-zA-Z_0-9]*)") );
    static QRegExp globalY( QString(
	    "([\n{()=] *)([a-zA-Z_][a-zA-Z_0-9]*)[ \n]*\\(") );

    static int funique = 1;

    QString t = untabified( code );
    int k;

    /*
      HTMLize.
    */
    t.replace( amp, QString("&amp;") );
    t.replace( lt, QString("&lt;") );
    t.replace( gt, QString("&gt;") );

    if ( res == 0 || !config->autoHrefs() )
	return t;

    MetaResolver metaRes( res );

    /*
      Add links to global include files.
    */
    k = 0;
    while ( (k = ginclude.search(t, k)) != -1 ) {
	QString fn = ginclude.cap( 1 );
	QString newFn = metaRes.href( fn );
	if ( newFn.length() != fn.length() )
	    t.replace( ginclude.pos(1), fn.length(), newFn );
	k++;
    }

    QMap<QString, StringSet> cinherits = inheritanceMap( t, dirPath );
    metaRes.setClassInheritanceMap( cinherits );

    /*
      Find member function definitions and add either '<a href="...">'
      or '<a name="...">'.
    */
    QMap<QString, QMap<QString, QString> > mfunctions;
    QMap<int, QString> classAtOffset;
    QMap<QString, StringSet>::Iterator c = cinherits.begin();
    while ( c != cinherits.end() ) {
	/*
	  A custom regular expression will find 'Class::memberX()' in
	  a jiffy.
	*/
	QRegExp memberX( c.key() + QString(
		"::([a-zA-Z_0-9]+)[ \n]*\\([^)]*(?:\\([^)]*\\)[^)]*)?\\)[ \n]*"
		"(?:const[ \n]*)?[{:]") );
	k = 0;
	while ( (k = memberX.search(t, k)) != -1 ) {
	    classAtOffset.insert( k, c.key() );
	    QString x = memberX.cap( 1 );
	    int xpos = memberX.pos( 1 );

	    QString newX = metaRes.href( c.key() + gulbrandsen + x + parenParen,
					 x );
	    if ( newX.length() == x.length() ) {
		if ( localLinks &&
		     !mfunctions[c.key()].contains(x + parenParen) ) {
		    mfunctions[c.key()].insert( x + parenParen,
						QString("#f%1").arg(funique) );
		    QString aname =
			    QString( "<a name=\"f%1\"></a>" ).arg( funique );
		    t.insert( k, aname );
		    funique++;
		    k += aname.length();
		}
		k += memberX.matchedLength();
	    } else {
		t.replace( xpos, x.length(), newX );
		k += memberX.matchedLength() + newX.length() - x.length();
	    }
	}
	++c;
    }
    metaRes.setMemberFunctionMap( mfunctions );

    /*
      Look for variable definition or similar, add link to the data
      type, and remember the type of the variable.
    */
    QMap<QString, StringSet> types;
    k = 0;
    while ( (k = yHasTypeX.search(t, k)) != -1 ) {
	QString x = yHasTypeX.cap( 1 );
	int xpos = yHasTypeX.pos( 1 );
	QString y = yHasTypeX.cap( 2 );

	QString newX = metaRes.href( x );
	if ( newX.length() != x.length() )
	    t.replace( xpos, x.length(), newX );
	if ( !y.isEmpty() )
	    types[y].insert( x );

	/*
	  Without the minus one at the end, 'void member(Class var)' would give
	  'member' as a variable of type 'void', but would ignore 'Class var'.
	*/
	k += yHasTypeX.matchedLength() + newX.length() - x.length() - 1;
    }

    /*
      Look for 'var = new Class'.
    */
    k = 0;
    while ( (k = xNewY.search(t, k)) != -1 ) {
	QString x = xNewY.cap( 1 );
	QString y = xNewY.cap( 2 );
	types[x].insert( y );
	k += xNewY.matchedLength();
    }

    insertSomeQtStuffThatCannotHarm( &types );

    /*
      Add link to 'new Class'.
    */
    k = 0;
    while ( (k = newClassX.search(t, k)) != -1 ) {
	QString x = newClassX.cap( 1 );

	QString newX = metaRes.href( x );
	if ( newX.length() != x.length() )
	    t.replace( k + newClassX.matchedLength() - x.length(), x.length(),
		       newX );
	k += newClassX.matchedLength() + newX.length() - x.length();
    }

    /*
      Add link to any of

	  var.method()
	  var->method()
	  var, SIGNAL(method())
	  var, SLOT(method()).
    */
    k = 0;
    while ( (k = xDotY.search(t, k)) != -1 ) {
	QString x = xDotY.cap( 1 );
	QString y = xDotY.cap( 2 );
	int ypos = xDotY.pos( 2 );

	QStringList::ConstIterator s = types[x].begin();
	while ( s != types[x].end() ) {
	    QString newY = metaRes.href( *s + gulbrandsen + y + parenParen, y );
	    if ( newY.length() != y.length() ) {
		t.replace( ypos, y.length(), newY );
		k += newY.length() - y.length();
		break;
	    }
	    ++s;
	}
	k += xDotY.matchedLength();
    }

    /*
      Add link to 'Class::method()'.
    */
    k = 0;
    while ( (k = xIsStaticZOfY.search(t, k)) != -1 ) {
	QString x = xIsStaticZOfY.cap( 1 );
	QString z = xIsStaticZOfY.cap( 3 );

	QString newZ = metaRes.href( x + parenParen, z );
	if ( newZ.length() != z.length() )
	    t.replace( k + xIsStaticZOfY.matchedLength() - z.length(),
		       z.length(), newZ );
	k += xIsStaticZOfY.matchedLength() + newZ.length() - z.length();
    }

    /*
      Add link to ': Class'.
    */
    k = 0;
    while ( (k = classX.search(t, k)) != -1 ) {
	QString x = classX.cap( 1 );

	QString newX = metaRes.href( x );
	if ( newX.length() != x.length() )
	    t.replace( k + classX.matchedLength() - x.length(), x.length(),
		       newX );
	k += classX.matchedLength() + newX.length() - x.length();
    }

    /*
      Add link to 'globalFunction()' and 'method()' (i.e., 'this->method()').
    */
    QString curClass;
    k = 0;
    while ( (k = globalY.search(t, k)) != -1 ) {
	QString x = globalY.cap( 1 );
	QString y = globalY.cap( 2 );

	while ( !classAtOffset.isEmpty() && classAtOffset.begin().key() < k ) {
	    curClass = *classAtOffset.begin();
	    classAtOffset.remove( classAtOffset.begin() );
	}

	QString newY = metaRes.href( y + parenParen, y );
	if ( newY.length() == y.length() && !curClass.isEmpty() )
	    newY = metaRes.href( curClass + gulbrandsen + y + parenParen, y );

	if ( newY.length() != y.length() )
	    t.replace( k + x.length(), y.length(), newY );

	// minus one for the same reason as elsewhere
	k += globalY.matchedLength() + newY.length() - y.length() - 1;
    }

    return t;
}
