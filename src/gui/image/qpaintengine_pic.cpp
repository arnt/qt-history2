/****************************************************************************
**
** Implementation of the QPicturePaintEngine class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the xml module of the Qt GUI Toolkit.
** EDITIONS: FREE, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "private/qpaintengine_p.h"
#include "private/qpainter_p.h"
#include "qbuffer.h"
#include "qbytearray.h"
#include "qdatastream.h"
#include "qpaintengine_pic_p.h"
#include "qrect.h"

static const char  *mfhdr_tag = "QPIC";		// header tag
static const Q_UINT16 mfhdr_maj = 6;		// major version #
static const Q_UINT16 mfhdr_min = 0;		// minor version #

class QPicturePaintEnginePrivate : public QPaintEnginePrivate
{
    Q_DECL_PUBLIC(QPicturePaintEngine);

public:
    QBuffer *pictb;
    QDataStream s;

    int trecs;
    QRect brect;
    bool formatOk;
    int	formatMajor;
    int	formatMinor;
    QPainter *pt;
};

#define d d_func()
#define q q_func()

QPicturePaintEngine::QPicturePaintEngine(QBuffer *buf)
    : QPaintEngine(*(new QPicturePaintEnginePrivate), CanRenderText)
{
    Q_ASSERT(buf);
    d->pictb = buf;
    d->formatMajor = mfhdr_maj;
    d->formatMinor = mfhdr_min;
    d->formatOk = false;
    d->pt = 0;
    d->trecs = 0;
}

QPicturePaintEngine::QPicturePaintEngine(QPaintEnginePrivate &dptr, QBuffer *buf)
    : QPaintEngine(dptr, CanRenderText)
{
    Q_ASSERT(buf);
    d->pictb = buf;
    d->formatMajor = mfhdr_maj;
    d->formatMinor = mfhdr_min;
    d->formatOk = false;
    d->pt = 0;
    d->trecs = 0;
}

QPicturePaintEngine::~QPicturePaintEngine()
{
}

bool QPicturePaintEngine::begin(const QPaintDevice *pdev, QPainterState *state, bool unclipped)
{
    Q_ASSERT(state->painter);
    d->pt = state->painter;
    d->s.setDevice(d->pictb);
    d->s.setVersion(d->formatMajor);

    QByteArray empty;
    d->pictb->setBuffer( empty );		// reset byte array in buffer
    d->pictb->open( IO_WriteOnly );
    d->s.writeRawBytes( mfhdr_tag, 4 );
    d->s << (Q_UINT16)0 << (Q_UINT16)d->formatMajor << (Q_UINT16)d->formatMinor;
    d->s << (Q_UINT8)PdcBegin << (Q_UINT8)sizeof(Q_INT32);
    d->brect = QRect();
    if ( d->formatMajor >= 4 ) {
	d->s << (Q_INT32)d->brect.left() << (Q_INT32)d->brect.top()
	  << (Q_INT32)d->brect.width() << (Q_INT32)d->brect.height();
    }
    d->trecs = 0;
    d->s << (Q_UINT32)d->trecs;			// total number of records
    d->formatOk = false;
    setActive(true);
    return true;
}

bool QPicturePaintEngine::end()
{
    d->trecs++;
    d->s << (Q_UINT8)PdcEnd << (Q_UINT8)0;
    int cs_start = sizeof(Q_UINT32);		// pos of checksum word
    int data_start = cs_start + sizeof(Q_UINT16);
    int brect_start = data_start + 2*sizeof(Q_INT16) + 2*sizeof(Q_UINT8);
    int pos = d->pictb->at();
    d->pictb->at( brect_start );
    if ( d->formatMajor >= 4 ) { // bounding rectangle
	d->s << (Q_INT32)d->brect.left() << (Q_INT32)d->brect.top()
	  << (Q_INT32)d->brect.width() << (Q_INT32)d->brect.height();
    }
    d->s << (Q_UINT32)d->trecs;			// write number of records
    d->pictb->at( cs_start );
    QByteArray buf = d->pictb->buffer();
    Q_UINT16 cs = (Q_UINT16)qChecksum( buf.constData()+data_start, pos-data_start );
    d->s << cs;				// write checksum
    d->pictb->close();
    setActive(false);
    return true;
}

void QPicturePaintEngine::updatePen(QPainterState *ps)
{
    d->trecs++;
    d->s << (Q_UINT8)PdcSetPen;
    d->s << (Q_UINT8)0;
    int pos = d->pictb->at();
    d->s << ps->pen;
    writeCmdLength(pos, QRect(), false);
}

void QPicturePaintEngine::updateBrush(QPainterState *ps)
{
    d->trecs++;
    d->s << (Q_UINT8)PdcSetBrush;
    d->s << (Q_UINT8)0;
    int pos = d->pictb->at();
    d->s << ps->brush;
    writeCmdLength(pos, QRect(), false);
}

void QPicturePaintEngine::updateFont(QPainterState *ps)
{
    d->trecs++;
    d->s << (Q_UINT8)PdcSetFont;
    d->s << (Q_UINT8)0;
    int pos = d->pictb->at();
    d->s << ps->font;
    writeCmdLength(pos, QRect(), false);
}

void QPicturePaintEngine::updateRasterOp(QPainterState *ps)
{
    d->trecs++;
    d->s << (Q_UINT8) PdcSetROP;
    d->s << (Q_UINT8) 0;
    int pos = d->pictb->at();
    d->s << (Q_INT8) ps->rasterOp;
    writeCmdLength(pos, QRect(), false);
}

void QPicturePaintEngine::updateBackground(QPainterState *ps)
{
    d->trecs++;
    d->s << (Q_UINT8) PdcSetBkColor;
    d->s << (Q_UINT8) 0;
    int pos = d->pictb->at();
    d->s << ps->bgColor;
    writeCmdLength(pos, QRect(), false);
    d->trecs++;
    d->s << (Q_UINT8) PdcSetBkMode;
    d->s << (Q_UINT8) 0;
    pos = d->pictb->at();
    d->s << (Q_INT8) ps->bgMode;
    writeCmdLength(pos, QRect(), false);
}

void QPicturePaintEngine::updateXForm(QPainterState *ps)
{
//     d->trecs++;
//     d->s << (Q_UINT8)PdcSetWMatrix;
//     d->s << (Q_UINT8)0;
//     int pos = d->pictb->at();
//     d->s << ps->matrix << (Q_INT8) false; // ### fix combine param
//     writeCmdLength(pos, QRect(), false);
}

void QPicturePaintEngine::updateClipRegion(QPainterState *ps)
{
    d->trecs++;
    d->s << (Q_UINT8)PdcSetClipRegion;
    d->s << (Q_UINT8)0;
    int pos = d->pictb->at();
    d->s << ps->clipRegion << (Q_INT8) QPainter::CoordDevice; // ### fix coord mode
    writeCmdLength(pos, QRect(), false);
}

void QPicturePaintEngine::writeCmdLength(int pos, const QRect &r, bool corr)
{
    int newpos = d->pictb->at();		// new position
    int length = newpos - pos;
    QRect br(r);

    if ( length < 255 ) {			// write 8-bit length
	d->pictb->at(pos - 1);			// position to right index
	d->s << (Q_UINT8)length;
    } else {					// write 32-bit length
	d->s << (Q_UINT32)0;				// extend the buffer
	d->pictb->at(pos - 1);			// position to right index
	d->s << (Q_UINT8)255;			// indicate 32-bit length
	char *p = d->pictb->buffer().data();
	memmove( p+pos+4, p+pos, length );	// make room for 4 byte
	d->s << (Q_UINT32)length;
	newpos += 4;
    }
    d->pictb->at( newpos );				// set to new position

    if ( br.isValid() ) {
	if ( corr ) {				// widen bounding rect
	    int w2 = d->pt->pen().width() / 2;
	    br.setCoords( br.left() - w2, br.top() - w2,
			  br.right() + w2, br.bottom() + w2 );
	}
#ifndef QT_NO_TRANSFORMATIONS
	br = d->pt->worldMatrix().map( br );
#endif
	if ( d->pt->hasClipping() ) {
	    QRect cr = d->pt->clipRegion().boundingRect();
	    br &= cr;
	}
	if ( br.isValid() )
	    d->brect |= br;		     	// merge with existing rect
    }
}

void QPicturePaintEngine::drawLine(const QPoint &p1, const QPoint &p2)
{
    d->trecs++;
    d->s << (Q_UINT8)PdcDrawLine;
    d->s << (Q_UINT8)0;
    int pos = d->pictb->at();
    d->s << p1 << p2;
    writeCmdLength(pos, QRect(p1, p2).normalize(), true);
}

void QPicturePaintEngine::drawRect(const QRect &r)
{
    d->trecs++;
    d->s << (Q_UINT8)PdcDrawRect;
    d->s << (Q_UINT8)0;
    int pos = d->pictb->at();
    d->s << r;
    writeCmdLength(pos, r, true);
}

void QPicturePaintEngine::drawPoint(const QPoint &p)
{
    d->trecs++;
    d->s << (Q_UINT8)PdcDrawPoint;
    d->s << (Q_UINT8)0;
    int pos = d->pictb->at();
    d->s << p;
    writeCmdLength(pos, QRect(p,p), true);
}

void QPicturePaintEngine::drawPoints(const QPointArray &pa, int index, int npoints)
{
}

void QPicturePaintEngine::drawWinFocusRect(const QRect &r, bool xorPaint, const QColor &bgColor)
{
}

void QPicturePaintEngine::drawRoundRect(const QRect &r, int xRnd, int yRnd)
{
    d->trecs++;
    d->s << (Q_UINT8)PdcDrawRoundRect;
    d->s << (Q_UINT8)0;
    int pos = d->pictb->at();
    d->s << r << (Q_INT16)xRnd << (Q_INT16)yRnd;
    writeCmdLength(pos, r, true);
}

void QPicturePaintEngine::drawEllipse(const QRect &r)
{
    d->trecs++;
    d->s << (Q_UINT8)PdcDrawEllipse;
    d->s << (Q_UINT8)0;
    int pos = d->pictb->at();
    d->s << r;
    writeCmdLength(pos, r, true);
}

void QPicturePaintEngine::drawArc(const QRect &r, int a, int alen)
{
    d->trecs++;
    d->s << (Q_UINT8)PdcDrawArc;
    d->s << (Q_UINT8)0;
    int pos = d->pictb->at();
    d->s << r << (Q_INT16)a << (Q_INT16)alen;
    writeCmdLength(pos, r, true);
}

void QPicturePaintEngine::drawPie(const QRect &r, int _a, int alen)
{
    d->trecs++;
    d->s << (Q_UINT8)PdcDrawPie;
    d->s << (Q_UINT8)0;
    int pos = d->pictb->at();
    d->s << r << (Q_INT16)_a << (Q_INT16)alen;
    writeCmdLength(pos, r, true);
}

void QPicturePaintEngine::drawChord(const QRect &r, int _a, int alen)
{
    d->trecs++;
    d->s << (Q_UINT8)PdcDrawChord;
    d->s << (Q_UINT8)0;
    int pos = d->pictb->at();
    d->s << r << (Q_INT16)_a << (Q_INT16)alen;
    writeCmdLength(pos, r, true);
}

void QPicturePaintEngine::drawLineSegments(const QPointArray &a, int index, int nlines)
{
    d->trecs++;
    d->s << (Q_UINT8)PdcDrawLineSegments;
    d->s << (Q_UINT8)0;
    int pos = d->pictb->at();
    d->s << a;
    writeCmdLength(pos, a.boundingRect(), true);
}

void QPicturePaintEngine::drawPolyline(const QPointArray &a, int index, int npoints)
{
    d->trecs++;
    d->s << (Q_UINT8)PdcDrawPolyline;
    d->s << (Q_UINT8)0;
    int pos = d->pictb->at();
    d->s << a;
    writeCmdLength(pos, a.boundingRect(), true);
}

void QPicturePaintEngine::drawPolygon(const QPointArray &a, bool winding, int, int)
{
    d->trecs++;
    d->s << (Q_UINT8)PdcDrawPolygon;
    d->s << (Q_UINT8)0;
    int pos = d->pictb->at();
    d->s << a << (Q_INT8) winding;
    writeCmdLength(pos, a.boundingRect(), true);
}

void QPicturePaintEngine::drawConvexPolygon(const QPointArray &a, int index, int npoints)
{
    drawPolygon(a, false, index, npoints);
}

void QPicturePaintEngine::drawCubicBezier(const QPointArray &a, int index)
{
#ifndef QT_NO_BEZIER
    d->trecs++;
    d->s << (Q_UINT8)PdcDrawCubicBezier;
    d->s << (Q_UINT8)0;
    int pos = d->pictb->at();
    d->s << a;
    writeCmdLength(pos, a.cubicBezier().boundingRect(), true);
#endif
}

void QPicturePaintEngine::drawPixmap(const QRect &r, const QPixmap &pm, const QRect &sr)
{
}

void QPicturePaintEngine::drawTiledPixmap(const QRect &r, const QPixmap &pixmap, const QPoint &s, bool optim)
{
}

void QPicturePaintEngine::drawTextItem(const QPoint &p, const QTextItem &ti, int textflags)
{
    d->trecs++;
    d->s << (Q_UINT8)PdcDrawText2;
    d->s << (Q_UINT8)0;
    int pos = d->pictb->at();
    d->s << p << QString(ti.chars, ti.num_chars);
    writeCmdLength(pos, QRect(p, p), true);
}
