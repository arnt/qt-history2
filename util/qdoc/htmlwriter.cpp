/*
  htmlwriter.cpp
*/

#include <qfile.h>
#include <qdir.h>

#include <stdarg.h>

#include "config.h"
#include "htmlwriter.h"
#include "messages.h"

QMap<QString, StringSet> HtmlWriter::tmap;
QString HtmlWriter::styl;
QString HtmlWriter::posth;
QString HtmlWriter::addr;

HtmlWriter::HtmlWriter( const QString& fileName )
    : fn( fileName ), headFlushed( FALSE ), footFlushed( FALSE )
{
    QString file = config->outputDir() + QChar( '/' ) + fileName;
    out = fopen( QFile::encodeName(file), "w" );
    if ( out == 0 ) {
	syswarning( "Cannot open '%s' for writing HTML", file.latin1() );
	return;
    }
}

HtmlWriter::~HtmlWriter()
{
    if ( out != 0 ) {
	enterFooter();
	if ( !addr.isEmpty() )
	    putsMeta( addr.latin1() );
	putsMeta( "</body>\n</html>\n" );
	fclose( out );
    }
}

void HtmlWriter::enterFooter()
{
    if ( footFlushed )
	return;
    footFlushed = TRUE;

    // the eof marker makes HtmlReader's task easier
    putsMeta( "<!-- eof -->\n" );
}

void HtmlWriter::printfMeta( const char *fmt, ... )
{
    if ( out == 0 )
	return;
    flushHead();

    va_list ap;

    va_start( ap, fmt );
    vfprintf( out, fmt, ap );
    va_end( ap );
}

void HtmlWriter::putsMeta( const char *str )
{
    if ( out == 0 )
	return;
    flushHead();

    fputs( str, out );
}

void HtmlWriter::puts( const char *str )
{
    if ( out == 0 )
	return;
    flushHead();

    char ch;
    while ( (ch = *str++) != '\0' ) {
	switch ( ch ) {
	case '&':
	    fputs( "&amp;", out );
	    break;
	case '<':
	    fputs( "&lt;", out );
	    break;
	case '>':
	    fputs( "&gt;", out );
	    break;
	default:
	    putc( ch, out );
	}
    }
}

void HtmlWriter::printFnord()
{
    putsMeta( "<a href=\"http://www.kbuxton.com/discordia/fnord.html\">"
	      "fnord</a>" );
}

void HtmlWriter::flushHead()
{
    if ( headFlushed )
	return;
    headFlushed = TRUE;

    if ( t.isEmpty() && !h.isEmpty() )
	t = h;
    else if ( h.isEmpty() && !t.isEmpty() )
	h = t;

    putsMeta( "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0"
	      " Transitional//EN\">\n" );
    putsMeta( "<html>\n<head>\n" );
    putsMeta( "<meta http-equiv=\"Content-Type\" content=\"text/html;"
	      " charset=ISO-8859-1\">\n" );
    if ( !t.isEmpty() ) {
	putsMeta( "<title>" );
	puts( t.latin1() );
	putsMeta( "</title>\n" );
    }
    tmap[t].insert( fn );
    if ( !styl.isEmpty() ) {
	putsMeta( "<style type=\"text/css\"><!--\n" );
	putsMeta( styl.latin1() );
	putsMeta( "\n--></style>\n" );
    }
    putsMeta( "</head>\n<body>\n" );

    if ( !posth.isEmpty() )
	putsMeta( posth.latin1() );

    putsMeta( "<h1 align=center>" );

    if ( h.isEmpty() )
	puts( fn );
    else
	putsMeta( h.latin1() );

    if ( !sh.isEmpty() ) {
	putsMeta( "<br><small>" );
	putsMeta( sh.latin1() );
	putsMeta( "</small>" );
    }
    putsMeta( "</h1>\n\n" );
}
