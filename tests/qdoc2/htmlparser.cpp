/*
  htmlparser.cpp
*/

#include <qfile.h>
#include <qstring.h>

#include <stdio.h>

#include "messages.h"
#include "steering.h"

QString yyFileName;
static FILE *yyIn;
static int yyCh;

static QString getBefore( const QString& subStr )
{
    int matchedLen = 0;
    QString t;

    while ( matchedLen < (int) subStr.length() ) {
	if ( yyCh == EOF )
	    break;

	t += QChar( yyCh );

	if ( subStr[matchedLen] == QChar(yyCh) )
	    matchedLen++;
	else
	    matchedLen = 0;

	yyCh = getc( yyIn );
    }
    t.truncate( t.length() - matchedLen );
    int k = t.find( QString("<!-- eof -->") );
    if ( k != -1 )
	t.truncate( k );
     return t;
} 

static void skipUntil( const QString& subStr )
{
    int matchedLen = 0;

    while ( matchedLen < (int) subStr.length() && yyCh != EOF ) {
	if ( subStr[matchedLen] == QChar(yyCh) )
	    matchedLen++;
	else
	    matchedLen = 0;

	yyCh = getc( yyIn );
    }
}

static void parseClass( Steering *steering )
{
    skipUntil( QString("<h2>Detailed Description</h2>\n") );
    QString html = getBefore( QString("<hr><h") );
    if ( !html.isEmpty() )
	steering->addHtmlChunk( yyFileName, HtmlChunk(html) );

    while ( TRUE ) {
	skipUntil( QString("3 class=fn>") );
	skipUntil( QString("<a name=") );
	QString anchor = getBefore( QChar('>') );
	skipUntil( QString("</h3>") );

	html = getBefore( QString("<h") );
	while ( yyCh != '3' && yyCh != 'r' && yyCh != EOF ) {
	    html += QString( "<h" );
	    html += getBefore( QString("<h") );
	}

	int k = html.find( QString("<p>Reimplemented from ") );
	if ( k == -1 )
	    k = html.find( QString("<p>Reimplemented in ") );

	if ( k != -1 )
	    html.truncate( k );

	if ( html.isEmpty() )
	    break;

	if ( !anchor.isEmpty() )
	    steering->addHtmlChunk( yyFileName + QChar('#') + anchor,
				    HtmlChunk(html) );
    }
}

static void parseOther( Steering *steering )
{
    QString html = getBefore( QString("#magicwordthatyoushouldavoid") );
    if ( !html.isEmpty() )
	steering->addHtmlChunk( yyFileName, HtmlChunk(html) );
}

void parseHtmlFile( Steering *steering, const QString& filePath )
{
    yyFileName = filePath.mid( filePath.findRev(QChar('/')) + 1 );
    yyIn = fopen( QFile::encodeName(filePath), "r" );
    if ( yyIn == 0 ) {
	syswarning( "Cannot open HTML file '%s' for reading",
		    filePath.latin1() );
	return;
    }
    yyCh = getc( yyIn );

    skipUntil( QString("<h1 align=center>") );
    QString heading = getBefore( QString("</h1>\n\n") );

    if ( heading.find(QString("Class Reference")) != -1 ) {
	parseClass( steering );
    } else if ( heading.right(2) != QString(".h") &&
		heading.find(QString("Member List")) == -1 ) {
	parseOther( steering );
    }

    fclose( yyIn );
}
