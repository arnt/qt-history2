/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qprinter_x11.cpp#3 $
**
** Implementation of QPrinter class for X-Windows
**
** Author  : Haavard Nord
** Created : 950810
**
** Copyright (C) 1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qprinter.h"
#include "qpaintdc.h"
#include "qpsprn.h"
#include "qfile.h"
#include "qapp.h"
#include <stdlib.h>
#include <unistd.h>

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qprinter_x11.cpp#3 $";
#endif


/*****************************************************************************
  Internal QPrintDialog class
 *****************************************************************************/

#include "qmsgbox.h"

#if 0
class QPrintDialog : public QDialog
{
//    Q_OBJECT
public:
    QPrintDialog( QWidget *parent=0, const char *name=0 );
   ~QPrintDialog();
};
#endif

/*****************************************************************************
  QPrinter member functions
 *****************************************************************************/

// QPrinter states

#define PST_IDLE	0
#define PST_ACTIVE	1
#define PST_ERROR	2
#define PST_ABORTED	3


/*----------------------------------------------------------------------------
  Constructs a printer paint device.
 ----------------------------------------------------------------------------*/

QPrinter::QPrinter()
    : QPaintDevice( PDT_PRINTER | PDF_EXTDEV )
{
    pdrv = new QPSPrinter( this );
    orient = Portrait;
    from_pg = to_pg  = ncopies = 1;
    min_pg  = max_pg = 0;
    state = PST_IDLE;
    printer_name = getenv( "PRINTER" );
    output_file = FALSE;
    print_prog = "/usr/bin/lpr";
}

/*----------------------------------------------------------------------------
  Constructs the printer paint device.
 ----------------------------------------------------------------------------*/

QPrinter::~QPrinter()
{
    delete pdrv;
}


/*----------------------------------------------------------------------------
  Advances to a new page on the printer.
  Returns TRUE if successful, otherwise FALSE.
 ----------------------------------------------------------------------------*/

bool QPrinter::newPage()
{
    if ( state == PST_ACTIVE && pdrv )
	((QPSPrinter*)pdrv)->cmd( PDC_PRT_NEWPAGE, 0, 0 );
    return FALSE;
}


/*----------------------------------------------------------------------------
  Aborts the print job.
  Returns TRUE if successful, otherwise FALSE.
  \sa aborted()
 ----------------------------------------------------------------------------*/

bool QPrinter::abort()
{
    if ( state == PST_ACTIVE && pdrv ) {
	((QPSPrinter*)pdrv)->cmd( PDC_PRT_ABORT, 0, 0 );
	state = PST_ABORTED;
    }
    return state == PST_ABORTED;
}

/*----------------------------------------------------------------------------
  Returns TRUE is the printer job was aborted, otherwise FALSE.
  \sa abort()
 ----------------------------------------------------------------------------*/

bool QPrinter::aborted() const
{
    return state == PST_ABORTED;
}


/*----------------------------------------------------------------------------
  Opens a printer setup dialog and asks the user to specify what printer
  to use and miscellaneous printer settings.

  Returns TRUE if the user pressed "Ok" to print, or FALSE if the
  user cancelled the operation.

  \warning Not yet fully implemented for X-Windows.
 ----------------------------------------------------------------------------*/

bool QPrinter::select( QWidget *parent )
{
    return QMessageBox::message("Print","\nAre you sure you want to print\n",
				"Lets go",parent);
}


/*----------------------------------------------------------------------------
  \internal
  Handles painter commands to the printer.
 ----------------------------------------------------------------------------*/

bool QPrinter::cmd( int c, QPainter *paint, QPDevCmdParam *p )
{
    QPSPrinter *ps = (QPSPrinter*)pdrv;
    if ( c ==  PDC_BEGIN ) {			// begin; start printing
	if ( ps && state == PST_IDLE ) {
	    char *fname  = output_file ? output_filename : tmpnam(0);
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
	    if ( ps->cmd( c, paint, p ) )	// successful
		state = PST_ACTIVE;
	    else {				// could not start printing
		ps->device = 0;
		delete output;
		state = PST_ERROR;
	    }
	    return TRUE;
	}
    }
    else if ( c == PDC_END ) {			// end; printing done
	if ( ps && state != PST_ERROR ) {
	    ps->cmd( c, paint, p );
	    QFile *output = (QFile*)ps->device;
	    if ( output ) {
		if ( !output_file ) {		// send to printer
		    QString pr = printer_name.copy();
		    if ( pr.isEmpty() )		// not set, then read $PRINTER
			pr = getenv( "PRINTER" );
		    if ( pr.isEmpty() ) {	// no printer set
#if defined(DEBUG)
			warning( "QPrinter: No default printer." );
#endif
		    }
		    else {
			pr.insert( 0, "-P" );
			if ( fork() == 0 ) {	// child process
			    if ( execl(print_prog.data(), pr.data(),
				       output->name(), 0) == -1 ) {
#if defined(DEBUG)
				debug( "QPrinter: Exec error" );
#endif
			    }
			    else {
#if defined(DEBUG)
				debug( "QPrinter: Job done" );
#endif
			    }
			    exit( 0 );		// exit print job
			}
		    }
		}
		output->close();
		delete output;
		ps->device = 0;
	    }
	}
	state = PST_IDLE;
	return TRUE;
    }
    else {
	if ( state == PST_ACTIVE ) {
	    return ps->cmd( c, paint, p );
	}
    }
    return FALSE;
}
