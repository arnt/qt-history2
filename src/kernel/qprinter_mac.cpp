/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qprinter_mac.cpp
**
** Implementation of QPrinter class for mac
**
** Created : 001019
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
** licenses for Unix/X11/FIXME may use this file in accordance with the Qt Commercial
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

// NOT REVISED

/*****************************************************************************
  QPrinter member functions
 *****************************************************************************/

// QPrinter states

#define PST_IDLE	0
#define PST_ACTIVE	1
#define PST_ERROR	2
#define PST_ABORTED	3


QPrinter::QPrinter()
    : QPaintDevice( QInternal::Printer | QInternal::ExternalDevice )
{
    //mac specific
    pformat = kPMNoPageFormat;
    psettings = kPMNoPrintSettings;

    //other
    orient = Portrait;
    page_size = A4;
    ncopies = 1;
    from_pg = to_pg = min_pg = max_pg = 0;
    state = PST_IDLE;
    output_file = FALSE;
    to_edge	= FALSE;
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
    return FALSE;
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
    if(*s == kPMNoPrintSettings) {
	if( PMCreatePrintSettings(s) != noErr )
	    return FALSE;
    } else {
	if( PMSessionValidatePrintSettings(psession, *s, kPMDontWantBoolean) != noErr )
	    return FALSE;
    }
    PMSetPageRange(*s, minPage(), maxPage());
    PMSetFirstPage(*s, fromPage(), TRUE);
    PMSetLastPage(*s, toPage(), TRUE);
    PMSetColorMode(*s, colorMode() == GrayScale ? kPMGray : kPMColor);
    PMSetCopies(*s, numCopies(), TRUE);
    return TRUE;
}

bool 
QPrinter::prepare(PMPageFormat *f)
{
    if( *f == kPMNoPageFormat ) {
	if(PMCreatePageFormat(f) != noErr)
	    return FALSE;
    } else {
	if(PMSessionValidatePageFormat(psession, *f,kPMDontWantBoolean) != noErr)
	    return FALSE;
    }
    PMSetOrientation(*f, orientation() == Portrait ? kPMPortrait : kPMLandscape, TRUE);
    return TRUE;
}


bool QPrinter::setup( QWidget *  )
{
    if(PMCreateSession(&psession) != noErr)
	return FALSE;
    if( ( qApp->style().inherits("QAquaStyle") ) ) {
	Boolean ret;

	//page format
	if(!prepare(&pformat))
	    return FALSE;
	if(PMSessionPageSetupDialog(psession, pformat, &ret) != noErr || !ret)
	    return FALSE;

	//get values
	PMOrientation o;
	if(PMGetOrientation(pformat, &o) == noErr)
	    setOrientation(o == kPMPortrait ? Portrait : Landscape);
	
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
	    setMinMax(min, max);

	PMColorMode cm;
	if(PMGetColorMode(psettings, &cm) == noErr)
	    setColorMode(cm == kPMGray ? GrayScale : Color);

	return TRUE;
    } else if ( QPrintDialog::getPrinterSetup( this ) ) {
	if(!prepare(&pformat) || !prepare(&psettings))
	    return FALSE;
	return TRUE;
    }
    return FALSE;
}


bool QPrinter::cmd( int, QPainter *, QPDevCmdParam * )
{
#if 0
    if ( c ==  PdcBegin ) {			// begin; start printing
    } else if ( c == PdcEnd ) {
    } else {					// all other commands...
	if ( c == PdcDrawPixmap || c == PdcDrawImage ) {
	    return FALSE;			// don't bitblt
	}
    }
#endif
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
	    val = (int)(r.right - r.left) / 72;
	break;
    }
    case QPaintDeviceMetrics::PdmHeight:
    {
	PMRect r;
	if(PMGetAdjustedPaperRect(pformat, &r) == noErr)
	    val = (int)(r.bottom - r.top) / 72;
	break;
    }
    case QPaintDeviceMetrics::PdmDpiY:
    case QPaintDeviceMetrics::PdmDpiX:
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
