/**********************************************************************
** $Id: //depot/qt/main/src/kernel/qpsprinter.cpp#11 $
**
** Implementation of QPSPrinter class
**
** Author  : Eirik Eng
** Created : 941003
**
** Copyright (C) 1994,1995 by Troll Tech AS.  All rights reserved.
**
***********************************************************************/

#include "qpsprn.h"
#include "qpainter.h"
#include "qpaintdc.h"
#include "qimage.h"
#include "qdatetm.h"

#include "qfile.h"
#include "qbuffer.h"

RCSTAG("$Id: //depot/qt/main/src/kernel/qpsprinter.cpp#11 $")


#if !defined(QT_HEADER_PS)
static char *ps_header =
#include "qpshdr.cpp"
;
#endif


/*!
  \class QPSPrinter qpsprn.h
  \brief Internal class used by QPrinter under X-Windows to generate
  PostScript (tm).

  \internal
*/

/*!
  \internal
  Constructs a PS printer driver connected to the printer \e prt.
*/

QPSPrinter::QPSPrinter( QPrinter *prt )
    : QPaintDevice( PDT_PRINTER | PDF_EXTDEV )
{
    printer = prt;
    device = 0;
}

QPSPrinter::~QPSPrinter()
{
}

//
// Sets a new font for PostScript
//

static void ps_setFont( QTextStream *s, const QFont *f, QString *fonts )
{
    if ( f->rawMode() ) {
	QFont fnt( "Helvetica", 12 );
	ps_setFont( s, &fnt, fonts );
	return;
    }
    if ( f->pointSize() == 0 ) {
#if defined(CHECK_RANGE)
	warning( "QPrinter: Cannot set a font with zero point size." );
#endif
	return;
    }

    QString family = f->family();
    QString ps;
    int	 weight = f->weight();
    bool italic = f->italic();
    bool times	= FALSE;
    bool symbol = FALSE;
    family = family.lower();
    if ( family == "courier" )
	ps = "/Courier";
    else if ( family == "times" ) {
	ps = "/Times";
	times = TRUE;
    }
    else if ( family == "symbol" ) {
	ps = "/Symbol";
	symbol = TRUE;
    }
    else
	ps = "/Helvetica";
    QString extra;
    if ( weight >= QFont::Bold && !symbol )
	extra = "Bold";
    if ( italic && !symbol ) {
	if ( times )
	    extra += "Italic";
	else
	    extra += "Oblique";
    }
    if ( !extra.isEmpty() ) {
	ps += '-';
	ps += extra;
    }
    else {
	if ( times )
	    ps += "-Roman";
    }
    QString fontMatrix;
    fontMatrix.sprintf( "[ %d 0 0 -%d 0 0 ]", f->pointSize(), f->pointSize() );
    *s << ps << " findfont " << fontMatrix << " makefont setfont\n";
    ps.remove( 0, 1 );	// removes the '/'
    ps += ' ';
    if ( !fonts->contains(ps) )
	*fonts += ps;
}

static void hexOut( QTextStream &stream, int i )
{
    if ( i < 0x10 )
	stream << '0';
    stream << i;
}

static void ps_dumpTransparentBitmapData( QTextStream &stream, QImage &img )
{
    stream.setf( QTextStream::hex );

    int width  = img.width();
    int height = img.height();
    int numBytes = (width + 7)/8;
    uchar *scanLine;
    int x,y;
    int count = -1;
    for( y = 0 ; y < height ; y++ ) {
	scanLine = img.scanLine(y);
	for( x = 0 ; x < numBytes ; x++ ) {
	    hexOut( stream, scanLine[x] );
	    if ( !(count++ % 66) )
		stream << '\n';
	}
    }
    if ( --count % 66 )
	stream << '\n';

    stream.setf( QTextStream::dec );
}

static void ps_dumpPixmapData( QTextStream &stream, QImage img,
			       QColor fgCol, QColor bgCol )
{
    stream.setf( QTextStream::hex );

    if ( img.depth() == 1 ) {
	img.convertDepth( 8 );
	if ( img.color(0) == 0 ) {			// black
	    img.setColor( 0, fgCol.rgb() );
	    img.setColor( 1, bgCol.rgb() );
	} else {
	    img.setColor( 0, bgCol.rgb() );
	    img.setColor( 1, fgCol.rgb() );
	}
    }

    int width  = img.width();
    int height = img.height();
    int pixWidth = img.depth() == 8 ? 1 : 3;
    uchar *scanLine;
    ulong cval;
    int x,y;
    int count = -1;
    for( y = 0 ; y < height ; y++ ) {
	scanLine = img.scanLine(y);
	for( x = 0 ; x < width ; x++ ) {
	    if ( pixWidth == 1 ) {
		cval = img.color( scanLine[x] );
		hexOut( stream, QRED(cval) );
		hexOut( stream, QGREEN(cval) );
		hexOut( stream, QBLUE(cval) );
	    } else {
		hexOut( stream, scanLine[3*x] );
		hexOut( stream, scanLine[3*x + 1] );
		hexOut( stream, scanLine[3*x + 2] );
	    }
	    if ( !(count++ % 11) )
		stream << '\n';
	}
    }
    if ( --count % 11 )
	stream << '\n';

    stream.setf( QTextStream::dec );
}

#undef XCOORD
#undef YCOORD
#undef WIDTH
#undef HEIGHT
#undef POINT
#undef RECT
#undef INT_ARG
#undef COLOR

#define XCOORD(x)	(float)(x)
#define YCOORD(y)	(float)(y)
#define WIDTH(w)	(float)(w)
#define HEIGHT(h)	(float)(h)

#define POINT(index)	XCOORD(p[index].point->x()) << ' ' <<		\
			YCOORD(p[index].point->y()) << ' '
#define RECT(index)	XCOORD(p[index].rect->x())  << ' ' <<		\
			YCOORD(p[index].rect->y())  << ' ' <<		\
			WIDTH (p[index].rect->width())	<< ' ' <<	\
			HEIGHT(p[index].rect->height()) << ' '
#define INT_ARG(index)	p[index].ival << ' '
#define COLOR(x)	(x).red()   << ' ' <<	\
			(x).green() << ' ' <<	\
			(x).blue()  << ' '


bool QPSPrinter::cmd( int c , QPainter *paint, QPDevCmdParam *p )
{
    if ( c == PDC_BEGIN ) {			// start painting
	pageCount   = 1;			// initialize state
	dirtyMatrix = TRUE;
	fontsUsed   = "";
	const char *title   = printer->docName();
	const char *creator = printer->creator();
	if ( !title )				// default document name
	    title = "Unknown";
	if ( !creator )				// default creator
	    creator = "Qt";
	stream.setDevice( device );
	stream << "%!PS-Adobe-1.0\n";		// write document header
	stream << "%%Creator: " << creator << '\n';
	stream << "%%Title: "	<< title   << '\n';
	stream << "%%CreationDate:" << QDateTime::currentDateTime().toString()
	  << '\n';
	stream << "%%Pages: (atend)\n";
	stream << "%%DocumentFonts: (atend)\n";
	stream << "%%EndComments\n\n";
	if ( printer->numCopies() > 1 )
	    stream << "/#copies " << printer->numCopies() << " def\n";

#if defined(QT_HEADER_PS)
	QFile f( "/usr/lib/qtheader.ps" );	// read predefined PS header
	if ( !f.open(IO_ReadOnly|IO_Raw) )
	    fatal( "Cannot open /usr/lib/qtheader.ps" );
	QByteArray a(f.size());
	f.readBlock( a.data(), f.size() );
	f.close();
	stream.writeRawBytes( a.data(), a.size() );
#else
	stream << ps_header;
#endif

	stream << "\n%%Page: " << pageCount << ' ' << pageCount << endl;
	return TRUE;
    }

    if ( c == PDC_END ) {			// painting done
	stream << "QtFinish\n";
	stream << "%%Trailer\n";
	stream << "%%Pages: " << pageCount << '\n';
	stream << "%%DocumentFonts: " << fontsUsed << '\n';
	device->close();
	stream.unsetDevice();
    }

    if ( c >= PDC_DRAW_FIRST && c <= PDC_DRAW_LAST ) {
	if ( dirtyMatrix ) {
	    QWMatrix tmp;
	    if ( paint->hasViewXForm() ) {
		QRect viewport = paint->viewport();
		QRect window   = paint->window();
		tmp.translate( viewport.x(), viewport.y() );
		tmp.scale( 1.0 * viewport.width()  / window.width(),
			   1.0 * viewport.height() / window.height() );
		tmp.translate( -window.x(), -window.y() );
	    }
	    if ( paint->hasWorldXForm() ) {
		tmp = paint->worldMatrix() * tmp;
	    }
	    stream << "[ "
		   << tmp.m11() << ' ' << tmp.m12() << ' '
		   << tmp.m21() << ' ' << tmp.m22() << ' '
		   << tmp.dx()	<< ' ' << tmp.dy()  << " ] ST\n";
	    dirtyMatrix = FALSE;
	}
    }

    switch( c ) {
	case PDC_DRAWPOINT:
	    stream << POINT(0) << "P\n";
	    break;
	case PDC_MOVETO:
	    stream << POINT(0) << "M\n";
	    break;
	case PDC_LINETO:
	    stream << POINT(0) << "L\n";
	    break;
	case PDC_DRAWLINE:
	    stream << POINT(0) << POINT(1) << "DL\n";
	    break;
	case PDC_DRAWRECT:
	    stream << RECT(0) << "R\n";
	    break;
	case PDC_DRAWROUNDRECT:
	    stream << RECT(0) << INT_ARG(1) << INT_ARG(2) << "RR\n";
	    break;
	case PDC_DRAWELLIPSE:
	    stream << RECT(0) << "E\n";
	    break;
	case PDC_DRAWARC:
	    stream << RECT(0) << INT_ARG(1) << INT_ARG(2) << "A\n";
	    break;
	case PDC_DRAWPIE:
	    stream << RECT(0) << INT_ARG(1) << INT_ARG(2) << "PIE\n";
	    break;
	case PDC_DRAWCHORD:
	    stream << RECT(0) << INT_ARG(1) << INT_ARG(2) << "CH\n";
	    break;
	case PDC_DRAWLINESEGS:
	    if ( p[0].ptarr->size() > 0 ) {
		QPointArray a = *p[0].ptarr;
		QPoint pt;
		for ( int i=0; i<a.size(); i+=2 ) {
		    pt = a.point( i );
		    stream << XCOORD(pt.x()) << ' '
			   << YCOORD(pt.y()) << " MT\n";
		    pt = a.point( i+1 );
		    stream << XCOORD(pt.x()) << ' '
			   << YCOORD(pt.y()) << " LT\n";
		}
		stream << "QtStroke\n";
	    }
	    break;
	case PDC_DRAWPOLYLINE:
	    if ( p[0].ptarr->size() > 0 ) {
		QPointArray a = *p[0].ptarr;
		QPoint pt = a.point( 0 );
		stream << XCOORD(pt.x()) << ' ' << YCOORD(pt.y()) << " MT\n";
		for ( int i=1; i<a.size(); i++ ) {
		    pt = a.point( i );
		    stream << XCOORD(pt.x()) << ' '
			   << YCOORD(pt.y()) << " LT\n";
		}
		stream << "QtStroke\n";
	    }
	    break;
	case PDC_DRAWPOLYGON:
	    if ( p[0].ptarr->size() > 0 ) {
		QPointArray a = *p[0].ptarr;
		if ( p[1].ival )
		    stream << "/WFi true def";
		else
		    stream << "/WFi false def";
		QPoint pt = a.point(0);
		stream << "NP\n";
		stream << XCOORD(pt.x()) << ' '
		       << XCOORD(pt.y()) << " MT\n";
		for( int i=1; i<(int)a.size(); i++) {
		    pt = a.point( i );
		    stream << XCOORD(pt.x()) << ' '
			   << YCOORD(pt.y()) << " LT\n";
		}
		stream << "CP\n";
		stream << "QtFill\n";
		stream << "QtStroke\n";
	    }
	    break;
	case PDC_DRAWBEZIER:
	    if ( p[0].ptarr->size() > 0 ) {
		QPointArray a = p[0].ptarr->bezier();
		QPDevCmdParam param;
		param.ptarr = &a;
		cmd( PDC_DRAWPOLYLINE, paint, &param );
	    }
	    break;
	case PDC_DRAWTEXT:
	    stream << POINT(0) << "(" << p[1].str << ") T\n";
	    break;
	case PDC_DRAWTEXTFRMT:;
	    return FALSE;			// uses Qt instead
	case PDC_DRAWPIXMAP: {
	    if ( p[1].pixmap->isNull() )
		break;
	    int depth = p[1].pixmap->depth();
	    if ( depth == 1 ) {
		warning( "QPrinter: Sorry, pixmaps with depth 1 are not "
			 "supported in Qt 0.92 - Try an 8 bit pixmap." );
		return FALSE;
	    }
	    if ( depth != 1 && depth != 8 && depth != 32 ) {
		warning( "QPrinter::cmd: Unsupported image depth "
			 "(1, 8 or 24 supported).");
		break;
	    }

	    QPoint pnt = *(p[0].point);
	    stream << pnt.x() << " " << pnt.y() << " TR\n";
	    QImage img;
	    img = *(p[1].pixmap);
	    bool mask = ( paint->backgroundMode() == TransparentMode &&
			  depth == 1 );
	    int width  = img.width();
	    int height = img.height();

	    QColor fgCol = paint->pen().color();
	    QColor bgCol = paint->backgroundColor();
	    if ( mask )
		stream << COLOR(fgCol) << "CRGB SRGB\n";
	    stream << "/sl " << (mask ? (width + 7)/8 : width*3)
		   << " string def\n";
	    stream << width << " " << height;
	    if ( !mask )
		stream << " 8 ";
	    stream << "[1 0 0 1 0 0] { currentfile sl readhexstring pop }\n";
	    if ( mask ) {
		stream << "imagemask\n";
		QColor fgCol = paint->pen().color();
		ps_dumpTransparentBitmapData( stream, img );
	    } else {
		stream << "false 3 colorimage\n";
		ps_dumpPixmapData( stream, img, fgCol, bgCol );
	    }
	    stream << -pnt.x() << " " << -pnt.y() << " TR\n";
	    break;
	}
	case PDC_SAVE:
	    stream << "SV\n";
	    break;
	case PDC_RESTORE:
	    stream << "RS\n";
	    break;
	case PDC_SETBKCOLOR:
	    stream << COLOR(*(p[0].color)) << "BC\n";
	    break;
	case PDC_SETBKMODE:
	    stream << "/OMo ";
	    if ( p[0].ival == TransparentMode )
		stream << "false";
	    else
		stream << "true";
	    stream << " def\n";
	    break;
	case PDC_SETROP:
#if defined(DEBUG)
	    if ( p[0].ival != CopyROP )
		debug( "QPrinter: Raster operation setting not supported" );
#endif
	    break;
	case PDC_SETBRUSHORIGIN:
	    break;
	case PDC_SETFONT:
	    ps_setFont( &stream, p[0].font, &fontsUsed );
	    break;
	case PDC_SETPEN:
	    if ( p[0].pen->width() == 0 )
		stream << p[0].pen->style()	       << " 0.3 "
		       << COLOR(p[0].pen->color()) << "PE\n";
	    else
		stream << p[0].pen->style() << ' ' << p[0].pen->width()
		       << COLOR(p[0].pen->color()) << "PE\n";
	    break;
	case PDC_SETBRUSH:
	    if ( p[0].brush->style() == CustomPattern ) {
		warning( "QPrinter: Pixmap brush not supported" );
		return FALSE;
	    }
	    stream << p[0].brush->style() << ' '
		   << COLOR(p[0].brush->color()) << "B\n";
	    break;
	case PDC_SETTABSTOPS:
	case PDC_SETTABARRAY:
	    return FALSE;
	case PDC_SETUNIT:
	    break;
	case PDC_SETVXFORM:
	case PDC_SETWINDOW:
	case PDC_SETVIEWPORT:
	case PDC_SETWXFORM:
	case PDC_SETWMATRIX:
	    dirtyMatrix = TRUE;
	    break;
	case PDC_SETCLIP:
#if defined(DEBUG)
	    debug( "QPrinter: Clipping not supported" );
#endif
	    break;
	case PDC_SETCLIPRGN:
#if defined(DEBUG)
	    debug( "QPrinter: Clipping not supported" );
#endif
	    break;
	case PDC_PRT_NEWPAGE:
	    stream << "showpage\n";
	    pageCount++;
	    stream << "\n%%Page: " << pageCount << ' ' << pageCount << endl;
	    break;
	case PDC_PRT_ABORT:
	    break;
	default:
	    break;
    }
    return TRUE;
}
