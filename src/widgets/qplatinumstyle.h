/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qplatinumstyle.h#4 $
**
** Definition of Platinum-like style class
**
** Created : 981231
**
** Copyright (C) 1998-2000 Troll Tech AS.  All rights reserved.
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

#ifndef QPLATINUMSTYLE_H
#define QPLATINUMSTYLE_H

#ifndef QT_H
#include "qwindowsstyle.h"
#include "qpalette.h"
#endif // QT_H

#if QT_FEATURE_STYLE_PLATINUM

class Q_EXPORT QPlatinumStyle : public QWindowsStyle
{
    Q_OBJECT
public:
    QPlatinumStyle();
    virtual ~QPlatinumStyle();
    void drawPopupPanel( QPainter *p, int x, int y, int w, int h,
			 const QColorGroup &,  int lineWidth = 2,
			 const QBrush *fill = 0 );
    void drawButton( QPainter *p, int x, int y, int w, int h,
		     const QColorGroup &g, bool sunken = FALSE,
		     const QBrush *fill = 0 );
    QRect buttonRect( int x, int y, int w, int h);
    void drawBevelButton( QPainter *p, int x, int y, int w, int h,
			  const QColorGroup &g, bool sunken = FALSE,
			  const QBrush *fill = 0 );
    void drawPushButton( QPushButton* btn, QPainter *p);
    void drawPushButtonLabel( QPushButton* btn, QPainter *p);
    void getButtonShift( int &x, int &y);

    void scrollBarMetrics( const QScrollBar*, int&, int&, int&, int&);
    void drawScrollBarControls( QPainter*,  const QScrollBar*, int sliderStart, uint controls, uint activeControl );
    ScrollControl scrollBarPointOver( const QScrollBar* sb, int sliderStart, const QPoint& p );

    QSize indicatorSize() const;
    void drawIndicator( QPainter* p, int x, int y, int w, int h,  const QColorGroup &g,
			int state, bool down = FALSE, bool enabled = TRUE );
    void drawIndicatorMask( QPainter *p, int x, int y, int w, int h, int state );

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
			     Orientation, bool tickAbove, bool tickBelow );
    void drawSliderMask( QPainter *p,
			 int x, int y, int w, int h,
			 Orientation, bool tickAbove, bool tickBelow);
    void drawSliderGroove( QPainter *p,
			   int x, int y, int w, int h,
			   const QColorGroup& g, QCOORD c,
			   Orientation );


    int maximumSliderDragDistance() const;

    void drawCheckMark( QPainter *p, int x, int y, int w, int h,
			     const QColorGroup &g,
			     bool act, bool dis );
    void polishPopupMenu( QPopupMenu* );

    int extraPopupMenuItemWidth( bool checkable, int maxpmw, QMenuItem* mi, const QFontMetrics& fm );
    int popupMenuItemHeight( bool checkable, QMenuItem* mi, const QFontMetrics& fm );
    void drawPopupMenuItem( QPainter* p, bool checkable, int maxpmw, int tab, QMenuItem* mi,
			    const QPalette& pal, bool act, bool enabled,
			    int x, int y, int w, int h);

protected:
    void drawScrollBarBackground( QPainter *p, int x, int y, int w, int h,
				  const QColorGroup &g, bool horizontal, const QBrush* fill = 0);
    QColor mixedColor(const QColor &, const QColor &);
    void drawRiffles( QPainter* p,  int x, int y, int w, int h,
		      const QColorGroup &g, bool horizontal );
private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QPlatinumStyle( const QPlatinumStyle & );
    QPlatinumStyle& operator=( const QPlatinumStyle & );
#endif
};

#endif // QT_FEATURE_STYLE_PLATINUM

#endif // QPLATINUMSTYLE_H
