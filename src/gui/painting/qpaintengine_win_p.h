/****************************************************************************
**
** Definition of QPaintEngine(for Windows) private data.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QWIN32PAINTENGINE_P_H
#define QWIN32PAINTENGINE_P_H

#include <windows.h>

#include "qnamespace.h"

#define COLOR_VALUE(c) ((d->flags & RGBColor) ? RGB(c.red(),c.green(),c.blue()) : c.pixel())

static const short rasterOpCodes[] = {
    R2_COPYPEN,	// CopyROP
    R2_MERGEPEN,	// OrROP
    R2_XORPEN,	// XorROP
    R2_MASKNOTPEN,	// NotAndROP
    R2_NOTCOPYPEN,	// NotCopyROP
    R2_MERGENOTPEN,	// NotOrROP
    R2_NOTXORPEN,	// NotXorROP
    R2_MASKPEN,	// AndROP
    R2_NOT,		// NotROP
    R2_BLACK,	// ClearROP
    R2_WHITE,	// SetROP
    R2_NOP,		// NopROP
    R2_MASKPENNOT,	// AndNotROP
    R2_MERGEPENNOT,	// OrNotROP
    R2_NOTMASKPEN,	// NandROP
    R2_NOTMERGEPEN	// NorROP
};

class QWin32PaintEnginePrivate
{
public:
    QWin32PaintEnginePrivate() :
	hwnd(0),
	hdc(0),
	hpen(0),
	hfont(0),
	hbrush(0),
	hbrushbm(0),
	holdpal(0),
	flags(0),
	penRef(0),
	brushRef(0),
	nocolBrush(false),
	pixmapBrush(false),
	usesWidgetDC(false),
	pStyle(Qt::SolidLine),
	pWidth(0),
	pColor(0),
	bColor(0),
	device(0)
    {
    }

    HWND              	hwnd;
    HDC               	hdc;
    HPEN              	hpen;
    HFONT	      	hfont;
    HBRUSH	      	hbrush;
    HBITMAP	      	hbrushbm;
    HPALETTE            holdpal;
    uint 		flags;

    void 	      	*penRef;
    void 		*brushRef;

    uint 		nocolBrush:1;
    uint		pixmapBrush:1;
    uint                usesWidgetDC:1;

    Qt::RasterOp        rasterOp;
    Qt::PenStyle 	pStyle;
    int			pWidth;
    COLORREF            pColor;
    COLORREF            bColor;

    uint 		fontFlags;

    const QPaintDevice        *device;
};

class QWin32PrinterPaintEnginePrivate : public QWin32PaintEnginePrivate
{
public:
    QWin32PrinterPaintEnginePrivate()
	: printer(0),
	  state(0)
    {
    }
    QPrinter *printer;
    int state;
};

#endif
