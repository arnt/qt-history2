/****************************************************************************
**
** Definition of Aqua-like style class
**
** Created : 001129
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

#ifndef QAQUASTYLE_H
#define QAQUASTYLE_H

#ifndef QT_H
//#include "qmotifstyle.h"
#include "qwindowsstyle.h"
#include "qpalette.h"
#include "qvaluelist.h"
#include "qmap.h"
#endif // QT_H

#ifndef QT_NO_STYLE_AQUA

#if defined(QT_PLUGIN_STYLE_AQUA)
#define Q_EXPORT_STYLE_AQUA
#else
#define Q_EXPORT_STYLE_AQUA Q_EXPORT
#endif

class QAquaStylePrivate;

class Q_EXPORT_STYLE_AQUA QAquaStyle : public QWindowsStyle
{
    Q_OBJECT
public:
    QAquaStyle();
    virtual ~QAquaStyle();

    void polish( QWidget * w );
    void unPolish( QWidget * w );
    void polish( QPalette & pal );
    void unPolish( QPalette & pal );

    void drawPopupPanel( QPainter *p, int x, int y, int w, int h,
                         const QColorGroup &,  int lineWidth = 2,
                         const QBrush *fill = 0 );
    void drawButton( QPainter *p, int x, int y, int w, int h,
                     const QColorGroup &g, bool sunken = FALSE,
                     const QBrush *fill = 0 );
    void drawToolButton( QToolButton * btn, QPainter * p );

    QRect buttonRect( int x, int y, int w, int h) const;
    void  drawBevelButton( QPainter *p, int x, int y, int w, int h,
                           const QColorGroup &g, bool sunken = FALSE,
                           const QBrush *fill = 0 );
    void drawPushButton( QPushButton* btn, QPainter *p );
    void drawPushButtonLabel( QPushButton* btn, QPainter *p );
    void getButtonShift( int &x, int &y ) const;

    void scrollBarMetrics( const QScrollBar*, int&, int&, int&, int&) const;
    void drawScrollBarControls( QPainter*,  const QScrollBar*, int sliderStart,
                                uint controls, uint activeControl );
    QSize indicatorSize() const;
    void  drawIndicator( QPainter* p, int x, int y, int w, int h,
                         const QColorGroup &g, int state, bool down = FALSE,
                         bool enabled = TRUE );
    void  drawIndicatorMask( QPainter *p, int x, int y, int w, int h,
                             int state );

    QSize exclusiveIndicatorSize() const;
    void  drawExclusiveIndicator( QPainter* p,  int x, int y, int w, int h,
                                  const QColorGroup &g, bool on,
                                  bool down = FALSE, bool enabled = TRUE );

    void drawExclusiveIndicatorMask( QPainter* p,  int x, int y, int w, int h,
                                     bool on );
    void drawComboButton( QPainter *p, int x, int y, int w, int h,
                          const QColorGroup &g, bool sunken = FALSE,
                          bool editable = FALSE,
                          bool enabled = TRUE,
                          const QBrush *fill = 0 );
    QRect comboButtonRect( int x, int y, int w, int h) const;
    QRect comboButtonFocusRect( int x, int y, int w, int h) const;

    int  sliderLength() const;
    void drawSlider( QPainter *p,
                     int x, int y, int w, int h,
                     const QColorGroup &g,
                     Orientation, bool tickAbove, bool tickBelow );
    void drawSliderMask( QPainter *p,
                         int x, int y, int w, int h,
                         Orientation, bool tickAbove, bool tickBelow);
    void drawSliderGroove( QPainter *p,
                           int x, int y, int w, int h,
                           const QColorGroup& g, QCOORD c,
                           Orientation );


    int  maximumSliderDragDistance() const;
    void drawCheckMark( QPainter *p, int x, int y, int w, int h,
                        const QColorGroup &g, bool act, bool dis );
    void drawPanel( QPainter *p, int x, int y, int w, int h,
                    const QColorGroup &, bool sunken=FALSE,
                    int lineWidth = 1, const QBrush *fill = 0 );
    void polishPopupMenu( QPopupMenu * );

    int  extraPopupMenuItemWidth( bool checkable, int maxpmw, QMenuItem* mi,
                                  const QFontMetrics& fm ) const;
    int  popupMenuItemHeight( bool checkable, QMenuItem* mi,
                              const QFontMetrics& fm ) const;
    void drawPopupMenuItem( QPainter* p, bool checkable, int maxpmw, int tab,
                            QMenuItem* mi, const QPalette& pal, bool act,
                            bool enabled, int x, int y, int w, int h);
    void drawMenuBarItem( QPainter* p, int x, int y, int w, int h,
                          QMenuItem* mi, QColorGroup& g, bool active,
                          bool down, bool hasFocus );

    int buttonDefaultIndicatorWidth() const;

    void  drawTab( QPainter* p,  const QTabBar* tb, QTab* t , bool selected );
    QRect pushButtonContentsRect( QPushButton* btn ) const;

    void  drawToolBarHandle( QPainter *p, const QRect &r,
                             Qt::Orientation orientation,
                             bool highlight, const QColorGroup &cg,
                             bool drawBorder );

    void  drawToolBarPanel( QPainter *p, int x, int y, int w, int h,
                            const QColorGroup &, const QBrush * fill = 0 );

    void  drawToolBarSeparator( QPainter * p, int x, int y, int w, int h,
                                const QColorGroup & cg, Qt::Orientation orientation );
    QSize toolBarSeparatorSize( Qt::Orientation orientation ) const;
    void  drawMenuBarPanel( QPainter *p, int x, int y, int w, int h,
                            const QColorGroup &, const QBrush * fill = 0 );
    void  drawFocusRect( QPainter*, const QRect &,
                         const QColorGroup &, const QColor* bg = 0,
                         bool = FALSE );


protected:
    void drawScrollBarBackground( QPainter *p, int x, int y, int w, int h,
                                  const QColorGroup &g, bool horizontal,
                                  const QBrush* fill = 0);
    bool eventFilter( QObject *, QEvent * );
    void timerEvent( QTimerEvent * );

private:        // Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QAquaStyle( const QAquaStyle & );
    QAquaStyle& operator=( const QAquaStyle & );
#endif
    QPalette oldPalette;

    QAquaStylePrivate *d;
};

#endif // QT_NO_STYLE_AQUA

#endif // QAQUASTYLE_H
