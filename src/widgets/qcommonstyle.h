/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qcommonstyle.h#4 $
**
** Definition of QCommonStyle class
**
** Created : 980616
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
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
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QCOMMONSTYLE_H
#define QCOMMONSTYLE_H

#ifndef QT_H
#include "qstyle.h"
#endif // QT_H

#ifndef QT_NO_COMPLEXWIDGETS

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
    ScrollControl scrollBarPointOver( const QScrollBar* sb, int sliderStart, const QPoint& p );

    // sliders
    void drawSliderMask( QPainter *p,
				 int x, int y, int w, int h,
				 Orientation, bool tickAbove, bool tickBelow);
    void drawSliderGrooveMask( QPainter *p,
				       int x, int y, int w, int h,
				       QCOORD c,
				       Orientation );
    int maximumSliderDragDistance() const;

    // popups
    int popupSubmenuIndicatorWidth( const QFontMetrics& fm  );

    void drawMenuBarItem( QPainter* p, int x, int y, int w, int h,
				    QMenuItem* mi, QColorGroup& g,
				    bool enabled );

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QCommonStyle( const QCommonStyle & );
    QCommonStyle &operator=( const QCommonStyle & );
#endif
};



#endif // QT_NO_COMPLEXWIDGETS

#endif // QCOMMONSTYLE_H
