/****************************************************************************
** $Id: //depot/qt/main/util/qpic2cpp/qpic2cpp.cpp#4 $
**
** This is a utility program for converting Qt metafiles to C++ code
**
** Author  : Haavard Nord
** Created : 940802
**
** Copyright (C) 1994 by Troll Tech AS.  All rights reserved.
**
** --------------------------------------------------------------------------
** This utility translates a portable Qt metafile into a standalone C++
** function that performs the drawing functions in the metafile.
**
** A Qt metafile starts with a header containing a tag, a checksum and
** a version number, followed by drawing commands.
** Each command consists of an identifier tag, a length identifier and
** the actual drawing parameters. Qt metafiles are upwards compatible.
*****************************************************************************/

#include <qmetafil.h>
#include <qrect.h>
#include <qpntarry.h>
#include <qwxfmat.h>
#include <qcolor.h>
#include <qfile.h>
#include <qbuffer.h>
#include <qdstream.h>


static const Q_UINT32 mfhdr_tag = 0x11140d06;	// header tag
static const int    mfhdr_maj = 1;		// major version #
static const int    mfhdr_min = 0;		// minor version #

bool genCPlusPlus( QString fileName, QFile & );


int qMain( int argc, char **argv )		// plain main
{
    if ( argc < 2 ) {				// print usage
	fprintf( stderr, "usage: \n\tqmf2cpp <Qt metafile>\n" );
	return 1;
    }
    int retcode = 0;
    QString fn = argv[1];			// get filename argument
    QFile file( argv[1] );
    if ( !file.open( IO_ReadOnly ) ) {		// cannot open file
	fprintf( stderr, "qmf2cpp: Cannot open %s", (char *)fn );
	return 1;
    }
    if ( !genCPlusPlus( fn, file ) )		// could not translate metafile
	retcode = 1;
    return retcode;
}


bool genCPlusPlus( QString fileName, QFile &file )
{
    if ( file.size() == 0 ) {			// nothing recorded
	warning( "qmf2cpp: Empty metafile" );
	return FALSE;
    }
    QByteArray bytes( file.size() );		// create byte array
    file.readBlock( bytes.data(), file.size() );// read metafile into array
    file.close();
    QBuffer iodevice( bytes );			// use buffer device
    QDataStream s;
    iodevice.open( IO_ReadOnly );
    s.setDevice( &iodevice );    
    int pa_count = 0;
    int c, len, i1, i2;
    ulong ul;
    QPoint p, p2;
    QRect r;
    QPointArray a;
    QColor color;
    char *str;

    Q_UINT32 tag;				// metafile tag
    Q_UINT16 cs;				// checksum
    int major, minor;
    s >> tag >> cs >> major >> minor;
    if ( tag != mfhdr_tag ) {
	fprintf( stderr, "qmf2cpp: Not a metafile: %s\n", (char *)fileName );
	return FALSE;
    }
    int start_data = sizeof(tag) + sizeof(cs);
    if ( qchecksum(bytes.data()+start_data, bytes.size()-start_data) != cs ) {
	fprintf( stderr, "qmf2cpp: Invalid checksum: %s\n", (char *)fileName );
	return FALSE;
    }
    printf( "// C++ code generated from Qt metafile %s (ver. %d.%d)\n",
	    (char *)fileName, major, minor );

    printf( "void paintIt( QPaintDevice *pdev )\n{\n" );
    printf( "    QPainter painter;\n" );
    printf( "    painter.begin( pdev );\n" );
    while ( !s.device()->atEnd() ) {
	s >> c;					// get command
	s >> len;				// get param length
	if ( c == PDC_DRAWLINESEGS || c == PDC_DRAWPOLYLINE ||
	     c == PDC_DRAWPOLYGON ) {
	    s >> a;				// read point array
	    printf( "    static QCOOT pa_data_%d[] = {\n\t", pa_count );
	    for ( int i=0; i<a.size(); i++ ) {	// create C++ point array
		if ( i != 0 ) {
		    printf( "," );
		    if ( i % 8 == 0 )
			printf( "\n\t" );
		}
		printf( " %d,%d", a.point(i).x(), a.point(i).y() );
	    }
	    printf( " };\n" );
	    printf( "    QPointArray pa_%d( pa_data_%d, %d );\n",
		    pa_count, pa_count, a.size() );	    
	}
	if ( c != PDC_END )
	    printf( "    painter." );
	switch ( c ) {
	    case PDC_DRAWPOINT:
	        s >> p;
	    	printf( "drawPoint( %d, %d );\n", p.x(), p.y() );
	        break;
	    case PDC_MOVETO:
	        s >> p;
	    	printf( "moveTo( %d, %d );\n", p.x(), p.y() );
	        break;
	    case PDC_LINETO:
	        s >> p;
	    	printf( "lineTo( %d, %d );\n", p.x(), p.y() );
		break;
	    case PDC_DRAWLINE:
	        s >> p >> p2;
		printf( "drawLine( %d, %d, %d, %d );\n", p.x(), p.y(),
			p2.x(), p2.y() );
		break;
	    case PDC_DRAWRECT:
		s >> r;
		printf( "drawRect( %d, %d, %d, %d );\n",
		 	r.left(), r.top(), r.width(), r.height() );
		break;
	    case PDC_DRAWROUNDRECT:
		s >> r >> i1 >> i2;
		printf( "drawRoundRect( %d, %d, %d, %d, %d, %d );\n",
			r.left(), r.top(), r.width(), r.height(),
		        i1, i2 );
		break;
	    case PDC_DRAWELLIPSE:
		s >> r;
		printf( "drawEllipse( %d, %d, %d, %d );\n",
		 	r.left(), r.top(), r.width(), r.height() );
		break;
	    case PDC_DRAWARC:
		s >> r >> i1 >> i2;
		printf( "drawArc( %d, %d, %d, %d, %d, %d );\n",
			r.left(), r.top(), r.width(), r.height(),
		        i1, i2 );
		break;
	    case PDC_DRAWPIE:
		s >> r >> i1 >> i2;
		printf( "drawPie( %d, %d, %d, %d, %d, %d );\n",
			r.left(), r.top(), r.width(), r.height(),
		        i1, i2 );
		break;
	    case PDC_DRAWCHORD:
		s >> r >> i1 >> i2;
		printf( "drawChord( %d, %d, %d, %d, %d, %d );\n",
			r.left(), r.top(), r.width(), r.height(),
		        i1, i2 );
		break;
	    case PDC_DRAWLINESEGS:
		printf( "drawLineSegments( pa_%d );\n", pa_count++ );
		break;
	    case PDC_DRAWPOLYLINE:
		printf( "drawPolyline( pa_%d );\n", pa_count++ );
		break;
	    case PDC_DRAWPOLYGON:
		s >> i1;
		printf( "drawPolygon( pa_%d, %d );\n", pa_count++, i1 );
		break;
	    case PDC_DRAWTEXT:
	        s >> p >> str;
	        printf( "drawText( %d,%d, \"%s\" );\n", p.x(), p.y(),
		        str );		
	        delete str;
	        break;
	    case PDC_SETBKCOLOR:
	        s >> ul;
		color.setRGB( ul );
	        printf( "setBackgroundColor( QColor(%d,%d,%d) );\n",
			color.red(), color.green(), color.blue() );
	        break;
	    case PDC_SETBKMODE:
	        s >> i1;
	        printf( "setBackgroundMode( %d );\n", i1 );
	        break;
	    case PDC_SETROP:
		s >> i1;
		printf( "setRasterOp( (RasterOp)%d );\n", i1 );
		break;
	    case PDC_SETPEN:
	        s >> i1 >> i2 >> ul;
		color.setRGB( ul );
	        printf( "setPen( QPen( QColor(%d,%d,%d), %d, (PenStyle)%d ) );\n",
			color.red(), color.green(), color.blue(), i2, i1 );
		break;
	    case PDC_SETBRUSH:
		s >> i1 >> ul;
		color.setRGB( ul );
		printf( "setBrush( QBrush( QColor(%d,%d,%d), (BrushStyle)%d ) );\n",
			color.red(), color.green(), color.blue(), i1 );
		break;
	    case PDC_SETVXFORM:
	        s >> i1;
	        printf( "setViewXForm( %d );\n", i1 );
	        break;
	    case PDC_SETSOURCEVIEW:
	        s >> r;
	        printf( "setSourceView( QRect(%d,%d,%d,%d) );\n",
			r.left(), r.top(), r.width(), r.height() );
	        break;
	    case PDC_SETTARGETVIEW:
	        s >> r;
	        printf( "setTargetView( QRect(%d,%d,%d,%d) );\n",
			r.left(), r.top(), r.width(), r.height() );
	        break;
	    case PDC_SETWXFORM:
	        s >> i1;
	        printf( "setWorldXForm( %d );\n", i1 );
	        break;
	    case PDC_SETWXFMATRIX: {
		QWXFMatrix m;
		s >> m;
	        printf( "setWxfMatrix( QWXFMatrix(%g,%g,%g,%g,%g,%g) );\n",
			m.m11(), m.m12(), m.m21(), m.m22(), m.dx(), m.dy() );
	        }
		break;

	    case PDC_END:
		break;
	    default:
		if ( len )			// skip unknown params
		    s.device()->at( s.device()->at()+len );
		printf( "isActive(); // NOTE: Skipping unsupported feature\n");
	}
    }
    printf( "    painter.end();\n}\n" );
}
