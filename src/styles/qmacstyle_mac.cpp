/****************************************************************************
** $Id: $
**
** Implementation of Motif-like style class
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

#include "qmacstyle_mac.h"

#ifdef Q_WS_MAC
#include <qscrollbar.h>
#include <qpainter.h>
#include <qdrawutil.h>

QMacStyle::QMacStyle(  ) 
{
}

QMacStyle::~QMacStyle()
{
}

#define HORIZONTAL      (sb->orientation() == QScrollBar::Horizontal)
#define VERTICAL        !HORIZONTAL
#define SLIDER_MIN      9 // ### Mac says 6 but that's too small
#define MAC_BORDER defaultFrameWidth()

/*! \reimp */

void QMacStyle::scrollBarMetrics( const QScrollBar* sb,
                                    int &sliderMin, int &sliderMax,
                                    int &sliderLength, int &buttonDim ) const
{
    int maxLength;
    int b = MAC_BORDER;
    int length = HORIZONTAL ? sb->width()  : sb->height();
    int extent = HORIZONTAL ? sb->height() : sb->width();

    if ( length > ( extent - b*2 - 1 )*2 + b*2  )
        buttonDim = extent - b*2;
    else
        buttonDim = ( length - b*2 )/2 - 1;

    sliderMin = b + buttonDim;
    maxLength  = length - b*2 - buttonDim*2;

    if ( sb->maxValue() == sb->minValue() ) {
        sliderLength = maxLength;
    } else {
        uint range = sb->maxValue()-sb->minValue();
        sliderLength = (sb->pageStep()*maxLength)/
                        (range + sb->pageStep());
        if ( sliderLength < SLIDER_MIN || range > INT_MAX/2 )
            sliderLength = SLIDER_MIN;
        if ( sliderLength > maxLength )
            sliderLength = maxLength;
    }
    sliderMax = sliderMin + maxLength - sliderLength;

}


void QMacStyle::drawScrollBarControls( QPainter* p, const QScrollBar* sb,
                                         int sliderStart, uint controls,
                                         uint activeControl )
{
#define ADD_LINE_ACTIVE ( activeControl == AddLine )
#define SUB_LINE_ACTIVE ( activeControl == SubLine )
    QColorGroup g  = sb->colorGroup();

    int sliderMin, sliderMax, sliderLength, buttonDim;
    scrollBarMetrics( sb, sliderMin, sliderMax, sliderLength, buttonDim );

    if ( controls == (AddLine | SubLine | AddPage | SubPage | Slider | First | Last ) )
        qDrawShadePanel( p, sb->rect(), g, TRUE );

    if (sliderStart > sliderMax) { // sanity check
        sliderStart = sliderMax;
    }

    int b = MAC_BORDER;
    int dimB = buttonDim;
    QRect addB;
    QRect subB;
    QRect addPageR;
    QRect subPageR;
    QRect sliderR;
    int addX, addY, subX, subY;
    int length = HORIZONTAL ? sb->width()  : sb->height();
    int extent = HORIZONTAL ? sb->height() : sb->width();

    if ( HORIZONTAL ) {
        subY = addY = ( extent - dimB ) / 2;
        subX = b;
        addX = length - dimB - b;
    } else {
        subX = addX = ( extent - dimB ) / 2;
        subY = b;
        addY = length - dimB - b;
    }

    subB.setRect( subX,subY,dimB,dimB );
    addB.setRect( addX,addY,dimB,dimB );

    int sliderEnd = sliderStart + sliderLength;
    int sliderW = extent - b*2;
    if ( HORIZONTAL ) {
        subPageR.setRect( subB.right() + 1, b,
                          sliderStart - subB.right() - 1 , sliderW );
        addPageR.setRect( sliderEnd, b, addX - sliderEnd, sliderW );
        sliderR .setRect( sliderStart, b, sliderLength, sliderW );
    } else {
        subPageR.setRect( b, subB.bottom() + 1, sliderW,
                          sliderStart - subB.bottom() - 1 );
        addPageR.setRect( b, sliderEnd, sliderW, addY - sliderEnd );
        sliderR .setRect( b, sliderStart, sliderW, sliderLength );
    }

    if ( controls & AddLine )
        drawArrow( p, VERTICAL ? DownArrow : RightArrow,
                   ADD_LINE_ACTIVE, addB.x(), addB.y(),
                   addB.width(), addB.height(), g, TRUE );
    if ( controls & SubLine )
        drawArrow( p, VERTICAL ? UpArrow : LeftArrow,
                   SUB_LINE_ACTIVE, subB.x(), subB.y(),
                   subB.width(), subB.height(), g, TRUE );

    QBrush fill = g.brush( QColorGroup::Mid );
    if (sb->backgroundPixmap() ){
        fill = QBrush( g.mid(), *sb->backgroundPixmap() );
    }

    if ( controls & SubPage )
        p->fillRect( subPageR, fill );

    if ( controls & AddPage )
        p->fillRect( addPageR, fill );

    if ( controls & Slider ) {
        QPoint bo = p->brushOrigin();
        p->setBrushOrigin(sliderR.topLeft());
        if ( sliderR.isValid() )
            drawBevelButton( p, sliderR.x(), sliderR.y(),
                             sliderR.width(), sliderR.height(), g,
                             FALSE, &g.brush( QColorGroup::Button ) );

        //      qDrawShadePanel( p, sliderR, g, FALSE, 2, &g.fillButton() );
        p->setBrushOrigin(bo);
    }

}



#endif
