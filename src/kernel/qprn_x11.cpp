/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qprn_x11.cpp#2 $
**
** Implementation of QPrinter class for X-Windows
**
** Author  : Eirik Eng
** Created : 941003
**
** Copyright (C) 1994,1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qprinter.h"
#include "qpsprn.h"
#include "qapp.h"

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qprn_x11.cpp#2 $";
#endif


// QPrinter states

#define PST_IDLE	0
#define PST_ACTIVE	1
#define PST_ERROR	2
#define PST_ABORTED	3


QPrinter::QPrinter()
    : QPaintDevice( PDT_PRINTER | PDF_EXTDEV )
{
    pdrv = new QPSPrinter( this );
    orient = Portrait;
    from_pg = to_pg  = ncopies = 1;
    min_pg  = max_pg = 0;
    state = PST_IDLE;
}

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
    if ( state == PST_ACTIVE && pdrv ) {
	pdrv->cmd( PDC_PRT_NEWPAGE, 0, 0 );
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
    if ( state == PST_ACTIVE && pdrv ) {
	pdrv->cmd( PDC_PRT_ABORT, 0, 0 );
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

  Returns TRUE if the user pressed "Ok" to print, or FALSE if the
  user cancelled the operation.

  \warning Not yet implemented for X-Windows.
*/

bool QPrinter::select( QWidget * )
{
    debug( "QPrinter::select: Not implemented" );
    return FALSE;
}


/*!
  \internal
  Handles painter commands to the printer.
*/

bool QPrinter::cmd( int c, QPainter *paint, QPDevCmdParam *p )
{
    if ( c ==  PDC_BEGIN ) {			// begin; start printing
	if ( state == PST_IDLE && pdrv ) {
	    QFile *output = new QFile( "/tmp/output.ps" );
	    output->open( IO_ReadWrite | IO_Truncate );
	    pdrv->device = output;
	    if ( pdrv->cmd( c, paint, p ) ) {
		state = PST_ACTIVE;
		return TRUE;
	    }
	}
    }
    else if ( c == PDC_END ) {			// end; printing done
	if ( pdrv ) {
	    pdrv->cmd( c, paint, p );
	    QFile *output = pdrv->device;
	    if ( output ) {
		output->close();
		system( "lpr /tmp/output.ps" );
	    }
	}
	state = PST_IDLE;
	return TRUE;
    }
    else {
	if ( state == PST_ACTIVE ) {
	    pdrv->cmd( c, paint, p );
	    return TRUE;
	}
    }
    return FALSE;
}
