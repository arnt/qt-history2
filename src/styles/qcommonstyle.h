/****************************************************************************
** $Id: //depot/qt/main/src/styles/qcommonstyle.h#21 $
**
** Definition of QCommonStyle class
**
** Created : 980616
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

#ifndef QCOMMONSTYLE_H
#define QCOMMONSTYLE_H

#ifndef QT_H
#include "qstyle.h"
#include "qpushbutton.h" // compile!
#endif // QT_H

#ifndef QT_NO_STYLE

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
    // ### 8< - Make it compile!
    QRect buttonRect( int x, int y, int w, int h ) const {
	return QRect( x, y, w, h );
    }
    QRect pushButtonContentsRect( const QPushButton & b ) const {
	return QRect( b.x(), b.y(), b.width(), b.height() );
    }
    int menuButtonIndicatorWidth( int h ) const {
	return h;
    }
    void scrollBarMetrics( const QScrollBar*,
			   int&, int&, int&, int&) const { }
    void drawScrollBarControls( QPainter*,  const QScrollBar*,
				int , uint ,
				uint  )  { }
    ScrollControl scrollBarPointOver( const QScrollBar *,
				      int , const QPoint & ) { return NoScroll; }
    QSize indicatorSize() const { return QSize(); }
    void drawIndicator( QPainter *, int, int, int, int, const QColorGroup &,
			int, bool = FALSE, bool = TRUE ) { }
    QSize exclusiveIndicatorSize() const { return QSize(); }
    void drawExclusiveIndicator(QPainter *, int, int, int, int, const QColorGroup &,
				bool, bool = FALSE, bool = TRUE) { }
    // ### 8<

    virtual void drawPrimitive( PrimitiveOperation op,
				QPainter *p,
				const QRect &r,
				const QColorGroup &cg,
				PFlags flags = PStyle_Default,
				void *data = 0 ) const;

    virtual void drawControl( ControlElement element,
			      QPainter *p,
			      const QWidget *w,
			      const QRect &r,
			      const QColorGroup &cg,
			      CFlags how = CStyle_Default,
			      void *data = 0 ) const;

    virtual QRect subRect( SubRect r, const QWidget *w ) const;

    virtual void drawComplexControl( ComplexControl control,
				     QPainter *p,
				     const QWidget *w,
				     const QRect &r,
				     const QColorGroup &cg,
				     CFlags flags = CStyle_Default,
				     SCFlags sub = SC_None,
				     SCFlags subActive = SC_None,
				     void *data = 0 ) const;

    virtual QRect querySubControlMetrics( ComplexControl control,
					  const QWidget *w,
					  SubControl sc,
					  void *data = 0 ) const;
    virtual SubControl querySubControl( ComplexControl control,
					const QWidget *w,
					const QPoint &pos,
					void *data = 0 ) const;

    virtual int pixelMetric( PixelMetric m, const QWidget *w = 0 ) const;

    virtual QSize sizeFromContents( ContentsType s,
				    const QWidget *w,
				    const QSize &contentsSize,
				    void *data = 0 ) const;

    virtual int styleHint( StyleHint sh,
			   const QWidget *w = 0,
			   void **returnData = 0 ) const;








    // tabbars
    void tabbarMetrics( const QTabBar*, int&, int&, int& ) const;
    void drawTab( QPainter*, const QTabBar*, QTab*, bool selected );

    //menubars
    void drawMenuBarItem( QPainter* p, int x, int y, int w, int h,
			  QMenuItem* mi, QColorGroup& g,
			  bool active, bool down, bool hasFocus );

    int menuBarFrameWidth() const { return 2; }

    // header
    void drawHeaderSection( QPainter *p, int x, int y, int w, int h, const QColorGroup &g, bool down );

    // groupbox
    void drawGroupBoxTitle( QPainter *p, int x, int y, int w, int h, const QColorGroup &g, const QString &text, bool enabled );
    void drawGroupBoxFrame( QPainter *p, int x, int y, int w, int h, const QColorGroup &g, const QGroupBox *gb );

    // statusbar
    void drawStatusBarSection( QPainter *p, int x, int y, int w, int h, const QColorGroup &g, bool permanent );
    void drawSizeGrip( QPainter *p, int x, int y, int w, int h, const QColorGroup &g );

    // title bar
    virtual void titleBarMetrics( const QTitleBar*, int&, int&, int&, int&) const;
    virtual void drawTitleBarControls( QPainter*,  const QTitleBar*,
				       uint controls, uint activeControl );
    virtual TitleControl titleBarPointOver( const QTitleBar*, const QPoint& );

    ListViewItemControl listViewItemPointOver( const QListViewItem *, const QPoint & );


private:        // Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QCommonStyle( const QCommonStyle & );
    QCommonStyle &operator=( const QCommonStyle & );
#endif
};



#endif // QT_NO_STYLE

#endif // QCOMMONSTYLE_H
