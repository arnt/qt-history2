/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpaintdc.h#7 $
**
** Definition of QPaintDevice constants and flags
**
** Author  : Haavard Nord
** Created : 940721
**
** Copyright (C) 1994,1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QPAINTDC_H
#define QPAINTDC_H

#include "qwindefs.h"


// Painter device cmd() identifiers (for programmable, extended devices)

#define PDC_RESERVED_START      0		// codes 0-99 are reserved
#define PDC_RESERVED_STOP	99		//   for Troll Tech

#define PDC_NOP			0		//  <void>
#define PDC_DRAW_START		1
#define PDC_DRAWPOINT		1		// point
#define PDC_MOVETO		2		// point
#define PDC_LINETO		3		// point
#define PDC_DRAWLINE		4		// point,point
#define PDC_DRAWRECT		5		// rect
#define PDC_DRAWROUNDRECT	6		// rect,ival,ival
#define PDC_DRAWELLIPSE		7		// rect
#define PDC_DRAWARC		8		// rect,ival,ival
#define PDC_DRAWPIE		9		// rect,ival,ival
#define PDC_DRAWCHORD		10		// rect,ival,ival
#define PDC_DRAWLINESEGS	11		// ptarr
#define PDC_DRAWPOLYLINE	12		// ptarr
#define PDC_DRAWPOLYGON		13		// ptarr,ival
#define PDC_DRAWTEXT		14		// point,str
#define PDC_DRAWTEXTFRMT	15		// rect,ival,str
#define PDC_DRAWPIXMAP		16		// point,pixmap
#define PDC_DRAW_STOP		16
#define PDC_BEGIN		30		//  <void>
#define PDC_END			31		//  <void>
#define PDC_SAVE		32		//  <void>
#define PDC_RESTORE		33		//  <void>
#define PDC_SETBKCOLOR		40		// color
#define PDC_SETBKMODE		41		// ival
#define PDC_SETROP		42		// ival
#define PDC_SETBRUSHORIGIN	43		// point
#define PDC_SETFONT		45		// font
#define PDC_SETPEN		46		// pen
#define PDC_SETBRUSH		47		// brush
#define PDC_SETTABSTOPS		48		// ival
#define PDC_SETTABARRAY		49		// ival,ivec
#define PDC_SETUNIT		50		// ival
#define PDC_SETVXFORM		51		// ival
#define PDC_SETSOURCEVIEW	52		// rect
#define PDC_SETTARGETVIEW	53		// rect
#define PDC_SETWXFORM		54		// ival
#define PDC_SETWMATRIX		55		// matrix,ival
#define PDC_SETCLIP		60		// ival
#define PDC_SETCLIPRGN		61		// rgn

union QPDevCmdParam {
    int		 	 ival;
    int			*ivec;
    const char		*str;
    const QPoint	*point;
    const QRect		*rect;
    const QPointArray 	*ptarr;
    const QPixMap	*pixmap;
    const QColor	*color;
    const QFont		*font;
    const QPen		*pen;
    const QBrush	*brush;
    const QRegion	*rgn;
    const QWorldMatrix	*matrix;
};

// Painter device metric() identifiers (for all devices)

#define PDM_WIDTH		1
#define PDM_HEIGHT		2
#define PDM_WIDTHMM		3
#define PDM_HEIGHTMM		4
#define PDM_NUMCOLORS		5
#define PDM_NUMPLANES		6


#endif // QPAINTDC_H
