/****************************************************************************
** $Id$
**
** Implementation of QPrinter class for mac
**
** Created : 001019
**
** Copyright (C) 1992-2002 Trolltech AS.  All rights reserved.
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
#include "qt_mac.h"
#include <qstyle.h>
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


QPrinter::QPrinter(PrinterMode m) : QPaintDevice(QInternal::Printer | QInternal::ExternalDevice)
{
    if(PMCreateSession(&psession) != noErr)
	psession = NULL;

    switch(m) {
    case Compatible:
	devFlags |= QInternal::CompatibilityMode;
	// fall through
    case PrinterResolution:
    case HighResolution: {
#if 0
	bool found = FALSE;
	PMPrinter printer;
	if(psession && PMSessionGetCurrentPrinter(psession, &printer) == noErr) {
	    PMResolution pres;
	    if(PMPrinterGetPrinterResolution(printer, kPMMaximumValue, &pres) == noErr) {
		found = TRUE;
		res = (int)pres.vRes; //obviously I need to divide this by SOMETHING, but what at this point? FIXME!
		qDebug("%d", res);
	    }
	}
	if(!found)
	    res = 600; //just to have something
	break;
#endif
    }
    case ScreenResolution: {
	short vr, hr;
	ScreenRes(&hr, &vr);
	res = vr;
	break; }
    }

    //mac specific
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
    if(PMSessionEndPage(psession) != noErr)  {//end the last page
        state = PST_ERROR;
        return FALSE;
    }
    if(PMSessionBeginPage(psession, pformat, NULL) != noErr)  { //start a new one
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
        if(PMCreatePrintSettings(s) != noErr)
            return FALSE;
        if(PMSessionDefaultPrintSettings(psession, *s) != noErr)
            return FALSE;
    } else {
        if(PMSessionValidatePrintSettings(psession, *s, kPMDontWantBoolean) != noErr )
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
    if(*f == kPMNoPageFormat) {
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


void QPrinter::setPrinterName(const QString &name)
{
    if (state != 0) {
#if defined(QT_CHECK_STATE)
        qWarning("QPrinter::setPrinterName: Cannot do this during printing");
#endif
        return;
    }
    printer_name = name;
}

extern void qt_init_app_proc_handler();
extern void qt_release_app_proc_handler();

bool QPrinter::setup(QWidget *)
{
    if(!psession && PMCreateSession(&psession) != noErr)
        return FALSE;
    if(qApp->style().inherits(QMAC_DEFAULT_STYLE) || qApp->style().inherits("QMacStyle")) {
        Boolean ret;
	QMacBlockingFunction block;
	qt_release_app_proc_handler();
        //setup
        if(!prepare(&psettings)) {
	    qt_init_app_proc_handler();
            return FALSE;
	}
        if(PMSessionPrintDialog(psession, psettings, pformat, &ret) != noErr || !ret ) {
	    qt_init_app_proc_handler();
            return FALSE;
	}

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
        if(!prepare(&pformat)) {
	    qt_init_app_proc_handler();
            return FALSE;
	}
        if(PMSessionPageSetupDialog(psession, pformat, &ret) != noErr || !ret) {
	    qt_init_app_proc_handler();
            return FALSE;
	}

        //get values
        PMOrientation o;
        if(PMGetOrientation(pformat, &o) == noErr)
            setOrientation(o == kPMPortrait ? Portrait : Landscape);
	qt_init_app_proc_handler();

	//Finally we update the scale so the resolution is effected by it
	double oldscale=0;
	PMGetScale(pformat, &oldscale);
	PMSetScale(pformat, (((double)7200.0) / res) * (oldscale / 100));	
	PMSessionValidatePageFormat(psession, pformat, kPMDontWantBoolean);
        return TRUE;
    } else if(QPrintDialog::getPrinterSetup(this)) {
        if(!prepare(&pformat) || !prepare(&psettings))
            return FALSE;
        return TRUE;
    }
    return FALSE;
}


bool QPrinter::cmd(int c, QPainter *, QPDevCmdParam *)
{
    if(!psession && PMCreateSession(&psession) != noErr)
        return FALSE;

    if (c ==  PdcBegin) {                     // begin; start printing
        if(state != PST_IDLE) {
            qDebug("printer: two PdcBegin(s).");
            return FALSE;
        }

        PMRect rect;
        OSStatus r;

        //validate the settings
        if(PMSessionValidatePrintSettings(psession, psettings, kPMDontWantBoolean) != noErr)
            return FALSE;
        if(PMSessionValidatePageFormat(psession, pformat, kPMDontWantBoolean) != noErr)
            return FALSE;
        if((r=PMGetAdjustedPageRect(pformat, &rect)) != noErr)
            return FALSE;

        if(PMSessionBeginDocument(psession, psettings, pformat) != noErr) //begin the document
            return FALSE;
        if(PMSessionBeginPage(psession, pformat, &rect) != noErr ) //begin the page
            return FALSE;
        if(PMSessionGetGraphicsContext(psession, kPMGraphicsContextQuickdraw, 
					&hd) != noErr) //get the gworld
            return FALSE;
        state = PST_ACTIVE;
    } else if(c == PdcEnd) {
        if(hd && state != PST_IDLE) {
            PMSessionEndPage(psession);
            PMSessionEndDocument(psession);
            hd = NULL;
        }
        state  = PST_IDLE;
    } else {                                    // all other commands...
        if((state == PST_ACTIVE || state == PST_ERROR ) && PMSessionError(psession) != noErr)
            return FALSE;
    }
    return TRUE;
}


int QPrinter::metric(int m) const
{
    int val = 1;
    switch(m) {
    case QPaintDeviceMetrics::PdmWidth:
    {
	if(fullPage()) {
	    PMRect r;
	    if(PMGetAdjustedPaperRect(pformat, &r) == noErr)
		val = (int)(r.right - r.left);
	} else {
	    PMRect r;
	    if(PMGetAdjustedPageRect(pformat, &r) == noErr)
		val = (int)(r.right - r.left);
	}
        break;
    }
    case QPaintDeviceMetrics::PdmHeight:
    {
	if(fullPage()) {
	    PMRect r;
	    if(PMGetAdjustedPaperRect(pformat, &r) == noErr)
		val = (int)(r.bottom - r.top);
	} else {
	    PMRect r;
	    if(PMGetAdjustedPageRect(pformat, &r) == noErr)
		val = (int)(r.bottom - r.top);
	}
        break;
    }
    case QPaintDeviceMetrics::PdmPhysicalDpiX:
    case QPaintDeviceMetrics::PdmPhysicalDpiY: {
	PMPrinter printer;
	if(PMSessionGetCurrentPrinter(psession, &printer) == noErr) {
	    PMResolution resolution;
	    PMPrinterGetPrinterResolution(printer, kPMCurrentValue, &resolution);
	    val = (int)resolution.vRes;
	    break;
	}
	//otherwise fall through
    }
    case QPaintDeviceMetrics::PdmDpiY:
    case QPaintDeviceMetrics::PdmDpiX:
	val = res;
	break;
    case QPaintDeviceMetrics::PdmWidthMM:
        // double rounding error here.  hooray.
        val = metric(QPaintDeviceMetrics::PdmWidth);
        val = (val * 254 + 5*res) / (10*res);
        break;
    case QPaintDeviceMetrics::PdmHeightMM:
        val = metric(QPaintDeviceMetrics::PdmHeight);
        val = (val * 254 + 5*res) / (10*res);
        break;
    case QPaintDeviceMetrics::PdmNumColors:
        val = (1 << metric(QPaintDeviceMetrics::PdmDepth));
        break;
    case QPaintDeviceMetrics::PdmDepth:
        val = 24;
        break;
    default:
        val = 0;
#if defined(QT_CHECK_RANGE)
        qWarning("QPixmap::metric: Invalid metric command");
#endif
    }
    return val;
}


QSize QPrinter::margins() const
{
    uint t, l;
    margins(&t, &l, NULL, NULL);
    return QSize(t, l);
}

void QPrinter::setMargins(uint, uint, uint, uint)
{
}

void QPrinter::margins(uint *top, uint *left, uint *bottom, uint *right) const
{
    PMRect paperr, pager;
    if(PMGetAdjustedPaperRect(pformat, &paperr) != noErr || PMGetAdjustedPageRect(pformat, &pager) != noErr) {
	qWarning("That shouldn't happen %s:%d", __FILE__, __LINE__);
	return;
    }
    if(top)
	*top = (uint)(pager.top - paperr.top);
    if(left)
	*left = (uint)(pager.left - paperr.left);
    if(bottom)
	*bottom = (uint)(paperr.bottom - pager.bottom);
    if(right)
	*right = (uint)(paperr.right - pager.right);
}

#endif
