/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qprinter_x11.cpp#1 $
**
** Implementation of QPrinter class (PostScript) for X-Windows
**
** Author  : Eirik Eng
** Created : 941003
**
** Copyright (C) 1994,1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qprinter.h"
#include "qpaintdc.h"
#include "qapp.h"
#include <windows.h>

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qprinter_x11.cpp#1 $";
#endif


// QPrinter states

#define PST_IDLE	0
#define PST_ACTIVE	1
#define PST_ERROR	2
#define PST_ABORTED	3


QPrinter::QPrinter()
    : QPaintDevice( PDT_PRINTER | PDF_EXTDEV )
{
    from_pg = to_pg  = ncopies = 1;
    min_pg  = max_pg = 0;
    state = PST_IDLE;
}

QPrinter::~QPrinter()
{
}



/*!
  Advances to a new page on the printer.
  Returns TRUE if successful, otherwise FALSE.
*/

bool QPrinter::newPage()
{
    if ( hdc && state == PST_ACTIVE ) {
	cmd( 0, 0, 0 );
    }
    return FALSE;
}


/*!
  Aborts the print job.
  Returns TRUE if successful, otherwise FALSE.
  \sa aborted()
*/

bool QPrinter::abort()
{
    if ( state == PST_ACTIVE )
	state = PST_ABORTED;
    return state == PST_ABORTED;
}

/*!
  Returns TRUE is the printer job was aborted, otherwise FALSE.
  \sa abort()
*/

bool QPrinter::aborted() const
{
    return state == PST_ABORTED;
}


/*!
  Opens a printer setup dialog and asks the user to specify what printer
  to use and miscellaneous printer settings.

  Returns TRUE if the user pressed "Ok" to print, or FALSE if the
  user cancelled the operation.

  \warning Not implemented for X-Windows yet.
*/

bool QPrinter::select( QWidget *parent )
{
    debug( "QPrinter::select: Not implemented" );
    return FALSE;
}


bool QPrinter::cmd( int c, QPainter *, QPDevCmdParam *p )
{
    if ( c ==  PDC_BEGIN ) {			// begin; start printing
    }
    else if ( c == PDC_END ) {			// end; printing done
    }
    return TRUE;
}


#if 0

#include "qprinter.h"
#include "q2matrix.h"
#include "qprtdrv.h"
#include "qiodev.h"
#include "qdict.h"
#include "qstack.h"
#include "qdatetm.h"

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qprinter_x11.cpp#1 $";
#endif


struct QPrinterDriverInfo
{
    QString	       name;  // name of printer driver
    QInstallPrtDrvFunc f;     // func that returns a new instance of the driver
};

typedef declare(QDictM,QPrinterDriverInfo) QPrtDrvDict;

static QPrtDrvDict *prtDrvDict = 0; // dict of all installed printer drivers
static char *defaultPrinterType = 0; // name of default printer driver
static bool defaultDriversInstalled = FALSE;


static void cleanupPrtDrvDict()		   // called when application quits
{
    if ( prtDrvDict ) {
	prtDrvDict->setAutoDelete( TRUE );
	delete prtDrvDict;
    }
}

void qInternalAddPrinterDriver( const char *name, QInstallPrtDrvFunc f)
{
    if ( !prtDrvDict ) {
	prtDrvDict = new QPrtDrvDict( 17, TRUE, FALSE ); // don't copy keys.
	CHECK_PTR( prtDrvDict );
    }
    
    QPrinterDriverInfo *tmp = new QPrinterDriverInfo;
    CHECK_PTR( tmp );
    tmp->name = name;	  // copy string
    tmp->f    = f;
    prtDrvDict->insert( tmp->name, tmp );
}


#if defined(CHECK_STATE)
inline void notActive( const char *s )
{
    warning("QInternalPrinter::%s: Ignored, printer not in use by a QPainter.",
	    s);
}
#endif


QInternalPrinter::QInternalPrinter()
{
    drv = 0;
    dev = 0;
    nCopies = 1;
    if ( !defaultPrinterType )
	setDefaultPrinterType( "ps" );	  
}

QInternalPrinter::~QInternalPrinter()
{
    delete drv;
}

void QInternalPrinter::newPage()
{
    if ( !paintingActive() ) {
#if defined(CHECK_STATE)
	notActive( "newPage" );
#endif
	return;
    }	     
    drv->cmd( PDC_PRT_NEWPAGE, 0 );
}

void QInternalPrinter::setCopies( uint n )
{
    if ( paintingActive() ) {
#if defined(CHECK_STATE)
    warning("QInternalPrinter::setCopies: "
            "Cannot set number of copies, printer is in use by a QPainter.");
#endif
        return;
    }

    nCopies = n;
}

void QInternalPrinter::abort()
{
    if ( !paintingActive() ) {
#if defined(CHECK_STATE)
	notActive("abort");
#endif
	return;
    }	     
    drv->cmd( PDC_PRT_ABORT, 0 );
}

void QInternalPrinter::setTitle( const char *t )
{
    if ( paintingActive() ) {
#if defined(CHECK_STATE)
    warning("QInternalPrinter::setTitle: "
            "Cannot set title of document, printer is in use by a QPainter.");
#endif
        return;
    }

    docTitle = t;
}

void QInternalPrinter::setCreator( const char *c )
{
    if ( paintingActive() ) {
#if defined(CHECK_STATE)
    warning("QInternalPrinter::setCreator: "
           "Cannot set creator of document, printer is in use by a QPainter.");
#endif
        return;
    }

    docCreator = c;
}

const char * QInternalPrinter::title() const
{
    return docTitle.isNull() ? "Qt postscript" : docTitle;
}

const char * QInternalPrinter::creator() const
{
    return docCreator.isNull() ? "Qt" : docCreator;
}

uint QInternalPrinter::noOfCopies() const
{
    return nCopies;
}


class QTextStream;

struct PSDriverState {
    QRect      window;
    QRect      viewport;
    Q2DMatrix  worldMatrix;
    bool       worldXForm;
    bool       viewXForm;
    bool       dirtyMatrix;
};

declare(QStackM,PSDriverState);

class QPSPrinterDriver : public QPrinterDriver
{
public:
    QPSPrinterDriver();
    bool	cmd( int, QPDevCmdParam * );

private:
    void	setMatrix();

    QTextStream            *s;
    QStackM(PSDriverState)  st;

    uint        pageCount;
    QRect       window;
    QRect       viewport;
    Q2DMatrix	worldMatrix;
    bool	worldXForm;
    bool        viewXForm;
    bool        dirtyMatrix;
};

QPRINTER_DRIVER_INSTALLER(QPSPrinterDriver)

bool QInternalPrinter::setPrinterType( const char *typeName )
{
    if ( paintingActive() ) {
#if defined(CHECK_STATE)
	warning( "QInternalPrinter::setPrinterType: Cannot change printer"
		 " while printer is active" );
#endif
	return FALSE;
    }

    if ( !prtDrvDict || !defaultDriversInstalled ) {
	qInstallPrinterDriver("ps",QPSPrinterDriver); // will create prtDrvDict
	defaultDriversInstalled = TRUE;
    }

    QPrinterDriverInfo *tmp = prtDrvDict->find(typeName);
    if ( tmp ) {				// printer driver found?
	delete drv;
	drv = (*(tmp->f))();
	CHECK_PTR( drv );
	return TRUE;
    }
    else
	return FALSE;
}

void QInternalPrinter::setDefaultPrinterType( const char *typeName )
{
    if ( !typeName ) {
#if defined(CHECK_NULL)
	warning( "QInternalPrinter::setDefaultPrinterType: NULL argument" );
#endif
	return;
    }

    delete[] defaultPrinterType;
    defaultPrinterType = strdup( typeName );
}

void QInternalPrinter::setDevice( QIODevice *d )
{
    dev = d;
}

bool QInternalPrinter::cmd( int command, QPDevCmdParam *params )
{
    if ( command == PDC_BEGIN ) {
	if ( !drv ) {
	    if ( !setPrinterType( defaultPrinterType ) ) {
#if defined(CHECK_RANGE)
		warning("QInternalPrinter::cmd: Could not find driver for"
			" default printer \"%s\".", defaultPrinterType );
#endif
		return FALSE;
	    }
	    drv->setDevice( dev );
	}
        QPDevCmdParam tmp[3];
        tmp[0].str  = creator();
        tmp[1].str  = title();
        tmp[2].ival = noOfCopies();
	return drv->cmd( PDC_BEGIN, tmp );
    }

    if ( !paintingActive() ) {
#if defined(CHECK_STATE)
	notActive("cmd");
#endif
	return FALSE;
    }	     
    return drv->cmd( command, params );
}


#include "qtstream.h"
#include "qfile.h"
#include "qpntarry.h"


QPSPrinterDriver::QPSPrinterDriver()
    : window( 0, 0, 600, 450 ), viewport( 0, 0, 600, 450 )
{
    s	        = new QTextStream;
    pageCount   = 0;
    worldXForm  = FALSE;
    viewXForm   = FALSE;
    dirtyMatrix = FALSE;
    st.setAutoDelete( TRUE );
}

static float qPSXCoord(float f)
{
    return f;
}

static float qPSYCoord(float f)
{
    return f;
}

static float qPSWidth(float f)
{
    return f;
}

static float qPSHeight(float f)
{
    return f;
}

void QPSPrinterDriver::setMatrix()
{
    Q2DMatrix tmp;
       
    if ( viewXForm ) {
        tmp.translate( viewport.x(), viewport.y() );
        tmp.scale( 1.0 * viewport.width()  / window.width(),
                   1.0 * viewport.height() / window.height() );
        tmp.translate( -window.x(), -window.y() );
    }

    if ( worldXForm )
        tmp = worldMatrix * tmp;

    *s << "[ " << tmp.m11() << ' '
       << tmp.m12() << ' '
       << tmp.m21() << ' '
       << tmp.m22() << ' '
       << tmp.dx()  << ' '
       << tmp.dy()  << " ] ST\n";
}


bool QPSPrinterDriver::cmd( int command , QPDevCmdParam *p )
{
#undef X_COORD
#undef Y_COORD
#undef RECT_COORDS
#undef INT_ARG
#undef UINT_ARG
#undef COLOR
#undef PA

#define POINT(index)	   qPSXCoord(p[index].point->x()) << ' ' <<   \
                           qPSYCoord(p[index].point->y()) << ' '
#define RECT(index)        qPSXCoord(p[index].rect->x())  << ' ' << \
			   qPSYCoord(p[index].rect->y())  << ' ' << \
			   qPSWidth (p[index].rect->width())  << ' ' << \
			   qPSHeight(p[index].rect->height()) << ' '
#define INT_ARG(index)	   p[index].ival  << ' '
#define UINT_ARG(index)	   p[index].ul << ' '
#define COLOR(x)	   (x).red()   << ' ' << \
			   (x).green() << ' ' << \
			   (x).blue()  << ' '
#define PA(index) (p[index].ptarr)

    if ( command >= PDC_DRAW_START && command <= PDC_DRAW_STOP ) {
        if ( dirtyMatrix )
            setMatrix();
    }

    switch(command) {
	case PDC_DRAWPOINT:
	    *s << POINT(0) << "P\n";
	    break;
	case PDC_MOVETO:
	    *s << POINT(0) << "M\n";
	    break;
	case PDC_LINETO:
	    *s << POINT(0) << "L\n";
	    break;
	case PDC_DRAWLINE:
	    *s << POINT(0) << POINT(1) << "DL\n";
	    break;
	case PDC_DRAWRECT:
	    *s << RECT(0) << "R\n";
	    break;
	case PDC_DRAWROUNDRECT:
	    *s << RECT(0) << INT_ARG(1) << INT_ARG(2) << "RR\n";
	    break;
	case PDC_DRAWELLIPSE:
	    *s << RECT(0) << "E\n";
	    break;
	case PDC_DRAWARC:
	    *s << RECT(0) << INT_ARG(1) << INT_ARG(2) << "A\n";
	    break;
	case PDC_DRAWPIE:
	    *s << RECT(0) << INT_ARG(1) << INT_ARG(2) << "PIE\n";
	    break;
	case PDC_DRAWCHORD:
	    *s << RECT(0) << INT_ARG(1) << INT_ARG(2) << "CH\n";
	    break;
	case PDC_DRAWLINESEGS:
	    break;
	case PDC_DRAWPOLYLINE:
	    break;
	case PDC_DRAWPOLYGON: {
	    if (PA(0)->size() == 0)
		return TRUE;
	    QPoint tmp = PA(0)->point(0);
	    *s << "NP\n";
	    *s << qPSXCoord(tmp.x()) << ' '
	       << qPSYCoord(tmp.y()) << " MT\n";
	    for( int i = 1 ; i < PA(0)->size() ; i++) {
		tmp = PA(0)->point(i);
		*s << qPSXCoord(tmp.x()) << ' '
		   << qPSYCoord(tmp.y()) << " LT\n";
	    }
	    *s << "CP\n";
	    *s << "QtFill\n";
	    *s << "QtStroke\n";
	    }
	    break;
	case PDC_DRAWTEXT:
	    *s << POINT(0) << "(" << p[1].str << ") T\n";
	    break;
	case PDC_DRAWTEXTFRMT:
	    break;
	case PDC_BEGIN: {
	    s->setDevice( device() );
            *s << "%!PS-Adobe-1.0\n";
            *s << "%%Creator: " << p[0].str << endl;
            *s << "%%Title: "   << p[1].str << endl;
            *s << "%%CreationDate:" << QDateTime::currentDateTime().toString()
	       << endl;
            *s << "%%Pages: (atend)\n";
            *s << "%%DocumentFonts: (atend)\n";
            *s << "%%EndComments\n\n";
            if ( p[2].ival != 1 )
	        *s << "/#copies " << p[2].ival << " def\n";

	    QFile f( "qtheader.ps" );
	    f.open( IO_ReadOnly|IO_Raw );
	    QByteArray a(f.size());
	    f.readBlock( a.data(), f.size() );
	    f.close();
	    s->writeRawBytes( a.data(), a.size() );
            pageCount++;
            *s << "\n%%Page: " << pageCount << ' ' << pageCount << endl;
	    break;
	}
	case PDC_END:
	    *s << "QtFinish\n";
	    *s << "%%Trailer\n";
	    *s << "%%Pages: " << pageCount << endl;
	    *s << "%%DocumentFonts: Courier\n";
	    device()->close();
	    s->unsetDevice();
	    break;
	case PDC_SAVE: {
            PSDriverState *tmp = new PSDriverState;
            tmp->window      = window;
            tmp->viewport    = viewport;
            tmp->worldMatrix = worldMatrix;
            tmp->worldXForm  = worldXForm;
            tmp->viewXForm   = viewXForm;
            tmp->dirtyMatrix = dirtyMatrix;
            st.push( tmp );
            *s << "SV\n";
            break;
        }
	case PDC_RESTORE: {
            if ( st.count() == 0 )
                break;
            PSDriverState *tmp = st.pop();
            window      = tmp->window;
            viewport    = tmp->viewport;
            worldMatrix = tmp->worldMatrix;
            worldXForm  = tmp->worldXForm;
            viewXForm   = tmp->viewXForm;
            dirtyMatrix = tmp->dirtyMatrix;
            *s << "RS\n";
            break;
        }
	case PDC_SETBKCOLOR:
	    *s << COLOR(*(p[0].color)) << "BC\n";
	    break;
	case PDC_SETBKMODE:
	    *s << "/OMo ";
	    if ( p[0].ival == TransparentMode )
		*s << "false";
	    else
		*s << "true";
	    *s << " def\n";
	    break;
	case PDC_SETROP:
	    break;
	case PDC_SETPEN:
	    if ( p[0].pen->width() == 0 )
		*s << p[0].pen->style()        << " 0.3 "
                   << COLOR(p[0].pen->color()) << "PE\n";
	    else
		*s << p[0].pen->style() << ' ' << p[0].pen->width() 
                   << COLOR(p[0].pen->color()) << "PE\n";
	    break;
	case PDC_SETBRUSH:
	    *s << p[0].brush->style() << ' ' 
               << COLOR(p[0].brush->color()) << "B\n";
	    break;
	case PDC_SETVXFORM:
            if ( viewXForm == p[0].ival )
                break;
            viewXForm   = p[0].ival;
            dirtyMatrix = TRUE;
	    break;
	case PDC_SETWINDOW:
            window      = *(p[0].rect);
            viewXForm   = TRUE;
            dirtyMatrix = TRUE;
	    break;
	case PDC_SETVIEWPORT:
            viewport    = *(p[0].rect);
            viewXForm   = TRUE;
            dirtyMatrix = TRUE;
	    break;
        case PDC_SETWXFORM:
            if ( viewXForm == p[0].ival )
                break;
            worldXForm  = p[0].ival;
            dirtyMatrix = TRUE;
	    break;
        case PDC_SETWMATRIX:
            worldMatrix = *(p[0].matrix);
            worldXForm  = TRUE;
            dirtyMatrix = TRUE;
	    break;
	case PDC_SETCLIP:
	    break;
	case PDC_SETCLIPRGN:
	    break;
	case PDC_PRT_NEWPAGE:
	    *s << "showpage\n";
            pageCount++;
            *s << "\n%%Page: " << pageCount << ' ' << pageCount << endl;
	    break;
	case PDC_PRT_ABORT:
	    return cmd( PDC_END,0 );
	    break;
	default:
	    break;
    }
    return TRUE;
#undef X_COORD
#undef Y_COORD
#undef RECT_COORDS
#undef INT_ARG
#undef UINT_ARG
#undef COLOR
#undef PA
}

#endif
