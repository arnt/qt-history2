/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpaintdevicedefs.h#1 $
**
** Definition of QPaintDevice constants and flags
**
** Author  : Haavard Nord
** Created : 940721
**
** Copyright (C) 1994 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QPAINTDC_H
#define QPAINTDC_H

#include "qwindefs.h"


// Painter device cmd() identifiers (for programmable, extended devices)

#define PDC_RESERVED_START      0		// codes 0-99 are reserved
#define PDC_RESERVED_STOP	99		//   for Troll Tech

#define PDC_NOP			0		//  <void>
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
#define PDC_DRAWTEXTALIGN	15		// NI
#define PDC_DRAWPIXMAP		16		// point,pixmap
#define PDC_BEGIN		20		//  <void>
#define PDC_END			21		//  <void>
#define PDC_SAVE		22		//  <void>
#define PDC_RESTORE		23		//  <void>
#define PDC_SETBKCOLOR		30		// color
#define PDC_SETBKMODE		31		// ival
#define PDC_SETROP		32		// ival
#define PDC_SETBRUSHORIGIN	33		// point
#define PDC_SETFONT		35		// font
#define PDC_SETPEN		36		// pen
#define PDC_SETBRUSH		37		// brush
#define PDC_SETUNIT		40		// ival
#define PDC_SETVXFORM		41		// ival
#define PDC_SETSOURCEVIEW	42		// rect
#define PDC_SETTARGETVIEW	43		// rect
#define PDC_SETWXFORM		44		// ival
#define PDC_SETWXFMATRIX	45		// matrix,ival
#define PDC_SETCLIP		50		// ival
#define PDC_SETCLIPRGN		51		// rgn

union QPDevCmdParam {
    int		 ival;
    char	*str;
    QPoint	*point;
    QRect	*rect;
    QPointArray *ptarr;
    QPixMap	*pixmap;
    QColor	*color;
    QFont	*font;
    QPen	*pen;
    QBrush	*brush;
    QRegion	*rgn;
    QWXFMatrix  *matrix;
};

// Painter device metric() identifiers (for all devices)

#define PDM_WIDTH		1
#define PDM_HEIGHT		2
#define PDM_MMWIDTH		3
#define PDM_MMHEIGHT		4
#define PDM_NUMCOLORS		5
#define PDM_COLORPLANES		6


#endif // QPAINTDC_H
