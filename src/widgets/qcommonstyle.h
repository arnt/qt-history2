/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qcommonstyle.h#2 $
**
** Definition of QCommonStyle class
**
** Created : 980616
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QCOMMONSTYLE_H
#define QCOMMONSTYLE_H

#include "qstyle.h"

class Q_EXPORT QCommonStyle: public QStyle
{
    Q_OBJECT
private:
    QCommonStyle(GUIStyle);
    ~QCommonStyle();
  
    friend class QMotifStyle;
    friend class QWindowsStyle;
    friend class QPlatinumStyle;
public:

    // "combo box"
    void drawComboButton( QPainter *p, int x, int y, int w, int h,
				  const QColorGroup &g, bool sunken = FALSE,
				  bool editable = FALSE,
				  bool enabled = TRUE,
				  const QBrush *fill = 0 );
    QRect comboButtonRect( int x, int y, int w, int h);
    QRect comboButtonFocusRect( int x, int y, int w, int h);

    void drawComboButtonMask( QPainter *p, int x, int y, int w, int h);



    // push buttons
    void drawPushButtonLabel( QPushButton* btn, QPainter *p);

    void getButtonShift( int &x, int &y);

    // frame
    int defaultFrameWidth() const;

    // tabbars
    void tabbarMetrics( const QTabBar*, int&, int&, int& );
    void drawTab( QPainter*, const QTabBar*, QTab*, bool selected );
    void drawTabMask( QPainter*, const QTabBar*, QTab*, bool selected );

    // scrollbars
    ScrollControl scrollBarPointOver( const QScrollBar*, int sliderStart, const QPoint& );

    // sliders
    void drawSliderMask( QPainter *p,
				 int x, int y, int w, int h,
				 Orientation, bool tickAbove, bool tickBelow);
    void drawSliderGrooveMask( QPainter *p,
				       int x, int y, int w, int h,
				       QCOORD c,
				       Orientation );
    int maximumSliderDragDistance() const;
};



#endif // QCOMMONSTYLE_H
