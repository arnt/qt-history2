/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qsgistyle.h#1 $
**
** Definition of SGI-like style class
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

#ifndef QSGISTYLE_H
#define QSGISTYLE_H

#ifndef QT_H
#include "qmotifstyle.h"
#include "qpalette.h"
#include "qguardedptr.h"
#endif // QT_H

#if QT_FEATURE_STYLE_SGI

class QButton;

#if defined(Q_TEMPLATEDLL)
// MOC_SKIP_BEGIN
template class Q_EXPORT QGuardedPtr<QWidget>;
// MOC_SKIP_END
#endif

class Q_EXPORT QSGIStyle: public QMotifStyle
{
    Q_OBJECT
public:
    QSGIStyle( bool useHighlightCols = FALSE );
    virtual ~QSGIStyle();

    void polish( QWidget* );
    void unPolish( QWidget* );
    void polish( QApplication* );
    void polish( QPalette& );

    int defaultFrameWidth() const;

    void drawPanel( QPainter*p, int x, int y, int w, int h, const QColorGroup &g,
            bool sunken = FALSE, int lineWidth = 1, const QBrush* fill = 0 );
    void drawSeparator( QPainter *p, int x1, int y1, int x2, int y2,
	    const QColorGroup &g, bool sunken = TRUE,
	    int lineWidth = 1, int midLineWidth = 0 );
    void drawButton( QPainter *p, int x, int y, int w, int h,
            const QColorGroup &g, bool sunken = FALSE,
            const QBrush *fill = 0 );
    void drawBevelButton( QPainter *p, int x, int y, int w, int h,
            const QColorGroup &g, bool sunken = FALSE,
            const QBrush *fill = 0 );
    void drawPushButton( QPushButton*, QPainter* );
    void drawArrow( QPainter *p, ArrowType type, bool down,
		    int x, int y, int w, int h,
		    const QColorGroup &g, bool enabled, const QBrush *fill = 0 );
    // checkbox
    QSize indicatorSize() const;
    void drawIndicator( QPainter* p, int x, int y, int w, int h,
            const QColorGroup& g, int state, bool down = FALSE, bool enabled = TRUE );
    void drawCheckMark( QPainter* p, int x, int y, int w, int h,
            const QColorGroup& g, bool act, bool dis );
    void drawIndicatorMask( QPainter* p, int x, int y, int w, int h, int s );

    // radio-buttons
    QSize exclusiveIndicatorSize() const;
    void drawExclusiveIndicator( QPainter* p,  int x, int y, int w, int h, const QColorGroup &g,
				 bool on, bool down = FALSE, bool enabled = TRUE );
    void drawExclusiveIndicatorMask( QPainter *p, int x, int y, int w, int h, bool on);

    // combobox
    void drawComboButton( QPainter *p, int x, int y, int w, int h,
			  const QColorGroup &g, bool sunken = FALSE,
			  bool editable = FALSE,
			  bool enabled = TRUE,
			  const QBrush *fill = 0 );
    QRect comboButtonRect( int x, int y, int w, int h);
    QRect comboButtonFocusRect( int x, int y, int w, int h);

    // scrollbar
    void scrollBarMetrics( const QScrollBar*, int&, int&, int&, int&);
    void drawScrollBarControls( QPainter* p, const QScrollBar*, int sliderStart, uint controls, uint activeControl );

    // slider
    void drawSlider( QPainter* p, int x, int y, int w, int h, const QColorGroup& g,
                Orientation orient, bool tickAbove, bool tickBelow );
    void drawSliderMask( QPainter* p, int x, int y, int w, int h, const QColorGroup& g,
                Orientation orient, bool tickAbove, bool tickBelow );
    void drawSliderGroove( QPainter* p, int x, int y, int w, int h,
                const QColorGroup& g, QCOORD c, Orientation orient );
    void drawSliderGrooveMask( QPainter* p, int x, int y, int w, int h,
                const QColorGroup& g, QCOORD c, Orientation orient );
    // tabs
    void drawTab( QPainter *p, const QTabBar *tb, QTab* t, bool selected );
    void drawTabMask( QPainter *p, const QTabBar *tb, QTab* t, bool selected );
    // splitter
    int splitterWidth() const;
    void drawSplitter( QPainter *p, int x, int y, int w, int h,
                const QColorGroup& g, Orientation orient );
    // popupmenu
    int popupMenuItemHeight( bool checkable, QMenuItem* mi, const QFontMetrics& fm );
    void drawPopupPanel( QPainter* p, int x, int y, int w, int h, const QColorGroup& g,
                int lineWidth = 2, const QBrush* fill = 0 );
    void drawPopupMenuItem( QPainter* p, bool checkable, int maxpmw, int tab, QMenuItem* mi,
                const QPalette& pal,
                bool act, bool enabled, int x, int y, int w, int h);
protected:
    bool eventFilter( QObject*, QEvent*);

private:
    QGuardedPtr<QWidget> lastWidget;
#if defined(Q_DISABLE_COPY)
    QSGIStyle( const QSGIStyle & );
    QSGIStyle& operator=( const QSGIStyle & );
#endif

};

#endif // QT_FEATURE_STYLE_SGI

#endif // QSGISTYLE_H
