/**********************************************************************
** $Id: //depot/qt/main/src/kernel/qpsprinter.cpp#35 $
**
** Implementation of QPSPrinter class
**
** Created : 941003
**
** Copyright (C) 1994-1997 by Troll Tech AS.  All rights reserved.
**
***********************************************************************/

#include "qpsprn.h"
#include "qpainter.h"
#include "qpaintdc.h"
#include "qimage.h"
#include "qdatetm.h"

#include "qfile.h"
#include "qbuffer.h"

RCSTAG("$Id: //depot/qt/main/src/kernel/qpsprinter.cpp#35 $");

#if !defined(QT_HEADER_PS)
     // produced from qpshdr.txt
static char *ps_header =
"/D {bind def} bind def /ED {exch def} D /LT {lineto} D /MT {moveto} D /S\n"
"{stroke} D /SW {setlinewidth} D /CP {closepath} D /RL {rlineto} D /NP\n"
"{newpath} D /CM {currentmatrix} D /SM {setmatrix} D /TR {translate} D\n"
"/SRGB {setrgbcolor} D /SC {aload pop SRGB} D /GS {gsave} D /GR {grestore}\n"
"D /BSt 0 def /LWi 1 def /PSt 1 def /Cx 0 def /Cy 0 def /WFi false def /OMo\n"
"false def /BCol [ 1 1 1 ] def /PCol [ 0 0 0 ] def /BkCol [ 1 1 1 ] def /nS\n"
"0 def /QS { PSt 0 ne { LWi SW GS PCol SC true GPS 0 setdash S OMo PSt 1 ne\n"
"and { GR BkCol SC false GPS dup 0 get setdash S } { GR } ifelse } if } D\n"
"/QF { GS BSt 2 ge BSt 8 le and { BDArr BSt 2 sub get setgray fill } if BSt\n"
"9 ge BSt 14 le and { BF } if BSt 1 eq { BCol SC WFi { fill } { eofill }\n"
"ifelse } if GR } D /PF { GS BSt 2 ge BSt 8 le and { BDArr BSt 2 sub get\n"
"setgray WFi { fill } { eofill } ifelse } if BSt 9 ge BSt 14 le and { BF }\n"
"if BSt 1 eq { BCol SC WFi { fill } { eofill } ifelse } if GR } D /BDArr[\n"
"0.94 0.88 0.63 0.50 0.37 0.12 0.6 ] def /ArcDict 6 dict def ArcDict begin\n"
"/tmp matrix def end /ARC { ArcDict begin /ang2 ED /ang1 ED /h ED /w ED /y\n"
"ED /x ED tmp CM pop x w 2 div add y h 2 div add TR 1 h w div neg scale\n"
"ang2 0 ge {0 0 w 2 div ang1 ang1 ang2 add arc } {0 0 w 2 div ang1 ang1\n"
"ang2 add arcn} ifelse tmp SM end } D /QI { /savedContext save def clippath\n"
"pathbbox 3 index /PageX ED 0 index /PageY ED 3 2 roll exch sub neg /PageH\n"
"ED sub neg /PageW ED PageX PageY TR 1 -1 scale /defM matrix CM def /Cx 0\n"
"def /Cy 0 def 255 255 255 BC /OMo false def 1 0 0 0 0 PE 0 0 0 0 B } D /QP\n"
"{ savedContext restore showpage } D /P { NP MT 0.5 0.5 rmoveto 0 -1 RL -1\n"
"0 RL 0 1 RL CP PCol SC fill } D /M { /Cy ED /Cx ED } D /L { NP Cx Cy MT\n"
"/Cy ED /Cx ED Cx Cy LT QS } D /DL { 4 2 roll NP MT LT QS } D /RDict 4 dict\n"
"def /R { RDict begin /h ED /w ED /y ED /x ED NP x y MT 0 h RL w 0 RL 0 h\n"
"neg RL CP QF QS end } D /RRDict 6 dict def /RR { RRDict begin /yr ED /xr\n"
"ED /h ED /w ED /y ED /x ED xr 0 le yr 0 le or {x y w h R} {xr 100 ge yr\n"
"100 ge or {x y w h E} { /rx xr w mul 200 div def /ry yr h mul 200 div def\n"
"/rx2 rx 2 mul def /ry2 ry 2 mul def NP x rx add y MT x w add rx2 sub y rx2\n"
"ry2 90 -90 ARC x w add rx2 sub y h add ry2 sub rx2 ry2 0 -90 ARC x y h add\n"
"ry2 sub rx2 ry2 270 -90 ARC x y rx2 ry2 180 -90 ARC CP QF QS } ifelse }\n"
"ifelse end } D /EDict 5 dict def EDict begin /tmp matrix def end /E {\n"
"EDict begin /h ED /w ED /y ED /x ED tmp CM pop x w 2 div add y h 2 div add\n"
"translate 1 h w div scale NP 0 0 w 2 div 0 360 arc tmp SM QF QS end } D /A\n"
"{ 16 div exch 16 div exch NP ARC QS } D /PieDict 6 dict def /PIE { PieDict\n"
"begin /ang2 ED /ang1 ED /h ED /w ED /y ED /x ED NP x w 2 div add y h 2 div\n"
"add MT x y w h ang1 16 div ang2 16 div ARC CP QF QS end } D /CH { 16 div\n"
"exch 16 div exch NP ARC CP QF QS } D /BZ { curveto QS } D /CRGB { 255 div\n"
"3 1 roll 255 div 3 1 roll 255 div 3 1 roll } D /SV { BSt LWi PSt Cx Cy WFi\n"
"OMo BCol PCol BkCol /nS nS 1 add def GS } D /RS { nS 0 gt { GR /BkCol ED\n"
"/PCol ED /BCol ED /OMo ED /WFi ED /Cy ED /Cx ED /PSt ED /LWi ED /BSt ED\n"
"/nS nS 1 sub def } if } D /BC { CRGB BkCol astore pop } D /B { CRGB BCol\n"
"astore pop /BSt ED } D /PE { CRGB PCol astore pop /LWi ED /PSt ED LWi 0 eq\n"
"{ 0.3 /LWi ED } if } D /ST { defM setmatrix concat } D /T { 3 1 roll MT\n"
"PCol SC show } D /BFDict 2 dict def /BF { BSt 9 ge BSt 14 le and { BFDict\n"
"begin GS WFi { clip } { eoclip } ifelse defM SM pathbbox 3 index 3 index\n"
"translate 4 2 roll 3 2 roll exch sub /h ED sub /w ED OMo { NP 0 0 MT 0 h\n"
"RL w 0 RL 0 h neg RL CP BkCol SC fill } if BCol SC 0.3 SW BSt 9 eq BSt 11\n"
"eq or { 0 4 h { NP dup 0 exch MT w exch LT S } for } if BSt 10 eq BSt 11\n"
"eq or { 0 4 w { NP dup 0 MT h LT S } for } if BSt 12 eq BSt 14 eq or { w h\n"
"gt { 0 6 w h add { NP dup h MT h sub 0 LT S } for } { 0 6 w h add { NP dup\n"
"w exch MT w add 0 exch LT S } for } ifelse } if BSt 13 eq BSt 14 eq or { w\n"
"h gt { 0 6 w h add { NP dup 0 MT h sub h LT S } for } { 0 6 w h add { NP\n"
"dup 0 exch MT w add w exch LT S } for } ifelse } if GR end } if } D /LArr[\n"
"[] [] [ 10 3 ] [ 3 10 ] [ 3 3 ] [ 3 3 ] [ 5 3 3 3 ] [ 3 5 3 3 ] [ 5 3 3 3\n"
"3 3 ] [ 3 5 3 3 3 3 ] ] def /GPS { PSt 1 ge PSt 5 le and { { LArr PSt 1\n"
"sub 2 mul get } { LArr PSt 2 mul 1 sub get } ifelse } { [] } ifelse } D\n"
"%%EndProlog";
#endif


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
    if ( family == "courier" ) {
	ps = "/Courier";
    } else if ( family == "times" ) {
	ps = "/Times";
	times = TRUE;
    } else if ( family == "symbol" ) {
	ps = "/Symbol";
	symbol = TRUE;
    } else {
	ps = "/Helvetica";
    }
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
    } else {
	if ( times )
	    ps += "-Roman";
    }
    QString fontMatrix;
    fontMatrix.sprintf( "[ %d 0 0 -%d 0 0 ]", f->pointSize(), f->pointSize() );
    *s << ps << " findfont " << fontMatrix << " makefont setfont\n";
    ps.remove( 0, 1 );				// removes the '/'
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
    stream.setf( QTextStream::hex, QTextStream::basefield );

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

    stream.setf( QTextStream::dec, QTextStream::basefield );
}

static void ps_dumpPixmapData( QTextStream &stream, QImage img,
			       QColor fgCol, QColor bgCol )
{
    stream.setf( QTextStream::hex, QTextStream::basefield );

    if ( img.depth() == 1 ) {
	img = img.convertDepth( 8 );
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
    int pixWidth = img.depth() == 8 ? 1 : 4;
    uchar *scanLine;
    uint cval;
    int x,y;
    int count = -1;
    for( y = 0 ; y < height ; y++ ) {
	scanLine = img.scanLine(y);
	for( x = 0 ; x < width ; x++ ) {
	    if ( pixWidth == 1 ) {
		cval = img.color( scanLine[x] );
		hexOut( stream, qRed(cval) );
		hexOut( stream, qGreen(cval) );
		hexOut( stream, qBlue(cval) );
	    } else {
		hexOut( stream, scanLine[4*x] );
		hexOut( stream, scanLine[4*x + 1] );
		hexOut( stream, scanLine[4*x + 2] );
	    }
	    if ( !(count++ % 11) )
		stream << '\n';
	}
    }
    if ( --count % 11 )
	stream << '\n';

    stream.setf( QTextStream::dec, QTextStream::basefield );
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
    if ( c == PDC_SETDEV ) {			// preset io device
	device = p->device;
	epsf = TRUE;
    } else if ( c == PDC_BEGIN ) {		// start painting
	if (epsf) {
	    // ... ##### handle things differently
	}
	pageCount    = 1;			// initialize state
	dirtyMatrix  = TRUE;
	dirtyNewPage = FALSE;			// setup done by QPainter
	                                        // for the first page.
	fontsUsed    = "";
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
	QFile f( "/usr/lib/qpshdr.txt" );	// read predefined PS header
	if ( !f.open(IO_ReadOnly|IO_Raw) )
	    fatal( "Cannot open /usr/lib/qpshdr.txt" );
	QByteArray a(f.size());
	f.readBlock( a.data(), f.size() );
	f.close();
	stream.writeRawBytes( a.data(), a.size() );
#else
	stream << ps_header;
#endif
	stream << "\n%%Page: " << pageCount << ' ' << pageCount << endl;
	stream << "QI\n";
	orientationSetup();
	return TRUE;
    }

    if ( c == PDC_END ) {			// painting done
	stream << "QP\n";
	stream << "%%Trailer\n";
	stream << "%%Pages: " << pageCount << '\n';
	stream << "%%DocumentFonts: " << fontsUsed << '\n';
	if (!epsf) // is it our device to close?
	    device->close();
	stream.unsetDevice();
    }

    if ( c >= PDC_DRAW_FIRST && c <= PDC_DRAW_LAST ) {
	if ( dirtyMatrix ) 
	    matrixSetup( paint );
	if ( dirtyNewPage ) 
	    newPageSetup( paint );
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
		for ( int i=0; i<(int)a.size(); i+=2 ) {
		    pt = a.point( i );
		    stream << XCOORD(pt.x()) << ' '
			   << YCOORD(pt.y()) << " MT\n";
		    pt = a.point( i+1 );
		    stream << XCOORD(pt.x()) << ' '
			   << YCOORD(pt.y()) << " LT\n";
		}
		stream << "QS\n";
	    }
	    break;
	case PDC_DRAWPOLYLINE:
	    if ( p[0].ptarr->size() > 1 ) {
		QPointArray a = *p[0].ptarr;
		QPoint pt = a.point( 0 );
		stream << XCOORD(pt.x()) << ' ' << YCOORD(pt.y()) << " MT\n";
		for ( int i=1; i<(int)a.size(); i++ ) {
		    pt = a.point( i );
		    stream << XCOORD(pt.x()) << ' '
			   << YCOORD(pt.y()) << " LT\n";
		}
		stream << "QS\n";
	    }
	    break;
	case PDC_DRAWPOLYGON:
	    if ( p[0].ptarr->size() > 2 ) {
		QPointArray a = *p[0].ptarr;
		if ( p[1].ival )
		    stream << "/WFi true def\n";
		QPoint pt = a.point(0);
		stream << "NP\n";
		stream << XCOORD(pt.x()) << ' '
		       << YCOORD(pt.y()) << " MT\n";
		for( int i=1; i<(int)a.size(); i++) {
		    pt = a.point( i );
		    stream << XCOORD(pt.x()) << ' '
			   << YCOORD(pt.y()) << " LT\n";
		}
		stream << "CP PF QS\n";
		if ( p[1].ival )
		    stream << "/WFi false def\n";
	    }
	    break;
	case PDC_DRAWQUADBEZIER:
	    if ( p[0].ptarr->size() == 4 ) {
		QPointArray a = *p[0].ptarr;
		stream << XCOORD(a[0].x()) << ' '
		       << YCOORD(a[0].y()) << " MT ";
		for ( int i=1; i<4; i++ ) {
		    stream << XCOORD(a[i].x()) << ' '
			   << YCOORD(a[i].y()) << ' ';
		}
		stream << "BZ\n";
	    }
	    break;
	case PDC_DRAWTEXT:
	    if ( p[1].str && strlen( p[1].str ) ) {
		char * tmp = new char[ strlen( p[1].str ) * 2 + 2 ];
#if defined(CHECK_NULL)
		CHECK_PTR( tmp );
#endif
		const char * from = p[1].str;
		char * to = tmp;
		while ( *from ) {
		    if ( *from == '\\' || *from == '(' || *from == ')' )
			*to++ = '\\';		// escape special chars
		    *to++ = *from++;
		}
		*to = '\0';
		stream << POINT(0) << "(" << tmp << ") T\n";
		delete [] tmp;
	    }
	    break;
	case PDC_DRAWTEXTFRMT:;
	    return FALSE;			// uses Qt instead
	case PDC_DRAWPIXMAP: {
	    if ( p[1].pixmap->isNull() )
		break;
	    QPoint pnt = *(p[0].point);
	    stream << pnt.x() << " " << pnt.y() << " TR\n";
	    QImage img;
	    img = *(p[1].pixmap);
	    bool mask  = FALSE;
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
//		QColor fgCol = paint->pen().color();
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
	    stream << (int)p[0].pen->style() << ' ' << p[0].pen->width()
		   << ' ' << COLOR(p[0].pen->color()) << "PE\n";
	    break;
	case PDC_SETBRUSH:
	    if ( p[0].brush->style() == CustomPattern ) {
#if defined(DEBUG)
		warning( "QPrinter: Pixmap brush not supported" );
#endif
		return FALSE;
	    }
	    stream << (int)p[0].brush->style()	 << ' '
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
	    stream << "QP\n";
	    pageCount++;
	    stream << "\n%%Page: " << pageCount << ' ' << pageCount << endl;
	    stream << "QI\n";
	    dirtyNewPage = TRUE;
	    orientationSetup();
	    break;
	case PDC_PRT_ABORT:
	    break;
	default:
	    break;
    }
    return TRUE;
}

void QPSPrinter::matrixSetup( QPainter *paint )
{
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

void QPSPrinter::orientationSetup()
{
    if ( printer->orientation() == QPrinter::Landscape ) {
	stream << "PageW 0 TR 90 rotate\n";
	stream << "/defM matrix CM def\n";
    }

}

void QPSPrinter::newPageSetup( QPainter *paint )
{
    QPDevCmdParam param[1];
    QPen   defaultPen;			// default drawing tools
    QBrush defaultBrush;

    param[0].color = &paint->backgroundColor();
    if ( *param[0].color != white )
	cmd( PDC_SETBKCOLOR, paint, param );

    param[0].ival = paint->backgroundMode();
    if (param[0].ival != TransparentMode )
	cmd( PDC_SETBKMODE, paint, param );

    param[0].font = &paint->font();
    cmd( PDC_SETFONT, paint, param );

    param[0].pen = &paint->pen();
    if (*param[0].pen != defaultPen )
	cmd( PDC_SETPEN, paint,param );

    param[0].brush = &paint->brush();
    if (*param[0].brush != defaultBrush )
	cmd( PDC_SETBRUSH, paint, param);

    if ( paint->hasViewXForm() || paint->hasWorldXForm() )
	matrixSetup( paint );

    dirtyNewPage = FALSE;
}


