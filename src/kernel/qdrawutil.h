/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qdrawutil.h#2 $
**
** Definition of draw utilities
**
** Author  : Haavard Nord
** Created : 950920
**
** Copyright (C) 1995-1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QDRAWUTL_H
#define QDRAWUTL_H

#include "qpainter.h"
#include "qpalette.h"


//
// Standard shade drawing
//


void drawShadeLine( QPainter *p, int x1, int y1, int x2, int y2,
		    const QColorGroup &g, bool sunken = TRUE,
		    int lineWidth = 1, int midLineWidth = 0 );

void drawShadeLine( QPainter *p, const QPoint &p1, const QPoint &p2,
		    const QColorGroup &g, bool sunken = TRUE,
		    int lineWidth = 1, int midLineWidth = 0 );

void drawShadeRect( QPainter *p, int x, int y, int w, int h,
		    const QColorGroup &, bool sunken=FALSE,
		    int lineWidth = 1, int midLineWidth = 0,
		    const QBrush *fill = 0 );

void drawShadeRect( QPainter *p, const QRect &r,
		    const QColorGroup &, bool sunken=FALSE,
		    int lineWidth = 1, int midLineWidth = 0,
		    const QBrush *fill = 0 );

void drawShadePanel( QPainter *p, int x, int y, int w, int h,
		     const QColorGroup &, bool sunken=FALSE,
		     int lineWidth = 1, const QBrush *fill = 0 );

void drawShadePanel( QPainter *p, const QRect &r,
		     const QColorGroup &, bool sunken=FALSE,
		     int lineWidth = 1, const QBrush *fill = 0 );

void drawWinButton( QPainter *p, int x, int y, int w, int h,
		    const QColorGroup &g, bool sunken = FALSE,
		    const QBrush *fill = 0 );

void drawWinButton( QPainter *p, const QRect &r,
		    const QColorGroup &g, bool sunken = FALSE,
		    const QBrush *fill = 0 );

void drawWinPanel( QPainter *p, int x, int y, int w, int h,
		   const QColorGroup &, bool sunken=FALSE,
		   const QBrush *fill = 0 );

void drawWinPanel( QPainter *p, const QRect &r,
		   const QColorGroup &, bool sunken=FALSE,
		   const QBrush *fill = 0 );

void drawPlainRect( QPainter *p, int x, int y, int w, int h, const QColor &,
		    int lineWidth = 1, const QBrush *fill = 0 );

void drawPlainRect( QPainter *p, const QRect &r, const QColor &,
		    int lineWidth = 1, const QBrush *fill = 0 );


#endif // QDRAWUTL_H
