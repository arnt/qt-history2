/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qstyle.h#37 $
**
** Definition of QStyle class
**
** Created : 980616
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
#ifndef QSTYLE_H
#define QSTYLE_H

#ifndef QT_H
#include "qwindowdefs.h"
#include "qobject.h"
#endif // QT_H

#ifndef QT_NO_STYLE
class QButton;
class QPushButton;
class QScrollBar;
class QTabBar;
class QTab;
class QPopupMenu;
class QMenuItem;
class QToolButton;
class QTabWidget;
class QSpinBox;
class QGroupBox;

class Q_EXPORT QStyle: public QObject
{
    Q_OBJECT
    GUIStyle gs;

private:
    QStyle(GUIStyle);
    QStyle();
    friend class QCommonStyle;

public:
    virtual ~QStyle();

#ifndef QT_NO_COMPAT
    operator GUIStyle() const { return gs; }
    bool operator==(GUIStyle s) const { return gs==s; }
    bool operator!=(GUIStyle s) const { return gs!=s; }
#endif

    GUIStyle guiStyle() const { return gs; }

    // abstract section

    virtual void polish( QWidget* );
    virtual void unPolish( QWidget* );

    virtual void polish( QApplication*);
    virtual void unPolish( QApplication*);

    virtual void polish( QPalette&);

    virtual QRect itemRect( QPainter *p, int x, int y, int w, int h,
		    int flags, bool enabled,
		    const QPixmap *pixmap,
		    const QString& text, int len=-1 ) const;

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

    //virtual void drawShade(...); // ### add 3.0

    virtual QRect buttonRect( int x, int y, int w, int h) const;

    virtual void drawButtonMask( QPainter *p, int x, int y, int w, int h);

    virtual void drawBevelButton( QPainter *p, int x, int y, int w, int h,
		     const QColorGroup &g, bool sunken = FALSE,
		     const QBrush *fill = 0 ) = 0;

    virtual QRect bevelButtonRect( int x, int y, int w, int h) const;

    virtual void drawPanel( QPainter *p, int x, int y, int w, int h,
		    const QColorGroup &, bool sunken=FALSE,
		    int lineWidth = 1, const QBrush *fill = 0 );

    virtual void drawPopupPanel( QPainter *p, int x, int y, int w, int h,
				 const QColorGroup &,  int lineWidth = 2,
				 const QBrush *fill = 0 );

    virtual void drawArrow( QPainter *p, Qt::ArrowType type, bool down,
		     int x, int y, int w, int h,
		     const QColorGroup &g, bool enabled, const QBrush *fill = 0 ) = 0;

    // toolbutton
    virtual void drawToolButton( QPainter *p, int x, int y, int w, int h,
		     const QColorGroup &g, bool sunken, 
		     const QBrush *fill = 0 );
    virtual void drawToolButton( QPainter *p, int x, int y, int w, int h,
		     const QColorGroup &g, bool on, bool down, bool enabled, bool autoRaised = FALSE,
		     const QBrush *fill = 0 );
    virtual QRect toolButtonRect(  int x, int y, int w, int h);
    virtual void drawDropDownButton( QPainter *p, int x, int y, int w, int h,
		     const QColorGroup &g, bool down, bool enabled, bool autoRaised = FALSE,
		     const QBrush *fill = 0 );

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


    // focus
    virtual void drawFocusRect( QPainter*, const QRect &,
				const QColorGroup &, const QColor* bg = 0,
				bool = FALSE ) = 0;


    // concrete section depending on Qt's widget cluster ( The line is
    // hard to draw sometimes)

    // "combo box"
    // virtual void drawComboButton( QComboBox* cbx, QPainter* p ); // ### add, 3.0
    virtual void drawComboButton( QPainter *p, int x, int y, int w, int h,
				  const QColorGroup &g, bool sunken = FALSE,
				  bool editable = FALSE,
				  bool enabled = TRUE,
				  const QBrush *fill = 0 );
    virtual QRect comboButtonRect( int x, int y, int w, int h) const;
    virtual QRect comboButtonFocusRect( int x, int y, int w, int h) const;
    virtual void drawComboButtonMask( QPainter *p, int x, int y, int w, int h);

    // push buttons
    virtual void drawPushButton( QPushButton* btn, QPainter *p) = 0;
    /// ### add this in 3.0
    // virtual void drawPushButtonMask(QPushButton *btn, QPainter *p) = 0;
    virtual void drawPushButtonLabel( QPushButton* btn, QPainter *p) = 0;
    virtual QRect pushButtonContentsRect( QPushButton* btn ) const = 0;
    virtual int menuButtonIndicatorWidth( int h ) const;
    virtual void getButtonShift( int &x, int &y) const;

    // frame
    virtual int defaultFrameWidth() const;

    // tab bars
    virtual void tabbarMetrics( const QTabBar*,
				int&, int&, int& ) const = 0;
    virtual void drawTab( QPainter*, const QTabBar*, QTab*,
			  bool selected ) = 0;
    virtual void drawTabMask( QPainter*, const QTabBar*, QTab*,
			      bool selected ) = 0;
    virtual void tabBarExtensionMetrics( const QTabWidget *, int & w, int & h,
					 int & overlap ) const;
    virtual void drawTabBarExtension( QPainter * p, int x, int y, int w, int h,
				      const QColorGroup & cg,
				      const QTabWidget * tw );

    // scrollbars
    enum ScrollControl { AddLine = 0x1 , SubLine  = 0x2 , AddPage = 0x4,
			 SubPage = 0x8 , First = 0x10, Last = 0x20,
			 Slider  = 0x40, NoScroll = 0x80 };

    virtual void scrollBarMetrics( const QScrollBar*,
		    int&, int&, int&, int&) const = 0;
    virtual void drawScrollBarControls( QPainter*,  const QScrollBar*,
					int sliderStart, uint controls,
					uint activeControl ) = 0;
    virtual ScrollControl scrollBarPointOver( const QScrollBar*,
					int sliderStart, const QPoint& ) = 0;

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

    // popup menus
    virtual void drawCheckMark( QPainter *p, int x, int y, int w, int h,
				const QColorGroup &g,
				bool act, bool dis ) = 0;
    virtual void polishPopupMenu( QPopupMenu* ) = 0;

    virtual int extraPopupMenuItemWidth( bool checkable, int maxpmw,
				QMenuItem* mi,
				const QFontMetrics& fm ) const = 0;
    virtual int popupSubmenuIndicatorWidth(
				const QFontMetrics& fm ) const;
    virtual int popupMenuItemHeight( bool checkable,
				QMenuItem* mi,
				const QFontMetrics& fm ) const = 0;
    virtual void drawPopupMenuItem( QPainter* p, bool checkable,
				    int maxpmw, int tab, QMenuItem* mi,
				    const QPalette& pal,
				    bool act, bool enabled,
				    int x, int y, int w, int h) = 0;
    virtual QSize scrollBarExtent() const;
    virtual int   buttonDefaultIndicatorWidth() const;
    virtual int   buttonMargin() const;
    virtual int   sliderThickness() const;

    // menu bars
    virtual void drawMenuBarItem( QPainter* p, int x, int y, int w, int h,
				  QMenuItem* mi, QColorGroup& g, bool active,
				  bool down, bool hasFocus = FALSE ) = 0;
    virtual int  menuBarFrameWidth() const;
    virtual void drawMenuBarPanel( QPainter *p, int x, int y, int w, int h,
				   const QColorGroup &, const QBrush *fill = 0 );

    // tool bars
    virtual int  toolBarHandleExtent() const;
    virtual void drawToolBarHandle( QPainter *p, const QRect &r,
				    Qt::Orientation orientation,
				    bool highlight, const QColorGroup &cg,
				    bool drawBorder = FALSE );
    virtual int  toolBarFrameWidth() const;
    virtual void drawToolBarPanel( QPainter *p, int x, int y, int w, int h,
				   const QColorGroup &, const QBrush *fill = 0 );
    virtual void drawToolBarSeparator( QPainter *p, int x, int y, int w, int h,
				       const QColorGroup & g,
				       Orientation orientation );
    virtual QSize toolBarSeparatorSize( Qt::Orientation orientation ) const;

    // title bar
    virtual void drawTitleBar( QPainter *p, int x, int y, int w, int h, 
			       const QColor &left, const QColor &right, 
			       bool active ) = 0;
    virtual void drawTitleBarLabel( QPainter *p, int x, int y, int w, int h, 
			       const QString &text, const QColor &tc, bool active ) = 0;
    virtual void drawTitleBarButton( QPainter *p, int x, int y, int w, int h, const QColorGroup &g, bool down ) = 0;
    virtual void drawTitleBarButtonLabel( QPainter *p, int x, int y, int w, int h, const QPixmap *, int button, bool down ) = 0;

    // header
    virtual void drawHeaderSection( QPainter *p, int x, int y, int w, int h, const QColorGroup &g, bool down ) = 0;

    // spinbox
    virtual int spinBoxFrameWidth() const = 0;
    virtual void drawSpinBoxButton( QPainter *p, int x, int y, int w, int h, const QColorGroup &g, const QSpinBox *sp, 
				bool downbtn, bool enabled, bool down ) = 0;
    virtual void drawSpinBoxSymbol( QPainter *p, int x, int y, int w, int h, const QColorGroup &g, const QSpinBox *sp,
				bool downbtn, bool enabled, bool down ) = 0;

    // groupbox
    virtual void drawGroupBoxTitle( QPainter *p,int x, int y, int w, int h, const QColorGroup &g, const QString &text, bool enabled ) = 0;
    virtual void drawGroupBoxFrame( QPainter *p, int x, int y, int w, int h, const QColorGroup &g, const QGroupBox *gb ) = 0;

    // statusbar
    virtual void drawStatusBarSection( QPainter *p, int x, int y, int w, int h, const QColorGroup &g, bool permanent ) = 0;
    virtual void drawSizeGrip( QPainter *p, int x, int y, int w, int h, const QColorGroup &g ) = 0;

    // progressbar
    virtual int progressChunkWidth() const = 0;
    virtual void drawProgressBar( QPainter *p, int x, int y, int w, int h, const QColorGroup &g ) = 0;
    virtual void drawProgressChunk( QPainter *p, int x, int y, int w, int h, const QColorGroup &g ) = 0;

private:
    class Private;
    Private * d;

private:        // Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QStyle( const QStyle & );
    QStyle& operator=( const QStyle & );
#endif
};

#endif // QT_NO_STYLE
#endif // QSTYLE_H
