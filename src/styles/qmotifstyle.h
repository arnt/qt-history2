/****************************************************************************
** $Id: $
**
** Definition of Motif-like style class
**
** Created : 981231
**
** Copyright (C) 1998-2000 Trolltech AS.  All rights reserved.
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

#ifndef QMOTIFSTYLE_H
#define QMOTIFSTYLE_H

#ifndef QT_H
#include "qcommonstyle.h"
#endif // QT_H

#ifndef QT_NO_STYLE_MOTIF

class QPalette;

#if defined(QT_PLUGIN_STYLE_MOTIF)
#define Q_EXPORT_STYLE_MOTIF
#else
#define Q_EXPORT_STYLE_MOTIF Q_EXPORT
#endif


class Q_EXPORT_STYLE_MOTIF QMotifStyle : public QCommonStyle
{
    Q_OBJECT
public:
    QMotifStyle( bool useHighlightCols=FALSE );
    virtual ~QMotifStyle();

    void setUseHighlightColors( bool );
    bool useHighlightColors() const;

    void polish( QPalette& );
    void polish( QWidget* );
    void polish( QApplication* );

    void drawButton( QPainter *p, int x, int y, int w, int h,
                     const QColorGroup &g, bool sunken = FALSE,
                     const QBrush *fill = 0 );
    void drawBevelButton( QPainter *p, int x, int y, int w, int h,
                          const QColorGroup &g, bool sunken = FALSE,
                          const QBrush *fill = 0 );
    void drawFocusRect( QPainter*,
                        const QRect&, const QColorGroup &, const QColor* =0, bool = FALSE );

    // "combo box"
    void drawComboButton( QPainter *p, int x, int y, int w, int h,
                          const QColorGroup &g, bool sunken = FALSE,
                          bool editable = FALSE,
                          bool enabled = TRUE,
                          const QBrush *fill = 0 );
    QRect comboButtonRect( int x, int y, int w, int h ) const;
    QRect comboButtonFocusRect( int x, int y, int w, int h ) const;


    void drawPushButton( QPushButton* btn, QPainter *p );

    void drawArrow( QPainter *p, ArrowType type, bool down,
                    int x, int y, int w, int h,
                    const QColorGroup &g, bool enabled, const QBrush *fill = 0 );
    QSize indicatorSize() const;
    void drawIndicator( QPainter* p, int x, int y, int w, int h,  const QColorGroup &g,
                        int state, bool down = FALSE, bool enabled = TRUE );


    QSize exclusiveIndicatorSize() const;
    void drawExclusiveIndicator( QPainter* p,  int x, int y, int w, int h, const QColorGroup &g,
                                 bool on, bool down = FALSE, bool enabled = TRUE );
    void drawExclusiveIndicatorMask( QPainter *p, int x, int y, int, int, bool );

    void tabbarMetrics( const QTabBar*, int&, int&, int& ) const;
    void drawTab( QPainter*,  const QTabBar*, QTab*, bool selected );

    void scrollBarMetrics( const QScrollBar*, int&, int&, int&, int& ) const;
    void drawScrollBarControls( QPainter*,  const QScrollBar*, int sliderStart,
                                uint controls, uint activeControl );
    void drawToolBarHandle( QPainter *p, const QRect &r,
                            Qt::Orientation orientation,
                            bool highlight, const QColorGroup &cg,
                            bool drawBorder = FALSE );

    int sliderLength() const;
    void drawSlider( QPainter *p,
                             int x, int y, int w, int h,
                             const QColorGroup &g,
                             Orientation, bool tickAbove, bool tickBelow );
    void drawSliderGroove( QPainter *p,
                           int x, int y, int w, int h,
                           const QColorGroup& g, QCOORD c,
                           Orientation );

    int splitterWidth() const;
    void drawSplitter( QPainter *p, int x, int y, int w, int h,
                       const QColorGroup &g, Orientation );


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
                            bool enabled, int x, int y, int w, int h );

    int buttonDefaultIndicatorWidth() const;
    int sliderThickness() const;
    void drawMenuBarItem( QPainter* p, int x, int y, int w, int h,
                          QMenuItem* mi, QColorGroup& g, bool active,
                          bool down, bool hasFocus );

    int spinBoxFrameWidth() const;

    // progressbar
    int progressChunkWidth() const;
    void drawProgressBar( QPainter *p, int x, int y, int w, int h, const QColorGroup &g );
    void drawProgressChunk( QPainter *p, int x, int y, int w, int h, const QColorGroup &g );

    // title bar
    QPixmap titleBarPixmap( const QTitleBar *, TitleControl );

    // listview item
    void drawListViewItemBranch( QPainter *, int, int, int, const QColorGroup & cg, QListViewItem * );
private:
    bool highlightCols;
private:        // Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QMotifStyle( const QMotifStyle & );
    QMotifStyle& operator=( const QMotifStyle & );
#endif
};

#endif // QT_NO_STYLE_MOTIF

#endif // QMOTIFSTYLE_H
