/**********************************************************************
** $Id: //depot/qt/main/src/kernel/qpsprn.cpp#2 $
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
#include "qdatetm.h"

#include "qfile.h"
#include "qbuffer.h"

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qpsprn.cpp#2 $";
#endif


/*!
  \class QPSPrinter qpsprn.h
  \brief Internal class used by QPrinter under X-Windows to generate
  PostScript (tm).
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

static void ps_setFont( QTextStream *s, const QFont *f )
{
    QString family = f->family();
    QString ps;
    int  weight = f->weight();
    bool italic = f->italic();
    bool times  = FALSE;
    family = family.lower();
    if ( family == "courier" )
	ps = "/Courier";
    else if ( family == "times" ) {
	ps = "/Times";
	times = TRUE;
    }
    else
	ps = "/Helvetica";
    QString extra;
    if ( weight > 60 )
	extra = "Bold";
    if ( italic ) {
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
}


#undef XCOORD
#undef YCOORD
#undef WIDTH
#undef HEIGHT
#undef POINT
#undef RECT
#undef INT_ARG
#undef COLOR
#undef PA

#define XCOORD(x)	(float)(x)
#define YCOORD(y)	(float)(y)
#define WIDTH(w)	(float)(w)
#define HEIGHT(h)	(float)(h)

#define POINT(index)	XCOORD(p[index].point->x()) << ' ' <<		\
		        YCOORD(p[index].point->y()) << ' '
#define RECT(index)	XCOORD(p[index].rect->x())  << ' ' <<		\
			YCOORD(p[index].rect->y())  << ' ' <<		\
			WIDTH (p[index].rect->width())  << ' ' <<	\
			HEIGHT(p[index].rect->height()) << ' '
#define INT_ARG(index)	p[index].ival << ' '
#define COLOR(x)	(x).red()   << ' ' <<	\
			(x).green() << ' ' <<	\
			(x).blue()  << ' '
#define PA(index) (p[index].ptarr)


bool QPSPrinter::cmd( int c , QPainter *paint, QPDevCmdParam *p )
{
    if ( c == PDC_BEGIN ) {			// start painting
	pageCount   = 1;			// initialize state
	dirtyMatrix = TRUE;

	const char *title   = printer->docName();
	const char *creator = printer->creator();
	if ( !title )				// default document name
	    title = "Unknown";
	if ( !creator )				// default creator
	    creator = "Qt";
	stream.setDevice( device );
	stream << "%!PS-Adobe-1.0\n";		// write document header
	stream << "%%Creator: " << creator << '\n';
	stream << "%%Title: "   << title   << '\n';
	stream << "%%CreationDate:" << QDateTime::currentDateTime().toString()
	  << '\n';
	stream << "%%Pages: (atend)\n";
	stream << "%%DocumentFonts: (atend)\n";
	stream << "%%EndComments\n\n";
	if ( printer->numCopies() > 1 )
	    stream << "/#copies " << printer->numCopies() << " def\n";

	QFile f( "/usr/lib/qtheader.ps" );	// read predefined PS header
	if ( !f.open(IO_ReadOnly|IO_Raw) )
	    fatal( "Cannot open /usr/lib/qtheader.ps" );
	QByteArray a(f.size());
	f.readBlock( a.data(), f.size() );
	f.close();
	stream.writeRawBytes( a.data(), a.size() );
	stream << "\n%%Page: " << pageCount << ' ' << pageCount << endl;
	return TRUE;
    }
    
    if ( c == PDC_END ) {			// painting done
	stream << "QtFinish\n";
	stream << "%%Trailer\n";
	stream << "%%Pages: " << pageCount << '\n';
	stream << "%%DocumentFonts: Courier\n";
	device->close();
	stream.unsetDevice();
    }

    if ( c >= PDC_DRAW_FIRST && c <= PDC_DRAW_LAST ) {
	if ( dirtyMatrix ) {
	    QWMatrix tmp;
	    if ( paint->hasViewXForm ) {
		QRect viewport = paint->viewport();
		QRect window   = paint->window();
		tmp.translate( viewport.x(), viewport.y() );
		tmp.scale( 1.0 * viewport.width()  / window.width(),
			   1.0 * viewport.height() / window.height() );
		tmp.translate( -window.x(), -window.y() );
	    }
	    if ( paint->hasWorldXForm )
		tmp = paint->worldMatrix() * tmp;
	    stream << "[ "
		   << tmp.m11() << ' ' << tmp.m12() << ' '
		   << tmp.m21() << ' ' << tmp.m22() << ' '
		   << tmp.dx()  << ' ' << tmp.dy()  << " ] ST\n";
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
	    if ( PA(0)->size() > 0 ) {
		QPointArray a = *PA(0);
		QPoint pt;
		for ( int i=0; i<a.size(); i+=2 ) {
		    pt = a.point( i );
		    stream << XCOORD(pt.x()) << ' '
			   << YCOORD(pt.y()) << " MT\n";
		    pt = a.point( i+1 );
		    stream << XCOORD(pt.x()) << ' '
			   << YCOORD(pt.y()) << " LT\n";
		}
	    }
	    break;
	case PDC_DRAWPOLYLINE:
	    if ( PA(0)->size() > 0 ) {
		QPointArray a = *PA(0);
		QPoint pt = a.point( 0 );
		stream << XCOORD(pt.x()) << ' ' << YCOORD(pt.y()) << " MT\n";
		for ( int i=1; i<a.size(); i++ ) {
		    pt = a.point( i );
		    stream << XCOORD(pt.x()) << ' '
			   << YCOORD(pt.y()) << " LT\n";
		}
	    }
	    break;
	case PDC_DRAWPOLYGON:
	    if ( PA(0)->size() > 0 ) {
		QPointArray a = *PA(0);
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
	    break;
	case PDC_DRAWTEXT:
	    stream << POINT(0) << "(" << p[1].str << ") T\n";
	    break;
	case PDC_DRAWTEXTFRMT: {
	    QPoint pt = p[0].rect->topLeft();
	    stream << XCOORD(pt.x()) << ' ' << YCOORD(pt.y())
		   << " (" << p[3].str << ") T\n";
	    }
	    break;
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
		debug( "QPSPrinter: Raster operation setting not supported" );
#endif
	    break;
	case PDC_SETBRUSHORIGIN:
	    break;
	case PDC_SETFONT:
	    ps_setFont( &stream, p[0].font );
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
	    stream << p[0].brush->style() << ' ' 
		   << COLOR(p[0].brush->color()) << "B\n";
	    break;
	case PDC_SETTABSTOPS:
	    break;
	case PDC_SETTABARRAY:
	    break;
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
	    debug( "QPSPrinter: Clipping not supported" );
#endif
	    break;
	case PDC_SETCLIPRGN:
#if defined(DEBUG)
	    debug( "QPSPrinter: Clipping not supported" );
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
