/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpaintdevice.h#2 $
**
** Definition of QPaintDevice class
**
** Author  : Haavard Nord
** Created : 940721
**
** Copyright (C) 1994 by Troll Tech as.  All rights reserved.
**
*****************************************************************************/

#ifndef QPAINTD_H
#define QPAINTD_H

#include "qwindefs.h"


// Painter device types (is-A)

#define PDT_UNDEF	0
#define PDT_WIDGET	1
#define PDT_PIXMAP	2
#define PDT_PRINTER	3
#define PDT_METAFILE	4

// Painter device commands (for unsupported devices)

#define PDC_RESERVED_START      0		// codes 0-999 are reserved
#define PDC_RESERVED_STOP	999		//   for internal use

#define PDC_DRAWPOINT		1		// p
#define PDC_MOVETO		2		// p
#define PDC_LINETO		3		// p
#define PDC_DRAWLINE		4		// p,p
#define PDC_DRAWRECT		5		// r
#define PDC_DRAWROUNDRECT	6		// r,i,i
#define PDC_DRAWELLIPSE		7		// r
#define PDC_DRAWARC		8		// r,i,i
#define PDC_DRAWPIE		9		// r,i,i
#define PDC_DRAWCHORD		10		// r,i,i
#define PDC_DRAWLINESEGS	11		// a
#define PDC_DRAWPOLYLINE	12		// a
#define PDC_DRAWPOLYGON		13		// a,i
#define PDC_DRAWTEXT		14		// p,s
#define PDC_DRAWTEXTALIGN	15		// NI
#define PDC_BEGIN		20
#define PDC_END			21
#define PDC_SETBKCOLOR		30		// l
#define PDC_SETBKMODE		31		// i
#define PDC_SETROP		32		// i
#define PDC_SETPEN		33		// i,i,ul
#define PDC_SETBRUSH		34		// i,ul
#define PDC_SETXFORM		40		// i
#define PDC_SETSOURCEVIEW	41		// r
#define PDC_SETTARGETVIEW	42		// r
#define PDC_SETCLIP		50		// i
#define PDC_SETCLIPRGN		51		// NI

union QPDevCmdParam {
    int		 i;
    ulong	 ul;
    char	*str;
    QPoint	*p;
    QRect	*r;
    QPointArray *a;
};


class QPaintDevice				// device for QPainter
{
friend class QPainter;
public:
    QPaintDevice();
    virtual ~QPaintDevice();

protected:
    int	     devType;				// type of device
#if defined(_WS_WIN_) || defined(_WS_WIN32_)
    HDC	     hdc;				// device context
#elif defined(_WS_PM_)
    HPS	     hps;				// presentation space
#elif defined(_WS_X11_)
    Display *dpy;				// display
    WId	     hd;				// handle to drawable
#endif

private:
#if defined(_WS_X11_)
    virtual bool cmd( int, QPDevCmdParam * );
#endif
};


#endif // QPAINTD_H
