/**********************************************************************
** $Id: //depot/qt/main/src/kernel/qprinter.cpp#4 $
**
** Implementation of QPrinter class
**
** Author  : Eirik Eng and Haavard Nord
** Created : 941003
**
** Copyright (C) 1994,1995 by Troll Tech AS.  All rights reserved.
**
***********************************************************************/

#include "qprinter.h"

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qprinter.cpp#4 $";
#endif


/*----------------------------------------------------------------------------
  \class QPrinter qprinter.h
  \brief The QPrinter class is a paint device that prints graphics on a
  printer.

  \ingroup paintdevice

  All window systems that Qt supports, except X-Windows, has built-in
  printer drivers.  For X-Windows, Qt provides Postscript (tm)
  printing.

  Drawing graphics on a printer is almost identical to drawing graphics
  in a widget or a pixmap.  The only difference is that the programmer
  must think about dividing the document into pages and handling abort
  commands.

  The newPage() function should be called to finish the current page and
  start printing a new page.

  If the user decides to abort printing, aborted() will return TRUE.
  The QPrinter class handles abortion automatically, but the programmer
  should from time to time check the aborted() flag and stop painting
  if the print job has been aborted.
 ----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
  \fn const char *QPrinter::printerName() const
  Returns the printer name.
  \sa setPrinterName()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  Sets the printer name.

  The default printer will be used if no printer name is set.

  Under X-Windows, the PRINTER environment variable defines the
  default printer.  Under any other window system, the window
  system defines the default printer.

  \sa printerName()
 ----------------------------------------------------------------------------*/

void QPrinter::setPrinterName( const char *name )
{
    printer_name = name;
}


/*----------------------------------------------------------------------------
  \fn const char *QPrinter::docName() const
  Returns the document name.
  \sa setDocName()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  Sets the document name.
 ----------------------------------------------------------------------------*/

void QPrinter::setDocName( const char *name )
{
    doc_name = name;
}


/*----------------------------------------------------------------------------
  \fn const char *QPrinter::creator() const
  Returns the creator name.
  \sa setCreator()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  Sets the creator name.

  Calling this function has only effect for the X-Windows version of Qt.
  The creator name is the name of the application that created the document.
  If no creator name is specified, then the creator will be set to "Qt".

  \sa creator()
 ----------------------------------------------------------------------------*/

void QPrinter::setCreator( const char *creator )
{
    creator_name = creator;
}


/*----------------------------------------------------------------------------
  \fn QPrinter::Orientation QPrinter::orientation() const
  Returns the orientation setting. The default value is \c QPrinter::Portrait.
  \sa setOrientation()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  Sets the print orientation.

  The orientation can be either \c QPrinter::Portrait or
  \c QPainter::Landscape.

  The printer driver reads this setting and prints using the specified
  orientation.

  \sa orientation()
 ----------------------------------------------------------------------------*/

void QPrinter::setOrientation( Orientation orientation )
{
    orient = orientation;
}


/*----------------------------------------------------------------------------
  \fn int QPrinter::fromPage() const
  Returns the from-page setting.  The programmer is responsible to read
  this setting and print accordingly.
  The default value is 1.

  \sa setFromPage(), toPage()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn int QPrinter::toPage() const
  Returns the to-page setting.  The programmer is responsible to read
  this setting and print accordingly.
  The default value is 1.

  \sa setFromPage(), fromPage()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  Sets the from page and to page.

  The from-page and to-page settings specify what pages to print.

  \sa fromPage(), toPage()
 ----------------------------------------------------------------------------*/

void QPrinter::setFromTo( int fromPage, int toPage )
{
    from_pg = fromPage;
    to_pg = toPage;
}


/*----------------------------------------------------------------------------
  \fn int QPrinter::minPage() const
  Returns the min-page setting.  The default value is 0.
  \sa maxPage(), setMinMax()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn int QPrinter::maxPage() const
  Returns the max-page setting.  The default value is 0.
  \sa minPage(), setMinMax()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  Sets the min page and max page.

  The min-page and max-page restrict the from-page and to-page settings.
  When the printer selection dialog comes up, the user cannot select
  from and to that are outsize the range specified by min and max pages.

  \sa minPage(), maxPage(), select()
 ----------------------------------------------------------------------------*/

void QPrinter::setMinMax( int minPage, int maxPage )
{
    min_pg = minPage;
    max_pg = maxPage;
}


/*----------------------------------------------------------------------------
  \fn int QPrinter::numCopies() const
  Returns the number of copies to be printed.  The default value is 1.
  \sa setNumCopies()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  Sets the number of pages to be printed.

  The printer driver reads this setting and prints the specified number of
  copies.

  \sa numCopies()
 ----------------------------------------------------------------------------*/

void QPrinter::setNumCopies( int numCopies )
{
    ncopies = numCopies;
}
