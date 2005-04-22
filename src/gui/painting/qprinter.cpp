/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QT_NO_PRINTER

#include "qprinter_p.h"
#include "qprinter.h"
#include "qprintengine.h"
#include "qlist.h"
#include <qprintdialog.h>
#include <qpagesetupdialog.h>

#if defined (Q_WS_WIN)
#include <private/qprintengine_win_p.h>
#elif defined (Q_WS_MAC)
#include <private/qprintengine_mac_p.h>
#elif defined (Q_OS_UNIX)
#include <private/qprintengine_ps_p.h>
#endif

#ifdef QT3_SUPPORT
#  include "qprintdialog.h"
#endif // QT3_SUPPORT

#define ABORT_IF_ACTIVE(location) \
    if (d->printEngine->printerState() == QPrinter::Active) { \
        qWarning("%s, cannot be changed while printer is active", location); \
        return; \
    }

/*!
  \class QPrinter qprinter.h
  \brief The QPrinter class is a paint device that paints on a printer.

  \ingroup multimedia
  \mainclass

  On Windows it uses the built-in printer drivers. On X11 it
  generates postscript and sends that to lpr, lp, or another
  printProgram().

  QPrinter is used in much the same way as QWidget and QPixmap are
  used. The big difference is that you must keep track of the pages.

  QPrinter supports a number of settable parameters, most of which
  can be changed by the end user through a QPrintDialog.

  The most important parameters are:
  \list
  \i setOrientation() tells QPrinter which page orientation to use (virtual).
  \i setPageSize() tells QPrinter what page size to expect from the
  printer.
  \i setResolution() tells QPrinter what resolution you wish the
  printer to provide (in dots per inch -- dpi).
  \i setFullPage() tells QPrinter whether you want to deal with the
  full page or just with the part the printer can draw on. The
  default is false, so that by default you should be able to paint
  on (0,0). If true the origin of the coordinate system will be in
  the top left corner of the paper and most probably the printer
  will not be able to paint something there due to it's physical
  margins.
  \i setNumCopies() tells QPrinter how many copies of the document
  it should print.
  \i setMinMax() tells QPrinter and QPrintDialog what the allowed
  range for fromPage() and toPage() are.
  \endlist

  Except where noted, you can only call the set functions before
  setup(), or between QPainter::end() and setup(). (Some may take
  effect between setup() and begin(), or between begin() and end(),
  but that's strictly undocumented and such behaviour may differ
  between platforms.)

  There are also some settings that the user sets (through the
  printer dialog) and that applications are expected to obey:

  \list

  \i pageOrder() tells the application program whether to print
  first-page-first or last-page-first.

  \i colorMode() tells the application program whether to print in
  color or grayscale. (If you print in color and the printer does
  not support color, Qt will try to approximate. The document may
  take longer to print, but the quality should not be made visibly
  poorer.)

  \i fromPage() and toPage() indicate what pages the application
  program should print.

  \i paperSource() tells the application progam which paper source
  to print from.

  \endlist

  You can call these functions to set default values before you open
  a QPrintDialog.

  Once you start printing, calling newPage() is essential. You will
  probably also need to look at the device metrics for the
  printer. Note that the paint device metrics are valid only after
  the QPrinter has been set up, i.e. after setup() has returned
  successfully.

  If you want to abort the print job, abort() will try its best to
  stop printing. It may cancel the entire job or just part of it.

  If your current locale converts "," to ".", you will need to set
  a locale that doesn't do this (e.g. the "C" locale) before using
  QPrinter.

  The TrueType font embedding for Qt's postscript driver uses code
  by David Chappell of Trinity College Computing Center.

  \legalese
  \code

  Copyright 1995, Trinity College Computing Center.
  Written by David Chappell.

  Permission to use, copy, modify, and distribute this software and
  its documentation for any purpose and without fee is hereby
  granted, provided that the above copyright notice appear in all
  copies and that both that copyright notice and this permission
  notice appear in supporting documentation. This software is
  provided "as is" without express or implied warranty.

  TrueType font support. These functions allow PPR to generate
  PostScript fonts from Microsoft compatible TrueType font files.

  The functions in this file do most of the work to convert a
  TrueType font to a type 3 PostScript font.

  Most of the material in this file is derived from a program called
  "ttf2ps" which L. S. Ng posted to the usenet news group
  "comp.sources.postscript". The author did not provide a copyright
  notice or indicate any restrictions on use.

  Last revised 11 July 1995.
  \endcode
*/

/*!
    \enum QPrinter::PrinterState

    \value Idle
    \value Active
    \value Aborted
    \value Error
*/

/*!
    \enum QPrinter::PrinterMode

    This enum describes the mode the printer should work in. It
    basically presets a certain resolution and working mode.

    \value ScreenResolution Sets the resolution of the print device to
    the screen resolution. This has the big advantage that the results
    obtained when painting on the printer will match more or less
    exactly the visible output on the screen. It is the easiest to
    use, as font metrics on the screen and on the printer are the
    same. This is the default value. ScreenResolution will produce a
    lower quality output than HighResolution and should only be used
    for drafts.

    \value PrinterResolution This value is deprecated. Is is
    equivalent to ScreenResolution on Unix and HighResolution on
    Windows and Mac. Due do the difference between ScreenResolution
    and HighResolution, use of this value may lead to non-portable
    printer code.

    \value HighResolution Use printer resolution on Windows, and set
    the resolution of the Postscript driver to 600dpi.
*/

/*!
  \enum QPrinter::Orientation

  This enum type (not to be confused with \c Orientation) is used
  to specify each page's orientation.

  \value Portrait the page's height is greater than its width (the
  default).

  \value Landscape the page's width is greater than its height.

  This type interacts with \l QPrinter::PageSize and
  QPrinter::setFullPage() to determine the final size of the page
  available to the application.
*/


/*!
  \enum QPrinter::PageSize

  This enum type specifies what paper size QPrinter should use.
  QPrinter does not check that the paper size is available; it just
  uses this information, together with QPrinter::Orientation and
  QPrinter::setFullPage(), to determine the printable area.

  The defined sizes (with setFullPage(true)) are:

  \value A0 841 x 1189 mm
  \value A1 594 x 841 mm
  \value A2 420 x 594 mm
  \value A3 297 x 420 mm
  \value A4 210 x 297 mm, 8.26 x 11.69 inches
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
  \value Comm10E 105 x 241 mm, US Common 10 Envelope
  \value DLE 110 x 220 mm
  \value Executive 7.5 x 10 inches, 191 x 254 mm
  \value Folio 210 x 330 mm
  \value Ledger 432 x 279 mm
  \value Legal 8.5 x 14 inches, 216 x 356 mm
  \value Letter 8.5 x 11 inches, 216 x 279 mm
  \value Tabloid 279 x 432 mm
  \value Custom
  \omitvalue NPageSize

  With setFullPage(false) (the default), the metrics will be a bit
  smaller; how much depends on the printer in use.
*/


/*!
  \enum QPrinter::PageOrder

  This enum type is used by QPrinter to tell the application program
  how to print.

  \value FirstPageFirst  the lowest-numbered page should be printed
  first.

  \value LastPageFirst  the highest-numbered page should be printed
  first.
*/

/*!
  \enum QPrinter::ColorMode

  This enum type is used to indicate whether QPrinter should print
  in color or not.

  \value Color  print in color if available, otherwise in grayscale.

  \value GrayScale  print in grayscale, even on color printers.
  Might be a little faster than \c Color. This is the default.
*/

/*!
  \enum QPrinter::PaperSource

  This enum type specifies what paper source QPrinter is to use.
  QPrinter does not check that the paper source is available; it
  just uses this information to try and set the paper source.
  Whether it will set the paper source depends on whether the
  printer has that particular source.

  \warning This is currently only implemented for Windows.

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

/*
  \enum QPrinter::PrintRange

  This enum is used to specify which print range the application
  should use to print.

  \value AllPages All the pages should be printed.
  \value Selection Only the selection should be printed.
  \value PageRange Print according to the from page and to page options.

  \sa setPrintRange(), printRange()
*/

/*
  \enum QPrinter::PrinterOption

  This enum describes various printer options that appear in the
  printer setup dialog. It is used to enable and disable these
  options in the setup dialog.

  \value PrintToFile Describes if print to file should be enabled.
  \value PrintSelection Describes if printing selections should be enabled.
  \value PrintPageRange Describes if printing page ranges (from, to) should
  be enabled

  \sa setOptionEnabled(), isOptionEnabled()
*/

/*!
    Creates a new printer object with the given \a mode.
*/
QPrinter::QPrinter(PrinterMode mode)
    : QPaintDevice(),
      d_ptr(new QPrinterPrivate)
{
    Q_D(QPrinter);
#if defined (Q_WS_WIN)
    d->printEngine = new QWin32PrintEngine(mode);
#elif defined (Q_WS_MAC)
    d->printEngine = new QMacPrintEngine(mode);
#elif defined (Q_OS_UNIX)
    d->printEngine = new QPSPrintEngine(mode);
#endif
}

/*!
    Destroys the printer object and frees any allocated resources. If
    the printer is destroyed while a print job is in progress this may
    or may not affect the print job.
*/
QPrinter::~QPrinter()
{
    Q_D(QPrinter);
#ifdef QT3_SUPPORT
    delete d->printDialog;
#endif
    delete d;
}


int QPrinter::devType() const
{
    return QInternal::Printer;
}

/*!
    Returns the printer name. This value is initially set to the name
    of the default printer.

    \sa setPrinterName()
*/
QString QPrinter::printerName() const
{
    Q_D(const QPrinter);
    return d->printEngine->property(QPrintEngine::PPK_PrinterName).toString();
}

/*!
    Sets the printer name to \a name.

    \sa printerName()
*/
void QPrinter::setPrinterName(const QString &name)
{
    Q_D(QPrinter);
    ABORT_IF_ACTIVE("QPrinter::setPrinterName()");
    d->printEngine->setProperty(QPrintEngine::PPK_PrinterName, name);
}


/*!
  \fn bool QPrinter::outputToFile() const

  Returns true if the output should be written to a file, or false
  if the output should be sent directly to the printer. The default
  setting is false.

  \sa setOutputToFile(), setOutputFileName()
*/


/*!
  \fn void QPrinter::setOutputToFile(bool enable)

  Specifies whether the output should be written to a file or sent
  directly to the printer.

  Will output to a file if \a enable is true, or will output
  directly to the printer if \a enable is false.

  \sa outputToFile(), setOutputFileName()
*/


/*!
  \fn QString QPrinter::outputFileName() const

  Returns the name of the output file. There is no default file
  name.

  \sa setOutputFileName(), setOutputToFile()
*/

QString QPrinter::outputFileName() const
{
    Q_D(const QPrinter);
    return d->printEngine->property(QPrintEngine::PPK_OutputFileName).toString();
}

/*!
  Sets the name of the output file to \a fileName.

  Setting a null or empty name (0 or "") disables output to a file,
  i.e. calls setOutputToFile(false). Setting a non-empty name
  enables output to a file, i.e. calls setOutputToFile(true).

  \sa outputFileName(), setOutputToFile()
*/

void QPrinter::setOutputFileName(const QString &fileName)
{
    Q_D(QPrinter);
    ABORT_IF_ACTIVE("QPrinter::setOutputFileName()");
    d->printEngine->setProperty(QPrintEngine::PPK_OutputFileName, fileName);
}


/*!
  Returns the name of the program that sends the print output to the
  printer.

  The default is to return a null string; meaning that QPrinter will
  try to be smart in a system-dependent way. On X11 only, you can
  set it to something different to use a specific print program. On
  Windows, this function returns the name of the printer device
  driver.

  \sa setPrintProgram() setPrinterSelectionOption()
*/
QString QPrinter::printProgram() const
{
    Q_D(const QPrinter);
    return d->printEngine->property(QPrintEngine::PPK_PrinterProgram).toString();
}


/*!
  Sets the name of the program that should do the print job to \a
  printProg.

  On X11, this function sets the program to call with the PostScript
  output. On other platforms, it has no effect.

  \sa printProgram()
*/
void QPrinter::setPrintProgram(const QString &printProg)
{
    Q_D(QPrinter);
    ABORT_IF_ACTIVE("QPrinter::setPrintProgram()");
    d->printEngine->setProperty(QPrintEngine::PPK_PrinterProgram, printProg);
}


/*!
  Returns the document name.

  \sa setDocName()
*/
QString QPrinter::docName() const
{
    Q_D(const QPrinter);
    return d->printEngine->property(QPrintEngine::PPK_DocumentName).toString();
}


/*!
  Sets the document name to \a name.
*/
void QPrinter::setDocName(const QString &name)
{
    Q_D(QPrinter);
    ABORT_IF_ACTIVE("QPrinter::setDocName()");
    d->printEngine->setProperty(QPrintEngine::PPK_DocumentName, name);
}


/*!
  Returns the name of the application that created the document.

  \sa setCreator()
*/
QString QPrinter::creator() const
{
    Q_D(const QPrinter);
    return d->printEngine->property(QPrintEngine::PPK_Creator).toString();
}


/*!
  Sets the name of the application that created the document to \a
  creator.

  This function is only applicable to the X11 version of Qt. If no
  creator name is specified, the creator will be set to "Qt"
  followed by some version number.

  \sa creator()
*/
void QPrinter::setCreator(const QString &creator)
{
    Q_D(QPrinter);
    ABORT_IF_ACTIVE("QPrinter::setCreator");
    d->printEngine->setProperty(QPrintEngine::PPK_Creator, creator);
}


/*!
  Returns the orientation setting. The default value is \c
  QPrinter::Portrait.

  \sa setOrientation()
*/
QPrinter::Orientation QPrinter::orientation() const
{
    Q_D(const QPrinter);
    return QPrinter::Orientation(d->printEngine->property(QPrintEngine::PPK_Orientation).toInt());
}


/*!
  Sets the print orientation to \a orientation.

  The orientation can be either \c QPrinter::Portrait or \c
  QPrinter::Landscape.

  The printer driver reads this setting and prints using the
  specified orientation. On Windows this setting won't take effect
  until the printer dialog is shown (using QPrintDialog).

  Windows only: This option can be changed while printing and will
  take effect from the next call to newPage().

  \sa orientation()
*/

void QPrinter::setOrientation(Orientation orientation)
{
    Q_D(QPrinter);
    d->printEngine->setProperty(QPrintEngine::PPK_Orientation, orientation);
}


/*!
  Returns the printer page size. The default value is system-dependent.

  \sa setPageSize() pageRect() paperRect()
*/
QPrinter::PageSize QPrinter::pageSize() const
{
    Q_D(const QPrinter);
    return QPrinter::PageSize(d->printEngine->property(QPrintEngine::PPK_PageSize).toInt());
}


/*!
  Sets the printer page size to \a newPageSize if that size is
  supported. The result if undefined if \a newPageSize is not
  supported.

  The default page size is system-dependent.

  This function is useful mostly for setting a default value that
  the user can override in the print dialog when you call setup().

  \sa pageSize() PageSize setFullPage() setResolution() pageRect() paperRect()
*/

void QPrinter::setPageSize(PageSize newPageSize)
{
    Q_D(QPrinter);
    ABORT_IF_ACTIVE("QPrinter::setPageSize()");
    if (newPageSize > NPageSize) {
        qWarning("QPrinter::SetPageSize: illegal page size %d", newPageSize);
        return;
    }
    d->printEngine->setProperty(QPrintEngine::PPK_PageSize, newPageSize);
}

/*!
    Sets the page order to \a pageOrder.

    The page order can be \c QPrinter::FirstPageFirst or \c
    QPrinter::LastPageFirst. The application is responsible for
    reading the page order and printing accordingly.

    This function is mostly useful for setting a default value that
    the user can override in the print dialog when you call setup().
*/

void QPrinter::setPageOrder(PageOrder pageOrder)
{
    Q_D(QPrinter);
    ABORT_IF_ACTIVE("QPrinter::setPageOrder()");
    d->printEngine->setProperty(QPrintEngine::PPK_PageOrder, pageOrder);
}


/*!
  Returns the current page order.

  The default page order is \c FirstPageFirst.
*/

QPrinter::PageOrder QPrinter::pageOrder() const
{
    Q_D(const QPrinter);
    return QPrinter::PageOrder(d->printEngine->property(QPrintEngine::PPK_PageOrder).toInt());
}


/*!
  Sets the printer's color mode to \a newColorMode, which can be
  either \c Color or \c GrayScale (the default).

  \sa colorMode()
*/

void QPrinter::setColorMode(ColorMode newColorMode)
{
    Q_D(QPrinter);
    ABORT_IF_ACTIVE("QPrinter::setColorMode()");
    d->printEngine->setProperty(QPrintEngine::PPK_ColorMode, newColorMode);
}


/*!
  Returns the current color mode. The default color mode is \c
  GrayScale.

  \sa setColorMode()
*/
QPrinter::ColorMode QPrinter::colorMode() const
{
    Q_D(const QPrinter);
    return QPrinter::ColorMode(d->printEngine->property(QPrintEngine::PPK_ColorMode).toInt());
}


/*!
  Returns the number of copies to be printed. The default value is 1.

  After a call to setup(), this value will return the number of
  times the application is required to print in order to match the
  number specified in the printer setup dialog. This has been done since
  some printer drivers are not capable of buffering up the copies and
  in those cases the application must make an explicit call to the
  print code for each copy.

  \sa setNumCopies()
*/

int QPrinter::numCopies() const
{
    Q_D(const QPrinter);
   return d->printEngine->property(QPrintEngine::PPK_NumberOfCopies).toInt();
}


/*!
  Sets the number of copies to be printed to \a numCopies.

  The printer driver reads this setting and prints the specified
  number of copies.

  \sa numCopies()
*/

void QPrinter::setNumCopies(int numCopies)
{
    Q_D(QPrinter);
    ABORT_IF_ACTIVE("QPrinter::setNumCopies()");
    d->printEngine->setProperty(QPrintEngine::PPK_NumberOfCopies, numCopies);
}


/*!
  \internal

  Returns true if collation is turned on when multiple copies is selected.
  Returns false if it is turned off when multiple copies is selected.

  \sa collateCopiesEnabled() setCollateCopiesEnabled() setCollateCopies()
*/
bool QPrinter::collateCopies() const
{
    Q_D(const QPrinter);
    return d->printEngine->property(QPrintEngine::PPK_CollateCopies).toBool();
}


/*!
  \internal

  Sets the default value for collation checkbox when the print dialog appears.
  If \a on is true, it will enable setCollateCopiesEnabled().
  The default value is false. This value will be changed by what the
  user presses in the print dialog.

  \sa collateCopiesEnabled() setCollateCopiesEnabled() collateCopies()
*/
void QPrinter::setCollateCopies(bool collate)
{
    Q_D(QPrinter);
    ABORT_IF_ACTIVE("QPrinter::setCollateCopies()");
    d->printEngine->setProperty(QPrintEngine::PPK_CollateCopies, collate);
}



/*!
  Sets QPrinter to have the origin of the coordinate system at the
  top-left corner of the paper if \a fp is true, or where it thinks
  the top-left corner of the printable area is if \a fp is false.

  The default is false. You can (probably) print on (0,0), and
  the device metrics will report something smaller than the size
  indicated by PageSize. (Note that QPrinter may be wrong on Unix
  systems: it does not have perfect knowledge of the physical
  printer.)

  If \a fp is true, the device metrics will report the exact same
  size as indicated by \c PageSize. It probably isn't possible to
  print on the entire page because of the printer's physical
  margins, so the application must account for the margins itself.

  \sa PageSize setPageSize() fullPage() width() height()
*/

void QPrinter::setFullPage(bool fp)
{
    Q_D(QPrinter);
    d->printEngine->setProperty(QPrintEngine::PPK_FullPage, fp);
}


/*!
  Returns true if the origin of the printer's coordinate system is
  at the corner of the page and false if it is at the edge of the
  printable area.

  See setFullPage() for details and caveats.

  \sa setFullPage() PageSize
*/

bool QPrinter::fullPage() const
{
    Q_D(const QPrinter);
    return d->printEngine->property(QPrintEngine::PPK_FullPage).toBool();
}


/*!
  Requests that the printer prints at \a dpi or as near to \a dpi as
  possible.

  This setting affects the coordinate system as returned by, for
  example QPainter::viewport().

  The value depends on the \c PrintingMode used in the QPrinter
  constructor. By default, the the screen's dpi value is used.

  This function must be called before setup() to have an effect on
  all platforms.

  \sa resolution() setPageSize()
*/

void QPrinter::setResolution(int dpi)
{
    Q_D(QPrinter);
    ABORT_IF_ACTIVE("QPrinter::setResolution()");
    d->printEngine->setProperty(QPrintEngine::PPK_Resolution, dpi);
}


/*!
  Returns the current assumed resolution of the printer, as set by
  setResolution() or by the printer subsystem.

  \sa setResolution()
*/

int QPrinter::resolution() const
{
    Q_D(const QPrinter);
    return d->printEngine->property(QPrintEngine::PPK_Resolution).toInt();
}

/*!
  Sets the paper source setting to \a source.

  Windows only: This option can be changed while printing and will
  take effect from the next call to newPage()

  \sa paperSource()
*/

void QPrinter::setPaperSource(PaperSource source)
{
    Q_D(QPrinter);
    d->printEngine->setProperty(QPrintEngine::PPK_PaperSource, source);
}

/*!
    Returns the printer's paper source. This is \c Manual or a printer
    tray or paper cassette.
*/
QPrinter::PaperSource QPrinter::paperSource() const
{
    Q_D(const QPrinter);
    return QPrinter::PaperSource(d->printEngine->property(QPrintEngine::PPK_PaperSource).toInt());
}

/*!
    Returns the page's rectangle; this is usually smaller than the
    paperRect() since the page normally has margins between its
    borders and the paper.

    \sa pageSize()
*/
QRect QPrinter::pageRect() const
{
    Q_D(const QPrinter);
    return d->printEngine->property(QPrintEngine::PPK_PageRect).toRect();
}

/*!
    Returns the paper's rectangle; this is usually larger than the
    pageRect().

   \sa pageRect()
*/
QRect QPrinter::paperRect() const
{
    Q_D(const QPrinter);
    return d->printEngine->property(QPrintEngine::PPK_PaperRect).toRect();
}

/*!
    \internal

    Returns the metric for the given \a id.
*/
int QPrinter::metric(PaintDeviceMetric id) const
{
    Q_D(const QPrinter);
    return d->printEngine->metric(id);
}

/*!
    Returns the paint engine used by the printer.
*/
QPaintEngine *QPrinter::paintEngine() const

{
    Q_D(const QPrinter);
// Being a bit safe, since we have multiple inheritance...
#if defined (Q_WS_WIN)
    return static_cast<QWin32PrintEngine*>(d->printEngine);
#elif defined (Q_WS_MAC)
    return static_cast<QMacPrintEngine *>(d->printEngine);
#elif defined (Q_OS_UNIX)
    return static_cast<QPSPrintEngine *>(d->printEngine);
#else
    return 0;
#endif
}


#if defined (Q_WS_WIN)
/*!
    Sets the page size to be used by the printer under Windows to \a
    pageSize.

    \warning This function is not portable so you may prefer to use
    setPageSize() instead.

    \sa winPageSize()
*/
void QPrinter::setWinPageSize(int pageSize)
{
    Q_D(QPrinter);
    ABORT_IF_ACTIVE("QPrinter::setWinPageSize()");
    d->printEngine->setProperty(QPrintEngine::PPK_WindowsPageSize, pageSize);
}

/*!
    Returns the page size used by the printer under Windows.

    \warning This function is not portable so you may prefer to use
    pageSize() instead.

    \sa setWinPageSize()
*/
int QPrinter::winPageSize() const
{
    Q_D(const QPrinter);
    return d->printEngine->property(QPrintEngine::PPK_WindowsPageSize).toInt();
}
#endif // Q_WS_WIN

/*!
    Returns a list of the resolutions (a list of dots-per-inch
    integers) that the printer says it supports.

    For X11 where all printing is directly to postscript, this
    function will always return a one item list containing only the
    postscript resolution, i.e., 72 (72 dpi -- but see \c PrinterMode).
*/
QList<int> QPrinter::supportedResolutions() const
{
    Q_D(const QPrinter);
    QList<QVariant> varlist
        = d->printEngine->property(QPrintEngine::PPK_SupportedResolutions).toList();
    QList<int> intlist;
    for (int i=0; i<varlist.size(); ++i)
        intlist << varlist.at(i).toInt();
    return intlist;
}

/*!
    Tells the printer to eject the current page and to continue
    printing on a new page. Returns true if this was successful;
    otherwise returns false.
*/
bool QPrinter::newPage()
{
    Q_D(QPrinter);
    return d->printEngine->newPage();
}

/*!
    Aborts the current print run. Returns true if the print run was
    successfully aborted; otherwise returns false.

    It is not always possible to abort a print job. For example, if
    all the data has gone to the printer but the printer cannot or
    will not cancel the job when asked to.
*/
bool QPrinter::abort()
{
    Q_D(QPrinter);
    return d->printEngine->abort();
}

#endif // QT_NO_PRINTER

#if 0
/*!
  \fn int QPrinter::minPage() const

  Returns the min-page setting, i.e. the lowest page number a user
  is allowed to choose. The default value is 0 which signifies 'any
  page'.

  \sa maxPage(), setMinMax() setFromTo()
*/

/*!
  \fn int QPrinter::maxPage() const

  Returns the max-page setting. A user can't choose a higher page
  number than maxPage() when they select a print range. The default
  value is 0 which signifies the last page (up to a maximum of
  9999).

  \sa minPage(), setMinMax() setFromTo()
*/

/*!
  Sets the min-page and max-page settings to \a minPage and \a
  maxPage respectively.

  The min-page and max-page restrict the from-page and to-page
  settings. When the printer setup dialog appears, the user cannot
  select a from page or a to page that are outside the range
  specified by min and max pages.

  \sa minPage(), maxPage(), setFromTo(), setup()
*/

void QPrinter::setMinMax(int minPage, int maxPage)
{
    min_pg = minPage;
    max_pg = maxPage;
    if (from_pg == 0 || from_pg < minPage)
	from_pg = minPage;
    if (to_pg == 0 || to_pg > maxPage)
	to_pg = maxPage;
}

/*!
  \fn bool QPrinter::collateCopiesEnabled() const

  \internal

  Returns true if the application should provide the user with the
  option of choosing a collated printout; otherwise returns false.

  Collation means that each page is printed in order, i.e. print the
  first page, then the second page, then the third page and so on, and
  then repeat this sequence for as many copies as have been requested.
  If you don't collate you get several copies of the first page, then
  several copies of the second page, then several copies of the third
  page, and so on.

  \sa setCollateCopiesEnabled() setCollateCopies() collateCopies()
*/

/*!
  \fn void QPrinter::setCollateCopiesEnabled(bool enable)

  \internal

  If \a enable is true (the default) the user is given the choice of
  whether to print out multiple copies collated in the print dialog.
  If \a enable is false, then collateCopies() will be ignored.

  Collation means that each page is printed in order, i.e. print the
  first page, then the second page, then the third page and so on, and
  then repeat this sequence for as many copies as have been requested.
  If you don't collate you get several copies of the first page, then
  several copies of the second page, then several copies of the third
  page, and so on.

  \sa collateCopiesEnabled() setCollateCopies() collateCopies()
*/


#endif

/*!
    Returns the current state of the printer. This may not always be
    accurate (for example if the printer doesn't have the capability
    of reporting its state to the operating system).
*/
QPrinter::PrinterState QPrinter::printerState() const
{
    Q_D(const QPrinter);
    return d->printEngine->printerState();
}


/*! \fn void QPrinter::margins(uint *top, uint *left, uint *bottom, uint *right) const

    Sets *\a top, *\a left, *\a bottom, *\a right to be the top,
    left, bottom, and right margins.

    This function has been superceded by paperRect() and pageRect().
    Use pageRect().top() - paperRect().top() for the top margin,
    pageRect().left() - paperRect().left() for the left margin,
    pageRect().bottom() - paperRect().bottom() for the bottom margin,
    and pageRect().right() - paperRect().right() for the right
    margin.

    \oldcode
        uint rightMargin;
        uint bottomMargin;
        printer->margins(0, 0, &bottomMargin, &rightMargin);
    \newcode
        int rightMargin = printer->pageRect().right() - printer->paperRect().right();
        int bottomMargin = printer->pageRect().bottom() - printer->paperRect().bottom();
    \endcode
*/

/*! \fn QSize QPrinter::margins() const

    \overload

    Returns a QSize containing the left margin and the top margin.

    This function has been superceded by paperRect() and pageRect().
    Use pageRect().left() - paperRect().left() for the left margin,
    and pageRect().top() - paperRect().top() for the top margin.

    \oldcode
        QSize margins = printer->margins();
        int leftMargin = margins.width();
        int topMargin = margins.height();
    \newcode
        int leftMargin = printer->pageRect().left() - printer->paperRect().left();
        int topMargin = printer->pageRect().top() - printer->paperRect().top();
    \endcode
*/

/*! \fn bool QPrinter::aborted()

    Use printerState() == QPrinter::Aborted instead.
*/

#ifdef Q_WS_WIN
/*!
    \internal
*/
HDC QPrinter::getDC() const
{
    Q_D(const QPrinter);
    return d->printEngine->getPrinterDC();
}

/*!
    \internal
*/
void QPrinter::releaseDC(HDC hdc) const
{
    Q_D(const QPrinter);
    d->printEngine->releasePrinterDC(hdc);
}
#endif


#ifndef Q_WS_WIN
/*!
    Returns the printer options selection string. This is useful only
    if the print command has been explicitly set.

    The default value (a null string) implies that the printer should
    be selected in a system-dependent manner.

    Any other value implies that the given value should be used.

    \sa setPrinterSelectionOption()
*/

QString QPrinter::printerSelectionOption() const
{
    Q_D(const QPrinter);
    return d->printEngine->property(QPrintEngine::PPK_SelectionOption).toString();
}

/*!
    Sets the printer to use \a option to select the printer. \a option
    is null by default (which implies that Qt should be smart enough
    to guess correctly), but it can be set to other values to use a
    specific printer selection option.

    If the printer selection option is changed while the printer is
    active, the current print job may or may not be affected.

    \sa printerSelectionOption()
*/

void QPrinter::setPrinterSelectionOption(const QString &option)
{
    Q_D(QPrinter);
    d->printEngine->setProperty(QPrintEngine::PPK_SelectionOption, option);
}

#endif


#ifdef QT3_SUPPORT

void QPrinter::setOutputToFile(bool f)
{
    if (f) {
        if (outputFileName().isEmpty())
            setOutputFileName("untitled_printer_document");
    } else {
        setOutputFileName(QString());
    }
}

bool qt_compat_QPrinter_printSetup(QPrinter *p, QPrinterPrivate *pd, QWidget *parent)
{
    if (!pd->printDialog)
        pd->printDialog = new QPrintDialog(p, parent);
    else if (parent)
        pd->printDialog->setParent(parent);

    return pd->printDialog->exec() != 0;
}


#ifdef Q_WS_MAC
bool qt_compat_QPrinter_pageSetup(QPrinter *p, QWidget *parent)
{
    QPageSetupDialog psd(p, parent);
    return psd.exec() != 0;
}

/*!
    Executes a page setup dialog so that the user can configure the type of
    page used for printing. Returns true if the contents of the dialog are
    accepted; returns false if the dialog is canceled.
*/
bool QPrinter::pageSetup(QWidget *parent)
{
    return qt_compat_QPrinter_pageSetup(this, parent);
}

/*!
    Executes a print setup dialog so that the user can configure the printing
    process. Returns true if the contents of the dialog are accepted; returns
    false if the dialog is canceled.
*/
bool QPrinter::printSetup(QWidget *parent)
{
    Q_D(QPrinter);
    return qt_compat_QPrinter_printSetup(this, d, parent);
}
#endif // Q_WS_MAC

/*!
    \compat

    Use QPrintDialog instead.

    \oldcode
        if (printer->setup(parent))
            ...
    \newcode
        QPrintDialog dialog(printer, parent);
        if (dialog.exec())
            ...
    \endcode
*/
bool QPrinter::setup(QWidget *parent)
{
    Q_D(QPrinter);
    return qt_compat_QPrinter_printSetup(this, d, parent)
#ifdef Q_WS_MAC
        && qt_compat_QPrinter_pageSetup(this, parent);
#endif
        ;
}

/*!
    \fn int QPrinter::fromPage() const

    \compat

    Use QPrintDialog instead.

    Returns the from-page setting. The default value is 0.

    If fromPage() and toPage() both return 0 this signifies 'print the
    whole document'.

    The programmer is responsible for reading this setting and
    printing accordingly.


    \sa setFromTo(), toPage()
*/

int QPrinter::fromPage() const
{
    Q_D(const QPrinter);
    if (!d->printDialog)
        const_cast<QPrinter*>(this)->d_func()->printDialog = new QPrintDialog(const_cast<QPrinter*>(this));
    return const_cast<QPrinter*>(this)->d_func()->printDialog->fromPage();
}

/*!
    \fn int QPrinter::toPage() const

    \compat

    Use QPrintDialog instead.

    Returns the to-page setting. The default value is 0.

    If fromPage() and toPage() both return 0 this signifies 'print the
    whole document'.

    The programmer is responsible for reading this setting and
    printing accordingly.

    \sa setFromTo(), fromPage()
*/

int QPrinter::toPage() const
{
    Q_D(const QPrinter);
    if (!d->printDialog)
        const_cast<QPrinter*>(this)->d_func()->printDialog = new QPrintDialog(const_cast<QPrinter*>(this));
    return const_cast<QPrinter*>(this)->d_func()->printDialog->toPage();
}

/*!
    \compat

    Use QPrintDialog instead.

    Sets the from-page and to-page settings to \a from and \a
    to respectively.

    The from-page and to-page settings specify what pages to print.

    If from and to both return 0 this signifies 'print the whole
    document'.

    This function is useful mostly to set a default value that the
    user can override in the print dialog when you call setup().

    Use QPrintDialog instead.

    \sa fromPage(), toPage(), setMinMax(), setup()
*/

void QPrinter::setFromTo(int from, int to)
{
    Q_D(QPrinter);
    if (!d->printDialog)
        const_cast<QPrinter*>(this)->d_func()->printDialog = new QPrintDialog(const_cast<QPrinter*>(this));
    d->printDialog->setFromTo(from, to);
}

/*!
    \fn int QPrinter::minPage() const

    \compat

    Use QPrintDialog instead.

    Returns the min-page setting, i.e. the lowest page number a user
    is allowed to choose. The default value is 0.

    \sa maxPage(), setMinMax() setFromTo()
*/
int QPrinter::minPage() const
{
    Q_D(const QPrinter);
    if (!d->printDialog)
        const_cast<QPrinter*>(this)->d_func()->printDialog = new QPrintDialog(const_cast<QPrinter*>(this));
    return const_cast<QPrinter*>(this)->d_func()->printDialog->minPage();
}

/*!
    \fn int QPrinter::maxPage() const

    \compat

    Use QPrintDialog instead.

    Returns the max-page setting. A user can't choose a higher page
    number than maxPage() when they select a print range. The default
    value is 0.

    \sa minPage(), setMinMax() setFromTo()
*/

int QPrinter::maxPage() const
{
    Q_D(const QPrinter);
    if (!d->printDialog)
        const_cast<QPrinter*>(this)->d_func()->printDialog = new QPrintDialog(const_cast<QPrinter*>(this));
    return const_cast<QPrinter *>(this)->d_func()->printDialog->maxPage();
}

/*!
    \compat

    Use QPrintDialog instead.

    Sets the min-page and max-page settings to \a minPage and \a
    maxPage respectively.

    The min-page and max-page restrict the from-page and to-page
    settings. When the printer setup dialog appears, the user cannot
    select a from page or a to page that are outside the range
    specified by min and max pages.

    \sa minPage(), maxPage(), setFromTo(), setup()
*/

void QPrinter::setMinMax( int minPage, int maxPage )
{
    Q_D(QPrinter);
    if (!d->printDialog)
        const_cast<QPrinter*>(this)->d_func()->printDialog = new QPrintDialog(const_cast<QPrinter*>(this));
    d->printDialog->setMinMax(minPage, maxPage);
}

/*!
  \fn bool QPrinter::collateCopiesEnabled() const

  \compat

  Use QPrintDialog instead

  \internal

  Returns true if the application should provide the user with the
  option of choosing a collated printout; otherwise returns false.

  Collation means that each page is printed in order, i.e. print the
  first page, then the second page, then the third page and so on, and
  then repeat this sequence for as many copies as have been requested.
  If you don't collate you get several copies of the first page, then
  several copies of the second page, then several copies of the third
  page, and so on.

  \sa setCollateCopiesEnabled() setCollateCopies() collateCopies()
*/

bool QPrinter::collateCopiesEnabled() const
{
    Q_D(const QPrinter);
    if (!d->printDialog)
        const_cast<QPrinter*>(this)->d_func()->printDialog = new QPrintDialog(const_cast<QPrinter*>(this));
    return const_cast<QPrinter*>(this)->d_func()->printDialog->isOptionEnabled(QPrintDialog::PrintCollateCopies);
}

/*!
    \fn void QPrinter::setCollateCopiesEnabled(bool enable)

    \compat

    Use QPrintDialog instead.

    \internal

    If \a enable is true (the default) the user is given the choice of
    whether to print out multiple copies collated in the print dialog.
    If \a enable is false, then collateCopies() will be ignored.

    Collation means that each page is printed in order, i.e. print the
    first page, then the second page, then the third page and so on, and
    then repeat this sequence for as many copies as have been requested.
    If you don't collate you get several copies of the first page, then
    several copies of the second page, then several copies of the third
    page, and so on.

  \sa collateCopiesEnabled() setCollateCopies() collateCopies()
*/

void QPrinter::setCollateCopiesEnabled(bool enable)
{
    Q_D(QPrinter);
    if (!d->printDialog)
        const_cast<QPrinter*>(this)->d_func()->printDialog = new QPrintDialog(const_cast<QPrinter*>(this));

    QPrintDialog::PrintDialogOptions opt = d->printDialog->enabledOptions();
    if (enable)
        opt |= QPrintDialog::PrintCollateCopies;
    else
        opt &= ~QPrintDialog::PrintCollateCopies;
    d->printDialog->setEnabledOptions(opt);
}

/*!
    \enum PrintRange
    \compat
    \value AllPages
    \value Selection
    \value PageRange
*/

/*!
    \compat

    Use QPrintDialog instead.

    Sets the default selected page range to be used when the print setup
    dialog is opened to \a range. If the PageRange specified by \a range is
    currently disabled the function does nothing.

    \sa printRange()
*/

void QPrinter::setPrintRange( PrintRange range )
{
    Q_D(QPrinter);
    if (!d->printDialog)
        const_cast<QPrinter*>(this)->d_func()->printDialog = new QPrintDialog(const_cast<QPrinter*>(this));
    d->printDialog->setPrintRange(QPrintDialog::PrintRange(range));
}

/*!
    \compat

    Use QPrintDialog instead.

    Returns the PageRange of the QPrinter. After the print setup dialog
    has been opened, this function returns the value selected by the user.

    \sa setPrintRange()
*/
QPrinter::PrintRange QPrinter::printRange() const
{
    Q_D(const QPrinter);
    if (!d->printDialog)
        const_cast<QPrinter*>(this)->d_func()->printDialog = new QPrintDialog(const_cast<QPrinter*>(this));
    return PrintRange(const_cast<QPrinter*>(this)->d_func()->printDialog->printRange());
}

/*!
    \enum PrinterOption
    \compat
    \value PrintToFile
    \value PrintSelection
    \value PrintPageRange
*/

/*!
    \compat

    Use QPrintDialog instead.

    Enables the printer option with the identifier \a option if \a
    enable is true, and disables option \a option if \a enable is false.

    \sa isOptionEnabled()
*/
void QPrinter::setOptionEnabled( PrinterOption option, bool enable )
{
    Q_D(QPrinter);
    if (!d->printDialog)
        const_cast<QPrinter*>(this)->d_func()->printDialog = new QPrintDialog(const_cast<QPrinter*>(this));
    QPrintDialog::PrintDialogOptions opt = d->printDialog->enabledOptions();
    if (enable)
        opt |= QPrintDialog::PrintDialogOption(1 << option);
    else
        opt &= ~QPrintDialog::PrintDialogOption(1 << option);
    d->printDialog->setEnabledOptions(opt);
}

/*!
    \compat

    Use QPrintDialog instead.

  Returns true if the printer option with identifier \a option is enabled;
  otherwise returns false.

  \sa setOptionEnabled()
 */
bool QPrinter::isOptionEnabled( PrinterOption option ) const
{
    Q_D(const QPrinter);
    if (!d->printDialog)
        const_cast<QPrinter*>(this)->d_func()->printDialog = new QPrintDialog(const_cast<QPrinter*>(this));
    return const_cast<QPrinter*>(this)->d_func()->printDialog->isOptionEnabled(QPrintDialog::PrintDialogOption(option));
}

#endif // QT3_SUPPORT

/*!
    \class QPrintEngine

    \brief The QPrintEngine class defines an interface for how QPrinter
    interacts with a given printing subsystem.

    The common use is to derive from both QPaintEngine and QPrintEngine
    when implementing a new printer.

    \sa QPaintEngine
*/

/*!
    \enum QPrintEngine::PrintEnginePropertyKey

    This enum is used to communicate properties between the print
    engine and QPrinter. A property may or may not be supported by a
    given print engine.

    \value PPK_CollateCopies A bool value describing wether the
    printout should be collated or not.

    \value PPK_ColorMode Refers to QPrinter::ColorMode, either color or
    monochrome.

    \value PPK_Creator

    \value PPK_DocumentName A string describing the document name in
    the spooler.

    \value PPK_FullPage A boolean describing if the printer should be
    full page or not.

    \value PPK_NumberOfCopies An integer specifying the number of
    copies

    \value PPK_Orientation Specifies a QPrinter::Orientation value.

    \value PPK_OutputFileName The output file name as a string. An
    empty file name indicates that we do not print to file.

    \value PPK_PageOrder Specifies a QPrinter::PageOrder value.

    \value PPK_PageRect A QRect specifying the page rectangle

    \value PPK_PageSize Specifies a QPrinter::PageSize value.

    \value PPK_PaperRect A QRect specifying the paper rectangle.

    \value PPK_PaperSource Specifies a QPrinter::PaperSource value.

    \value PPK_PrinterName A string specifying the name of the printer.

    \value PPK_PrinterProgram A string specifying the name of the
    printer program used for printing,

    \value PPK_Resolution An integer describing the dots per inch for
    this printer.

    \value PPK_SelectionOption

    \value PPK_SupportedResolutions A list of integer QVariants
    describing the set of supported resolutions that the printer has.

    \value PPK_WindowsPageSize An integer specifying a DM_PAPER entry
    on Windows(tm).

    \value PPK_CustomBase Basis for extension.
*/

/*!
    \fn QPrintEngine::~QPrintEngine()

    Destroys the print engine.
*/

/*!
    \fn void QPrintEngine::setProperty(PrintEnginePropertyKey key, const QVariant &value)

    Sets the print engine's property specified by \a key to the given \a value.

    \sa property()
*/

/*!
    \fn void QPrintEngine::property(PrintEnginePropertyKey key) const

    Returns the print engine's property specified by \a key.

    \sa property()
*/

/*!
    \fn bool QPrintEngine::newPage()

    Instructs the print engine to start a new page. Returns true if
    successful; otherwise returns false.
*/

/*!
    \fn bool QPrintEngine::abort()

    Instructs the print engine to abort the printing process. Returns
    true if successful; otherwise returns false.
*/

/*!
    \fn int QPrintEngine::metric(QPaintDevice::PaintDeviceMetric id) const

    Returns the metric for the given \a id.
*/

/*!
    \fn QPrinter::PrinterState QPrintEngine::printerState() const

    Returns the state of the printer used by the print engine.
*/

/*!
    \fn HDC QPrintEngine::getPrinterDC() const
    \internal
*/

/*!
    \fn void QPrintEngine::releasePrinterDC(HDC) const
    \internal
*/
