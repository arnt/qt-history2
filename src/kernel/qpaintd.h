/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpaintd.h#6 $
**
** Definition of QPaintDevice class
**
** Author  : Haavard Nord
** Created : 940721
**
** Copyright (C) 1994 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QPAINTD_H
#define QPAINTD_H

#include "qwindefs.h"
#include "qrect.h"


// Painter device types (is-A)

#define PDT_UNDEF	0x00
#define PDT_WIDGET	0x01
#define PDT_PIXMAP	0x02
#define PDT_PRINTER	0x03
#define PDT_METAFILE	0x04
#define PDT_MASK	0x0f


// Painter device flags

#define PDF_EXTDEV	0x10
#define PDF_PAINTACTIVE	0x20


// Painter device commands (for programmable, extended devices)

#define PDC_RESERVED_START      0		// codes 0-99 are reserved
#define PDC_RESERVED_STOP	99		//   for Troll Tech

#define PDC_NOP			0		// void
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
#define PDC_DRAWPIXMAP		16		// NI
#define PDC_BEGIN		20		// void
#define PDC_END			21		// void
#define PDC_SAVE		22		// void
#define PDC_RESTORE		23		// void
#define PDC_SETBKCOLOR		30		// l
#define PDC_SETBKMODE		31		// i
#define PDC_SETROP		32		// i
#define PDC_SETBRUSHORIGIN	33		// p
#define PDC_SETFONT		35		// s	NOTE!!! more details
#define PDC_SETPEN		36		// i,i,ul
#define PDC_SETBRUSH		37		// i,ul	NOTE!!! bitmap support
#define PDC_SETUNIT		40		// i
#define PDC_SETVXFORM		41		// i
#define PDC_SETSOURCEVIEW	42		// r
#define PDC_SETTARGETVIEW	43		// r
#define PDC_SETWXFORM		44		// i
#define PDC_SETWXFMATRIX	45		// m,i
#define PDC_SETCLIP		50		// i
#define PDC_SETCLIPRGN		51		// NI

union QPDevCmdParam {
    int		 i;
    ulong	 ul;
    char	*str;
    QPoint	*p;
    QRect	*r;
    QPointArray *a;
    QWXFMatrix  *m;
};


class QPaintDevice				// device for QPainter
{
friend class QPainter;
public:
    QPaintDevice();
    virtual ~QPaintDevice();

    int	     devType()	      const { return devFlags & PDT_MASK; }
    bool     paintingActive() const { return (devFlags & PDF_PAINTACTIVE) ==
					     PDF_PAINTACTIVE; }

    void     bitBlt( int sx, int sy, int sw, int sh, QPaintDevice *dest,
		     int dx, int dy, RasterOp =CopyROP );
    void     bitBlt( const QRect &srcrect, QPaintDevice *dest,
		     const QPoint &dpos, RasterOp =CopyROP );

protected:
    void     setDevType( uint t )   { devFlags = t; }
				
#if defined(_WS_WIN_)
    HDC	     hdc;				// device context
#elif defined(_WS_PM_)
    HPS	     hps;				// presentation space
#elif defined(_WS_X11_)
    Display *dpy;				// display
    WId	     hd;				// handle to drawable
#endif

private:
    uint     devFlags;				// device flags
#if defined(_WS_X11_)
    virtual bool cmd( int, QPDevCmdParam * );
#endif
};


inline void QPaintDevice::bitBlt( const QRect &r, QPaintDevice *pd,
				  const QPoint &p, RasterOp rop )
{
    bitBlt( r.x(), r.y(), r.width(), r.height(), pd, p.x(), p.y(), rop );
}


#endif // QPAINTD_H
