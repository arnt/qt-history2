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

static int painter_drawpoly()
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
	QPointArray a(3);
	a[0] = QPoint(qrnd(640),qrnd(480));
	a[1] = QPoint(qrnd(640),qrnd(480));
	a[2] = QPoint(qrnd(640),qrnd(480));
	p->drawPolygon( a );
    }
    return i;
}

static int painter_drawstring(const QString &s)
{
    int i;
    QPainter *p = qperf_painter();
    for ( i=0; i<10000; i++ ) {
	p->drawText(qrnd(640),qrnd(480),s);
    }
    return i;
}

static int painter_drawstring(const QString &s, int f)
{
    int i;
    QPainter *p = qperf_painter();
    for ( i=0; i<10000; i++ ) 
	p->drawText(qrnd(600),qrnd(480),100,30,f,s);
    return i;
}

static inline QString text_string() { return "Trolltech"; }
static inline int painter_drawtext() { return painter_drawstring(text_string()); }
static inline int painter_drawtext_left() { return painter_drawstring(text_string(), Qt::AlignLeft); }
static inline int painter_drawtext_right() { return painter_drawstring(text_string(), Qt::AlignRight); }
static inline int painter_drawtext_center() { return painter_drawstring(text_string(), Qt::AlignCenter); }

static inline const QString &unicode_string() 
{ 
    static QString ret;
    if(ret.isNull()) {
	ret += QChar(0x00C3); //latin1
	ret += QChar(0x03B1); //greek
	ret += QChar(0x820a); //han
	ret += QChar(0x0628); //unicode
    }
    return ret; 
}
static inline int painter_drawunicode() { return painter_drawstring(unicode_string()); }
static inline int painter_drawunicode_left() { return painter_drawstring(unicode_string(), Qt::AlignLeft); }
static inline int painter_drawunicode_right() { return painter_drawstring(unicode_string(), Qt::AlignRight); }
static inline int painter_drawunicode_center() { return painter_drawstring(unicode_string(), Qt::AlignCenter); }

static int painter_save()
{
    int i;
    QPainter *p = qperf_painter();
    for ( i=0; i<10000; i++ ) {
	p->save();
	p->save();
	p->restore();
	p->restore();
    }
    return i;
}


QPERF_BEGIN(painter,"QPainter tests")
    QPERF(painter_drawline,"Draw line")
    QPERF(painter_drawrect,"Draw rectangle outline")
    QPERF(painter_fillrect,"Draw filled rectangle")
    QPERF(painter_drawpoly,"Draw polygon")
    QPERF(painter_drawtext,"Draw text without formatting")
    QPERF(painter_drawtext_left,"Draw text, left aligned")
    QPERF(painter_drawtext_right,"Draw text, right aligned")
    QPERF(painter_drawtext_center,"Draw text, centered")
    QPERF(painter_drawunicode,"Draw unicode-text without formatting")
    QPERF(painter_drawunicode_left,"Draw unicode-text, left aligned")
    QPERF(painter_drawunicode_right,"Draw unicode-text, right aligned")
    QPERF(painter_drawunicode_center,"Draw unicode-text, centered")
    QPERF(painter_save,"Save and restore")
QPERF_END(painter)
