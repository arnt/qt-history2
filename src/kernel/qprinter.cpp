/**********************************************************************
** $Id: $
**
** Implementation of QPrinter class
**
** Created : 941003
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qprinter.h"

#ifndef QT_NO_PRINTER

/*!
  \class QPrinter qprinter.h
  \brief The QPrinter class is a paint device that paints on a printer.

  \ingroup drawing

  On Windows it uses the built-in printer drivers.  On X11 it
  generates postscript and sends that to lpr, lp, or another print
  command.

  QPrinter is used much the same way as QWidget and QPixmap are
  used.  The big difference is that you must keep track of the pages.

  QPrinter supports a number of settable parameters, most of which can be
  changed by the end user when the application calls QPrinter::setup().

  The most important parameters are: <ul>
  <li> setOrientation() tells QPrinter to turn the page (virtual).
  <li> setPageSize() tells QPrinter what page size to expect from the
  printer.
  <li> setResolution() tells QPrinter what resolution you wish the
  printer to provide (in dpi).
  <li> setFullPage() tells QPrinter whether you want to deal with the
  full page (so you can have accurate margins, etc.) or just with
  the part the printer can draw on.  The default is FALSE, so that by
  default you can probably paint on (0,0) but the document's margins
  are unknown.

  <li> setNumCopies() tells QPrinter how many copies of the document
  it should print.
  <li> setMinMax() tells QPrinter and QPrintDialog what the allowed
  range for fromPage() and toPage() are.
  </ul>

  Except where noted, you can only call the set functions before
  setup(), or between QPainter::end() and setup(). (Some may have
  effect between setup() and begin(), or between begin() and end(),
  but that's strictly undocumented and such behaviour may differ
  depending on platform.)

  There are also some settings that the user sets (through the printer
  dialog) and that applications are expected to obey: <ul>

  <li> pageOrder() tells the application program whether to print
  first-page-first or last-page-first.

  <li> colorMode() tells the application program whether to print
  in color or grayscale.  (If you print in color and the printer does
  not support color, Qt will try to approximate.  The document may
  take longer to print, but the quality should not be made visibly
  poorer.)

  <li> fromPage() and toPage() indicate what pages the application
  program should print.

  <li> paperSource() tells the application progam which paper source
  to print from.

  </ul>

  You can of course call these functions to establish defaults
  before you ask the user through QPrinter::setup().

  Once you start printing, newPage() is essential.  You will
  probably also need to look at the QPaintDeviceMetrics for the
  printer (see the <a href="simple-application.html#printer">simple
  print function</a> in the Application walk-through). Note that the
  paint device metrics are valid only after the QPrinter has been set
  up, i.e., after setup() has returned successfully. If you want
  high-quality printing with accurate margins, setFullPage(TRUE) is
  a must.

  If you want to abort the print job, abort() will make a best effort.
  It may cancel the entire job or just some of it.

  \omit Need a function to setup() without a dialog (ie. use defaults).
*/

/*! \enum QPrinter::PrinterMode

  This enum describes the mode the printer should work in. It basically
  presets a certain resolution and working mode.

  \value ScreenResolution Sets the resolution of the print device to the
  screen resolution. This has the big advantage, that the results obtained
  when painting on the printer will match more or less exactly the visible
  output on the screen. It is the easiest to use, as fontmetrics on the
  screen and on the printer are the same. This is the default value.

  \value PrinterResolution Use the physical resolution of the printer on
  Windows. On Unix, set the postscript resolution to 72 dpi.

  \value HighResolution Use printer resolution on windows, set the resolution
  of the postscript driver to 600dpi.

  \value Compatible Almost the same as PrinterResolution, but keeps some
  peculiarities of the printer dirver of Qt-2.x. This is useful, when porting an
  application from Qt-2.x to Qt-3.


 */

/*! \enum QPrinter::Orientation

  This enum type (not to be confused with Qt::Orientation) is used to
  decide how Qt should print on each sheet.

  \value Portrait the page's height is greater than its width (the
  default).

  \value Landscape the page's width is greater than its height.

  This type interacts with QPrinter::PageSize and
  QPrinter::setFullPage() to determine the final size of the page
  available to the application.
*/


/*! \enum QPrinter::PageSize

  This enum type decides what paper size QPrinter is to use.  QPrinter
  does not check that the paper size is available; it just uses this
  information, together with Orientation and QPrinter::setFullPage(), to
  determine the printable area (see QPaintDeviceMetrics).

  The defined sizes (with setFullPage( TRUE )) are

  \value A0 841 x 1189 mm
  \value A1 594 x 841 mm
  \value A2 420 x 594 mm
  \value A3 297 x 420 mm
  \value A4 210 x 297 mm, 8.26 x 11.7 inches
  \value A5 148 x 210 mm
  \value A6 105 x 148 mm
  \value A7 74 x 105 mm
  \value A8 52 x 74 mm
  \value A9 37 x 52 mm
  \value B0 1030 x 1456 mm
  \value B1 728 x 1030 mm
  \value B10 32 x 45 mm
  \value B2 515 x 728 mm
  \value B3 364 x 515 mm
  \value B4 257 x 364 mm
  \value B5 182 x 257 mm, 7.17 x 10.13 inches
  \value B6 128 x 182 mm
  \value B7 91 x 128 mm
  \value B8 64 x 91 mm
  \value B9 45 x 64 mm
  \value C5E 163 x 229 mm
  \value Comm10E 105 x 241 mm, US Common #10 Envelope
  \value DLE 110 x 220 mm
  \value Executive 7.5 x 10 inches, 191 x 254 mm
  \value Folio 210 x 330 mm
  \value Ledger 432 x 279 mm
  \value Legal 8.5 x 14 inches, 216 x 356 mm
  \value Letter 8.5 x 11 inches, 216 x 279 mm
  \value Tabloid 279 x 432 mm

  With setFullPage(FALSE) (the default), the metrics will be a bit
  smaller; how much depends on the printer in use.
*/


/*! \enum QPrinter::PageOrder

  This enum type is used by QPrinter/QPrintDialog to tell the
  application program how to print.  The possible values are

  \value FirstPageFirst  the lowest-numbered page should
  be printed first.

  \value LastPageFirst  the highest-numbered page should
  be printed first.
*/

/*! \enum QPrinter::ColorMode

  This enum type is used to indicate whether QPrinter should print in
  color or not.  The possible values are:

  \value Color  print in color if available, otherwise in grayscale.  This
  is the default.

  \value GrayScale  print in grayscale, even on color printers.
  Might be a little faster than \c Color.
*/

/*! \enum QPrinter::PaperSource

  This enum type decides what paper source QPrinter is to use.  QPrinter
  does not check that the paper source is available; it just uses this
  information to try and set the paper source.  Whether it will set the
  paper source depends on whether the printer has that particular source.

  Note: It is currently only implemented for Windows.

  \value OnlyOne
  \value Lower
  \value Middle
  \value Manual
  \value Envelope
  \value EnvelopeManual
  \value Auto
  \value Tractor
  \value SmallFormat
  \value LargeFormat
  \value LargeCapacity
  \value Cassette
  \value FormSource
*/

/*!
  \fn QString QPrinter::printerName() const

  Returns the printer name.  This value is initially set to the name of the
  default printer.

  \sa setPrinterName()
*/

/*!
  \fn bool QPrinter::outputToFile() const
  Returns TRUE if the output should be written to a file, or FALSE if the
  output should be sent directly to the printer.
  The default setting is FALSE.

  This function is currently supported only under X11.

  \sa setOutputToFile(), setOutputFileName()
*/

/*!
  Specifies whether the output should be written to a file or sent
  directly to the printer.

  Will output to a file if \e enable is TRUE, or will output directly
  to the printer if \e enable is FALSE.

  This function is currently supported only under X11.

  \sa outputToFile(), setOutputFileName()
*/

void QPrinter::setOutputToFile( bool enable )
{
    if ( state != 0 ) {
#if defined(QT_CHECK_STATE)
        qWarning( "QPrinter::setOutputToFile: Cannot do this during printing" );
#endif
        return;
    }
    output_file = enable;
}


/*!
  \fn QString QPrinter::outputFileName() const
  Returns the name of the output file.  There is no default file name.
  \sa setOutputFileName(), setOutputToFile()
*/

/*!
  Sets the name of the output file.

  Setting a null name (0 or "") disables output to a file, i.e., calls
  setOutputToFile(FALSE). Setting a non-null name enables output to a
  file, i.e. calls setOutputToFile(TRUE).

  This function is currently supported only under X11.

  \sa outputFileName(), setOutputToFile()
*/

void QPrinter::setOutputFileName( const QString &fileName )
{
    if ( state != 0 ) {
#if defined(QT_CHECK_STATE)
        qWarning("QPrinter::setOutputFileName: Cannot do this during printing");
#endif
        return;
    }
    output_filename = fileName;
    output_file = !output_filename.isEmpty();
}


/*!
  \fn QString QPrinter::printProgram() const

  Returns the name of the program that sends the print output to the printer.

  The default is to return a null string; meaning that QPrinter will
  try to be smart in a system-dependent way.  On X11 only, you can set
  it to something different to use a specific print program.

  On Windows, this function returns the name of the printer device driver.

  \sa setPrintProgram() setPrinterSelectionOption()
*/

/*!
  Sets the name of the program that should do the print job.

  On X11, this function sets the program to call with the PostScript
  output.  On other platforms, it has no effect.

  \sa printProgram()
*/

void QPrinter::setPrintProgram( const QString &printProg )
{
    print_prog = printProg;
}


/*!
  \fn QString QPrinter::docName() const
  Returns the document name.
  \sa setDocName()
*/

/*!
  Sets the document name.
*/

void QPrinter::setDocName( const QString &name )
{
    if ( state != 0 ) {
#if defined(QT_CHECK_STATE)
        qWarning( "QPrinter::setDocName: Cannot do this during printing" );
#endif
        return;
    }
    doc_name = name;
}


/*!
  \fn QString QPrinter::creator() const
  Returns the creator name.
  \sa setCreator()
*/

/*!
  Sets the creator name.

  Calling this function has effect only for the X11 version of Qt.
  The creator name is the name of the application that created the
  document.  If no creator name is specified, the creator will be
  set to "Qt" with some version number.

  \sa creator()
*/

void QPrinter::setCreator( const QString &creator )
{
    creator_name = creator;
}


/*!
  \fn Orientation QPrinter::orientation() const

  Returns the orientation setting.  The default value is \c
  QPrinter::Portrait.
  \sa setOrientation()
*/

/*!
  Sets the print orientation.

  The orientation can be either \c QPrinter::Portrait or
  \c QPrinter::Landscape.

  The printer driver reads this setting and prints using the specified
  orientation.  On Windows however, this setting won't take effect until
  the printer dialog is shown (using QPrinter::setup() ).

  \sa orientation()
*/

void QPrinter::setOrientation( Orientation orientation )
{
    orient = orientation;
}


/*!
  \fn PageSize QPrinter::pageSize() const

  Returns the printer page size.  The default value is system-dependent.

  \sa setPageSize()
*/


/*!  Sets the printer page size to \a newPageSize if that size is
  supported. The result if undefined if \a newPageSize is not
  supported.

  The default page size is system-dependent.

  This function is useful mostly for setting a default value that the
  user can override in the print dialog when you call setup().

  \sa pageSize() PageSize setFullPage() setResolution()
*/

void QPrinter::setPageSize( PageSize newPageSize )
{
    if ( newPageSize > NPageSize ) {
#if defined(QT_CHECK_STATE)
        qWarning("QPrinter::SetPageSize: illegal page size %d", newPageSize );
#endif
        return;
    }
    page_size = newPageSize;
}


/*!  Sets the page order to \a newPageOrder.

  The page order can be \c QPrinter::FirstPageFirst or \c
  QPrinter::LastPageFirst.  The application programmer is responsible
  for reading the page order and printing accordingly.

  This function is useful mostly for setting a default value that the
  user can override in the print dialog when you call setup().
*/

void QPrinter::setPageOrder( PageOrder newPageOrder )
{
    page_order = newPageOrder;
}


/*!  Returns the current page order.

  The default page order is FirstPageFirst.
*/

QPrinter::PageOrder QPrinter::pageOrder() const
{
    return page_order;
}


/*!  Sets the printer's color mode to \a newColorMode, which can be
  one of \c Color (the default) and \c GrayScale.

  \sa colorMode()
*/

void QPrinter::setColorMode( ColorMode newColorMode )
{
    color_mode = newColorMode;
}


/*!  Returns the current color mode.  The default color mode is \c
  Color.

  \sa setColorMode()
*/

QPrinter::ColorMode QPrinter::colorMode() const
{
    return color_mode;
}


/*!
  \fn int QPrinter::fromPage() const
  Returns the from-page setting.  The default value is 0.

  The programmer is responsible for reading this setting and printing
  accordingly.

  \sa setFromTo(), toPage()
*/

/*!
  \fn int QPrinter::toPage() const
  Returns the to-page setting.  The default value is 0.

  The programmer is responsible for reading this setting and printing
  accordingly.

  \sa setFromTo(), fromPage()
*/

/*!
  Sets the from-page and to-page settings.

  The from-page and to-page settings specify what pages to print.

  This function is useful mostly to set a default value that the
  user can override in the print dialog when you call setup().

  \sa fromPage(), toPage(), setMinMax(), setup()
*/

void QPrinter::setFromTo( int fromPage, int toPage )
{
    if ( state != 0 ) {
#if defined(QT_CHECK_STATE)
        qWarning( "QPrinter::setFromTo: Cannot do this during printing" );
#endif
        return;
    }
    from_pg = fromPage;
    to_pg = toPage;
}


/*!
  \fn int QPrinter::minPage() const
  Returns the min-page setting, i.e. the lowest pagenumber a user
  is allowed to choose.  The default value is 0.
  \sa maxPage(), setMinMax()
*/

/*!
  \fn int QPrinter::maxPage() const
  Returns the max-page setting. A user can't choose a higher pagenumber
  than maxPage() when he or she selects a printrange. The default value is 0.
  \sa minPage(), setMinMax()
*/

/*!
  Sets the min-page and max-page settings.

  The min-page and max-page restrict the from-page and to-page
  settings.  When the printer setup dialog comes up, the user cannot
  select from and to that are outside the range specified by min and
  max pages.

  \sa minPage(), maxPage(), setFromTo(), setup()
*/

void QPrinter::setMinMax( int minPage, int maxPage )
{
    min_pg = minPage;
    max_pg = maxPage;
}


/*!
  \fn int QPrinter::numCopies() const
  Returns the number of copies to be printed.  The default value is 1.
  \sa setNumCopies()
*/

/*!
  Sets the number of pages to be printed.

  The printer driver reads this setting and prints the specified number of
  copies.

  \sa numCopies(), setup()
*/

void QPrinter::setNumCopies( int numCopies )
{
    ncopies = numCopies;
}


/*!  Returns the printer options selection string.  This is
useful only if the print command has been explicitly set.

The default value (a null string) implies to select printer in a
system-dependent manner.

Any other value implies to use that value.

\sa setPrinterSelectionOption()
*/

QString QPrinter::printerSelectionOption() const
{
    return option_string;
}


/*!  Sets the printer to use \a option to select printer.  \a option
is null by default (meaning to be a little smart), but it can be set to
other values to use a specific printer selection option.

If the printer selection option is changed while the printer is
active, the current print job may or may not be affected.
*/

void QPrinter::setPrinterSelectionOption( const QString & option )
{
    option_string = option;
}


/*!  Sets QPrinter to have the origin of the coordinate system at the
top-left corner of the paper if \a fp is TRUE, or where it thinks the
top-left corner of the printable area is if \a fp is FALSE.

The default is FALSE. You can (probably) print on (0,0), and
QPaintDeviceMetrics will report something smaller than the size
indicated by PageSize.  (Note that QPrinter may be wrong - it does not
have perfect knowledge of the physical printer.)

If you set it to TRUE, QPaintDeviceMetrics will report the exact same size
as indicated by PageSize, but you cannot print on all of that - you have
to take care of the output margins yourself.

\sa PageSize setPageSize() QPaintDeviceMetrics fullPage()
*/

void QPrinter::setFullPage( bool fp )
{
    to_edge = fp;
}


/*!  Returns TRUE if the origin of the printer's coordinate system is
at the corner of the sheet and FALSE if it is at the edge of the
printable area.

See setFullPage() for more detail and some warnings.

\sa setFullPage() PageSize QPaintDeviceMetrics
*/

bool QPrinter::fullPage() const
{
    return to_edge;
}


/*! Requests the printer to operate at \a dpi if possible, or do the
best it can.

This setting affects the coordinate system as returned by
e.g. QPaintDeviceMetrics and QPainter::viewport().

The value depends on the PrintingMode used in the QPrinter constructor.
By default, the dpi value of the screen is used.

This function must be called before setup() to have an effect on all platforms.

\sa resolution() setPageSize()
*/

void QPrinter::setResolution( int dpi )
{
    res = dpi;
}


/*! Returns the current assumed resolution of the printer, as set by
setResolution() or the printer subsystem.

\sa setResolution()
*/

int QPrinter::resolution() const
{
    return res;
}

/*! Sets the paper source setting.

    \sa paperSource()
*/

void QPrinter::setPaperSource( PaperSource source )
{
    paper_source = source;
}

/*! Returns the currently set paper source of the printer.

    \sa setPaperSource()
*/

QPrinter::PaperSource QPrinter::paperSource() const
{
    return paper_source;
}

#endif // QT_NO_PRINTER

