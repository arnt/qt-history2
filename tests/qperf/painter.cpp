#include "qperf.h"
#include <qpixmap.h>
#include <qbitmap.h>
#include <qpainter.h>
#include <qimage.h>
#include <stdlib.h>


static void painter_init()
{
}

static int painter_drawtext()
{
    int i;
    QPainter *p = qperf_painter();
    QString s = "Troll Tech";
    for ( i=0; i<10000; i++ ) {
	p->drawText(qrnd(600),qrnd(480),s);
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
    QPERF(painter_drawtext,"Draw text without formatting")
    QPERF(painter_drawtext_left,"Draw text, left aligned")
    QPERF(painter_drawtext_right,"Draw text, right aligned")
    QPERF(painter_drawtext_center,"Draw text, centered")
QPERF_END(painter)
