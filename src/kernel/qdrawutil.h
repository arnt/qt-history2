/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qdrawutil.h#12 $
**
** Definition of draw utilities
**
** Created : 950920
**
** Copyright (C) 1995-1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QDRAWUTIL_H
#define QDRAWUTIL_H

#ifndef QT_H
#include "qpainter.h"
#include "qpalette.h"
#endif // QT_H


//
// Standard shade drawing
//

void qDrawShadeLine( QPainter *p, int x1, int y1, int x2, int y2,
		     const QColorGroup &g, bool sunken = TRUE,
		     int lineWidth = 1, int midLineWidth = 0 );

void qDrawShadeLine( QPainter *p, const QPoint &p1, const QPoint &p2,
		     const QColorGroup &g, bool sunken = TRUE,
		     int lineWidth = 1, int midLineWidth = 0 );

void qDrawShadeRect( QPainter *p, int x, int y, int w, int h,
		     const QColorGroup &, bool sunken=FALSE,
		     int lineWidth = 1, int midLineWidth = 0,
		     const QBrush *fill = 0 );

void qDrawShadeRect( QPainter *p, const QRect &r,
		     const QColorGroup &, bool sunken=FALSE,
		     int lineWidth = 1, int midLineWidth = 0,
		     const QBrush *fill = 0 );

void qDrawShadePanel( QPainter *p, int x, int y, int w, int h,
		      const QColorGroup &, bool sunken=FALSE,
		      int lineWidth = 1, const QBrush *fill = 0 );

void qDrawShadePanel( QPainter *p, const QRect &r,
		      const QColorGroup &, bool sunken=FALSE,
		      int lineWidth = 1, const QBrush *fill = 0 );

void qDrawWinButton( QPainter *p, int x, int y, int w, int h,
		     const QColorGroup &g, bool sunken = FALSE,
		     const QBrush *fill = 0 );

void qDrawWinButton( QPainter *p, const QRect &r,
		     const QColorGroup &g, bool sunken = FALSE,
		     const QBrush *fill = 0 );

void qDrawWinPanel( QPainter *p, int x, int y, int w, int h,
		    const QColorGroup &, bool sunken=FALSE,
		    const QBrush *fill = 0 );

void qDrawWinPanel( QPainter *p, const QRect &r,
		    const QColorGroup &, bool sunken=FALSE,
		    const QBrush *fill = 0 );

void qDrawPlainRect( QPainter *p, int x, int y, int w, int h, const QColor &,
		     int lineWidth = 1, const QBrush *fill = 0 );

void qDrawPlainRect( QPainter *p, const QRect &r, const QColor &,
		     int lineWidth = 1, const QBrush *fill = 0 );


//
// Other useful drawing functions
//

QRect qItemRect( QPainter *p, GUIStyle gs, int x, int y, int w, int h,
		int flags, bool enabled,
		const QPixmap *pixmap, const char *text, int len=-1 );

void qDrawItem( QPainter *p, GUIStyle gs, int x, int y, int w, int h,
		int flags, const QColorGroup &g, bool enabled,
		const QPixmap *pixmap, const char *text, int len=-1 );

enum ArrowType
    { UpArrow, DownArrow, LeftArrow, RightArrow };

void qDrawArrow( QPainter *p, ArrowType type, GUIStyle style, bool down,
		 int x, int y, int w, int h,
		 const QColorGroup &g, bool enabled );

#endif // QDRAWUTIL_H
