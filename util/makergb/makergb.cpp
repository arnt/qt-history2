/****************************************************************************
** $Id: //depot/qt/main/util/makergb/makergb.cpp#4 $
**
** makergb - Utility to generate X11 RGB color lookup function
**
** Author  : Haavard Nord
** Created : 940112
**
** Copyright (C) 1994-1997 by Troll Tech AS.  All rights reserved.
**
** --------------------------------------------------------------------------
** This utility generates source code to lookup X11 RGB colors from X11
** color names. It is particularly useful for platforms that do not have
** the X11 color database.
** To generate this code, you'll need an X11 rgb.txt file that contains
** color names and their RGB values.
**
** Usage: makergb rgb.txt > file.cpp
*****************************************************************************/

#include <qstring.h>
#include <qvector.h>
#include <qdict.h>
#include <qregexp.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>


const int  MAX_COLORS = 2000;

struct Color {					// data read from rgb.txt
    QString name;
    int     r,g,b;
};


class ColorVector : public QVector<Color>	// vector of Color data
{
public:
    ColorVector( uint sz ) : QVector<Color>(sz)
	{ setAutoDelete(TRUE); }
   ~ColorVector()
	{ clear(); }
private:
    int compareItems( GCI c1, GCI c2 )
	{ return stricmp(((Color*)c1)->name,((Color*)c2)->name); }
};


ColorVector *readData( FILE *f )
{
    QDict<int> alreadyRead;
    QRegExp rx(" ");
    ColorVector *cv = new ColorVector(MAX_COLORS);
    int n = 0;
    while ( !feof( f ) ) {
	int r, g, b;
	char buf[200];
	fscanf( f, "%d%d%d", &r, &g, &b );	// read r,g,b
	if ( feof( f ) )
	    break;
	fgets( buf, 200, f );			// read color name
	QString name = buf;
	name = name.stripWhiteSpace();
	name = name.lower();
	name.replace(rx,"");
	if ( !alreadyRead.find(name) ) {
	    alreadyRead.insert(name,(int*)1);
	    Color *c = new Color;		// add info to vector
	    c->name = name;
	    c->r = r;
	    c->g = g;
	    c->b = b;
	    cv->insert( n++, c );
	}
	if ( n >= MAX_COLORS )			// vector full
	    break;
    }
    cv->resize( n );				// resize to current size
#if 0
    for ( int i=0; i<n; i++ ) {
	debug( "%d: %s", i, cv->at(i)->name.data() );
    }
    exit(1);
#endif
    return cv;
}


const char *header =
"/****************************************************************************\n"
"** Color lookup based on a name.
"*****************************************************************************/\n\n"
"#if !defined(NO_COLORNAME)\n\n"
"#include <stdlib.h>\n"
"#include <string.h>\n\n\n"
"#if defined(RGB)\n#undef RGB\n#endif\n"
"#define RGB(r,g,b) (r+g*256+b*65536L)\n\n";

const char *rgb_tblsize =
"const int rgbTblSize = %d;\n\n";

const char *rgb_struct =
"struct RGBData {\n    uint value;\n    char *name;\n} rgbTbl[] = {\n";

const char *rgb_func =
"static int rgbCmp( RGBData *d1, RGBData *d2 )\n{\n    "
"return stricmp( d1->name, d2->name );\n}\n\n"
"uint qGetRGBValue( const char *name )\n{\n    "
"RGBData x;\n    x.name = (char *)name;\n    "
"RGBData *r = (RGBData*)bsearch((char*)&x, (char*)rgbTbl, rgbTblSize,\n"
"\t\t\t\t   sizeof(RGBData), rgbCmp);\n"
"    return r ? r->value : 0x80000000;\n}\n\n\n"
"#else\n\n\n"
"uint qGetRGBValue( const char * )\n{\n    return 0x80000000;\n}\n\n\n"
"#endif // NO_COLORNAME\n";

void outputData( ColorVector *cv )
{
    printf( header );
    printf( rgb_tblsize, cv->count() );
    printf( rgb_struct );
    for ( int i=0; i<cv->count(); i++ ) {
	Color *c = cv->at( i );
	printf( "  { RGB(%3d,%3d,%3d),\t\"%s\" }", c->r, c->g, c->b,
		(const char *)c->name );
	if ( i == cv->count()-1 )
	    printf( " };\n\n" );
	else
	    printf( ",\n" );
    }
    printf( rgb_func );
}


int main( int argc, char **argv )
{
    FILE *f;
    if ( argc == 1 )
	f = stdin;
    else
    if ( argc == 2 ) {
	f = fopen( argv[1], "r" );
	if ( !f ) {
	    fprintf( stderr, "makergb: Cannot open file %s\n", argv[1] );
	    return 1;
	}
    } else {
	fprintf( stderr, "Usage:\n\tmakergb [file]\n\nInput file should be"
		 "in rgb.txt format\n" );
	return 1;
    }

    ColorVector *cv = readData( f );
    fclose( f );

    cv->sort();

    outputData( cv );
    delete cv;

    return 0;
}
