/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** EDITIONS: UNKNOWN
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <qmetafil.h>
#include <qrect.h>
#include <qpointarray.h>
#include <qwxfmat.h>
#include <qcolor.h>
#include <qfile.h>
#include <qbuffer.h>
#include <qdatastream.h>


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
	qWarning( "qmf2cpp: Empty metafile" );
	return false;
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
    QPolygon a;
    QColor color;
    char *str;

    Q_UINT32 tag;				// metafile tag
    Q_UINT16 cs;				// checksum
    int major, minor;
    s >> tag >> cs >> major >> minor;
    if ( tag != mfhdr_tag ) {
	fprintf( stderr, "qmf2cpp: Not a metafile: %s\n", (char *)fileName );
	return false;
    }
    int start_data = sizeof(tag) + sizeof(cs);
    if ( qchecksum(bytes.data()+start_data, bytes.size()-start_data) != cs ) {
	fprintf( stderr, "qmf2cpp: Invalid checksum: %s\n", (char *)fileName );
	return false;
    }
    printf( "// C++ code generated from Qt metafile %s (ver. %d.%d)\n",
	    (char *)fileName, major, minor );

    printf( "void paintIt( QPaintDevice *pdev )\n{\n" );
    printf( "    QPainter painter;\n" );
    printf( "    painter.begin( pdev );\n" );
    while ( !s.device()->atEnd() ) {
	s >> c;					// get command
	s >> len;				// get param length
	if ( c == PdcDrawLineSegments || c == PdcDrawPolyline ||
	     c == PdcDrawPolygon ) {
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
	    printf( "    QPolygon pa_%d( pa_data_%d, %d );\n",
		    pa_count, pa_count, a.size() );
	}
	if ( c != PdcEnd )
	    printf( "    painter." );
	switch ( c ) {
	    case PdcDrawPoint:
	        s >> p;
	    	printf( "drawPoint( %d, %d );\n", p.x(), p.y() );
	        break;
	    case PdcMoveTo:
	        s >> p;
	    	printf( "moveTo( %d, %d );\n", p.x(), p.y() );
	        break;
	    case PdcLineTo:
	        s >> p;
	    	printf( "lineTo( %d, %d );\n", p.x(), p.y() );
		break;
	    case PdcDrawLine:
	        s >> p >> p2;
		printf( "drawLine( %d, %d, %d, %d );\n", p.x(), p.y(),
			p2.x(), p2.y() );
		break;
	    case PdcDrawRect:
		s >> r;
		printf( "drawRect( %d, %d, %d, %d );\n",
		 	r.left(), r.top(), r.width(), r.height() );
		break;
	    case PdcDrawRoundRect:
		s >> r >> i1 >> i2;
		printf( "drawRoundRect( %d, %d, %d, %d, %d, %d );\n",
			r.left(), r.top(), r.width(), r.height(),
		        i1, i2 );
		break;
	    case PdcDrawEllipse:
		s >> r;
		printf( "drawEllipse( %d, %d, %d, %d );\n",
		 	r.left(), r.top(), r.width(), r.height() );
		break;
	    case PdcDrawArc:
		s >> r >> i1 >> i2;
		printf( "drawArc( %d, %d, %d, %d, %d, %d );\n",
			r.left(), r.top(), r.width(), r.height(),
		        i1, i2 );
		break;
	    case PdcDrawPie:
		s >> r >> i1 >> i2;
		printf( "drawPie( %d, %d, %d, %d, %d, %d );\n",
			r.left(), r.top(), r.width(), r.height(),
		        i1, i2 );
		break;
	    case PdcDrawChord:
		s >> r >> i1 >> i2;
		printf( "drawChord( %d, %d, %d, %d, %d, %d );\n",
			r.left(), r.top(), r.width(), r.height(),
		        i1, i2 );
		break;
	    case PdcDrawLineSegments:
		printf( "drawLineSegments( pa_%d );\n", pa_count++ );
		break;
	    case PdcDrawPolyline:
		printf( "drawPolyline( pa_%d );\n", pa_count++ );
		break;
	    case PdcDrawPolygon:
		s >> i1;
		printf( "drawPolygon( pa_%d, %d );\n", pa_count++, i1 );
		break;
	    case PdcDrawText:
	        s >> p >> str;
	        printf( "drawText( %d,%d, \"%s\" );\n", p.x(), p.y(),
		        str );
	        delete str;
	        break;
	    case PdcSetBkColor:
	        s >> ul;
		color.setRGB( ul );
	        printf( "setBackgroundColor( QColor(%d,%d,%d) );\n",
			color.red(), color.green(), color.blue() );
	        break;
	    case PdcSetBkMode:
	        s >> i1;
	        printf( "setBackgroundMode( %d );\n", i1 );
	        break;
	    case PdcSetROP:
		s >> i1;
		printf( "setRasterOp( (RasterOp)%d );\n", i1 );
		break;
	    case PdcSetPen:
	        s >> i1 >> i2 >> ul;
		color.setRGB( ul );
	        printf( "setPen( QPen( QColor(%d,%d,%d), %d, (PenStyle)%d ) );\n",
			color.red(), color.green(), color.blue(), i2, i1 );
		break;
	    case PdcSetBrush:
		s >> i1 >> ul;
		color.setRGB( ul );
		printf( "setBrush( QBrush( QColor(%d,%d,%d), (BrushStyle)%d ) );\n",
			color.red(), color.green(), color.blue(), i1 );
		break;
	    case PdcSetVXform:
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
	    case PdcSetWXform:
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

	    case PdcEnd:
		break;
	    default:
		if ( len )			// skip unknown params
		    s.device()->at( s.device()->at()+len );
		printf( "isActive(); // NOTE: Skipping unsupported feature\n");
	}
    }
    printf( "    painter.end();\n}\n" );
}
