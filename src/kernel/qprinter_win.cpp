/****************************************************************************
** $Id: $
**
** Implementation of QPrinter class for Win32
**
** Created : 950810
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Windows may use this file in accordance with the Qt Commercial
** License Agreement provided with the Software.
**
** This file is not available for use under any other license without
** express written permission from the copyright holder.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qprinter.h"

#ifndef QT_NO_PRINTER

#include "qpainter.h"
#include "qpixmap.h"
#include "qimage.h"
#include "qwidget.h"
#include "qapplication.h"
#include "qt_windows.h"
#include "qpaintdevicemetrics.h"
#include "qapplication_p.h"

#if defined(__MINGW32__)
#include <malloc.h>
#define DOCINFOA DOCINFO
#endif

#ifdef Q_OS_TEMP
#include <commdlg.h>
#endif

// QPrinter states

#define PST_IDLE        0
#define PST_ACTIVE      1
#define PST_ABORTED     2

#ifdef IN
#undef IN
#endif
#ifdef MM
#undef MM
#endif

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

// ### deal with ColorMode GrayScale in qprinter_win.cpp.

QPrinter::QPrinter( PrinterMode m )
    : QPaintDevice( QInternal::Printer | QInternal::ExternalDevice )
{
    orient      = Portrait;
    page_size   = A4;
    page_order = FirstPageFirst;
    color_mode = GrayScale;
    ncopies     = 1;
    appcolcopies  = FALSE;
    usercolcopies = TRUE;
    from_pg     = to_pg = min_pg  = max_pg = 0;
    state       = PST_IDLE;
    output_file = FALSE;
    to_edge     = FALSE;
    viewOffsetDone = FALSE;
    painter     = 0;
    doc_name = "document1";
    hdevmode  = 0;
    hdevnames = 0;

#if defined(UNICODE)
#ifndef Q_OS_TEMP
    if ( qWinVersion() & Qt::WV_NT_based ) {
#endif
        PRINTDLG pd;
        memset( &pd, 0, sizeof(PRINTDLG) );
        pd.lStructSize = sizeof(PRINTDLG);
        pd.Flags = PD_RETURNDEFAULT | PD_RETURNDC;
        if ( PrintDlg( &pd ) != 0 )
            readPdlg( &pd );
#ifndef Q_OS_TEMP
    } else
#endif
#endif
#ifndef Q_OS_TEMP
    {
        PRINTDLGA pd;
        memset( &pd, 0, sizeof(PRINTDLGA) );
        pd.lStructSize = sizeof(PRINTDLGA);
        pd.Flags = PD_RETURNDEFAULT | PD_RETURNDC;
        if ( PrintDlgA( &pd ) != 0 )
            readPdlgA( &pd );
    }
#endif
    switch ( m ) {
	case ScreenResolution: 
	{
	    HDC dc = GetDC( 0 );
	    res = GetDeviceCaps( dc, LOGPIXELSY );
	    break;
	}
        case Compatible:
            devFlags |= QInternal::CompatibilityMode;
        case PrinterResolution:
        case HighResolution:
            res = metric( QPaintDeviceMetrics::PdmPhysicalDpiY );
    }
}

QPrinter::~QPrinter()
{
    if ( hdevmode ) {
        GlobalFree( hdevmode );
        hdevmode = 0;
    }
    if ( hdevnames ) {
        GlobalFree( hdevnames );
        hdevnames = 0;
    }

    if ( hdc ) {
        DeleteDC( hdc );
        hdc = 0;
    }
}


bool QPrinter::newPage()
{
    bool success = FALSE;
    if ( hdc && state == PST_ACTIVE ) {
        bool restorePainter = FALSE;
        if ( (qWinVersion()& Qt::WV_DOS_based) && painter && painter->isActive() ) {
            painter->save();               // EndPage/StartPage ruins the DC
            restorePainter = TRUE;
        }
        if ( EndPage(hdc) != SP_ERROR && StartPage(hdc) != SP_ERROR )
            success = TRUE;
        else
            state = PST_ABORTED;
        if ( restorePainter ) {
	    SetMapMode(hdc, MM_ANISOTROPIC);
	    SetWindowExtEx(hdc, res, res, NULL);
	    SetViewportExtEx(hdc, GetDeviceCaps(hdc, LOGPIXELSX), GetDeviceCaps(hdc, LOGPIXELSY), NULL);
            painter->restore();
	}
        if ( (qWinVersion()& Qt::WV_DOS_based) ) {
            if ( fullPage() ) {
                QSize margs = margins();
                OffsetViewportOrgEx( hdc, -margs.width(), -margs.height(), 0 );
            }
            SetTextAlign( hdc, TA_BASELINE );
        }
    }
    return success;
}

/*!
    \internal
    Sets the state of the printer to be 'active'.
*/
void QPrinter::setActive()
{
    state = PST_ACTIVE;
}

/*!
    \internal
    Sets the state of the printer to be 'idle'.
*/
void QPrinter::setIdle()
{
    state = PST_IDLE;
}


bool QPrinter::abort()
{
    if ( state == PST_ACTIVE )
        state = PST_ABORTED;
    return state == PST_ABORTED;
}

bool QPrinter::aborted() const
{
    return state == PST_ABORTED;
}


typedef struct
{
    int winSourceName;
    QPrinter::PaperSource qtSourceName;
} PaperSourceNames;

static PaperSourceNames sources[] = {
    { DMBIN_ONLYONE,        QPrinter::OnlyOne },
    { DMBIN_LOWER,          QPrinter::Lower },
    { DMBIN_MIDDLE,         QPrinter::Middle },
    { DMBIN_MANUAL,         QPrinter::Manual },
    { DMBIN_ENVELOPE,       QPrinter::Envelope },
    { DMBIN_ENVMANUAL,      QPrinter::EnvelopeManual },
    { DMBIN_AUTO,           QPrinter::Auto },
    { DMBIN_TRACTOR,        QPrinter::Tractor },
    { DMBIN_SMALLFMT,       QPrinter::SmallFormat },
    { DMBIN_LARGEFMT,       QPrinter::LargeFormat },
    { DMBIN_LARGECAPACITY,  QPrinter::LargeCapacity },
    { DMBIN_CASSETTE,       QPrinter::Cassette },
    { DMBIN_FORMSOURCE,     QPrinter::FormSource }
};

static QPrinter::PaperSource mapDevmodePaperSource( int s )
{
    int i = 0;
    while ( (sources[i].winSourceName > 0) && (sources[i].winSourceName != s) )
        i++;
    return sources[i].qtSourceName;
}

static int mapPaperSourceDevmode( QPrinter::PaperSource s )
{
    int i = 0;
    while ( (sources[i].qtSourceName >= 0) && (sources[i].qtSourceName != s) ) {
        i++;
    }
    return sources[i].winSourceName;
}

typedef struct
{
    int winSizeName;
    QPrinter::PageSize qtSizeName;
} PageSizeNames;

static PageSizeNames names[] = {
          { DMPAPER_LETTER,             QPrinter::Letter },
          { DMPAPER_LETTERSMALL,        QPrinter::Letter },
          { DMPAPER_TABLOID,            QPrinter::Tabloid },
          { DMPAPER_LEDGER,             QPrinter::Ledger },
          { DMPAPER_LEGAL,              QPrinter::Legal },
          //      { DMPAPER_STATEMENT,          QPrinter:: },
          { DMPAPER_EXECUTIVE,          QPrinter::Executive },
          { DMPAPER_A3,                 QPrinter::A3 },
          { DMPAPER_A4,                 QPrinter::A4 },
          { DMPAPER_A4SMALL,            QPrinter::A4 },
          { DMPAPER_A5,                 QPrinter::A5 },
          { DMPAPER_B4,                 QPrinter::B4 },
          { DMPAPER_B5,                 QPrinter::B5 },
          { DMPAPER_FOLIO,              QPrinter::Folio },
          //{ DMPAPER_QUARTO,           QPrinter:: },
          //{ DMPAPER_10X14,            QPrinter:: },
          //{ DMPAPER_11X17,            QPrinter:: },
          //{ DMPAPER_NOTE,             QPrinter:: },
          //{ DMPAPER_ENV_9,            QPrinter:: },
          { DMPAPER_ENV_10,             QPrinter::Comm10E },
          //{ DMPAPER_ENV_11,           QPrinter:: },
          //{ DMPAPER_ENV_12,           QPrinter:: },
          //{ DMPAPER_ENV_14,           QPrinter:: },
          //{ DMPAPER_CSHEET,           QPrinter:: },
          //{ DMPAPER_DSHEET,           QPrinter:: },
          //{ DMPAPER_ESHEET,           QPrinter:: },
          { DMPAPER_ENV_DL,             QPrinter::DLE },
          //{ DMPAPER_ENV_C5,           QPrinter:: },
          { DMPAPER_ENV_C3,             QPrinter::C5E },
          //{ DMPAPER_ENV_C4,           QPrinter:: },
          //{ DMPAPER_ENV_C6,           QPrinter:: },
          //{ DMPAPER_ENV_C65,          QPrinter:: },
          //{ DMPAPER_ENV_B4,           QPrinter:: },
          //{ DMPAPER_ENV_B5,           QPrinter:: },
          //{ DMPAPER_ENV_B6,           QPrinter:: },
          //{ DMPAPER_ENV_ITALY,                QPrinter:: },
          //{ DMPAPER_ENV_MONARCH,      QPrinter:: },
          //{ DMPAPER_ENV_PERSONAL,     QPrinter:: },
          //{ DMPAPER_FANFOLD_US,               QPrinter:: },
          //{ DMPAPER_FANFOLD_STD_GERMAN,       QPrinter:: },
          //{ DMPAPER_FANFOLD_LGL_GERMAN,       QPrinter:: },
          //{ DMPAPER_ISO_B4,           QPrinter:: },
          //{ DMPAPER_JAPANESE_POSTCARD,        QPrinter:: },
          //{ DMPAPER_9X11,             QPrinter:: },
          //{ DMPAPER_10X11,            QPrinter:: },
          //{ DMPAPER_15X11,            QPrinter:: },
          //{ DMPAPER_ENV_INVITE,               QPrinter:: },
          //{ DMPAPER_RESERVED_48,      QPrinter:: },
          //{ DMPAPER_RESERVED_49,      QPrinter:: },
          { DMPAPER_LETTER_EXTRA,       QPrinter::Letter },
          { DMPAPER_LEGAL_EXTRA,        QPrinter::Legal },
          { DMPAPER_TABLOID_EXTRA,      QPrinter::Tabloid },
          { DMPAPER_A4_EXTRA,           QPrinter::A4},
          { DMPAPER_LETTER_TRANSVERSE,  QPrinter::Letter},
          { DMPAPER_A4_TRANSVERSE,      QPrinter::A4},
          { DMPAPER_LETTER_EXTRA_TRANSVERSE,    QPrinter::Letter },
          { DMPAPER_A_PLUS,             QPrinter::A4 },
          { DMPAPER_B_PLUS,             QPrinter::A3 },
          { DMPAPER_LETTER_PLUS,        QPrinter::Letter },
          { DMPAPER_A4_PLUS,            QPrinter::A4 },
          { DMPAPER_A5_TRANSVERSE,      QPrinter::A5 },
          { DMPAPER_B5_TRANSVERSE,      QPrinter::B5 },
          { DMPAPER_A3_EXTRA,           QPrinter::A3 },
          { DMPAPER_A5_EXTRA,           QPrinter::A5 },
          { DMPAPER_B5_EXTRA,           QPrinter::B5 },
          { DMPAPER_A2,                 QPrinter::A2 },
          { DMPAPER_A3_TRANSVERSE,      QPrinter::A3 },
          { DMPAPER_A3_EXTRA_TRANSVERSE,        QPrinter::A3 },
          { 0, QPrinter::Custom }
};

static QPrinter::PageSize mapDevmodePageSize( int s )
{
    int i = 0;
    while ( (names[i].winSizeName > 0) && (names[i].winSizeName != s) )
        i++;
    return names[i].qtSizeName;
}

static int mapPageSizeDevmode( QPrinter::PageSize s )
{
    int i = 0;
    while ( (names[i].winSizeName > 0) && (names[i].qtSizeName != s) )
    i++;
    return names[i].winSizeName;
}

/*!
    returns the Windows pageSize as defined in the DEVMODE structure.
    \warning this method is only available on Windows.
*/
short QPrinter::winPageSize() const
{
    return hdevmode ? ((DEVMODE *)hdevmode)->dmPaperSize : 0;
}

/*
  Copy the settings from the Windows structures into QPrinter
*/
void QPrinter::readPdlg( void* pdv )
{
    // Note: Remember to reflect any changes here in readPdlgA below!

    PRINTDLG* pd = (PRINTDLG*)pdv;
    output_file = (pd->Flags & PD_PRINTTOFILE) != 0;
    from_pg = pd->nFromPage;
    to_pg = pd->nToPage;
    ncopies = pd->nCopies;
    if ( pd->Flags & PD_COLLATE )
        usercolcopies = TRUE;
    else
        usercolcopies = FALSE;
    if ( hdc ) {
       DeleteDC( hdc );
       viewOffsetDone = FALSE;
    }
    hdc = pd->hDC;
    if ( pd->hDevMode ) {
        DEVMODE* dm = (DEVMODE*)GlobalLock( pd->hDevMode );
        if ( dm ) {
            if ( dm->dmOrientation == DMORIENT_PORTRAIT )
                orient = Portrait;
            else
                orient = Landscape;
            setPageSize( mapDevmodePageSize( dm->dmPaperSize ) );
            setPaperSource( mapDevmodePaperSource( dm->dmDefaultSource ) );
	    if (pd->Flags & PD_USEDEVMODECOPIESANDCOLLATE)
            	ncopies = dm->dmCopies;
            if ( dm->dmCollate == DMCOLLATE_TRUE )
                usercolcopies = TRUE;
            else
                usercolcopies = FALSE;
	    if ( dm->dmColor == DMCOLOR_COLOR )
		color_mode = Color;
	    else
		color_mode = GrayScale;
        }
        GlobalUnlock( pd->hDevMode );
    }

    if ( pd->hDevNames ) {
        DEVNAMES* dn = (DEVNAMES*)GlobalLock( pd->hDevNames );
        if ( dn ) {
            TCHAR* prName = ((TCHAR*)dn) + dn->wDeviceOffset;
            setPrinterName( qt_winQString( prName ) );
            TCHAR* drName = ((TCHAR*)dn) + dn->wDriverOffset;
            setPrintProgram( qt_winQString( drName ) );
        }
        GlobalUnlock( pd->hDevNames );
    }

    if ( pd->hDevMode ) {
        if ( hdevmode )
            GlobalFree( hdevmode );
        hdevmode = pd->hDevMode;
        pd->hDevMode = 0;
    }
    if ( pd->hDevNames ) {
        if ( hdevnames )
            GlobalFree( hdevnames );
        hdevnames = pd->hDevNames;
        pd->hDevNames = 0;
    }
}


void QPrinter::readPdlgA( void* pdv )
{
    // Note: Remember to reflect any changes here in readPdlg above!
    PRINTDLGA* pd = (PRINTDLGA*)pdv;
    output_file = (pd->Flags & PD_PRINTTOFILE) != 0;
    from_pg = pd->nFromPage;
    to_pg = pd->nToPage;
    ncopies = pd->nCopies;
    if ( pd->Flags & PD_COLLATE )
        usercolcopies = TRUE;
    else
	usercolcopies = FALSE;
    if ( hdc ) {
       DeleteDC( hdc );
       viewOffsetDone = FALSE;
    }
    hdc	= pd->hDC;
    if ( pd->hDevMode ) {
	DEVMODEA* dm = (DEVMODEA*)GlobalLock( pd->hDevMode );
	if ( dm ) {
	    if ( dm->dmOrientation == DMORIENT_PORTRAIT )
		orient = Portrait;
	    else
		orient = Landscape;
	    setPageSize( mapDevmodePageSize( dm->dmPaperSize ) );
            setPaperSource( mapDevmodePaperSource( dm->dmDefaultSource ) );
            if (pd->Flags & PD_USEDEVMODECOPIESANDCOLLATE)
            	ncopies = dm->dmCopies;
            if ( dm->dmCollate == DMCOLLATE_TRUE )
                usercolcopies = TRUE;
	    else
		usercolcopies = FALSE;
	    if ( dm->dmColor == DMCOLOR_COLOR )
		color_mode = Color;
	    else
		color_mode = GrayScale;
        }
        GlobalUnlock( pd->hDevMode );
    }

    if ( pd->hDevNames ) {
        DEVNAMES* dn = (DEVNAMES*)GlobalLock( pd->hDevNames );
        // (There is no DEVNAMESA)
        if ( dn ) {
            char* prName = ((char*)dn) + dn->wDeviceOffset;
            setPrinterName( QString::fromLocal8Bit( prName ) );
            char* drName = ((char*)dn) + dn->wDriverOffset;
            setPrintProgram( QString::fromLocal8Bit( drName ) );
        }
        GlobalUnlock( pd->hDevNames );
    }

    if ( pd->hDevMode ) {
        if ( hdevmode )
            GlobalFree( hdevmode );
        hdevmode = pd->hDevMode;
        pd->hDevMode = 0;
    }
    if ( pd->hDevNames ) {
        if ( hdevnames )
            GlobalFree( hdevnames );
        hdevnames = pd->hDevNames;
        pd->hDevNames = 0;
    }
}

static void setDefaultPrinter(const QString &printerName, HANDLE *hmode, HANDLE *hnames)
{
    HANDLE hdevmode = *hmode;
    HANDLE hdevnames = *hnames;
    // Open the printer by name, to get a HANDLE
    HANDLE hPrinter;
    if (! OpenPrinter((TCHAR *)qt_winTchar(printerName,TRUE),&hPrinter,NULL)) {
        qDebug("OpenPrinter(%s) failed, error %d",printerName.latin1(),GetLastError());
        return;
    }

    // Obtain PRINTER_INFO_2 and close printer afterwords
    DWORD nbytes, rbytes;
    GetPrinter(hPrinter,2,NULL,0,&nbytes);
    PRINTER_INFO_2 *pinf2 = (PRINTER_INFO_2 *)GlobalAlloc(GPTR,nbytes);
    BOOL callOk = GetPrinter(hPrinter,2,(LPBYTE)pinf2,nbytes,&rbytes);
    ClosePrinter(hPrinter);
    if (! callOk) {
            qDebug("GetPrinter() failed, error %d",GetLastError());
            GlobalFree(pinf2);
            return;
    }


    // There are drivers with no pDevMode structure!
    if ( pinf2->pDevMode ) {
        // Allocate a global HANDLE for a DEVMODE Structure
        size_t szDEVMODE = sizeof(DEVMODE) + pinf2->pDevMode->dmDriverExtra;
        if ( hdevmode ) {
            GlobalFree( hdevmode );
            hdevmode = 0;
        }
        hdevmode = GlobalAlloc(GHND,szDEVMODE);
        Q_ASSERT(hdevmode != 0);
        DEVMODE *pDevMode = (DEVMODE *)GlobalLock(hdevmode);
        Q_ASSERT(pDevMode != 0);

        // Copy DEVMODE from PRINTER_INFO_2 Structure
        memcpy(pDevMode,pinf2->pDevMode,szDEVMODE);
        GlobalUnlock(hdevmode);
    }

    // Allocate a global HANDLE for a DEVNAMES Structure
    DWORD   lDrvrName = lstrlen(pinf2->pDriverName) + 1;
    DWORD   lPrntName = lstrlen(pinf2->pPrinterName) + 1;
    DWORD   lPortName = lstrlen(pinf2->pPortName) + 1;
    if ( hdevnames ) {
        GlobalFree( hdevnames );
        hdevnames = 0;
    }
    hdevnames = GlobalAlloc(GHND,(lDrvrName + lPrntName + lPortName) * sizeof(TCHAR) + sizeof(DEVNAMES));
    Q_ASSERT(hdevnames != 0);
    DEVNAMES *pDevNames = (DEVNAMES *)GlobalLock(hdevnames);
    Q_ASSERT(pDevNames != 0);

    // Create DEVNAMES Information from PRINTER_INFO_2 Structure
    int tcOffset = sizeof(DEVNAMES) / sizeof(TCHAR);
    Q_ASSERT(sizeof(DEVNAMES) == tcOffset * sizeof(TCHAR));

    pDevNames->wDriverOffset = tcOffset;
    memcpy((LPTSTR)pDevNames + tcOffset,pinf2->pDriverName,lDrvrName * sizeof(TCHAR));
    tcOffset += lDrvrName;

    pDevNames->wDeviceOffset = tcOffset;
    memcpy((LPTSTR)pDevNames + tcOffset,pinf2->pPrinterName,lPrntName * sizeof(TCHAR));
    tcOffset += lPrntName;

    pDevNames->wOutputOffset = tcOffset;
    memcpy((LPTSTR)pDevNames + tcOffset,pinf2->pPortName,lPortName * sizeof(TCHAR));
    tcOffset += lPortName;

    // This is (probably) not the Default Printer
    pDevNames->wDefault = 0;

    // Clean up
    GlobalUnlock(hdevnames);
    GlobalFree(pinf2);
    *hnames = hdevnames;
    *hmode = hdevmode;
}

void QPrinter::setPrinterName( const QString &name )
{
    if ( state != 0 ) {
#if defined(QT_CHECK_STATE)
        qWarning( "QPrinter::setPrinterName: Cannot do this during printing" );
#endif
        return;
    }
    printer_name = name;
    setDefaultPrinter( name, &hdevmode, &hdevnames );
}

bool QPrinter::setup( QWidget *parent )
{
    if ( parent )
        parent = parent->topLevelWidget();
    else
        parent = qApp->mainWidget();

    bool result = FALSE;

    // Must handle the -A and -W versions separately; they're incompatible
#if defined(UNICODE)
#ifndef Q_OS_TEMP
    if ( qWinVersion() & Qt::WV_NT_based ) {
#endif // Q_OS_TEMP
        PRINTDLG pd;
        memset( &pd, 0, sizeof(PRINTDLG) );
        pd.lStructSize = sizeof(PRINTDLG);

        pd.hDevMode   = hdevmode;
        pd.hDevNames  = hdevnames;
        if (pd.hDevMode)
            result = TRUE;
        else {
            pd.Flags = PD_RETURNDEFAULT;
            result = PrintDlg( &pd ) != 0;
        }

	if ( result ) {
	    // writePdlg {
	    pd.Flags = PD_RETURNDC;
#if 0
	    if ( appcolcopies ) 	    
		pd.Flags |= PD_NOPAGENUMS;
	    else
		pd.Flags |= PD_USEDEVMODECOPIESANDCOLLATE;
#else
	    // We want the Collate checkbox to be visible -- for that we have
	    // to specify PD_NOPAGENUMS if we don't set pd.nMinPage and
	    // pd.nMaxPage. In all other cases, we don't need to specify this
	    // option; furthermore, it is harmful to specify it in those cases,
	    // since that disables the selection of only printing certain
	    // pages. (I don't know why this is like this, but my experiments
	    // showed this behaviour.)
	    if ( min_pg==0 && max_pg==0 )
		pd.Flags |= PD_NOPAGENUMS;
#endif
	    if ( usercolcopies )
		pd.Flags |= PD_COLLATE;
            if ( outputToFile() )
                pd.Flags |= PD_PRINTTOFILE;
            pd.hwndOwner = parent ? parent->winId() : 0;
            pd.nFromPage = QMAX(from_pg,min_pg);
            pd.nToPage   = QMIN(to_pg,max_pg);
            if ( pd.nFromPage > pd.nToPage )
                pd.nFromPage = pd.nToPage = 0;
            pd.nMinPage  = min_pg;
            pd.nMaxPage  = max_pg;
            pd.nCopies   = ncopies;

            if ( pd.hDevMode ) {
                DEVMODE* dm = (DEVMODE*)GlobalLock( pd.hDevMode );
                if ( dm ) {
                    if ( orient == Portrait )
			dm->dmOrientation = DMORIENT_PORTRAIT;
		    else
			dm->dmOrientation = DMORIENT_LANDSCAPE;
		    if ( color_mode == Color )
			dm->dmColor = DMCOLOR_COLOR;
		    else
			dm->dmColor = DMCOLOR_MONOCHROME;
		    dm->dmCopies = ncopies;
		    if ( usercolcopies )
			dm->dmCollate = DMCOLLATE_TRUE;
		    else
			dm->dmCollate = DMCOLLATE_FALSE;
		    dm->dmDefaultSource = mapPaperSourceDevmode( paper_source );
		    int winPageSize = mapPageSizeDevmode( pageSize() );
		    if ( winPageSize != 0 ) {
			dm->dmPaperSize = winPageSize;
		    } else if ( pageSize() < Custom ) {
			dm->dmPaperSize = 0;
			dm->dmPaperLength = paperSizes[ pageSize() ].height;
			dm->dmPaperWidth = paperSizes[ pageSize() ].width;
		    }            if ( colorMode() == Color )
			dm->dmColor = DMCOLOR_COLOR;
		    else
			dm->dmColor = DMCOLOR_MONOCHROME;
		    GlobalUnlock( pd.hDevMode );
                }
            }
            // } writePdlg
            result = PrintDlg( &pd );
            if ( result && pd.hDC == 0 )
                result = FALSE;
            if ( result )                               // get values from dlg
                readPdlg( &pd );
        }
#ifndef Q_OS_TEMP
    } else
#endif // Q_OS_TEMP
#endif
#ifndef Q_OS_TEMP
    {
        // Win95/98 A version; identical to the above!
        PRINTDLGA pd;
        memset( &pd, 0, sizeof(PRINTDLGA) );
        pd.lStructSize = sizeof(PRINTDLGA);

        pd.hDevMode   = hdevmode;
        pd.hDevNames  = hdevnames;
        if (pd.hDevMode)
            result = TRUE;
        else {
            pd.Flags         = PD_RETURNDEFAULT;
            result = PrintDlgA( &pd ) != 0;
        }

	if ( result ) {
	    pd.Flags = PD_RETURNDC;
#if 0
	    if ( appcolcopies ) 
		pd.Flags |= PD_NOPAGENUMS;
	    else
                pd.Flags |= PD_USEDEVMODECOPIESANDCOLLATE;
#else
	    // We want the Collate checkbox to be visible -- for that we have
	    // to specify PD_NOPAGENUMS if we don't set pd.nMinPage and
	    // pd.nMaxPage. In all other cases, we don't need to specify this
	    // option; furthermore, it is harmful to specify it in those cases,
	    // since that disables the selection of only printing certain
	    // pages. (I don't know why this is like this, but my experiments
	    // showed this behaviour.)
	    if ( min_pg==0 && max_pg==0 )
		pd.Flags |= PD_NOPAGENUMS;
#endif
	    if ( usercolcopies )
		pd.Flags |= PD_COLLATE;
            if ( outputToFile() )
                pd.Flags |= PD_PRINTTOFILE;
            pd.hwndOwner = parent ? parent->winId() : 0;
            pd.nFromPage = QMAX(from_pg,min_pg);
            pd.nToPage   = QMIN(to_pg,max_pg);
            if ( pd.nFromPage > pd.nToPage )
                pd.nFromPage = pd.nToPage = 0;
            pd.nMinPage  = min_pg;
            pd.nMaxPage  = max_pg;
            pd.nCopies   = ncopies;

        if ( pd.hDevMode ) {
            DEVMODEA* dm = (DEVMODEA*)GlobalLock( pd.hDevMode );
            if ( dm ) {
                if ( orient == Portrait )
                    dm->dmOrientation = DMORIENT_PORTRAIT;
                else
                    dm->dmOrientation = DMORIENT_LANDSCAPE;
                dm->dmCopies = ncopies;
		if ( usercolcopies )
		    dm->dmCollate = DMCOLLATE_TRUE;
		else
		    dm->dmCollate = DMCOLLATE_FALSE;
                dm->dmDefaultSource = mapPaperSourceDevmode( paper_source );
		int winPageSize = mapPageSizeDevmode( pageSize() );
		if ( winPageSize != 0 ) {
		    dm->dmPaperSize = winPageSize;
		} else if ( pageSize() < Custom ) {
		    dm->dmPaperSize = 0;
		    dm->dmPaperLength = paperSizes[ pageSize() ].height;
		    dm->dmPaperWidth = paperSizes[ pageSize() ].width;
		}
		if ( colorMode() == Color )
                    dm->dmColor = DMCOLOR_COLOR;
                else
                    dm->dmColor = DMCOLOR_MONOCHROME;
                GlobalUnlock( pd.hDevMode );
            }
        }
            result = PrintDlgA( &pd );
            if ( result && pd.hDC == 0 )
                result = FALSE;
            if ( result )
                readPdlgA( &pd );
        }
    }
#endif // Q_OS_TEMP

    SetMapMode(hdc, MM_ANISOTROPIC);
    SetWindowExtEx(hdc, res, res, NULL);
    SetViewportExtEx(hdc, GetDeviceCaps(hdc, LOGPIXELSX), GetDeviceCaps(hdc, LOGPIXELSY), NULL);

    return result;
}




static BITMAPINFO *getWindowsBITMAPINFO( const QPixmap &pixmap,
                                         const QImage &image )
{
    int w, h, d, ncols=2;
    if ( !pixmap.isNull() ) {
        w = pixmap.width();
        h = pixmap.height();
        d = pixmap.depth();
    } else {
        w = image.width();
        h = image.height();
        d = image.depth();
    }

    if ( w == 0 || h == 0 || d == 0 )           // invalid image or pixmap
        return 0;

    if ( d > 1 && d <= 8 ) {                    // set to nearest valid depth
        d = 8;                                  //   2..7 ==> 8
        ncols = 256;
    }
    else if ( d > 8 ) {
        d = 32;                                 //   > 8  ==> 32
        ncols = 0;
    }

    int   bpl = ((w*d+31)/32)*4;                // bytes per line
    int   bmi_len = sizeof(BITMAPINFO)+sizeof(RGBQUAD)*ncols;
    char *bmi_data = (char *)malloc( bmi_len );
    memset( bmi_data, 0, bmi_len );
    BITMAPINFO       *bmi = (BITMAPINFO*)bmi_data;
    BITMAPINFOHEADER *bmh = (BITMAPINFOHEADER*)bmi;
    bmh->biSize           = sizeof(BITMAPINFOHEADER);
    bmh->biWidth          = w;
    if ( !pixmap.isNull() && pixmap.isQBitmap() )
      bmh->biHeight       = h;
    else
      bmh->biHeight       = -h;
    bmh->biPlanes         = 1;
    bmh->biBitCount       = d;
    bmh->biCompression    = BI_RGB;
    bmh->biSizeImage      = bpl*h;
    bmh->biClrUsed        = ncols;
    bmh->biClrImportant   = 0;

    if ( ncols > 0  && !image.isNull()) {       // image with color map
        RGBQUAD *r = (RGBQUAD*)(bmi_data + sizeof(BITMAPINFOHEADER));
        ncols = QMIN(ncols,image.numColors());
        for ( int i=0; i<ncols; i++ ) {
            QColor c = image.color(i);
            r[i].rgbRed = c.red();
            r[i].rgbGreen = c.green();
            r[i].rgbBlue = c.blue();
            r[i].rgbReserved = 0;
        }
    }

    return bmi;
}


bool QPrinter::cmd( int c, QPainter *paint, QPDevCmdParam *p )
{
    if ( c ==  PdcBegin ) {                     // begin; start printing
        bool ok = state == PST_IDLE;
        if ( ok && !hdc ) {
            setup( 0 );
            if ( !hdc )
                ok = FALSE;
        }
#if defined(UNICODE)
#ifndef Q_OS_TEMP
        if ( qWinVersion() & Qt::WV_NT_based ) {
#endif
            DOCINFO di;
            memset( &di, 0, sizeof(DOCINFO) );
            di.cbSize = sizeof(DOCINFO);
            di.lpszDocName = (TCHAR*)qt_winTchar(doc_name,TRUE);
            if ( ok && StartDoc(hdc, &di) == SP_ERROR )
                ok = FALSE;
#ifndef Q_OS_TEMP
        } else
#endif
#endif
#ifndef Q_OS_TEMP
	{
            DOCINFOA di;
            memset( &di, 0, sizeof(DOCINFO) );
            di.cbSize = sizeof(DOCINFO);
#if defined(__MINGW32__)
            di.lpszDocName = (const TCHAR*)doc_name.unicode();
#else
            di.lpszDocName = doc_name.ascii();
#endif
            if ( ok && StartDocA(hdc, &di) == SP_ERROR )
                ok = FALSE;
        }
#endif
        if ( ok && StartPage(hdc) == SP_ERROR )
            ok = FALSE;
        if ( ok && fullPage() && !viewOffsetDone ) {
	    if ( qWinVersion() & Qt::WV_DOS_based ) {
		// StartPage destroys DC on Win95/98
		SetMapMode(hdc, MM_ANISOTROPIC);
		SetWindowExtEx(hdc, res, res, NULL);
		SetViewportExtEx(hdc, GetDeviceCaps(hdc, LOGPIXELSX), GetDeviceCaps(hdc, LOGPIXELSY), NULL);
            }
	    QSize margs = margins();

            OffsetViewportOrgEx( hdc, -margs.width(), -margs.height(), 0 );
            //### CS097 viewOffsetDone = TRUE;

        }
        if ( !ok ) {
            if ( hdc ) {
                DeleteDC( hdc );
                hdc = 0;
            }
            state = PST_IDLE;
            return FALSE;
        } else {
            state = PST_ACTIVE;
            painter = paint;
        }
    } else if ( c == PdcEnd ) {
        if ( hdc ) {
            if ( state == PST_ABORTED ) {
                AbortDoc( hdc );
            } else {
                EndPage( hdc );                 // end; printing done
                EndDoc( hdc );
            }
        }
        state = PST_IDLE;
    } else {                                    // all other commands...
        if ( state != PST_ACTIVE )              // aborted or error
            return FALSE;
        if ( hdc == 0 ) {                       // device unexpectedly reset
            state = PST_ABORTED;
            return FALSE;
        }
        if ( c == PdcDrawPixmap || c == PdcDrawImage ) {
            QRect rect    = *p[0].rect;
	    QPoint pos( rect.x(), rect.y() );
	    QPixmap pixmap;
            QImage  image;

            int w;
            int h;

            if ( c == PdcDrawPixmap ) {
                pixmap = *p[1].pixmap;
                w = pixmap.width();
                h = pixmap.height();
                if ( pixmap.isQBitmap() ) {
                    QColor bg = paint->backgroundColor();
                    QColor fg = paint->pen().color();
                    if ( (bg != Qt::white) || (fg != Qt::black) ) {
                        image = pixmap;
                        image.convertDepth( 8 );
                        image.setColor( 0, bg.rgb() );
                        image.setColor( 1, fg.rgb() );
                        pixmap = QPixmap();
                    }
                }
            } else {
                image = *p[1].image;
                w = image.width();
                h = image.height();
            }

            double xs = 1.0;                    // x stretch
            double ys = 1.0;                    // y stretch
            if ( paint ) {
                bool wxf = paint->hasWorldXForm();
                bool vxf = paint->hasViewXForm();
                if ( wxf || vxf ) {             // map position
                    pos = paint->xForm( pos );
                }
                if ( wxf ) {
                    QWMatrix m = paint->worldMatrix();
                    xs = m.m11();
                    ys = m.m22();
                }
                if ( vxf ) {
                    QRect vr = paint->viewport();
                    QRect wr = paint->window();
                    xs = xs * vr.width() / wr.width();
                    ys = ys * vr.height() / wr.height();
                }
            }
            int dw = qRound( xs * rect.width() );
            int dh = qRound( ys * rect.height() );
            BITMAPINFO *bmi = getWindowsBITMAPINFO( pixmap, image );
            BITMAPINFOHEADER *bmh = (BITMAPINFOHEADER*)bmi;
            uchar *bits;

            if ( image.isNull() ) {
                bits = new uchar[bmh->biSizeImage];
                // We are guaranteed that the QPainter does not pass
                // a multi cell pixmap, therefore we can access hbm().
                GetDIBits( pixmap.handle(), pixmap.hbm(), 0, h,
                           bits, bmi, DIB_RGB_COLORS );
            } else {
                bits = image.bits();
            }
            int rc = GetDeviceCaps(hdc,RASTERCAPS);
	    int bpp = GetDeviceCaps(hdc,BITSPIXEL);
	    if ( (rc & RC_STRETCHBLT) != 0 && ( bpp >= 8  || !( rc & RC_STRETCHDIB) ) ) {
                // StretchBlt supported
                HDC     hdcPrn = CreateCompatibleDC( hdc );
                HBITMAP hbm    = CreateDIBitmap( hdc, bmh, CBM_INIT,
                                                 bits, bmi, DIB_RGB_COLORS );
                HBITMAP oldHbm = (HBITMAP)SelectObject( hdcPrn, hbm );
                StretchBlt( hdc, pos.x(), pos.y(), dw, dh,
                            hdcPrn, 0, 0, w, h, SRCCOPY );
                SelectObject( hdcPrn, oldHbm );
                DeleteObject( hbm );
                DeleteObject( hdcPrn );
            } else if ( (rc & RC_STRETCHDIB) != 0 ) {
                // StretchDIBits supported
                StretchDIBits( hdc, pos.x(), pos.y(), dw, dh, 0, 0, w, h,
                               bits, bmi, DIB_RGB_COLORS, SRCCOPY );
	    }
            if ( image.isNull() ) {
                delete [] bits;
            }
            free( bmi );
            return FALSE;                       // don't bitblt
        }
    }
    return TRUE;
}


int QPrinter::metric( int m ) const
{
    if ( hdc == 0 )                     // not ready
        return 0;
    int val;
    switch ( m ) {
    case QPaintDeviceMetrics::PdmWidth:
        val = res * GetDeviceCaps( hdc, fullPage() ? PHYSICALWIDTH : HORZRES ) / GetDeviceCaps( hdc, LOGPIXELSX );
        break;
    case QPaintDeviceMetrics::PdmHeight:
        val = res * GetDeviceCaps( hdc, fullPage() ? PHYSICALHEIGHT : VERTRES ) / GetDeviceCaps( hdc, LOGPIXELSY );
        break;
    case QPaintDeviceMetrics::PdmDpiX:
        val = res;
        break;
    case QPaintDeviceMetrics::PdmDpiY:
        val = res;
        break;
    case QPaintDeviceMetrics::PdmPhysicalDpiX:
        val = GetDeviceCaps( hdc, LOGPIXELSX );
        break;
    case QPaintDeviceMetrics::PdmPhysicalDpiY:
        val = GetDeviceCaps( hdc, LOGPIXELSY );
        break;
    case QPaintDeviceMetrics::PdmWidthMM:
        if ( !fullPage() ) {
            val = GetDeviceCaps( hdc, HORZSIZE );
        }
        else {
            float wi = 25.4 * GetDeviceCaps( hdc, PHYSICALWIDTH );
            val = qRound( wi / GetDeviceCaps( hdc,  LOGPIXELSX ) );
        }
        break;
    case QPaintDeviceMetrics::PdmHeightMM:
        if ( !fullPage() ) {
            val = GetDeviceCaps( hdc, VERTSIZE );
        }
        else {
            float hi = 25.4 * GetDeviceCaps( hdc, PHYSICALHEIGHT );
            val = qRound( hi / GetDeviceCaps( hdc,  LOGPIXELSY ) );
        }
        break;
    case QPaintDeviceMetrics::PdmNumColors:
        val = GetDeviceCaps( hdc, NUMCOLORS );
        break;
    case QPaintDeviceMetrics::PdmDepth:
        val = GetDeviceCaps( hdc, PLANES );
        break;
    default:
#if defined(QT_CHECK_RANGE)
        qWarning( "QPrinter::metric: Invalid metric command" );
#endif
        return 0;
    }
    return val;
}


QSize QPrinter::margins() const
{
    if ( handle() == 0 )                        // not ready
        return QSize( 0, 0 );
    return QSize( GetDeviceCaps( handle(), PHYSICALOFFSETX ) * res / GetDeviceCaps( hdc, LOGPIXELSX ),
                  GetDeviceCaps( handle(), PHYSICALOFFSETY ) * res / GetDeviceCaps( hdc, LOGPIXELSY ) );
}


/*
  This private function creates a new HDC for the printer. It takes the
  different user settings into account. This is used by the setter functions,
  like setOrientation() to create a printer HDC with the appropriate settings
  -- otherwise you don't get the settings except if you show a print dialog.
*/
void QPrinter::reinit()
{
    if ( hdevmode ) {
	HDC hdcTmp;
#ifdef Q_OS_TEMP
	DEVMODE* dm = (DEVMODE*)GlobalLock( hdevmode );
        if ( dm ) {
	    dm->dmOrientation = orient==Portrait ? DMORIENT_PORTRAIT : DMORIENT_LANDSCAPE;
	    hdcTmp = CreateDC( L"WINSPOOL", dm->dmDeviceName, 0, dm );
	}
#else
#if defined(UNICODE)
	if ( qWinVersion() & Qt::WV_NT_based ) {
	    DEVMODE* dm = (DEVMODE*)GlobalLock( hdevmode );
	    if ( dm ) {
		dm->dmOrientation = orient==Portrait ? DMORIENT_PORTRAIT : DMORIENT_LANDSCAPE;
		hdcTmp = CreateDC( L"WINSPOOL", dm->dmDeviceName, 0, dm );
	    }
	} else
#endif
	{
	    DEVMODEA* dm = (DEVMODEA*)GlobalLock( hdevmode );
	    if ( dm ) {
		dm->dmOrientation = orient==Portrait ? DMORIENT_PORTRAIT : DMORIENT_LANDSCAPE;
		hdcTmp = CreateDCA( "WINSPOOL", dm->dmDeviceName, 0, dm );
	    }
	}
#endif
	if ( hdcTmp ) {
	    DeleteDC( hdc );
	    hdc = hdcTmp;
	}
	GlobalUnlock( hdevmode );
    }
}

#endif // QT_NO_PRINTER
