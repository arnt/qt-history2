/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qstyle.h#92 $
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
class QSpinWidget;
class QTitleBar;
class QListViewItem;
class QStylePrivate;


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

    // New QStyle API - most of these should probably be pure virtual

    virtual void polish( QWidget * );
    virtual void unPolish( QWidget * );

    virtual void polish( QApplication * );
    virtual void unPolish( QApplication * );

    virtual void polish( QPalette & );

    virtual QRect itemRect( QPainter *p, const QRect &r,
			    int flags, bool enabled,
			    const QPixmap *pixmap,
			    const QString &text, int len = -1 ) const;

    virtual void drawItem( QPainter *p, const QRect &r,
			   int flags, const QColorGroup &g, bool enabled,
			   const QPixmap *pixmap, const QString &text,
			   int len = -1, const QColor *penColor = 0 ) const;


    enum PrimitiveOperation {
	PO_ButtonCommand,
	PO_ButtonBevel,
	PO_ButtonTool,

	PO_FocusRect,

	PO_ArrowUp,
	PO_ArrowDown,
	PO_ArrowRight,
	PO_ArrowLeft

	/*
	  PO_Panel,
	  PO_PanelPopup,
	  PO_PanelMenu,
	  PO_PanelToolbar,
	  PO_Indicator,
	  PO_IndicatorMask,
	  PO_ExclusiveIndicator,
	  PO_ExclusiveIndicatorMask,
	  PO_CheckMark,

	  PO_ScrollBarLineUp,
	  PO_ScrollBarLineDown,
	  PO_ScrollBarLinePageUp,
	  PO_ScrollBarLinePageDown,
	  PO_ScrollBarLinePageSlider
	*/
    };

    enum PrimitiveOperationFlags {
	PStyle_Default = 		0x00000000,
	PStyle_Enabled = 		0x00000001,
	PStyle_Sunken = 		0x00000002,
	PStyle_Off =			0x00000004,
	PStyle_NoChange =		0x00000008,
	PStyle_On =			0x00000010

	/*
	  PStyle_FocusHighlight=	0x00000001,
	  PStyle_FocusAtBorder=		0X00000002
	*/
    };
    typedef uint PFlags;

    virtual void drawPrimitive( PrimitiveOperation op,
				QPainter *p,
				const QRect &r,
				const QColorGroup &cg,
				PFlags flags = PStyle_Default,
				void *data = 0 ) const = 0;


    enum ControlElement{
	CE_PushButton,
	CE_PushButtonLabel,
	CE_PushButtonMask

	/*
	  CE_Tab,
	  CE_MenuBarItem,
	*/
    };

    enum ControlElementFlags{
	CStyle_Default = 		0x00000000

	/*
	  ,
	  CStyle_Selected =	 	0x00000001
	*/
    };
    typedef uint CFlags;

    virtual void drawControl( ControlElement element,
			      QPainter *p,
			      const QWidget *widget,
			      const QRect &r,
			      const QColorGroup &cg,
			      CFlags how = CStyle_Default,
			      void *data = 0 ) const = 0;

    enum SubRect {
	SR_PushButtonContents,
	SR_PushButtonFocusRect

	/*
	  SR_DefaultFrameContents,
	  SR_PopupFrameContents,
	  SR_MenuFrameContents,

	  SR_ButtonContents,
	  SR_BevelButtonContents
	  SR_ToolButtonContents,
	*/
    };

    virtual QRect subRect( SubRect r, const QWidget *widget ) const = 0;


    enum ComplexControl{
	CC_SpinWidget
	/*
	  CC_ScrollBar,
	  CC_Slider,
	  CC_MenuItem,
	  CC_ComboBox
	*/
    };

    enum SubControl {
	SC_None = 		0x00000000,

	/*
	  ,

	  SC_ScrollBarAddLine = 	0x00000001,
	  SC_ScrollBarSubLine = 	0x00000002,
	  SC_ScrollBarAddPage = 	0x00000004,
	  SC_ScrollBarSubPage = 	0x00000008,
	  SC_ScrollBarFirst = 	0x00000010,
	  SC_ScrollBarLast = 	0x00000020,
	  SC_ScrollBarSlider = 	0x00000040,
	  SC_ScrollBarNoScroll = 	0x00000080,
	*/
	SC_SpinWidgetUp = 		0x00000001,
	SC_SpinWidgetDown = 		0x00000002,
	SC_SpinWidgetFrame = 		0x00000004,
	SC_SpinWidgetEditField =	0x00000008,
	SC_SpinWidgetButtonField =	0x00000010

	/*
	  SC_MenuItemCheck =	0x00000001,
	  SC_MenuItemLabel =	0x00000002,
	  SC_MenuItemAccel =	0x00000004,
	  SC_MenuItemSubMenu =	0x00000008,

	  SC_ComboBoxEditField =	0x00000001,
	  SC_ComboBoxArrow =	0x00000002,
	  SC_ComboBoxFocusRect =	0x00000004
	*/
    };
    typedef uint SCFlags;


    virtual void drawComplexControl( ComplexControl control,
				     QPainter *p,
				     const QWidget *widget,
				     const QRect &r,
				     const QColorGroup &cg,
				     CFlags flags = CStyle_Default,
				     SCFlags sub = SC_None,
				     SCFlags subActive = SC_None,
				     void *data = 0 ) const = 0;

    virtual QRect querySubControlMetrics( ComplexControl control,
					  const QWidget *widget,
					  SubControl sc,
					  void *data = 0 ) const = 0;
    virtual SubControl querySubControl( ComplexControl control,
					const QWidget *widget,
					const QPoint &pos,
					void *data = 0 ) const = 0;


    enum PixelMetric {
	PM_ButtonMargin,
	PM_ButtonDefaultIndicator,
	PM_MenuButtonIndicator,
	PM_ButtonShiftHorizontal,
	PM_ButtonShiftVertical,

	PM_DefaultFrameWidth,
	PM_SpinBoxFrameWidth

	/*
	  PM_PopupFrameWidth,
	  PM_MenuFrameWidth,

	  PM_SplitterWidth,
	  PM_SliderLength,
	  PM_MaximimumSliderDragDistance,
	  PM_TabBarOverlap,

	  PM_TabBarBaseHeight,
	  PM_TabBarBaseOverlap,

	  PM_MenuItemSeparation,

	  PM_ComboBoxAdditionalWidth,
	  PM_ComboBoxAdditionalHeight,

	  PM_SpinWidgetAdditionalWidth,
	  PM_SpinWidgetAdditionalHeight,

	  PM_ScrollBarExtent
	*/
    };

    virtual int pixelMetric( PixelMetric metic,
			     const QWidget *widget = 0 ) const = 0;


    enum ContentsType {
	CT_PushButtonContents
    };

    virtual QSize sizeFromContents( ContentsType contents,
				    const QWidget *widget,
				    const QSize &contentsSize,
				    void *data = 0 ) const = 0;

    enum StyleHint  {
	SH_ScrollBarBackgroundMode

	/*
	  FH_TabBarCentered
	*/
    };

    virtual int styleHint( StyleHint f,
			   const QWidget *w = 0,
			   void **returnData = 0 ) const = 0;








    // Old 2.x QStyle API

#ifndef QT_NO_COMPAT
    operator GUIStyle() const { return gs; }
    bool operator==(GUIStyle s) const { return gs==s; }
    bool operator!=(GUIStyle s) const { return gs!=s; }
#endif

    GUIStyle guiStyle() const { return gs; }

    // abstract section

    //
    // the new API still has these
    //
    // virtual void polish( QWidget* );
    // virtual void unPolish( QWidget* );
    // virtual void polish( QApplication*);
    // virtual void unPolish( QApplication*);
    // virtual void polish( QPalette&);

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

    // frame
    virtual int defaultFrameWidth() const;

    // tab bars
    virtual void tabbarMetrics( const QTabBar*,
				int&, int&, int& ) const = 0;
    virtual void drawTab( QPainter*, const QTabBar*, QTab*,
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
    virtual int sliderThickness() const;
    virtual void drawSlider( QPainter *p,
			     int x, int y, int w, int h,
			     const QColorGroup &g,
			     Orientation, bool tickAbove, bool tickBelow) = 0;

    virtual void drawSliderGroove( QPainter *p,
				   int x, int y, int w, int h,
				   const QColorGroup& g, QCOORD c,
				   Orientation ) = 0;
    virtual int maximumSliderDragDistance() const;

    // splitter
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
    enum TitleControl { TitleNone = 0x00,
			TitleSysMenu = 0x1 , TitleMinButton  = 0x2 , TitleMaxButton = 0x4,
			TitleCloseButton = 0x8 , TitleLabel = 0x10, TitleNormalButton = 0x20,
			TitleShadeButton=0x40, TitleUnshadeButton=0x80 };
    virtual QPixmap titleBarPixmap( const QTitleBar *, TitleControl ) = 0;
    virtual void titleBarMetrics( const QTitleBar*, int&, int&, int&, int&) const = 0;
    virtual void drawTitleBarControls( QPainter*,  const QTitleBar*,
				       uint controls, uint activeControl ) = 0;
    virtual TitleControl titleBarPointOver( const QTitleBar*, const QPoint& ) = 0;

    // listviewitem
    enum ListViewItemControl { ListViewNone = 0x00,
			       ListViewCheckBox = 0x1, ListViewController = 0x2, ListViewRadio = 0x4,
			       ListViewBranches = 0x8, ListViewExpand =0x10 };
    virtual void drawListViewItemBranch( QPainter *, int, int, int, const QColorGroup & cg, QListViewItem * ) = 0;
    virtual ListViewItemControl listViewItemPointOver( const QListViewItem *, const QPoint & ) = 0;

    // header
    virtual void drawHeaderSection( QPainter *p, int x, int y, int w, int h, const QColorGroup &g, bool down ) = 0;

    // spinbox
    virtual int spinBoxFrameWidth() const = 0;

    // spin widget
    virtual void drawSpinWidgetButton( QPainter *p, int x, int y, int w, int h,
				       const QColorGroup &g, QSpinWidget* sw,
				       bool downbtn, bool enabled, bool down ) = 0;
    virtual void drawSpinWidgetSymbol( QPainter *p, int x, int y, int w, int h,
				       const QColorGroup &g, QSpinWidget* sw,
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


protected:
    static const QWidget *contextWidget();


private:
    QStylePrivate * d;

#if defined(Q_DISABLE_COPY)
    QStyle( const QStyle & );
    QStyle& operator=( const QStyle & );
#endif
};

#endif // QT_NO_STYLE
#endif // QSTYLE_H
