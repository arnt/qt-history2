/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qplatinumstyle.h#3 $
**
** Definition of something or other
**
** Created : 979899
**
** Copyright (C) 1997 by Troll Tech AS.  All rights reserved.
**
****************************************************************************/

#ifndef QPLATINUMSTYLE_H
#define QPLATINUMSTYLE_H

#include "qwindowsstyle.h"

class Q_EXPORT QPlatinumStyle : public QWindowsStyle
{
public:
    QPlatinumStyle();
    void drawButton( QPainter *p, int x, int y, int w, int h,
		     const QColorGroup &g, bool sunken = FALSE,
		     const QBrush *fill = 0 );
    QRect buttonRect( int x, int y, int w, int h);
    void drawBevelButton( QPainter *p, int x, int y, int w, int h,
			  const QColorGroup &g, bool sunken = FALSE,
			  const QBrush *fill = 0 );
    void drawPushButton( QPushButton* btn, QPainter *p);
    void drawPushButtonLabel( QPushButton* btn, QPainter *p);

    void scrollbarMetrics( const QScrollBar*,  int *, int *, int * );
    void drawScrollbarControls( QPainter*,  const QScrollBar*, int sliderStart, uint controls, uint activeControl );

    QSize indicatorSize() const;
    void drawIndicator( QPainter* p, int x, int y, int w, int h,  const QColorGroup &g,
			bool on, bool down = FALSE, bool enabled = TRUE );
    void drawIndicatorMask( QPainter *p, int x, int y, int w, int h, bool on);

    QSize exclusiveIndicatorSize() const;
    void drawExclusiveIndicator( QPainter* p,  int x, int y, int w, int h, const QColorGroup &g,
				 bool on, bool down = FALSE, bool enabled = TRUE );

    void drawComboButton( QPainter *p, int x, int y, int w, int h,
			  const QColorGroup &g, bool sunken = FALSE,
			  bool editable = FALSE,
			  bool enabled = TRUE,
			  const QBrush *fill = 0 );
    QRect comboButtonRect( int x, int y, int w, int h);
    QRect comboButtonFocusRect( int x, int y, int w, int h);

    int sliderLength() const;
    void drawSlider( QPainter *p,
			     int x, int y, int w, int h,
			     const QColorGroup &g,
			     SliderDirection dir);
    void drawSliderGroove( QPainter *p,
			   int x, int y, int w, int h,
			   const QColorGroup& g, QCOORD c,
			   bool horizontal );


    int maximumSliderDragDistance() const;

    void initialize( QApplication*);
    //     void polish( QWidget* );

protected:
    void drawScrollbarBackground( QPainter *p, int x, int y, int w, int h,
				  const QColorGroup &g, bool horizontal, const QBrush* fill = 0);
    QColor mixedColor(const QColor &, const QColor &);
    void drawRiffles( QPainter* p,  int x, int y, int w, int h,
		      const QColorGroup &g, bool horizontal );
};


#endif
