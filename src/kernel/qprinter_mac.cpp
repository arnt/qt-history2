/****************************************************************************
** $Id: $
**
** Implementation of QPrinter class for mac
**
** Created : 001019
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Macintosh may use this file in accordance with the Qt Commercial
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

#include "qpaintdevicemetrics.h"
#include "qfile.h"
#include "qfileinfo.h"
#include "qdir.h"
#include "qpsprinter_p.h"
#include "qprintdialog.h"
#include "qapplication.h"
#include "qapplication_p.h"
#include <stdlib.h>
#include "qt_mac.h"
#include <qstyle.h>

// NOT REVISED

/*****************************************************************************
  QPrinter member functions
 *****************************************************************************/

// QPrinter states

#define PST_IDLE        0
#define PST_ACTIVE      1
#define PST_ERROR       2
#define PST_ABORTED     3


QPrinter::QPrinter( PrinterMode m )
    : QPaintDevice( QInternal::Printer | QInternal::ExternalDevice )
{
    //mac specific
    psession = NULL;
    pformat = kPMNoPageFormat;
    psettings = kPMNoPrintSettings;
    prepare(&pformat);
    prepare(&psettings);

    //other
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
            res = 80; // ### FIXME
            break;
        case Compatible:
            devFlags |= QInternal::CompatibilityMode;
            // fall through
        case PrinterResolution:
        case HighResolution:
            res = metric( QPaintDeviceMetrics::PdmPhysicalDpiY );
    }
}

QPrinter::~QPrinter()
{
#if 0
    if(pformat != kPMNoPageFormat)
        PMDisposePageFormat(pformat);
    if(psettings != kPMNoPrintSettings)
        PMDisposePrintSettings(psettings);
#endif
}


bool QPrinter::newPage()
{
    if( PMSessionEndPage(psession) != noErr )  {//end the last page
        state = PST_ERROR;
        return FALSE;
    }
    PMRect rect;
    if( PMGetAdjustedPageRect(pformat, &rect) != noErr ) {
        state = PST_ERROR;
        return FALSE;
    }
    if( PMSessionBeginPage(psession, pformat, &rect) != noErr )  { //start a new one
        state = PST_ERROR;
        return FALSE;
    }
    return TRUE;
}


bool QPrinter::abort()
{
    return FALSE;
}

bool QPrinter::aborted() const
{
    return FALSE;
}

bool
QPrinter::prepare(PMPrintSettings *s)
{
    if(!psession && PMCreateSession(&psession) != noErr)
        return FALSE;
    if(*s == kPMNoPrintSettings) {
        if( PMCreatePrintSettings(s) != noErr )
            return FALSE;
        if( PMSessionDefaultPrintSettings(psession, *s) != noErr)
            return FALSE;
    } else {
        if( PMSessionValidatePrintSettings(psession, *s, kPMDontWantBoolean) != noErr )
            return FALSE;
    }
    PMSetPageRange(*s, minPage()+1, maxPage()+1);
    PMSetFirstPage(*s, fromPage(), TRUE);
    PMSetLastPage(*s, toPage(), TRUE);
    PMSetColorMode(*s, colorMode() == GrayScale ? kPMGray : kPMColor);
    PMSetCopies(*s, numCopies(), TRUE);
    return TRUE;
}

bool
QPrinter::prepare(PMPageFormat *f)
{
    if(!psession && PMCreateSession(&psession) != noErr)
        return FALSE;
    if( *f == kPMNoPageFormat ) {
        if(PMCreatePageFormat(f) != noErr)
            return FALSE;
        if(PMSessionDefaultPageFormat(psession, *f) != noErr)
            return FALSE;
    } else {
        if(PMSessionValidatePageFormat(psession, *f,kPMDontWantBoolean) != noErr)
            return FALSE;
    }
    PMSetOrientation(*f, orientation() == Portrait ? kPMPortrait : kPMLandscape, TRUE);
    return TRUE;
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
}

bool QPrinter::setup( QWidget *  )
{
    if(!psession && PMCreateSession(&psession) != noErr)
        return FALSE;
    if( qApp->style().inherits(QMAC_DEFAULT_STYLE) || qApp->style().inherits("QMacStyle") ) {
        Boolean ret;

        //setup
        if(!prepare(&psettings))
            return FALSE;
        if(PMSessionPrintDialog(psession, psettings, pformat, &ret) != noErr || !ret )
            return FALSE;

        //get values
        UInt32 from, to;
        if(PMGetFirstPage(psettings, &from) == noErr && PMGetFirstPage(psettings, &to) == noErr)
            setFromTo(from, to);

        UInt32 copies;
        if(PMGetCopies(psettings, &copies) == noErr)
            setNumCopies(copies);

        UInt32 max, min;
        if(PMGetPageRange(psettings, &max, &min) == noErr)
            setMinMax(min-1, max-1);

        PMColorMode cm;
        if(PMGetColorMode(psettings, &cm) == noErr)
            setColorMode(cm == kPMGray ? GrayScale : Color);

        //page format
        if(!prepare(&pformat))
            return FALSE;
        if(PMSessionPageSetupDialog(psession, pformat, &ret) != noErr || !ret)
            return FALSE;

        //get values
        PMOrientation o;
        if(PMGetOrientation(pformat, &o) == noErr)
            setOrientation(o == kPMPortrait ? Portrait : Landscape);

        return TRUE;
    } else if ( QPrintDialog::getPrinterSetup( this ) ) {
        if(!prepare(&pformat) || !prepare(&psettings))
            return FALSE;
        return TRUE;
    }
    return FALSE;
}


bool QPrinter::cmd( int c, QPainter *, QPDevCmdParam * )
{
    if(!psession && PMCreateSession(&psession) != noErr)
        return FALSE;

    if ( c ==  PdcBegin ) {                     // begin; start printing
        if(state != PST_IDLE) {
            qDebug("printer: two PdcBegin(s).");
            return FALSE;
        }

        PMRect rect;
        OSStatus r;

        //validate the settings
        if( PMSessionValidatePrintSettings(psession, psettings, kPMDontWantBoolean) != noErr )
            return FALSE;
        if(PMSessionValidatePageFormat(psession, pformat, kPMDontWantBoolean) != noErr)
            return FALSE;
        if( (r=PMGetAdjustedPageRect(pformat, &rect)) != noErr )
            return FALSE;

        if(PMSessionBeginDocument(psession, psettings, pformat) != noErr) //begin the document
            return FALSE;
        if( PMSessionBeginPage(psession, pformat, &rect) != noErr ) //begin the page
            return FALSE;
        if( PMSessionGetGraphicsContext(psession, kPMGraphicsContextQuickdraw, 
					&hd) != noErr ) //get the gworld
            return FALSE;
        state = PST_ACTIVE;
    } else if ( c == PdcEnd ) {
        if(hd && state != PST_IDLE) {
            PMSessionEndPage(psession);
            PMSessionEndDocument(psession);
            hd = NULL;
        }
        state  = PST_IDLE;
    } else {                                    // all other commands...
        if( (state == PST_ACTIVE || state == PST_ERROR ) && PMSessionError(psession) != noErr)
            return FALSE;
    }
    return TRUE;
}


int QPrinter::metric( int m ) const
{
    int val = 1;
    switch ( m ) {
    case QPaintDeviceMetrics::PdmWidth:
    {
        PMRect r;
        if(PMGetAdjustedPaperRect(pformat, &r) == noErr)
            val = (int)(r.right - r.left);
        break;
    }
    case QPaintDeviceMetrics::PdmHeight:
    {
        PMRect r;
        if(PMGetAdjustedPaperRect(pformat, &r) == noErr)
            val = (int)(r.bottom - r.top);;
        break;
    }
    case QPaintDeviceMetrics::PdmDpiY:
    case QPaintDeviceMetrics::PdmDpiX:
    // ### FIXME: logical resolution scales with res!
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


QSize QPrinter::margins() const
{
    return (orient == Portrait) ? QSize( 36, 22 ) : QSize( 22, 36 );
}

#endif
