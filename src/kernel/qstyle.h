/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qstyle.h#11 $
**
** Definition of QStyle class
**
** Created : 980616
**
** Copyright (C) 1998 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QSTYLE_H
#define QSTYLE_H

#ifndef QT_H
#include "qwindowdefs.h"
#include "qobject.h"
#endif // QT_H

class QPushButton;
class QScrollBar;

class Q_EXPORT QStyle
{
    GUIStyle gs;
public:
    QStyle(GUIStyle);
    QStyle();
    virtual ~QStyle();





#ifndef NO_QT1_COMPAT
    operator GUIStyle() const { return gs; }
    int operator==(GUIStyle s) const { return gs==s; }
    int operator!=(GUIStyle s) const { return gs!=s; }
#endif

    GUIStyle guiStyle() const { return gs; }

    virtual void initialize( QApplication*);

    virtual void polish( QWidget* );

    virtual QRect itemRect( QPainter *p, int x, int y, int w, int h,
		    int flags, bool enabled,
		    const QPixmap *pixmap, const QString& text, int len=-1 );

    virtual void drawItem( QPainter *p, int x, int y, int w, int h,
		    int flags, const QColorGroup &g, bool enabled,
		    const QPixmap *pixmap, const QString& text,
			   int len=-1, bool bright_text = FALSE );


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
		     const QBrush *fill = 0 );

    virtual QRect buttonRect( int x, int y, int w, int h);

    virtual void drawButtonMask( QPainter *p, int x, int y, int w, int h);
			
    virtual void drawBevelButton( QPainter *p, int x, int y, int w, int h,
		     const QColorGroup &g, bool sunken = FALSE,
		     const QBrush *fill = 0 );

    virtual void drawToolButton( QPainter *p, int x, int y, int w, int h,
		     const QColorGroup &g, bool sunken = FALSE,
		     const QBrush *fill = 0 );

    virtual void drawPanel( QPainter *p, int x, int y, int w, int h,
		    const QColorGroup &, bool sunken=FALSE,
		    int lineWidth = 1, const QBrush *fill = 0 );

    enum ArrowType { UpArrow, DownArrow, LeftArrow, RightArrow };

    virtual void drawArrow( QPainter *p, ArrowType type, bool down,
		     int x, int y, int w, int h,
		     const QColorGroup &g, bool enabled, const QBrush *fill = 0 );

    // "radio button"
    virtual QSize exclusiveIndicatorSize() const;
    virtual void drawExclusiveIndicator( QPainter* p, int x, int y, int w, int h,
		    const QColorGroup &g, bool on, bool down = FALSE, bool enabled = TRUE );
    virtual void drawExclusiveIndicatorMask( QPainter *p, int x, int y, int w, int h, bool on);

    // "check box"
    virtual QSize indicatorSize() const;
    virtual void drawIndicator( QPainter* p, int x, int y, int w, int h, const QColorGroup &g,
				bool on, bool down = FALSE, bool enabled = TRUE );
    virtual void drawIndicatorMask( QPainter *p, int x, int y, int w, int h, bool on);

    // "combo box"
    virtual void drawComboButton( QPainter *p, int x, int y, int w, int h,
				  const QColorGroup &g, bool sunken = FALSE,
				  bool enabled = TRUE,
				  const QBrush *fill = 0 );
    virtual QRect comboButtonRect( int x, int y, int w, int h);

    virtual void drawComboButtonMask( QPainter *p, int x, int y, int w, int h);


    // focus
    virtual void drawFocusRect( QPainter*,
		    const QRect&, const QColorGroup & );


    // push buttons
    virtual void drawPushButton( QPushButton* btn, QPainter *p);
    virtual void drawPushButtonLabel( QPushButton* btn, QPainter *p);


    // scrollbars
    enum ScrollControl { ADD_LINE = 0x1 , SUB_LINE = 0x2 , ADD_PAGE = 0x4,
			    SUB_PAGE = 0x8 , FIRST    = 0x10, LAST	= 0x20,
			    SLIDER   = 0x40, NONE     = 0x80 };

    virtual void scrollbarMetrics( const QScrollBar*, int *, int *, int * ) = 0;
    virtual void drawScrollbarControls( QPainter*,  const QScrollBar*, int sliderStart, uint controls, uint activeControl ) = 0;

    // sliders
    enum SliderDirection {SlUp,SlDown,SlLeft,SlRight};
    virtual int sliderLength() const = 0;
    virtual void drawSlider( QPainter *p,
			     int x, int y, int w, int h,
			     const QColorGroup &g,
			     SliderDirection dir) = 0;

    virtual void drawSliderMask( QPainter *p,
				 int x, int y, int w, int h,
				 SliderDirection dir);
    virtual void drawSliderGroove( QPainter *p, 
				   int x, int y, int w, int h,
				   const QColorGroup& g, QCOORD c,
				   bool horizontal ) = 0;
    virtual void drawSliderGrooveMask( QPainter *p, 
				       int x, int y, int w, int h,
				       QCOORD c,
				       bool horizontal);
    virtual int maximumSliderDragDistance() const;

};

class Q_EXPORT QWindowsStyle : public QStyle
{
public:
    QWindowsStyle();
    void drawButton( QPainter *p, int x, int y, int w, int h,
		     const QColorGroup &g, bool sunken = FALSE,
		     const QBrush *fill = 0 );
    void drawBevelButton( QPainter *p, int x, int y, int w, int h,
			  const QColorGroup &g, bool sunken = FALSE,
			  const QBrush *fill = 0 );

    void drawPushButton( QPushButton* btn, QPainter *p);
    void drawPushButtonLabel( QPushButton* btn, QPainter *p);

    void drawArrow( QPainter *p, ArrowType type, bool down,
		    int x, int y, int w, int h,
		    const QColorGroup &g, bool enabled, const QBrush *fill = 0 );

    QSize indicatorSize() const;
    void drawIndicator( QPainter* p,  int x, int y, int w, int h, const QColorGroup &g,
			bool on, bool down = FALSE, bool enabled = TRUE );

    QSize exclusiveIndicatorSize() const;
    void drawExclusiveIndicator( QPainter* p,  int x, int y, int w, int h, const QColorGroup &g,
				 bool on, bool down = FALSE, bool enabled = TRUE );


    void drawComboButton( QPainter *p, int x, int y, int w, int h,
			  const QColorGroup &g, bool sunken = FALSE,
			  bool enabled = TRUE,
			  const QBrush *fill = 0 );
    QRect comboButtonRect( int x, int y, int w, int h);

    void scrollbarMetrics( const QScrollBar*,  int *, int *, int * );
    void drawScrollbarControls( QPainter*,  const QScrollBar*, int sliderStart, uint controls, uint activeControl );


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

protected:
    void drawWinShades( QPainter *p,
			int x, int y, int w, int h,
			const QColor &c1, const QColor &c2,
			const QColor &c3, const QColor &c4,
			const QBrush *fill );

//     void initialize( QApplication*);
//     void polish( QWidget* );
};

class Q_EXPORT QMotifStyle : public QStyle
{
public:
    QMotifStyle();
    void drawButton( QPainter *p, int x, int y, int w, int h,
		     const QColorGroup &g, bool sunken = FALSE,
		     const QBrush *fill = 0 );
    void drawBevelButton( QPainter *p, int x, int y, int w, int h,
			  const QColorGroup &g, bool sunken = FALSE,
			  const QBrush *fill = 0 );
    void drawPushButton( QPushButton* btn, QPainter *p);
    void drawPushButtonLabel( QPushButton* btn, QPainter *p);

    void drawArrow( QPainter *p, ArrowType type, bool down,
		    int x, int y, int w, int h,
		    const QColorGroup &g, bool enabled, const QBrush *fill = 0 );
    QSize indicatorSize() const;
    void drawIndicator( QPainter* p, int x, int y, int w, int h,  const QColorGroup &g,
			bool on, bool down = FALSE, bool enabled = TRUE );


    QSize exclusiveIndicatorSize() const;
    void drawExclusiveIndicator( QPainter* p,  int x, int y, int w, int h, const QColorGroup &g,
				 bool on, bool down = FALSE, bool enabled = TRUE );

    void scrollbarMetrics( const QScrollBar*,  int *, int *, int * );
    void drawScrollbarControls( QPainter*,  const QScrollBar*, int sliderStart, uint controls, uint activeControl );

    int sliderLength() const;
    void drawSlider( QPainter *p,
			     int x, int y, int w, int h,
			     const QColorGroup &g,
			     SliderDirection dir);
    void drawSliderGroove( QPainter *p, 
			   int x, int y, int w, int h,
			   const QColorGroup& g, QCOORD c,
			   bool horizontal );


//     void initialize( QApplication*);
//     void polish( QWidget* );
};

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
			  bool enabled = TRUE,
			  const QBrush *fill = 0 );
    QRect comboButtonRect( int x, int y, int w, int h);

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



class Q_EXPORT QHMotifStyle : public QMotifStyle
{
public:
    QHMotifStyle();
    void initialize( QApplication*);
    void polish( QWidget* );

    void drawButton( QPainter *p, int x, int y, int w, int h,
			     const QColorGroup &g, bool sunken = FALSE,
			     const QBrush *fill = 0 );
    void drawBevelButton( QPainter *p, int x, int y, int w, int h,
			  const QColorGroup &g, bool sunken = FALSE,
			  const QBrush *fill = 0 );
    QRect buttonRect( int x, int y, int w, int h);
    void drawButtonMask( QPainter *p, int x, int y, int w, int h);


    void drawPushButton( QPushButton* btn, QPainter *p);
    void drawPushButtonLabel( QPushButton* btn, QPainter *p);
};


#endif // QSTYLE_H
