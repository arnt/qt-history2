/****************************************************************************
**
** Implementation of QPrinter class for mac.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

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
#include "qprinter_p.h"

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
    d = new QPrinterPrivate;
    if(PMCreateSession(&psession) != noErr)
	psession = NULL;

    switch(m) {
    case Compatible:
	devFlags |= QInternal::CompatibilityMode;
	// fall through
    case PrinterResolution:
    case HighResolution: {
	bool found = FALSE;
	PMPrinter printer = 0;
        if(psession && PMSessionGetCurrentPrinter(psession, &printer) == noErr) {
	    PMResolution pres;
	    UInt32 count = 0, maxRes = 0;
	    if (PMPrinterGetPrinterResolutionCount( printer, &count ) == noErr && count)
		for( ; count > 0; --count )
		    if (PMPrinterGetIndexedPrinterResolution( printer, count, &pres ) == noErr) {
			found = TRUE;
			maxRes = QMAX( (uint)pres.vRes, maxRes );
			res = maxRes;
		    }
	}
	if(!found)
	    res = 600; //just to have something
	break;
    }
    case ScreenResolution: {
	short vr, hr;
	ScreenRes(&hr, &vr);
	res = vr;
	break; }
    }

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

    //mac specific
    pformat = kPMNoPageFormat;
    psettings = kPMNoPrintSettings;
    prepare(&pformat);
    prepare(&psettings);
    interpret(&pformat);
    interpret(&psettings);
    setPrintRange( AllPages );

    d->printerOptions = 0;
    setOptionEnabled( PrintToFile, TRUE );
    setOptionEnabled( PrintPageRange, TRUE );
    setPrintRange( AllPages );
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
    PMResolution pres;
    pres.hRes = res;
    pres.vRes = res;
    PMSetResolution(*f, &pres);
    return TRUE;
}


void QPrinter::setPrinterName(const QString &name)
{
    if (state != 0) {
        qWarning("Qt: QPrinter::setPrinterName: Cannot do this during printing");
        return;
    }
    printer_name = name;
}

bool QPrinter::setup(QWidget *w)
{
    if(!psession && PMCreateSession(&psession) != noErr)
        return FALSE;
    if(qApp->style().inherits(QMAC_DEFAULT_STYLE) || qApp->style().inherits("QMacStyle")) {
	return (printSetup(w) && pageSetup(w));
    } else if(QPrintDialog::getPrinterSetup(this)) {
        if(!prepare(&pformat) || !prepare(&psettings))
            return FALSE;
        return TRUE;
    }
    return FALSE;
}

void QPrinter::interpret(PMPrintSettings *s)
{
    //get values
    UInt32 from, to;
    if(PMGetFirstPage(*s, &from) == noErr && PMGetFirstPage(*s, &to) == noErr)
	setFromTo(from, to);

    UInt32 copies;
    if(PMGetCopies(*s, &copies) == noErr)
	setNumCopies(copies);

    UInt32 max, min;
    if(PMGetPageRange(*s, &min, &max) == noErr)
	setMinMax(min-1, max-1);

    PMColorMode cm;
    if(PMGetColorMode(*s, &cm) == noErr)
	setColorMode(cm == kPMGray ? GrayScale : Color);
}

void QPrinter::interpret(PMPageFormat *f)
{
    //get values
    PMOrientation o;
    if(PMGetOrientation(*f, &o) == noErr)
	setOrientation(o == kPMPortrait ? Portrait : Landscape);

    //Finally we update the scale so the resolution is effected by it
    PMSessionValidatePageFormat(psession, *f, kPMDontWantBoolean);
}

// shows the native mac print setup dialog
bool QPrinter::printSetup(QWidget *)
{
    if(!psession && PMCreateSession(&psession) != noErr)
        return FALSE;
    if(qApp->style().inherits(QMAC_DEFAULT_STYLE) || qApp->style().inherits("QMacStyle")) {
        Boolean ret;
	QMacBlockingFunction block;
        //setup
        if(!prepare(&psettings))
            return FALSE;
        if(PMSessionPrintDialog(psession, psettings, pformat, &ret) != noErr || !ret )
            return FALSE;
	interpret(&psettings);
	return TRUE;
    }
    return FALSE;
}

// shows the native mac page setup dialog
bool QPrinter::pageSetup(QWidget *)
{
    if(!psession && PMCreateSession(&psession) != noErr)
        return FALSE;
    if(qApp->style().inherits(QMAC_DEFAULT_STYLE) || qApp->style().inherits("QMacStyle")) {
        Boolean ret;
	QMacBlockingFunction block;
        //page format
        if(!prepare(&pformat))
            return FALSE;
        if(PMSessionPageSetupDialog(psession, pformat, &ret) != noErr || !ret)
            return FALSE;
	interpret(&pformat);
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
            qDebug("Qt: internal: printer: two PdcBegin(s).");
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
					&hd) != noErr) { //get the gworld
	    cg_hd = 0;
            return FALSE;
	}
	if(fullPage()) {
	    QSize marg(margins());
	    QMacSavedPortInfo mp(this);
	    SetOrigin(marg.width(), marg.height());
	} else {
	    SetOrigin(0, 0);
	}
        state = PST_ACTIVE;
    } else if(c == PdcEnd) {
        if(hd && state != PST_IDLE) {
	    CGContextRelease((CGContextRef)cg_hd);
	    cg_hd = 0;
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
        qWarning("Qt: QPixmap::metric: Invalid metric command");
    }
    return val;
}

QSize QPrinter::margins() const
{
    uint t, l;
    margins(&t, &l, NULL, NULL);
    return QSize(t, l);
}

Qt::HANDLE QPrinter::macCGHandle() const
{
    if(!cg_hd)
	CreateCGContextForPort((CGrafPtr)hd, (CGContextRef*)&cg_hd);
    return cg_hd;
}

void QPrinter::setMargins(uint, uint, uint, uint)
{
}

void QPrinter::margins(uint *top, uint *left, uint *bottom, uint *right) const
{
    PMRect paperr, pager;
    if(PMGetAdjustedPaperRect(pformat, &paperr) != noErr || PMGetAdjustedPageRect(pformat, &pager) != noErr) {
	qWarning("Qt: QPrinter: Unexpected condition reached %s:%d", __FILE__, __LINE__);
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
