/****************************************************************************
** $Id: //depot/qt/main/src/styles/qwindowsstyle.h#17 $
**
** Definition of Windows-like style class
**
** Created : 981231
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
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

#ifndef QWINDOWSSTYLE_H
#define QWINDOWSSTYLE_H

#ifndef QT_H
#include "qcommonstyle.h"
#endif // QT_H

#ifndef QT_NO_STYLE_WINDOWS

#if defined(QT_PLUGIN_STYLE_WINDOWS)
#define Q_EXPORT_STYLE_WINDOWS
#else
#define Q_EXPORT_STYLE_WINDOWS Q_EXPORT
#endif

class QWindowsStylePrivate;

class Q_EXPORT_STYLE_WINDOWS QWindowsStyle : public QCommonStyle
{
    Q_OBJECT
public:
    QWindowsStyle();
    virtual ~QWindowsStyle();


    // new stuff
    void drawComplexControl( ComplexControl control,
			     QPainter* p,
			     const QWidget* w,
			     const QRect& r,
			     const QColorGroup& cg,
			     CFlags flags = CStyle_Default,
			     SCFlags sub = SC_None,
			     SCFlags subActive = SC_None,
			     void* data = 0 ) const;

    void drawSubControl( SCFlags subCtrl,
			 QPainter* p,
			 const QWidget* w,
			 const QRect& r,
			 const QColorGroup& cg,
			 CFlags flags = CStyle_Default,
			 SCFlags subActive = SC_None,
			 void* data = 0 ) const;
    
    // old stuff 
    
    void polish( QWidget * );
    void unPolish( QWidget * );

    // new
    void drawPrimitive( PrimitiveOperation op,
			QPainter *p,
			const QRect &r,
			const QColorGroup &cg,
			PFlags flags = PStyle_Default,
			void *data = 0 ) const;

    void drawControl( ControlElement element,
		      QPainter *p,
		      const QWidget *widget,
		      const QRect &r,
		      const QColorGroup &cg,
		      CFlags how = CStyle_Default,
		      void *data = 0 ) const;

    QRect subRect( SubRect r, const QWidget *widget ) const;

    int pixelMetric( PixelMetric metic, const QWidget *widget = 0 ) const;

    QSize sizeFromContents( ContentsType contents,
			    const QWidget *w,
			    const QSize &contentsSize,
			    void *data ) const;

    // convenience
    void drawButton( QPainter *p, const QRect &r,
                     const QColorGroup &g, bool sunken) const;

    // old
    void drawButton( QPainter *p, int x, int y, int w, int h,
                     const QColorGroup &g, bool sunken = FALSE,
                     const QBrush *fill = 0 );
    void drawBevelButton( QPainter *p, int x, int y, int w, int h,
                          const QColorGroup &g, bool sunken = FALSE,
                          const QBrush *fill = 0 );
    void drawPushButton( QPushButton* btn, QPainter *p);
    void getButtonShift( int &x, int &y) const;

    void drawFocusRect( QPainter*,
			const QRect&, const QColorGroup &, const QColor* =0,  bool = FALSE );

    void drawPanel( QPainter *p, int x, int y, int w, int h,
                    const QColorGroup &, bool sunken=FALSE,
                    int lineWidth = 1, const QBrush *fill = 0 );

    void drawPopupPanel( QPainter *p, int x, int y, int w, int h,
                         const QColorGroup &,  int lineWidth = 2,
                         const QBrush *fill = 0 );

    void drawArrow( QPainter *p, ArrowType type, bool down,
                    int x, int y, int w, int h,
                    const QColorGroup &g, bool enabled, const QBrush *fill = 0 );

    void drawToolButton( QPainter *p, int x, int y, int w, int h,
			 const QColorGroup &g, bool on, bool down, bool enabled, bool autoRaised = FALSE,
			 const QBrush *fill = 0 );

    void drawDropDownButton( QPainter *p, int x, int y, int w, int h,
			     const QColorGroup &g, bool down, bool enabled, bool autoRaised = FALSE,
			     const QBrush *fill = 0 );

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
    QRect comboButtonRect( int x, int y, int w, int h) const;
    QRect comboButtonFocusRect( int x, int y, int w, int h) const;

    void tabbarMetrics( const QTabBar*, int&, int&, int& ) const;
    void drawTab( QPainter*, const QTabBar*, QTab*, bool selected );

    void scrollBarMetrics( const QScrollBar*, int&, int&, int&, int&) const;
    void drawScrollBarControls( QPainter*,  const QScrollBar*, int sliderStart,
                                uint controls, uint activeControl );


    int sliderLength() const;
    void drawSlider( QPainter *p,
		     int x, int y, int w, int h,
		     const QColorGroup &g,
		     Orientation, bool tickAbove, bool tickBelow);
    void drawSliderGroove( QPainter *p,
                           int x, int y, int w, int h,
                           const QColorGroup& g, QCOORD c,
                           Orientation );

    int maximumSliderDragDistance() const;

    int splitterWidth() const;
    void drawSplitter( QPainter *p, int x, int y, int w, int h,
		       const QColorGroup &g, Orientation);

    void drawCheckMark( QPainter *p, int x, int y, int w, int h,
			const QColorGroup &g,
			bool act, bool dis );
    void polishPopupMenu( QPopupMenu* );

    int extraPopupMenuItemWidth( bool checkable, int maxpmw, QMenuItem* mi,
                                 const QFontMetrics& fm ) const;
    int popupMenuItemHeight( bool checkable, QMenuItem* mi,
                             const QFontMetrics& fm ) const;
    void drawPopupMenuItem( QPainter* p, bool checkable, int maxpmw, int tab,
                            QMenuItem* mi, const QPalette& pal, bool act,
                            bool enabled, int x, int y, int w, int h);

    void drawMenuBarItem( QPainter* p, int x, int y, int w, int h,
                          QMenuItem* mi, QColorGroup& g,
                          bool active, bool down, bool hasFocus = FALSE );

    int buttonDefaultIndicatorWidth() const;
    int menuBarFrameWidth() const;

    int spinBoxFrameWidth() const;

    // progressbar
    int progressChunkWidth() const;
    void drawProgressBar( QPainter *p, int x, int y, int w, int h, const QColorGroup &g );
    void drawProgressChunk( QPainter *p, int x, int y, int w, int h, const QColorGroup &g );

    // title bar
    QPixmap titleBarPixmap( const QTitleBar *, TitleControl );

    // listview item
    void drawListViewItemBranch( QPainter *, int, int, int, const QColorGroup & cg, QListViewItem * );

protected:
    void drawWinShades( QPainter *p,
                        int x, int y, int w, int h,
                        const QColor &c1, const QColor &c2,
                        const QColor &c3, const QColor &c4,
                        const QBrush *fill ) const;

    bool eventFilter( QObject *o, QEvent *e );


private:        // Disabled copy constructor and operator=
    QWindowsStylePrivate *d;

#if defined(Q_DISABLE_COPY)
    QWindowsStyle( const QWindowsStyle & );
    QWindowsStyle& operator=( const QWindowsStyle & );
#endif
};

#endif // QT_NO_STYLE_WINDOWS

#endif // QWINDOWSSTYLE_H
