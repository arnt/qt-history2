/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qstyle.h#28 $
**
** Definition of QStyle class
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

#ifndef QSTYLE_H
#define QSTYLE_H

#ifndef QT_H
#include "qwindowdefs.h"
#include "qobject.h"
#endif // QT_H

class QButton;
class QPushButton;
class QScrollBar;
class QTabBar;
class QTab;
class QPopupMenu;
class QMenuItem;

class Q_EXPORT QStyle: public QObject
{
    Q_OBJECT
    GUIStyle gs;

private:
    QStyle(GUIStyle);
    QStyle();
    friend class QMotifStyle;
    friend class QWindowsStyle;
    friend class QPlatinumStyle;
public:
    virtual ~QStyle();





#ifndef NO_QT1_COMPAT
    operator GUIStyle() const { return gs; }
    int operator==(GUIStyle s) const { return gs==s; }
    int operator!=(GUIStyle s) const { return gs!=s; }
#endif

    GUIStyle guiStyle() const { return gs; }

    virtual void polish( QWidget* );
    virtual void unPolish( QWidget* );

    virtual void polish( QApplication*);
    virtual void unPolish( QApplication*);

    virtual void polish( QPalette&);

    virtual QRect itemRect( QPainter *p, int x, int y, int w, int h,
		    int flags, bool enabled,
		    const QPixmap *pixmap, const QString& text, int len=-1 );

    virtual void drawItem( QPainter *p, int x, int y, int w, int h,
		    int flags, const QColorGroup &g, bool enabled,
		    const QPixmap *pixmap, const QString& text,
			   int len=-1, const QColor* penColor = 0 );


    virtual void drawSeparator( QPainter *p, int x1, int y1, int x2, int y2,
		     const QColorGroup &g, bool sunken = TRUE,
		     int lineWidth = 1, int midLineWidth = 0 );

    virtual void drawRect( QPainter *p, int x, int y, int w, int h,
		    const QColor &, int lineWidth = 1,
		    const QBrush *fill = 0 );

    virtual void drawRectStrong( QPainter *p, int x, int y, int w, int h,
		     const QColorGroup &, bool sunken=FALSE,
		     int lineWidth = 1, int midLineWidth = 0,
		     const QBrush *fill = 0 );

    virtual void drawButton( QPainter *p, int x, int y, int w, int h,
		     const QColorGroup &g, bool sunken = FALSE,
		     const QBrush *fill = 0 ) = 0;

    virtual QRect buttonRect( int x, int y, int w, int h);

    virtual void drawButtonMask( QPainter *p, int x, int y, int w, int h);
			
    virtual void drawBevelButton( QPainter *p, int x, int y, int w, int h,
		     const QColorGroup &g, bool sunken = FALSE,
		     const QBrush *fill = 0 ) = 0;

    virtual void drawToolButton( QPainter *p, int x, int y, int w, int h,
		     const QColorGroup &g, bool sunken = FALSE,
		     const QBrush *fill = 0 );

    virtual void drawPanel( QPainter *p, int x, int y, int w, int h,
		    const QColorGroup &, bool sunken=FALSE,
		    int lineWidth = 1, const QBrush *fill = 0 );

    virtual void drawArrow( QPainter *p, Qt::ArrowType type, bool down,
		     int x, int y, int w, int h,
		     const QColorGroup &g, bool enabled, const QBrush *fill = 0 ) = 0;

    // "radio button"
    virtual QSize exclusiveIndicatorSize() const = 0;
    virtual void drawExclusiveIndicator( QPainter* p, int x, int y, int w, int h,
		    const QColorGroup &g, bool on, bool down = FALSE, bool enabled = TRUE ) = 0;
    virtual void drawExclusiveIndicatorMask( QPainter *p, int x, int y, int w, int h, bool on);

    // "check box"
    virtual QSize indicatorSize() const = 0;
    virtual void drawIndicator( QPainter* p, int x, int y, int w, int h, const QColorGroup &g,
				int state, bool down = FALSE, bool enabled = TRUE ) = 0;
    virtual void drawIndicatorMask( QPainter *p, int x, int y, int w, int h, int state);

    // "combo box"
    virtual void drawComboButton( QPainter *p, int x, int y, int w, int h,
				  const QColorGroup &g, bool sunken = FALSE,
				  bool editable = FALSE,
				  bool enabled = TRUE,
				  const QBrush *fill = 0 );
    virtual QRect comboButtonRect( int x, int y, int w, int h);
    virtual QRect comboButtonFocusRect( int x, int y, int w, int h);

    virtual void drawComboButtonMask( QPainter *p, int x, int y, int w, int h);


    // focus
    virtual void drawFocusRect( QPainter*, const QRect &,
				const QColorGroup &, const QColor* bg = 0,
				bool = FALSE ) = 0;

    // push buttons
    virtual void drawPushButton( QPushButton* btn, QPainter *p) = 0;
    virtual void drawPushButtonLabel( QPushButton* btn, QPainter *p);

    virtual void getButtonShift( int &x, int &y);

    // frame
    virtual int defaultFrameWidth() const;

    // tabbars
    virtual void tabbarMetrics( const QTabBar*, int&, int&, int& );
    virtual void drawTab( QPainter*, const QTabBar*, QTab*, bool selected );
    virtual void drawTabMask( QPainter*, const QTabBar*, QTab*, bool selected );

    // scrollbars
    enum ScrollControl { ADD_LINE = 0x1 , SUB_LINE = 0x2 , ADD_PAGE = 0x4,
			    SUB_PAGE = 0x8 , FIRST    = 0x10, LAST	= 0x20,
			    SLIDER   = 0x40, NONE     = 0x80 };

    virtual void scrollBarMetrics( const QScrollBar*, int&, int&, int&, int&) = 0;
    virtual void drawScrollBarControls( QPainter*,  const QScrollBar*, int sliderStart, uint controls, uint activeControl ) = 0;
    virtual ScrollControl scrollBarPointOver( const QScrollBar*, int sliderStart, const QPoint& );

    // sliders
    virtual int sliderLength() const = 0;
    virtual void drawSlider( QPainter *p,
			     int x, int y, int w, int h,
			     const QColorGroup &g,
			     Orientation, bool tickAbove, bool tickBelow) = 0;

    virtual void drawSliderMask( QPainter *p,
				 int x, int y, int w, int h,
				 Orientation, bool tickAbove, bool tickBelow);
    virtual void drawSliderGroove( QPainter *p,
				   int x, int y, int w, int h,
				   const QColorGroup& g, QCOORD c,
				   Orientation ) = 0;
    virtual void drawSliderGrooveMask( QPainter *p,
				       int x, int y, int w, int h,
				       QCOORD c,
				       Orientation );
    virtual int maximumSliderDragDistance() const;

    virtual int splitterWidth() const = 0;
    virtual void drawSplitter( QPainter *p,
			     int x, int y, int w, int h,
			     const QColorGroup &g,
			     Orientation) = 0;

    virtual void drawCheckMark( QPainter *p, int x, int y, int w, int h,
				const QColorGroup &g,
				bool act, bool dis ) = 0;
    virtual void polishPopupMenu( QPopupMenu* ) = 0;

    virtual int widthOfPopupCheckColumn( int maxpm ) = 0;
    virtual int extraPopupMenuItemWidth( bool checkable, QMenuItem* mi, const QFontMetrics& fm  ) = 0;
    virtual int popupMenuItemHeight( bool checkable, QMenuItem* mi, const QFontMetrics& fm  ) = 0;
    virtual void drawPopupMenuItem( QPainter* p, bool checkable, int tab, QMenuItem* mi, const QFontMetrics& fm,
				    bool act, int x, int y, int w, int h) = 0;

};



#endif // QSTYLE_H
