/****************************************************************************
** $Id: //depot/qt/main/examples/sheet/pie.cpp#1 $
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include <qpainter.h>
#include <math.h>
#include "pie.h"

#ifndef PI
#define	M_PI		3.14159265358979323846
#endif

#ifndef M_PI
#define M_PI PI
#endif

//
// Construct the PieView with buttons.
//

PieView::PieView( int *n, QString *s )
{
    setCaption( "Pie in the Sky Inc." );

    nums = n;
    strs = s;
    resize( 500, 350 );
}

void PieView::restart( int *n, QString *s )
{
    nums = n;
    strs = s;
    repaint();
}





//
// This function draws a pie chart
//




void PieView::drawPies( QPainter *p ) // zero terminated
{


    int sum = 0;
    int n = 0;
    {
        int *ip = nums;
        while (*ip) {
            sum += *ip++;
            n++;
        }
        if ( !sum )
            return;
    }
    QFont f( "times", 18, QFont::Bold );
    p->setFont( f );
    p->setPen( black );				// black pen outline

    int size;

    int apos = -90*16;
    int *val = nums;

    const int w = width();
    const int h = height();

    const int xd = w - w/5;
    const int yd = h - h/5;


    QString * s = strs;

    QBrush back(white);




    QRect qr(0,0,70,25);
    int i = 0;
    while ((     size = *val++ )) {
        QColor c;
        c.setHsv( ( i++ * 255)/n, 255, 255 );		// rainbow effect
        p->setBrush( c );			// solid fill with color c

        int a = ( size * 360 * 16 )/sum;
        p->drawPie( w/10, h/10, xd, yd, -apos, -a );
        apos += a;

        int x = (int) (cos(M_PI*2* (double)(apos-a/2) / 5760.0) * xd/2);
        int y = (int) (sin(M_PI*2* (double)(apos-a/2) / 5760.0) * yd/2);
        qr.moveCenter( QPoint(x+w/2,y+h/2) );
        p->setBrush(back);
        if ( s ) {
            if ( *s && **s ) {
                p->drawRect(qr);
                p->drawText(qr, AlignCenter, *s);
            }
            s++;
        }

    }

}

//
// Called when the widget needs to be updated.
//

void PieView::paintEvent( QPaintEvent * )
{
    QPainter paint( this );
    drawPies( &paint );
}
