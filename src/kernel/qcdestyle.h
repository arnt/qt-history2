/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qcdestyle.h#2 $
**
** Definition of something or other
**
** Created : 979899
**
** Copyright (C) 1997 by Troll Tech AS.  All rights reserved.
**
****************************************************************************/

#ifndef QCDESTYLE_H
#define QCDESTYLE_H

#include "qmotifstyle.h"

class Q_EXPORT QCDEStyle : public QMotifStyle
{
public:
    QCDEStyle( bool useHighlightCols = FALSE );

    int defaultFrameWidth() const;

    void drawArrow( QPainter *p, ArrowType type, bool down,
		    int x, int y, int w, int h,
		    const QColorGroup &g, bool enabled, const QBrush *fill = 0 );

    void drawIndicator( QPainter* p, int x, int y, int w, int h,  const QColorGroup &g,
			bool on, bool down = FALSE, bool enabled = TRUE );

    void drawExclusiveIndicator( QPainter* p,  int x, int y, int w, int h, const QColorGroup &g,
				 bool on, bool down = FALSE, bool enabled = TRUE );
};

#endif
