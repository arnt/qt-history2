/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qprn_x11.cpp#39 $
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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#endif
#if defined(_WS_X11_)
#include <X11/Xlib.h>
#endif
#if defined(_OS_OS2EMX_)
#include <process.h>
#endif

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
    pdrv = 0;
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

bool QPrinter::setup( QWidget * )
{
    return QPrintDialog::getPrinterSetup( this );
}


/*!
  \internal
  Handles painter commands to the printer.
*/

bool QPrinter::cmd( int c, QPainter *paint, QPDevCmdParam *p )
{
    if ( c ==  PDC_BEGIN ) {
	if ( state == PST_IDLE ) {
	    if ( output_file ) {
		int fd = ::open( output_filename.data(),
				 O_CREAT | O_NOCTTY | O_TRUNC | O_WRONLY,
				 0666 );
		if ( fd >= 0 ) {
		    pdrv = new QPSPrinter( this, fd );
		    state = PST_ACTIVE;
		}
	    } else {
		QString pr = printer_name.copy();
		if ( pr.isEmpty() )
		    pr = getenv( "PRINTER" );
		if ( pr.isEmpty() )
		    pr = "lp";
		pr.insert( 0, "-P" );
#if defined(_OS_WIN32_)
		// Not implemented
		// lpr needs -Sserver argument
#elif defined(_OS_OS2EMX_)
#error "this does not work - must be rewritten to spawn and pipe"
		if ( spawnlp(P_NOWAIT,print_prog.data(), print_prog.data(),
			     pr.data(), output->name(), 0) == -1 ) {
		    ;			// couldn't exec, ignored
		}
#else
		QApplication::flushX();
		int fds[2];
		if ( pipe( fds ) != 0 ) {
		    warning( "QPSPrinter: could not open pipe to print" );
		    state = PST_ERROR;
		    return FALSE;
		}
		if ( fork() == 0 ) {	// child process
		    dup2( fds[0], 0 );
#if defined(_WS_X11_)
		    // ###
		    // hack time... getting the maximum number of open
		    // files, if possible.  if not we assume it's 256.
		    int i;
#if defined(_SC_OPEN_MAX)
		    i = (int)sysconf( _SC_OPEN_MAX );
#elif defined(_POSIX_OPEN_MAX)
		    i = (int)_POSIX_OPEN_MAX;
#elif defined(OPEN_MAX)
		    i = (int)OPEN_MAX;
#else
		    i = 256;
#endif
		    while( --i > 0 )
			::close( i );
#endif
		    (void)execlp( print_prog.data(), print_prog.data(),
				  pr.data(), 0 );
		    // if execlp returns EACCES it couldn't find the
		    // program.  if no special print program has been
		    // set, let's try a little harder...
		    if ( print_prog == "lpr" && errno == EACCES ) {
			(void)execl( "/bin/lpr", "lpr", pr.data(), 0 );
			(void)execl( "/usr/bin/lpr", "lpr", pr.data(), 0 );
			pr[1] = 'd';
			(void)execlp( "lp", "lp", pr.data(), 0 );
			(void)execl( "/bin/lp", "lp", pr.data(), 0 );
			(void)execl( "/usr/bin/lp", "lp", pr.data(), 0 );
		    }
		    exit( 0 );
		} else {		// parent process
		    ::close( fds[0] );
		    pdrv = new QPSPrinter( this, fds[1] );
		    state = PST_ACTIVE;
		}
#endif
	    }
	    return ((QPSPrinter*)pdrv)->cmd( c, paint, p );
	} else {
	    // ignore it?  I don't know
	}
	return TRUE;
    } else {
	bool r = FALSE;
	if ( state == PST_ACTIVE && pdrv ) {
	    r = ((QPSPrinter*)pdrv)->cmd( c, paint, p );
	    if ( c == PDC_END ) {
		state = PST_IDLE;
		delete pdrv;
		pdrv = 0;
	    }
	}
	return r;
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
