/**********************************************************************
** $Id: //depot/qt/main/src/kernel/qprinter.cpp#5 $
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
static char ident[] = "$Id: //depot/qt/main/src/kernel/qprinter.cpp#5 $";
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
  Returns the printer name.  This value is initially set to the name of the
  default printer.
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
    if ( state != 0 ) {
#if defined(CHECK_STATE)
	warning( "QPrinter::setPrinterName: Cannot do this during printing" );
#endif
	return;
    }
    printer_name = name;
}


/*----------------------------------------------------------------------------
  \fn bool QPrinter::outputToFile() const
  Returns TRUE if the output should be written to a file, or FALSE if the
  output should be sent directly to the printer.
  The default setting is FALSE.

  This function is currently only supported under X-Windows.

  \sa setOutputToFile(), setOutputFileName()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  Specifies whether the output should be written to a file or sent
  directly to the printer.

  Will output to a file if \e enable is TRUE, or will output directly
  to the printer if \e enable is FALSE.

  This function is currently only supported under X-Windows.

  \sa outputToFile(), setOutputFileName()
 ----------------------------------------------------------------------------*/

void QPrinter::setOutputToFile( bool enable )
{
    if ( state != 0 ) {
#if defined(CHECK_STATE)
	warning( "QPrinter::setOutputToFile: Cannot do this during printing" );
#endif
	return;
    }
    output_file = enable;
}


/*----------------------------------------------------------------------------
  \fn const char *QPrinter::outputFileName() const
  Returns the name of the output file.  There is no default file name.
  \sa setOutputFileName(), setOutputToFile()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  Sets the name of the output file.

  Setting a null name (0 or "") disables output to a file, i.e.
  calls setOutputToFile(FALSE);
  Setting non-null name enables output to a file, i.e. calls
  setOutputToFile(TRUE).

  This function is currently only supported under X-Windows.

  \sa outputFileName(), setOutputToFile()
 ----------------------------------------------------------------------------*/

void QPrinter::setOutputFileName( const char *fileName )
{
    if ( state != 0 ) {
#if defined(CHECK_STATE)
	warning("QPrinter::setOutputFileName: Cannot do this during printing");
#endif
	return;
    }
    output_filename = fileName;
    output_file = !output_filename.isEmpty();
}


/*----------------------------------------------------------------------------
  \fn const char *QPrinter::printProgram() const
  Returns the name of the program that sends the print output to the printer.

  The default setting is "/usr/bin/lpr" under X-Windows.  This function
  returns 0 for all other window systems.

  \sa setPrintProgram()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  Sets the name of the program that should do the print job.

  If an output file has been defined, the printer driver will print to
  the output file instead of directly to the printer.

  This function is only supported under X-Windows.

  \sa printProgram()
 ----------------------------------------------------------------------------*/

void QPrinter::setPrintProgram( const char *printProg )
{
    if ( state != 0 ) {
#if defined(CHECK_STATE)
	warning( "QPrinter::setPrintProgram: Cannot do this during printing" );
#endif
	return;
    }
    print_prog = printProg;
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
    if ( state != 0 ) {
#if defined(CHECK_STATE)
	warning( "QPrinter::setDocName: Cannot do this during printing" );
#endif
	return;
    }
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
  \fn QPrinter::PageSize QPrinter::pageSize() const
  Returns the printer page size. The default value is \c QPrinter::A4.
  \sa setPageSize()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  Sets the printer page size.

  The page size can be one of:
  <ul>
  <li>QPrinter::A4</li> (8.26x11.7 inches, 210x297 mm)
  <li>QPrinter::B5</li> (7.17x10.13 inches, 182x257 mm)
  <li>QPrinter::Letter</li> (8.5x11 inches, 216x279 mm)
  <li>QPrinter::Legal</li> (8.5x14 inches, 216x356 mm)
  <li>QPrinter::Executive</li> (7.5x10 inches, 191x254 mm)
  </ul>

  \warning Not yet implemented for X-Windows.

  \sa pageSize()
 ----------------------------------------------------------------------------*/

void QPrinter::setPageSize( PageSize pageSize )
{
    page_size = pageSize;
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

  \warning Not yet implemented for X-Windows.

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
    if ( state != 0 ) {
#if defined(CHECK_STATE)
	warning( "QPrinter::setFromTo: Cannot do this during printing" );
#endif
	return;
    }
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
    if ( state != 0 ) {
#if defined(CHECK_STATE)
	warning( "QPrinter::setNumCopies: Cannot do this during printing" );
#endif
	return;
    }
    ncopies = numCopies;
}
