/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qwindowsstyle.h#12 $
**
** Definition of something or other
**
** Created : 979899
**
** Copyright (C) 1997 by Troll Tech AS.  All rights reserved.
**
****************************************************************************/

#ifndef QWINDOWSSTYLE_H
#define QWINDOWSSTYLE_H

#include "qstyle.h"

class Q_EXPORT QWindowsStyle : public QStyle
{
    Q_OBJECT
public:
    QWindowsStyle();
    void drawButton( QPainter *p, int x, int y, int w, int h,
		     const QColorGroup &g, bool sunken = FALSE,
		     const QBrush *fill = 0 );
    void drawBevelButton( QPainter *p, int x, int y, int w, int h,
			  const QColorGroup &g, bool sunken = FALSE,
			  const QBrush *fill = 0 );

    void drawFocusRect( QPainter*,
			const QRect&, const QColorGroup &, const QColor* =0,  bool = FALSE );

    void drawPushButton( QPushButton* btn, QPainter *p);

    void getButtonShift( int &x, int &y);

    void drawPanel( QPainter *p, int x, int y, int w, int h,
		    const QColorGroup &, bool sunken=FALSE,
		    int lineWidth = 1, const QBrush *fill = 0 );

    void drawArrow( QPainter *p, ArrowType type, bool down,
		    int x, int y, int w, int h,
		    const QColorGroup &g, bool enabled, const QBrush *fill = 0 );

    QSize indicatorSize() const;
    void drawIndicator( QPainter* p,  int x, int y, int w, int h, const QColorGroup &g,
			int s, bool down = FALSE, bool enabled = TRUE );

    QSize exclusiveIndicatorSize() const;
    void drawExclusiveIndicator( QPainter* p,  int x, int y, int w, int h, const QColorGroup &g,
				 bool on, bool down = FALSE, bool enabled = TRUE );
    void drawExclusiveIndicatorMask( QPainter *p, int x, int y, int w, int h, bool on);


    void drawComboButton( QPainter *p, int x, int y, int w, int h,
			  const QColorGroup &g, bool sunken = FALSE,
			  bool editable = FALSE,
			  bool enabled = TRUE,
			  const QBrush *fill = 0 );
    QRect comboButtonRect( int x, int y, int w, int h);
    QRect comboButtonFocusRect( int x, int y, int w, int h);

    void tabbarMetrics( const QTabBar*, int&, int&, int& );
    void drawTab( QPainter*, const QTabBar*, QTab*, bool selected );
    void drawTabMask( QPainter*, const QTabBar*, QTab*, bool selected );
    
    void scrollBarMetrics( const QScrollBar*, int&, int&, int&, int&);
    void drawScrollBarControls( QPainter*,  const QScrollBar*, int sliderStart, uint controls, uint activeControl );


    int sliderLength() const;
    void drawSlider( QPainter *p,
			     int x, int y, int w, int h,
			     const QColorGroup &g,
			     Orientation, bool tickAbove, bool tickBelow);
    void drawSliderMask( QPainter *p,
			 int x, int y, int w, int h,
			 Orientation, bool tickAbove, bool tickBelow);
    void drawSliderGroove( QPainter *p,
			   int x, int y, int w, int h,
			   const QColorGroup& g, QCOORD c,
			   Orientation );

    int maximumSliderDragDistance() const;

    int splitterWidth() const;
    void drawSplitter( QPainter *p, int x, int y, int w, int h,
			     const QColorGroup &g, Orientation);

protected:
    void drawWinShades( QPainter *p,
			int x, int y, int w, int h,
			const QColor &c1, const QColor &c2,
			const QColor &c3, const QColor &c4,
			const QBrush *fill );

};

#endif
