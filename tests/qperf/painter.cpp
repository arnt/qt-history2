#include "qperf.h"
#include <qpixmap.h>
#include <qbitmap.h>
#include <qpainter.h>
#include <qimage.h>


static void painter_init()
{
}

static int painter_drawtext()
{
    int i;
    QPainter *p = qperf_painter();
    for ( i=0; i<10000; i++ ) {
	p->drawText(qrnd(600),qrnd(480),"Troll Tech");
    }
    return i;
}

QPERF_BEGIN(painter,"QPainter tests")
    QPERF(painter_drawtext,"Draw text without formatting")
QPERF_END(painter)
