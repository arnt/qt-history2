/****************************************************************************
** $Id: //depot/qt/main/src/styles/qcommonstyle.h#12 $
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

     // push buttons
    void drawPushButtonLabel( QPushButton* btn, QPainter *p);
    QRect pushButtonContentsRect( QPushButton* btn ) const;

    // tabbars
    void tabbarMetrics( const QTabBar*, int&, int&, int& ) const;
    void drawTab( QPainter*, const QTabBar*, QTab*, bool selected );

    // scrollbars
    ScrollControl scrollBarPointOver( const QScrollBar* sb, int sliderStart, const QPoint& p );

    //menubars
    void drawMenuBarItem( QPainter* p, int x, int y, int w, int h,
			  QMenuItem* mi, QColorGroup& g,
			  bool active, bool down, bool hasFocus );

    int menuBarFrameWidth() const { return 2; }

    // header
    void drawHeaderSection( QPainter *p, int x, int y, int w, int h, const QColorGroup &g, bool down );

    // spin widget
    virtual void drawSpinWidgetButton( QPainter *p, int x, int y, int w, int h,
				       const QColorGroup &g, QSpinWidget *sw,
				       bool downbtn, bool enabled, bool down );
    virtual void drawSpinWidgetSymbol( QPainter *p, int x, int y, int w, int h,
				       const QColorGroup &g, QSpinWidget *sw,
				       bool downbtn, bool enabled, bool down );

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
