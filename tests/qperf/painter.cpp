#include "qperf.h"
#include <qpixmap.h>
#include <qbitmap.h>
#include <qpainter.h>
#include <qimage.h>
#include <stdlib.h>


static void painter_init()
{
}

static int painter_drawline()
{
    int i;
    QPainter *p = qperf_painter();
    int n = qperf_maxColors();
    QColor *c = qperf_colors();
    for ( i=0; i<10000; i++ ) {
	if ( n > 1 )
	    p->setPen( c[qrnd(n)] );
	p->drawLine(320,240,qrnd(640),qrnd(480));
    }
    return i;
}

static int painter_drawrect()
{
    int i;
    QPainter *p = qperf_painter();
    int n = qperf_maxColors();
    QColor *c = qperf_colors();
    for ( i=0; i<10000; i++ ) {
	if ( n > 1 )
	    p->setPen( c[qrnd(n)] );
	p->drawRect(qrnd(640-200),qrnd(480-200),200,200);
    }
    return i;
}

static int painter_fillrect()
{
    int i;
    QPainter *p = qperf_painter();
    int n = qperf_maxColors();
    QColor *c = qperf_colors();
    p->setPen(Qt::NoPen);
    p->setBrush(c[0]);
    for ( i=0; i<10000; i++ ) {
	if ( n > 1 )
	    p->setBrush( c[qrnd(n)] );
	p->drawRect(qrnd(640-200),qrnd(480-200),200,200);
    }
    return i;
}

static int painter_drawtext()
{
    int i;
    QPainter *p = qperf_painter();
    QString s = "Troll Tech";
    for ( i=0; i<10000; i++ ) {
	p->drawText(qrnd(640),qrnd(480),s);
    }
    return i;
}

static int painter_drawtext_left()
{
    int i;
    QPainter *p = qperf_painter();
    int x, y;
    QString s = "Troll Tech";
    for ( i=0; i<10000; i++ ) {
	x = qrnd(600);
	y = qrnd(480);
	p->drawText(x,y,100,30,Qt::AlignLeft,s);
    }
    return i;
}

static int painter_drawtext_right()
{
    int i;
    QPainter *p = qperf_painter();
    int x, y;
    QString s = "Troll Tech";
    for ( i=0; i<10000; i++ ) {
	x = qrnd(600);
	y = qrnd(480);
	p->drawText(x,y,100,30,Qt::AlignRight,s);
    }
    return i;
}

static int painter_drawtext_center()
{
    int i;
    QPainter *p = qperf_painter();
    int x, y;
    QString s = "Troll Tech";
    for ( i=0; i<10000; i++ ) {
	x = qrnd(600);
	y = qrnd(480);
	p->drawText(x,y,100,30,Qt::AlignCenter,s);
    }
    return i;
}

QPERF_BEGIN(painter,"QPainter tests")
    QPERF(painter_drawline,"Draw line")
    QPERF(painter_drawrect,"Draw rectangle outline")
    QPERF(painter_fillrect,"Draw filled rectangle")
    QPERF(painter_drawtext,"Draw text without formatting")
    QPERF(painter_drawtext_left,"Draw text, left aligned")
    QPERF(painter_drawtext_right,"Draw text, right aligned")
    QPERF(painter_drawtext_center,"Draw text, centered")
QPERF_END(painter)
