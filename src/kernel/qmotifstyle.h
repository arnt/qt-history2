/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qmotifstyle.h#13 $
**
** Definition of something or other
**
** Created : 979899
**
** Copyright (C) 1997 by Troll Tech AS.  All rights reserved.
**
****************************************************************************/

#ifndef QMOTIFSTYLE_H
#define QMOTIFSTYLE_H

#include "qstyle.h"
#include "qpalette.h"

class Q_EXPORT QMotifStyle : public QStyle
{
    Q_OBJECT
public:
    QMotifStyle( bool useHighlightCols = FALSE);

    void setUseHighlightColors( bool );
    bool useHighlightColors() const;

    void polish( QPalette&);

    void drawButton( QPainter *p, int x, int y, int w, int h,
		     const QColorGroup &g, bool sunken = FALSE,
		     const QBrush *fill = 0 );
    void drawBevelButton( QPainter *p, int x, int y, int w, int h,
			  const QColorGroup &g, bool sunken = FALSE,
			  const QBrush *fill = 0 );
    void drawFocusRect( QPainter*,
			const QRect&, const QColorGroup &, const QColor* =0, bool = FALSE );

    void drawPushButton( QPushButton* btn, QPainter *p);

    void drawArrow( QPainter *p, ArrowType type, bool down,
		    int x, int y, int w, int h,
		    const QColorGroup &g, bool enabled, const QBrush *fill = 0 );
    QSize indicatorSize() const;
    void drawIndicator( QPainter* p, int x, int y, int w, int h,  const QColorGroup &g,
			int state, bool down = FALSE, bool enabled = TRUE );


    QSize exclusiveIndicatorSize() const;
    void drawExclusiveIndicator( QPainter* p,  int x, int y, int w, int h, const QColorGroup &g,
				 bool on, bool down = FALSE, bool enabled = TRUE );
    void drawExclusiveIndicatorMask( QPainter *p, int x, int y, int, int, bool );
    void scrollBarMetrics( const QScrollBar*, int&, int&, int&, int&);
    void drawScrollBarControls( QPainter*,  const QScrollBar*, int sliderStart, uint controls, uint activeControl );

    int sliderLength() const;
    void drawSlider( QPainter *p,
			     int x, int y, int w, int h,
			     const QColorGroup &g,
			     Orientation, bool tickAbove, bool tickBelow);
    void drawSliderGroove( QPainter *p,
			   int x, int y, int w, int h,
			   const QColorGroup& g, QCOORD c,
			   Orientation );

    int splitterWidth() const;
    void drawSplitter( QPainter *p, int x, int y, int w, int h,
		       const QColorGroup &g, Orientation);
private:
    bool highlightCols;
};

#endif
