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

/*
  This is a rewrite of the processcode Perl subroutine of old qdoc.  Thanks to
  Arnt Gulbrandsen for writing it in the first place.

  This function converts plain C++ code into HTML and adds links automatically.
*/
QString processCodeHtml( const QString& code, const Resolver *res,
			 bool localLinks, const QString& dirPath )
{
    static QRegExp *amp = 0;
    static QRegExp *lt = 0;
    static QRegExp *gt = 0;
    static QRegExp *ginclude = 0;
    static QRegExp *linclude = 0;
    static QRegExp *xInheritsY = 0;
    static QRegExp *yHasTypeX = 0;
    static QRegExp *xNewY = 0;
    static QRegExp *newClassX = 0;
    static QRegExp *xDotY = 0;
    static QRegExp *staticZOfY = 0;
    static QRegExp *classX = 0;
    static QRegExp *globalY = 0;

    if ( amp == 0 ) {
	amp = new QRegExp( QChar('&') );
	lt = new QRegExp( QChar('<') );
	gt = new QRegExp( QChar('>') );
	ginclude = new QRegExp( QString("#include +&lt;([^&]*)&gt;") );
	linclude = new QRegExp( QString("#include +\"([^\"]*)\"") );

	if ( config->readExampleHeaders() )
	    xInheritsY = new QRegExp( QString(
		    "class +(?:[a-zA-Z_0-9]+[ \n]+)*([a-zA-Z_0-9]+)[ \n]*"
		    ":[ \n]*public[ \n]+([a-zA-Z_0-9]+)") );
	else
	    xInheritsY = new QRegExp( QString(
		    "::([a-zA-Z_0-9]+)\\([^){;]*\\)[ \n]*:[ \n]*([a-zA-Z_0-9]+)"
		    "[ \n]*\\(") );

	yHasTypeX = new QRegExp( QString(
		"(?:[\n:;{(]|const) +([a-zA-Z_][a-zA-Z_0-9]*)"
		"(?:&lt;[^;{}]+&gt;)?(?: *[*&] *| +)([a-zA-Z_][a-zA-Z_0-9]*)?"
		" *[,;()=]") );
	xNewY = new QRegExp( QString(
		"\n *([a-zA-Z_][a-zA-Z_0-9]*) *= *new +([a-zA-Z_0-9]+)") );
	newClassX = new QRegExp( QString("new +([a-zA-Z_][a-zA-Z_0-9]*)") );
	xDotY = new QRegExp( QString(
		"\\b([a-zA-Z_][a-zA-Z_0-9]*)"
		" *(?:\\.|-&gt;|,[ \n]*S(?:IGNAL|LOT)\\() *"
		"([a-zA-Z_][a-zA-Z_0-9]*)(?= *\\()") );
	staticZOfY = new QRegExp( QString(
		"[\n:;{(=] *(([a-zA-Z_0-9]+)::([a-zA-Z_0-9]+))(?= *\\()") );
	classX = new QRegExp( QString(
		":[ \n]*(?:p(?:ublic|r(?:otected|ivate))[ \n]+)?"
		"([a-zA-Z_][a-zA-Z_0-9]*)") );
	globalY = new QRegExp( QString(
		"([\n{()=] *)([a-zA-Z_][a-zA-Z_0-9]*)[ \n]*\\(") );
    }

    static int funique = 1;

    QString t = code;
    int k;

    /*
      Untabify.
    */
    int n = 0;
    for ( int i = 0; i < (int) t.length(); i++ ) {
	if ( t[i] == QChar('\t') ) {
	    t.replace( i, 1, QString("        " + n) );
	    n = 1;
	}

	if ( n == 7 || t[i] == QChar('\n') )
	    n = 0;
	else
	    n++;
    }

    /*
      HTMLize.
    */
    t.replace( *amp, QString("&amp;") );
    t.replace( *lt, QString("&lt;") );
    t.replace( *gt, QString("&gt;") );

    if ( res == 0 || !config->autoHrefs() )
	return t;

    MetaResolver metaRes( res );

    /*
      Add links to global include files.
    */
    k = 0;
    while ( (k = t.find(*ginclude, k)) != -1 ) {
	QString fn = ginclude->cap( 1 );
	QString newFn = metaRes.href( fn );
	if ( newFn.length() != fn.length() )
	    t.replace( ginclude->pos(1), fn.length(), newFn );
	k++;
    }

    /*
      Derive inheritance information from class definitions or from
      constructors.  Class definitions are more reliable, but they require more
      work.
    */
    QMap<QString, StringSet> cinherits;
    QString bigt = t;

    if ( config->readExampleHeaders() && !dirPath.isEmpty() ) {
	QDir dir( dirPath );
	k = 0;
	while ( (k = t.find(*linclude, k)) != -1 ) {
	    QString fn = linclude->cap( 1 );
	    if ( dir.exists(fn) ) {
		QFile f( dir.filePath(fn) );
		if ( f.open(IO_ReadOnly) ) {
		    QTextStream t( &f );
		    bigt += t.read();
		    f.close();
		}
	    }
	    k += linclude->matchedLength();
	}
    }

    k = 0;
    while ( (k = bigt.find(*xInheritsY, k)) != -1 ) {
	QString x = xInheritsY->cap( 1 );
	QString y = xInheritsY->cap( 2 );
	cinherits[x].insert( y );
	k += xInheritsY->matchedLength();
    }

    metaRes.setClassInheritanceMap( cinherits );

    /*
      Find member function definitions and add either '<a href=...>' or
      '<a name=...>'.
    */
    QMap<QString, QMap<QString, int> > mfunctions;
    QMap<int, QString> classAtOffset;
    QMap<QString, StringSet>::Iterator c = cinherits.begin();
    while ( c != cinherits.end() ) {
	/*
	  A custom regular expression will find the right thing in a jiffy.
	*/
	QRegExp memberX( c.key() + QString(
		"::([a-zA-Z_0-9]+)[ \n]*\\([^)]*(?:\\([^)]*\\)[^)]*)?\\)[ \n]*"
		"(?:const[ \n]*)?[{:]") );
	k = 0;
	while ( (k = t.find(memberX, k)) != -1 ) {
	    classAtOffset.insert( k, c.key() );
	    QString x = memberX.cap( 1 );
	    int xpos = memberX.pos( 1 );

	    QString newX = metaRes.href( c.key() + QString("::") + x, x );
	    if ( newX.length() == x.length() ) {
		if ( localLinks && !mfunctions[c.key()].contains(x) ) {
		    mfunctions[c.key()].insert( x, funique );
		    QString aname 
			= QString( "<a name=\"%1\"></a>" ).arg( funique );
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
      Look for variable definition or similar, add link to the data type, and
      remember the type of the variable.
    */
    QMap<QString, StringSet> types;
    k = 0;
    while ( (k = t.find(*yHasTypeX, k)) != -1 ) {
	QString x = yHasTypeX->cap( 1 );
	int xpos = yHasTypeX->pos( 1 );
	QString y = yHasTypeX->cap( 2 );

	QString newX = metaRes.href( x );
	if ( newX.length() != x.length() )
	    t.replace( xpos, x.length(), newX );
	if ( !y.isEmpty() )
	    types[y].insert( x );

	/*
	  Without the minus one at the end, 'void member(Class var)' would give
	  'member' as a variable of type 'void', but would ignore 'Class var'.
	*/
	k += yHasTypeX->matchedLength() + newX.length() - x.length() - 1;
    }

    /*
      Add some Qt stuff that cannot harm.
    */
    types[QString("qApp")].insert( QString("QApplication") );

    /*
      Look for 'var = new Class'.
    */
    k = 0;
    while ( (k = t.find(*xNewY, k)) != -1 ) {
	QString x = xNewY->cap( 1 );
	QString y = xNewY->cap( 2 );
	types[x].insert( y );
	k += xNewY->matchedLength();
    }

    /*
      Add link to 'new Class'.
    */
    k = 0;
    while ( (k = t.find(*newClassX, k)) != -1 ) {
	QString x = newClassX->cap( 1 );

	QString newX = metaRes.href( x );
	if ( newX.length() != x.length() )
	    t.replace( k + newClassX->matchedLength() - x.length(), x.length(),
		       newX );
	k += newClassX->matchedLength() + newX.length() - x.length();
    }

    /*
      Add link to any of

	  var.method()
	  var->method()
	  var, SIGNAL(method())
	  var, SLOT(method()).
    */
    k = 0;
    while ( (k = t.find(*xDotY, k)) != -1 ) {
	QString x = xDotY->cap( 1 );
	QString y = xDotY->cap( 2 );
	int ypos = xDotY->pos( 2 );

	QStringList::ConstIterator s = types[x].begin();
	while ( s != types[x].end() ) {
	    QString newY = metaRes.href( *s + QString("::") + y, y );
	    if ( newY.length() != y.length() ) {
		t.replace( ypos, y.length(), newY );
		k += newY.length() - y.length();
		break;
	    }
	    ++s;
	}
	k += xDotY->matchedLength();
    }

    /*
      Add link to 'Class::method()'.
    */
    k = 0;
    while ( (k = t.find(*staticZOfY, k)) != -1 ) {
	QString x = staticZOfY->cap( 1 );
	QString y = staticZOfY->cap( 2 );
	QString z = staticZOfY->cap( 3 );

	QString newZ = metaRes.href( x, z );
	if ( newZ.length() != z.length() )
	    t.replace( k + staticZOfY->matchedLength() - z.length(), z.length(),
		       newZ );
	k += staticZOfY->matchedLength() + newZ.length() - z.length();
    }

    /*
      Add link to ': Class'.
    */
    k = 0;
    while ( (k = t.find(*classX, k)) != -1 ) {
	QString x = classX->cap( 1 );

	QString newX = metaRes.href( x );
	if ( newX.length() != x.length() )
	    t.replace( k + classX->matchedLength() - x.length(), x.length(),
		       newX );
	k += classX->matchedLength() + newX.length() - x.length();
    }

    /*
      Add link to 'globalFunction()' and 'method()' (i.e., 'this->method()').
    */
    QString curClass;
    k = 0;
    while ( (k = t.find(*globalY, k)) != -1 ) {
	QString x = globalY->cap( 1 );
	QString y = globalY->cap( 2 );

	while ( !classAtOffset.isEmpty() && classAtOffset.begin().key() < k ) {
	    curClass = *classAtOffset.begin();
	    classAtOffset.remove( classAtOffset.begin() );
	}

	QString newY = metaRes.href( y, y );
	if ( newY.length() == y.length() && !curClass.isEmpty() )
	    newY = metaRes.href( curClass + QString("::") + y, y );

	if ( newY.length() != y.length() )
	    t.replace( k + x.length(), y.length(), newY );

	/*
	  Minus one for the same reason as elsewhere.
	*/
	k += globalY->matchedLength() + newY.length() - y.length() - 1;
    }
    return t;
}
