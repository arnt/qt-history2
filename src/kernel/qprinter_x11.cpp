/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qprinter_x11.cpp#32 $
**
** Implementation of QPrinter class for X11
**
** Created : 950810
**
** Copyright (C) 1995-1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qprinter.h"
#include "qfileinf.h"
#include "qdir.h"
#include "qpaintdc.h"
#include "qpsprn.h"
#include "qprndlg.h"
#include "qfile.h"
#include "qapp.h"
#include <stdlib.h>
#if !defined(_OS_WIN32_)
#include <unistd.h>
#endif
#if defined(_OS_OS2EMX_)
#include <process.h>
#endif

RCSTAG("$Id: //depot/qt/main/src/kernel/qprinter_x11.cpp#32 $");


/*****************************************************************************
  QPrinter member functions
 *****************************************************************************/

// QPrinter states

#define PST_IDLE	0
#define PST_ACTIVE	1
#define PST_ERROR	2
#define PST_ABORTED	3


/*!
  Constructs a printer paint device.
*/

QPrinter::QPrinter()
    : QPaintDevice( PDT_PRINTER | PDF_EXTDEV )
{
    pdrv = new QPSPrinter( this );
    orient = Portrait;
    page_size = A4;
    ncopies = 1;
    from_pg = to_pg = min_pg = max_pg = 0;
    state = PST_IDLE;
    printer_name = getenv( "PRINTER" );
    output_file = FALSE;
    print_prog = "lpr";
}

/*!
  Destroys the printer paint device and cleans up.
*/

QPrinter::~QPrinter()
{
    delete pdrv;
}


/*!
  Advances to a new page on the printer.
  Returns TRUE if successful, otherwise FALSE.
*/

bool QPrinter::newPage()
{
    if ( state == PST_ACTIVE && pdrv )
	return ((QPSPrinter*)pdrv)->cmd( PDC_PRT_NEWPAGE, 0, 0 );
    return FALSE;
}


/*!
  Aborts the print job.
  Returns TRUE if successful, otherwise FALSE.
  \sa aborted()
*/

bool QPrinter::abort()
{
    if ( state == PST_ACTIVE && pdrv ) {
	((QPSPrinter*)pdrv)->cmd( PDC_PRT_ABORT, 0, 0 );
	state = PST_ABORTED;
    }
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
  
  Now obsoleted by QPrintDialog::getPrinterSetup().

  Returns TRUE if the user pressed "Ok" to print, or FALSE if the
  user cancelled the operation.
*/

bool QPrinter::setup( QWidget *parent )
{
    QPrintDialog prndlg( this, parent );
    return prndlg.exec() == QDialog::Accepted;
}


/*!
  \internal
  Handles painter commands to the printer.
*/

bool QPrinter::cmd( int c, QPainter *paint, QPDevCmdParam *p )
{
    QPSPrinter *ps = (QPSPrinter*)pdrv;
    if ( c ==  PDC_BEGIN ) {			// begin; start printing
	if ( ps && state == PST_IDLE ) {
	    if ( !ps->epsf ) {
		char *fname = output_file ? output_filename.data() : tmpnam(0);
		if ( !fname || *fname == '\0' ) {
#if defined(DEBUG)
		    warning( "QPrinter: File name cannot be null" );
#endif
		    state = PST_ERROR;
		    return TRUE;
		}
		QFile *output = new QFile( fname );
		if ( !output->open(IO_ReadWrite|IO_Truncate) ) {
#if defined(DEBUG)
		    warning( "QPrinter: Could not create output file" );
#endif
		    delete output;			// could not create file
		    state = PST_ERROR;
		    return TRUE;
		}
		ps->device = output;
		if ( ps->cmd( c, paint, p ) ) {	// successful
		    state = PST_ACTIVE;
		} else {				// could not start printing
		    ps->device = 0;
		    delete output;
		    state = PST_ERROR;
		}
	    } else {
		if ( ps->cmd( c, paint, p ) ) {	// successful
		    state = PST_ACTIVE;
		} else {			// could not start printing
		    state = PST_ERROR;
		}
	    }
	    return TRUE;
	}
    } else if ( c == PDC_END ) {		// end; printing done
	if ( ps && state != PST_ERROR ) {
	    ps->cmd( c, paint, p );
	    QFile *output = (QFile*)ps->device;
	    if ( output ) {
		if ( !output_file ) {		// send to printer
		    QString pr = printer_name.copy();
		    if ( pr.isEmpty() )		// not set, then read $PRINTER
			pr = getenv( "PRINTER" );
		    if ( pr.isEmpty() ) {	// no printer set
			pr = "lp";
		    }
		    pr.insert( 0, "-P" );
#if defined(_OS_WIN32_)
		    // Not implemented
		    //	 lpr needs -Sserver argument
#elif defined(_OS_OS2EMX_)
		    if ( spawnlp(P_NOWAIT,print_prog.data(), print_prog.data(),
				 pr.data(), output->name(), 0) == -1 ) {
			;			// couldn't exec, ignored
		    }
#else
		    if ( fork() == 0 ) {	// child process
			if ( execlp(print_prog.data(), print_prog.data(),
				    pr.data(), output->name(), 0) == -1 ) {
			    ;			// couldn't exec, ignored
			}
			exit( 0 );		// exit print job
			QFileInfo fi( output->name() );
			fi.dir().remove( fi.fileName() );
		    }
#endif
		}
		output->close();
		delete output;
		
		ps->device = 0;
	    }
	}
	state = PST_IDLE;
	return TRUE;
    } else {
	if ( state == PST_ACTIVE || c == PDC_SETDEV ) {
	    return ps->cmd( c, paint, p );
	}
    }
    return FALSE;
}


/*!
  Internal implementation of the virtual QPaintDevice::metric() function.

  Use the QPaintDeviceMetrics class instead.

  \internal
  Hard coded return values for PostScript under X.
*/

int QPrinter::metric( int m ) const
{
    int val;
    PageSize s = pageSize();
#if defined(CHECK_RANGE)
    ASSERT( (uint)s <= (uint)Executive );
#endif
    static int widths[]	 = { 595, 516, 612, 612, 541 };
    static int heights[] = { 842, 729, 791, 1009, 720 };
    static int widthsMM[]  = { 210, 182, 216, 216, 191 };
    static int heightsMM[] = { 297, 257, 279, 356, 254 };
    switch ( m ) {
	case PDM_WIDTH:
	    val = orient == Portrait ? widths[ s ] :  heights[ s ];
	    break;
	case PDM_HEIGHT:
	    val = orient == Portrait ? heights[ s ] :  widths[ s ];
	    break;
	case PDM_WIDTHMM:
	    val = orient == Portrait ? widthsMM[ s ] :	heightsMM[ s ];
	    break;
	case PDM_HEIGHTMM:
	    val = orient == Portrait ? heightsMM[ s ] :	 widthsMM[ s ];
	    break;
	case PDM_NUMCOLORS:
	    val = 16777216;
	    break;
	case PDM_DEPTH:
	    val = 24;
	    break;
	default:
	    val = 0;
#if defined(CHECK_RANGE)
	    warning( "QPixmap::metric: Invalid metric command" );
#endif
    }
    return val;
}
