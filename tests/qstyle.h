/****************************************************************************
**
** Definition of QStyle class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
#ifndef QSTYLE_H
#define QSTYLE_H

#ifndef QT_H
#include "qwindowdefs.h"
#include "qobject.h"
#endif // QT_H

#ifndef QT_NO_STYLE

class Q_EXPORT QStyle: public QObject
{
    Q_OBJECT

private:
    QStyle(GUIStyle);
    QStyle();
    friend class QCommonStyle;

public:
    virtual ~QStyle();


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

    enum PrimitiveOperation {
	PO_ArrowUp,
	PO_ArrowDown,
	PO_ArrowRight,
	PO_ArrowLeft,
	PO_Button,
	PO_ButtonBevel,
	PO_ButtonTool,
	PO_Panel,
	PO_PanelPopup,
	PO_PanelMenu,
	PO_PanelToolbar,
	PO_Indicator,
	PO_IndicatorMask,
	PO_ExclusiveIndicator,
	PO_ExclusiveIndicatorMask,
	PO_FocusRect,
	PO_CheckMark,
	
	PO_ScrollBarLineUp,
	PO_ScrollBarLineDown,
	PO_ScrollBarLinePageUp,
	PO_ScrollBarLinePageDown,
	PO_ScrollBarLinePageSlider
	
    };

    enum PrimitiveOperationFlags {
	PStyle_Default = 		0x00000000,
	PStyle_Enabled = 		0x00000001,
	PStyle_Sunken = 		0x00000002,
	PStyle_Off = 		0x00000004,
	PStyle_NoChange=		0x00000008,
	PStyle_On=		0x00000010,
	
	PStyle_FocusHighlight=	0x00000001,
	PStyle_FocusAtBorder=	0x00000002
    };
    typedef uint Pflags

    virtual void drawPrimitive( PrimitiveOperation op,
				QPainter *p,
				const QRect& r,
				const QColorGroup &cg,
				PFlags flags = PStyle_Default,
				void* data = 0 );

    enum ControlElement{
	CE_Tab,
	CE_MenuBarItem,
	CE_PushButton,
	CE_PushButtonLabel // support QPushButton::drawButtonLabel() function for subclasses
    }

    enum ControlElementFlags{
	CStyle_Default = 		0x00000000,
	CStyle_Selected =	 	0x00000001
    }
    typedef uint CFlags;

    virtual void drawControl( ControlElement element,
			      QPainter* p,
			      QWidget* w,
			      const QColorGroup& cg,
			      const QRect& r,
			      CFlags how = CStyle_Default,
			      void* data = 0 );


    enum SubRect {
	SR_DefaultFrameContents,
	SR_PopupFrameContents,
	SR_MenuFrameContents,
	
	SR_ButtonContents,
	SR_BevelButtonContents,
	SR_PushButtonContents,
	SR_ToolButtonContents,
    }

    virtual QRect subRect( SubRect r, QWidget* w );

    enum ComplexControl{
	CC_ScrollBar,
	CC_SpinWidget,
	CC_Slider,
	CC_MenuItem,
	CC_ComboBox
    }

    enum SubControl {
	SC_None = 		0x00000000,
	
	SC_ScrollBarAddLine = 	0x00000001,
	SC_ScrollBarSubLine = 	0x00000002,
	SC_ScrollBarAddPage = 	0x00000004,
	SC_ScrollBarSubPage = 	0x00000008,
	SC_ScrollBarFirst = 	0x00000010,
	SC_ScrollBarLast = 	0x00000020,
	SC_ScrollBarSlider = 	0x00000040,
	SC_ScrollBarNoScroll = 	0x00000080,
	
	SC_SpinWidgetUp = 	0x00000001,
	SC_SpinWidgetDown = 	0x00000002,
	SC_SpinWidgetFrame = 	0x00000004,
	SC_SpinWidgetEditField = 	0x00000008,
	
	SC_MenuItemCheck =	0x00000001,
	SC_MenuItemLabel =	0x00000002,
	SC_MenuItemAccel =	0x00000004,
	SC_MenuItemSubMenu =	0x00000008,

	SC_ComboBoxEditField =	0x00000001,
	SC_ComboBoxArrow =	0x00000002,
	SC_ComboBoxFocusRect =	0x00000004
    }
    typedef uint SCFlags;


    virtual void drawComplexControl( ComplexControl control,
				     QPainter* p,
				     QWidget* w,
				     const QColorGroup& cg,
				     const QRect& r,
				     CFlags flags = CStyle_Default,
				     SCFlags sub = SC_None,
				     SCFlags subActive = SC_None,
				     void* data = 0
				     );

    virtual QRect querySubControlMetrics( ComplexControl control,
					  QWidget* w,
					  SubControl sc,
					  void* data = 0 );
    virtual SubControl querySubControl( ComplexControl control,
					QWidget* w,
					const QPoint& pos,
					void* data = 0 );


    enum PixelMetric {
	PM_DefaultFrameWidth,
	PM_PopupFrameWidth,
	PM_MenuFrameWidth,
	
	PM_ButtonDefaultIndicator,
	PM_SplitterWidth,
	PM_SliderLength,
	PM_MaximimumSliderDragDistance,
	PM_ButtonShiftHorizontal,
	PM_ButtonShiftVertical,
	PM_MenuButtonIndicatorWidth,
	PM_TabBarOverlap,
	
	PM_TabBarBaseHeight,
	PM_TabBarBaseOverlap,
	
	PM_MenuItemSeparation,
	
	PM_ComboBoxAdditionalWidth,
	PM_ComboBoxAdditionalHeight,

	PM_SpinWidgetAdditionalWidth,
	PM_SpinWidgetAdditionalHeight,
	
	PM_ScrollBarExtent
    }

    virtual int pixelMetric( PixelMetric m, QWidget* w = 0 );


    enum SizeHintConstraint {
	SHC_PushButton,
    }
    virtual QSize sizeHintConstraint( SizeHintConstraint s,
				      QWidget* w,
				      const QSize& sizeHint,
				      void* data = 0 );

    
    enum FeelHint  {
	FH_TabBarCentered
    }

    virtual int feelHint( FeelHint f, QWidget* w = 0, void** returnData = 0 );



private:
    Q_DISABLE_COPY(QStyle)
};

#endif // QT_NO_STYLE
#endif // QSTYLE_H
