/****************************************************************************
** $Id: $
**
** Implementation of QPrinter class for Unix
**
** Created : 950810
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
** licenses for Unix/X11 may use this file in accordance with the Qt Commercial
** License Agreement provided with the Software.
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

#include "qplatformdefs.h"
#include "qprinter.h"

#ifndef QT_NO_PRINTER

#include "qpaintdevicemetrics.h"
#include "qpsprinter_p.h"
#include "qprintdialog.h"
#include "qapplication.h"

#include <stdlib.h>


// NOT REVISED

/*****************************************************************************
  QPrinter member functions
 *****************************************************************************/

// QPrinter states

#define PST_IDLE        0
#define PST_ACTIVE      1
#define PST_ERROR       2
#define PST_ABORTED     3

// Default values for QPrinter members

struct PrinterDefaults {
    QString printerName;
    bool outputToFile;
    QString outputFileName;
    QPrinter::Orientation orientation;
    QPrinter::PageSize pageSize;
    QPrinter::PageOrder pageOrder;
    QPrinter::ColorMode colorMode;
    int numCopies;
};

static PrinterDefaults * globalPrinterDefaults = 0;

/*!
  Constructs a printer paint device with mode \a m.

  \sa QPrinter::PrinterMode
*/

QPrinter::QPrinter( PrinterMode m )
    : QPaintDevice( QInternal::Printer | QInternal::ExternalDevice )
{
    pdrv = 0;
    pid = 0;
    orient = Portrait;
    page_size = A4;
    page_order = FirstPageFirst;
    color_mode = GrayScale;
    ncopies = 1;
    from_pg = to_pg = min_pg = max_pg = 0;
    state = PST_IDLE;
    output_file = FALSE;
    to_edge     = FALSE;
    switch ( m ) {
        case ScreenResolution:
#ifdef Q_WS_QWS
            res = 72;
#else
            res = QPaintDevice::x11AppDpiY();
#endif
            break;
        case Compatible:
        case PrinterResolution:
            res = 72;
            break;
        case HighResolution:
            res = 600;
    }
}

/*!
  Destroys the printer paint device and cleans up.
*/

QPrinter::~QPrinter()
{
    delete pdrv;
    if ( pid ) {
        (void)::kill( pid, 6 );
        (void)::wait( 0 );
        pid = 0;
    }
}


/*!
  Advances to a new page on the printer.
  Returns TRUE if successful, otherwise FALSE.
*/

bool QPrinter::newPage()
{
    if ( state == PST_ACTIVE && pdrv )
        return ((QPSPrinter*)pdrv)->cmd( QPSPrinter::NewPage, 0, 0 );
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
        ((QPSPrinter*)pdrv)->cmd( QPSPrinter::AbortPrinting, 0, 0 );
        state = PST_ABORTED;
        if ( pid ) {
            (void)::kill( pid, 6 );
            (void)::wait( 0 );
            pid = 0;
        }
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
  Sets the printer name to \a name.

  The default printer will be used if no printer name is set.

  Under X11, the PRINTER environment variable defines the
  default printer.  Under any other window system, the window
  system defines the default printer.

  \sa printerName()
*/

void QPrinter::setPrinterName( const QString &name )
{
    if ( state != 0 ) {
#if defined(QT_CHECK_STATE)
        qWarning( "QPrinter::setPrinterName: Cannot do this during printing" );
#endif
        return;
    }
    printer_name = name;
}

static void deleteGlobalPrinterDefaults()
{
    delete globalPrinterDefaults;
    globalPrinterDefaults = 0;
}

/*!
  Opens a printer setup dialog, with parent \a parent, and asks the
  user to specify what printer to use and miscellaneous printer
  settings.

  Returns TRUE if the user pressed "OK" to print, or FALSE if the
  user cancelled the operation.
*/

bool QPrinter::setup( QWidget * parent )
{
#ifndef QT_NO_PRINTDIALOG
    bool result = QPrintDialog::getPrinterSetup( this, parent  );
#else
    bool result = FALSE;
#endif
    if ( result ) {
	if ( !globalPrinterDefaults ) {
	    globalPrinterDefaults = new PrinterDefaults;
	    qAddPostRoutine( deleteGlobalPrinterDefaults );
	}
	globalPrinterDefaults->printerName = printerName();
	globalPrinterDefaults->outputToFile = outputToFile();
	globalPrinterDefaults->outputFileName = outputFileName();
	globalPrinterDefaults->orientation = orientation();
	globalPrinterDefaults->pageSize = pageSize();
	globalPrinterDefaults->pageOrder = pageOrder();
	globalPrinterDefaults->colorMode = colorMode();
    }
    return result;
}

static void closeAllOpenFds()
{
    // hack time... getting the maximum number of open
    // files, if possible.  if not we assume it's the
    // larger of 256 and the fd we got
    int i;
#if defined(Q_OS_OS2EMX)
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
#endif // Q_OS_OS2EMX           // ways-to-set i
    while( --i > 0 )
        ::close( i );
}

/*!
  \internal
  Handles painter commands to the printer.
*/

bool QPrinter::cmd( int c, QPainter *paint, QPDevCmdParam *p )
{
    if ( c ==  PdcBegin ) {
        if ( state == PST_IDLE ) {
            if ( output_file ) {
                int fd = 0;
                fd = ::open( output_filename.local8Bit(),
                             O_CREAT | O_NOCTTY | O_TRUNC | O_WRONLY,
                             0666 );
                if ( fd >= 0 ) {
                    pdrv = new QPSPrinter( this, fd );
                    state = PST_ACTIVE;
                }
            } else {
                QString pr;
                if ( printer_name )
                    pr = printer_name;
                QApplication::flushX();
                int fds[2];
                if ( pipe( fds ) != 0 ) {
                    qWarning( "QPSPrinter: could not open pipe to print" );
                    state = PST_ERROR;
                    return FALSE;
                }

// ### shouldn't we use QProcess here????
#if 0 && defined(Q_OS_OS2EMX)
                // this code is still not used, and maybe it's not
                // usable either, any more.  if you want to use it,
                // you may need to fix it first.

                // old comment:

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
                pr.prepend( option_string ? option_string : "-P" ); // ###
                if ( spawnlp(P_NOWAIT,print_prog.data(), print_prog.data(),
                             pr.data(), output->name(), 0) == -1 ) {
                    ;                   // couldn't exec, ignored
                }
                dup2( tmp, 0 );
                ::close( tmp );
                pdrv = new QPSPrinter( this, fds[1] );
                state = PST_ACTIVE;
#else
                pid = fork();
                if ( pid == 0 ) {       // child process
                    // if possible, exit quickly, so the actual lp/lpr
                    // becomes a child of init, and ::waitpid() is
                    // guaranteed not to wait.
                    if ( fork() > 0 ) {
                        closeAllOpenFds();

			// try to replace this process with "true" - this prevents
			// global destructors from being called (that could possibly
			// do wrong things to the parent process)
			(void)execlp("true", "true", 0);
			(void)execl("/bin/true", "true", 0);
			(void)execl("/usr/bin/true", "true", 0);
                        ::exit( 0 );
                    }
                    dup2( fds[0], 0 );

                    closeAllOpenFds();

                    if ( print_prog ) {
                        pr.prepend( option_string ? option_string :
                                    QString::fromLatin1( "-P" ) );
                        (void)execlp( print_prog.ascii(), print_prog.ascii(),
                                      pr.ascii(), 0 );
                    } else {
                        // if no print program has been specified, be smart
                        // about the option string too.
                        const char * lprarg = 0;
                        QString lprhack;
                        const char * lparg = 0;
                        QString lphack;
                        if ( pr || option_string ) {
                            lprhack = pr;
                            lprhack.prepend( option_string ? option_string :
                                             QString::fromLatin1( "-P" ) );
                            lprarg = lprhack.ascii();
                            lphack = pr;
                            lphack.prepend( option_string ? option_string :
                                            QString::fromLatin1( "-d" ) );
                            lparg = lphack.ascii();
                        }
                        (void)execlp( "lp", "lp", lparg, 0 );
                        (void)execlp( "lpr", "lpr", lprarg, 0 );
                        (void)execl( "/bin/lp", "lp", lparg, 0 );
                        (void)execl( "/bin/lpr", "lpr", lprarg, 0 );
                        (void)execl( "/usr/bin/lp", "lp", lparg, 0 );
                        (void)execl( "/usr/bin/lpr", "lpr", lprarg, 0 );
                    }
                    // if we couldn't exec anything, close the fd,
                    // wait for a second so the parent process (the
                    // child of the GUI process) has exited.  then
                    // exit.
                    ::close( 0 );
                    (void)::sleep( 1 );
                    ::exit( 0 );
                } else {                // parent process
                    ::close( fds[0] );
                    pdrv = new QPSPrinter( this, fds[1] );
                    state = PST_ACTIVE;
                }
#endif // else part of Q_OS_OS2EMX
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
            if ( c == PdcEnd ) {
                state = PST_IDLE;
                delete pdrv;
                pdrv = 0;
                if ( pid ) {
                    (void)::waitpid( pid, 0, 0 );
                    pid = 0;
                }
            }
        }
        return r;
    }
    return TRUE;
}


#define MM(n) int((n * 720 + 127) / 254)
#define IN(n) int(n * 72)

struct PaperSize {
    int width, height;
};

static PaperSize paperSizes[QPrinter::NPageSize] =
{
    {  MM(210), MM(297) },      // A4
    {  MM(176), MM(250) },      // B5
    {  IN(8.5), IN(11) },       // Letter
    {  IN(8.5), IN(14) },       // Legal
    {  IN(7.5), IN(10) },       // Executive
    {  MM(841), MM(1189) },     // A0
    {  MM(594), MM(841) },      // A1
    {  MM(420), MM(594) },      // A2
    {  MM(297), MM(420) },      // A3
    {  MM(148), MM(210) },      // A5
    {  MM(105), MM(148) },      // A6
    {  MM(74), MM(105)},        // A7
    {  MM(52), MM(74) },        // A8
    {  MM(37), MM(52) },        // A9
    {  MM(1000), MM(1414) },    // B0
    {  MM(707), MM(1000) },     // B1
    {  MM(31), MM(44) },        // B10
    {  MM(500), MM(707) },      // B2
    {  MM(353), MM(500) },      // B3
    {  MM(250), MM(353) },      // B4
    {  MM(125), MM(176) },      // B6
    {  MM(88), MM(125) },       // B7
    {  MM(62), MM(88) },        // B8
    {  MM(44), MM(62) },        // B9
    {  MM(163), MM(229) },      // C5E
    {  MM(105), MM(241) },      // Comm10E
    {  MM(110), MM(220) },      // DLE
    {  MM(210), MM(330) },      // Folio
    {  MM(432), MM(279) },      // Ledger
    {  MM(279), MM(432) },      // Tabloid
};

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
#if defined(QT_CHECK_RANGE)
    Q_ASSERT( (uint)s < (uint)NPageSize );
#endif
    switch ( m ) {
    case QPaintDeviceMetrics::PdmWidth:
        val = orient == Portrait ? paperSizes[s].width : paperSizes[s].height;
        if ( res != 72 )
            val = (val * res + 36) / 72;
        if ( !fullPage() )
            val -= 2*margins().width();
        break;
    case QPaintDeviceMetrics::PdmHeight:
        val = orient == Portrait ? paperSizes[s].height : paperSizes[s].width;
        if ( res != 72 )
            val = (val * res + 36) / 72;
        if ( !fullPage() )
            val -= 2*margins().height();
        break;
    case QPaintDeviceMetrics::PdmDpiX:
        val = res;
        break;
    case QPaintDeviceMetrics::PdmDpiY:
        val = res;
        break;
        case QPaintDeviceMetrics::PdmPhysicalDpiX:
        case QPaintDeviceMetrics::PdmPhysicalDpiY:
            val = 72;
            break;
    case QPaintDeviceMetrics::PdmWidthMM:
        // double rounding error here.  hooray.
        val = metric( QPaintDeviceMetrics::PdmWidth );
        val = (val * 254 + 5*res) / (10*res);
        break;
    case QPaintDeviceMetrics::PdmHeightMM:
        val = metric( QPaintDeviceMetrics::PdmHeight );
        val = (val * 254 + 5*res) / (10*res);
        break;
    case QPaintDeviceMetrics::PdmNumColors:
        val = 16777216;
        break;
    case QPaintDeviceMetrics::PdmDepth:
        val = 24;
        break;
    default:
        val = 0;
#if defined(QT_CHECK_RANGE)
        qWarning( "QPixmap::metric: Invalid metric command" );
#endif
    }
    return val;
}


/*!  Returns the width of the left/right and top/bottom margins of the
printer.  This is a best-effort guess, not based on perfect knowledge.

If you have called setFullPage( TRUE ) (this is recommended for
high-quality printing), margins().width() may be treated as the
smallest sane left/right margin you can use, and margins().height() as
the smallest sane top/bottom margins you can use.

If you have called setFullPage( FALSE ) (this is the default),
margins() is automatically subtracted from the pageSize() by QPrinter.

\sa setFullPage() QPaintDeviceMetrics PageSize
*/

QSize QPrinter::margins() const
{
    if (orient == Portrait)
        return QSize( res/2, res/3 );

    return QSize( res/3, res/2 );
}

#endif
