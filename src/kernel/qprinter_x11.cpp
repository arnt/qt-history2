/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qprinter_x11.cpp#56 $
**
** Implementation of QPrinter class for X11
**
** Created : 950810
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees with valid Qt Professional Edition licenses may distribute and
** use this file in accordance with the Qt Professional Edition License
** provided at sale or upon request.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#include "qprinter.h"
#include "qfile.h"
#include "qfileinfo.h"
#include "qdir.h"
#include "qpaintdevicedefs.h"
#include "qpsprinter.h"
#include "qprintdialog.h"
#include "qapplication.h"
#include <stdlib.h>

#if defined(_OS_WIN32_)
#include <io.h>
#include <fcntl.h>
#else
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
#define INCL_DOSFILEMGR
#define INCL_DOSERRORS
#include <os2.h>
#endif

#if defined(_OS_QNX_)
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
    : QPaintDevice( QInternal::Printer | QInternal::ExternalDevice )
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

bool QPrinter::setup( QWidget * parent )
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
    if ( c ==  PDC_BEGIN ) {
	if ( state == PST_IDLE ) {
	    if ( output_file ) {
#if defined(_OS_WIN32_)
		int fd = open( output_filename,
				O_CREAT | O_BINARY | O_TRUNC | O_WRONLY );
#else
		int fd = ::open( output_filename,
				 O_CREAT | O_NOCTTY | O_TRUNC | O_WRONLY,
				 0666 );
#endif
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
#else
		QApplication::flushX();
		int fds[2];
		if ( pipe( fds ) != 0 ) {
		    warning( "QPSPrinter: could not open pipe to print" );
		    state = PST_ERROR;
		    return FALSE;
		}
#if 0 && defined(_OS_OS2EMX_)
		// this code is usable but not in use.  spawn() is
		// preferable to fork()/exec() for very large
		// programs.  if fork()/exec() is a problem and you
		// use OS/2, remove '0 && ' from the #if.
		int tmp;
		tmp = dup(0);
		dup2( fds[0], 0 );
		::close( fds[0] );
		fcntl(tmp, F_SETFD, FD_CLOEXEC);
		fcntl(fds[1], F_SETFD, FD_CLOEXEC);
		if ( spawnlp(P_NOWAIT,print_prog.data(), print_prog.data(),
			     pr.data(), output->name(), 0) == -1 ) {
		    ;			// couldn't exec, ignored
		}
		dup2( tmp, 0 );
		::close( tmp );
		pdrv = new QPSPrinter( this, fds[1] );
		state = PST_ACTIVE;
#else
		if ( fork() == 0 ) {	// child process
		    dup2( fds[0], 0 );
#if defined(_WS_X11_)
		    // hack time... getting the maximum number of open
		    // files, if possible.  if not we assume it's the
		    // larger of 256 and the fd we got
		    int i;
#if defined(_OS_OS2EMX_)
		    LONG req_count = 0;
		    ULONG rc, handle_count;
		    rc = DosSetRelMaxFH (&req_count, &handle_count);
		    /* if (rc != NO_ERROR) ... */
		    i = (int)handle_count;
#elif defined(_SC_OPEN_MAX)
		    i = (int)sysconf( _SC_OPEN_MAX );
#elif defined(_POSIX_OPEN_MAX)
		    i = (int)_POSIX_OPEN_MAX;
#elif defined(OPEN_MAX)
		    i = (int)OPEN_MAX;
#else
		    i = QMAX( 256, fds[0] );
#endif // ways-to-set i
		    while( --i > 0 )
			::close( i );
#endif // _WS_X11_
		    (void)execlp( print_prog, print_prog.ascii(),
				  pr.ascii(), 0 );
		    // if execlp returns EACCES it couldn't find the
		    // program.  if no special print program has been
		    // set, let's try a little harder...
		    if ( print_prog == "lpr" && ( errno == EACCES ||
						  errno == ENOENT ||
						  errno == ENOEXEC ) ) {
			(void)execl( "/bin/lpr", "lpr", pr.ascii(), 0 );
			(void)execl( "/usr/bin/lpr", "lpr", pr.ascii(), 0 );
			pr[1] = 'd';
			(void)execlp( "lp", "lp", pr.ascii(), 0 );
			(void)execl( "/bin/lp", "lp", pr.ascii(), 0 );
			(void)execl( "/usr/bin/lp", "lp", pr.ascii(), 0 );
		    }
		    exit( 0 );
		} else {		// parent process
		    ::close( fds[0] );
		    pdrv = new QPSPrinter( this, fds[1] );
		    state = PST_ACTIVE;
		}
#endif // else part of _OS_OS2EMX_
#endif // else part for #if _OS_WIN32_
	    }
	    if ( state == PST_ACTIVE && pdrv )
		return ((QPSPrinter*)pdrv)->cmd( c, paint, p );
	} else {
	    // ignore it?  I don't know
	}
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
    return TRUE;
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
    static int widths[]	 = { 595, 516, 612, 612, 541,
			     2384, 1684, 1191, 842, 420, 297, 210, 148, 105,
			     2920, 2064, 91, 1460, 1032, 729, 516, 363, 258,
			     181, 127, 461, 297, 312, 595, 1224, 792 };

    static int heights[] = { 842, 729, 791, 1009, 720,
			     3370, 2384, 1684, 1191, 595, 420, 297, 210, 148,
			     4127, 2920, 127, 2064, 1460, 1032, 729, 516, 363,
			     258, 181, 648, 684, 624, 935, 792, 1224 };
    switch ( m ) {
	case PDM_WIDTH:
	    val = orient == Portrait ? widths[ s ] : heights[ s ];
	    break;
	case PDM_HEIGHT:
	    val = orient == Portrait ? heights[ s ] : widths[ s ];
	    break;
	case PDM_WIDTHMM:
	    val = orient == Portrait ? widths[ s ] : heights[ s ];
	    val = (val * 254 + 360) / 720; // +360 to get the right rounding
	    break;
	case PDM_HEIGHTMM:
	    val = orient == Portrait ? heights[ s ] : widths[ s ];
	    val = (val * 254 + 360) / 720;
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
