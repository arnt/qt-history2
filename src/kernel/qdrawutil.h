/****************************************************************************
**
** Definition of draw utilities.
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

#ifndef QDRAWUTIL_H
#define QDRAWUTIL_H

#ifndef QT_H
#include "qnamespace.h"
#include "qstring.h" // char*->QString conversion
#endif // QT_H

class QPainter;
#ifdef QT_NO_COMPAT
class QColorGroup;
#endif
class QPalette;
class QPoint;
class QBrush;
class QRect;
class QPixmap;

#ifndef QT_NO_DRAWUTIL
//
// Standard shade drawing
//

Q_EXPORT void qDrawShadeLine( QPainter *p, int x1, int y1, int x2, int y2,
			      const QPalette &pal, bool sunken = TRUE,
			      int lineWidth = 1, int midLineWidth = 0 );

Q_EXPORT void qDrawShadeLine( QPainter *p, const QPoint &p1, const QPoint &p2,
			      const QPalette &pal, bool sunken = TRUE,
			      int lineWidth = 1, int midLineWidth = 0 );

Q_EXPORT void qDrawShadeRect( QPainter *p, int x, int y, int w, int h,
			      const QPalette &pal, bool sunken = FALSE,
			      int lineWidth = 1, int midLineWidth = 0,
			      const QBrush *fill = 0 );

Q_EXPORT void qDrawShadeRect( QPainter *p, const QRect &r,
			      const QPalette &pal, bool sunken = FALSE,
			      int lineWidth = 1, int midLineWidth = 0,
			      const QBrush *fill = 0 );

Q_EXPORT void qDrawShadePanel( QPainter *p, int x, int y, int w, int h,
			       const QPalette &pal, bool sunken = FALSE,
			       int lineWidth = 1, const QBrush *fill = 0 );

Q_EXPORT void qDrawShadePanel( QPainter *p, const QRect &r,
			       const QPalette &pal, bool sunken = FALSE,
			       int lineWidth = 1, const QBrush *fill = 0 );

Q_EXPORT void qDrawWinButton( QPainter *p, int x, int y, int w, int h,
			      const QPalette &pal, bool sunken = FALSE,
			      const QBrush *fill = 0 );

Q_EXPORT void qDrawWinButton( QPainter *p, const QRect &r,
			      const QPalette &pal, bool sunken = FALSE,
			      const QBrush *fill = 0 );

Q_EXPORT void qDrawWinPanel( QPainter *p, int x, int y, int w, int h,
			      const QPalette &pal, bool sunken = FALSE,
			     const QBrush *fill = 0 );

Q_EXPORT void qDrawWinPanel( QPainter *p, const QRect &r,
			      const QPalette &pal, bool sunken = FALSE,
			     const QBrush *fill = 0 );

Q_EXPORT void qDrawPlainRect( QPainter *p, int x, int y, int w, int h, const QColor &,
			      int lineWidth = 1, const QBrush *fill = 0 );

Q_EXPORT void qDrawPlainRect( QPainter *p, const QRect &r, const QColor &,
			      int lineWidth = 1, const QBrush *fill = 0 );


//
// Other obsolete drawing functions.
// Use QStyle::itemRect(), QStyle::drawItem() and QStyle::drawArrow() instead.
//
Q_EXPORT QRect qItemRect( QPainter *p, Qt::GUIStyle gs, int x, int y, int w, int h,
			  int flags, bool enabled,
			  const QPixmap *pixmap, const QString& text, int len=-1 );

Q_EXPORT void qDrawItem( QPainter *p, Qt::GUIStyle gs, int x, int y, int w, int h,
			 int flags, const QPalette &pal, bool enabled,
			 const QPixmap *pixmap, const QString& text,
			 int len=-1, const QColor* penColor = 0 );

Q_EXPORT void qDrawArrow( QPainter *p, Qt::ArrowType type, Qt::GUIStyle style, bool down,
			  int x, int y, int w, int h,
			  const QPalette &pal, bool enabled );

#endif // QT_NO_DRAWUTIL
#endif // QDRAWUTIL_H
