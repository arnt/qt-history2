/****************************************************************************
**
** Implementation of the QPicturePaintEngine class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
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
#include "private/qpicture_p.h"
#include "qbuffer.h"
#include "qbytearray.h"
#include "qdatastream.h"
#include "qpaintengine_pic_p.h"
#include "qpicture.h"
#include "qrect.h"

class QPicturePaintEnginePrivate : public QPaintEnginePrivate
{
    Q_DECLARE_PUBLIC(QPicturePaintEngine);
public:
    QDataStream s;
    QPainter *pt;
    QPicturePrivate *pic_d;
};

#define d d_func()
#define q q_func()
#define pic_d d->pic_d

QPicturePaintEngine::QPicturePaintEngine()
    : QPaintEngine(*(new QPicturePaintEnginePrivate), CanRenderText)
{
    d->pt = 0;
}

QPicturePaintEngine::QPicturePaintEngine(QPaintEnginePrivate &dptr)
    : QPaintEngine(dptr, CanRenderText)
{
    d->pt = 0;
}

QPicturePaintEngine::~QPicturePaintEngine()
{
}

// ### serialize unclipped?
bool QPicturePaintEngine::begin(QPaintDevice *pd, QPainterState *state, bool /* unclipped */)
{
    Q_ASSERT(pd);
    QPicture *pic = static_cast<QPicture *>(pd);

    pic_d = pic->d;

    Q_ASSERT(state->painter);
    d->pt = state->painter;
    d->s.setDevice(&pic_d->pictb);
    d->s.setVersion(pic_d->formatMajor);

    pic_d->pictb.open(IO_WriteOnly | IO_Truncate);
    d->s.writeRawBytes(mfhdr_tag, 4);
    d->s << (Q_UINT16) 0 << (Q_UINT16) pic_d->formatMajor << (Q_UINT16) pic_d->formatMinor;
    d->s << (Q_UINT8) PdcBegin << (Q_UINT8) sizeof(Q_INT32);
    pic_d->brect = QRect();
    if (pic_d->formatMajor >= 4) {
        QRect r = pic_d->brect;
        d->s << (Q_INT32) r.left() << (Q_INT32) r.top() << (Q_INT32) r.width()
             << (Q_INT32) r.height();
    }
    pic_d->trecs = 0;
    d->s << (Q_UINT32)pic_d->trecs; // total number of records
    pic_d->formatOk = false;
    setActive(true);
    return true;
}

bool QPicturePaintEngine::end()
{
    pic_d->trecs++;
    d->s << (Q_UINT8) PdcEnd << (Q_UINT8) 0;
    int cs_start = sizeof(Q_UINT32);                // pos of checksum word
    int data_start = cs_start + sizeof(Q_UINT16);
    int brect_start = data_start + 2*sizeof(Q_INT16) + 2*sizeof(Q_UINT8);
    int pos = pic_d->pictb.at();
    pic_d->pictb.at(brect_start);
    if (pic_d->formatMajor >= 4) { // bounding rectangle
        QRect r = pic_d->brect;
        d->s << (Q_INT32) r.left() << (Q_INT32) r.top() << (Q_INT32) r.width()
             << (Q_INT32) r.height();
    }
    d->s << (Q_UINT32) pic_d->trecs;                        // write number of records
    pic_d->pictb.at(cs_start);
    QByteArray buf = pic_d->pictb.buffer();
    Q_UINT16 cs = (Q_UINT16) qChecksum(buf.constData() + data_start, pos - data_start);
    d->s << cs;                                // write checksum
    pic_d->pictb.close();
    setActive(false);
    return true;
}

#define SERIALIZE_CMD(c) \
    pic_d->trecs++; \
    d->s << (Q_UINT8) c; \
    d->s << (Q_UINT8) 0; \
    pos = pic_d->pictb.at()

void QPicturePaintEngine::updatePen(QPainterState *ps)
{
    int pos;
    SERIALIZE_CMD(PdcSetPen);
    d->s << ps->pen;
    writeCmdLength(pos, QRect(), false);
}

void QPicturePaintEngine::updateBrush(QPainterState *ps)
{
    int pos;
    SERIALIZE_CMD(PdcSetBrush);
    d->s << ps->brush;
    writeCmdLength(pos, QRect(), false);
}

void QPicturePaintEngine::updateFont(QPainterState *ps)
{
    int pos;
    SERIALIZE_CMD(PdcSetFont);
    d->s << ps->font;
    writeCmdLength(pos, QRect(), false);
}

void QPicturePaintEngine::updateRasterOp(QPainterState *ps)
{
    pic_d->trecs++;
    d->s << (Q_UINT8) PdcSetROP;
    d->s << (Q_UINT8) 0;
    int pos = pic_d->pictb.at();
    d->s << (Q_INT8) ps->rasterOp;
    writeCmdLength(pos, QRect(), false);
}

void QPicturePaintEngine::updateBackground(QPainterState *ps)
{
    int pos;
    SERIALIZE_CMD(PdcSetBkColor);
    d->s << ps->bgBrush.color();
    writeCmdLength(pos, QRect(), false);

    SERIALIZE_CMD(PdcSetBkMode);
    d->s << (Q_INT8) ps->bgMode;
    writeCmdLength(pos, QRect(), false);
}

// ### Missing implementation?
void QPicturePaintEngine::updateXForm(QPainterState * /* ps */)
{
//     int pos;
//     SERIALIZE_CMD(PdcSetWMatrix);
//     d->s << ps->matrix << (Q_INT8) true; // ### fix combine param
//     writeCmdLength(pos, QRect(), false);

//     SERIALIZE_CMD(PdcSetWXform);
//     d->s << (Q_INT8) ps->WxF;
//     writeCmdLength(pos, QRect(), false);

//     SERIALIZE_CMD(PdcSetVXform);
//     d->s << (Q_INT8) ps->VxF;
//     writeCmdLength(pos, QRect(), false);
}

void QPicturePaintEngine::updateClipRegion(QPainterState *ps)
{
    int pos;
    SERIALIZE_CMD(PdcSetClipRegion);
    d->s << ps->clipRegion << Q_INT8(0);
    writeCmdLength(pos, QRect(), false);

    SERIALIZE_CMD(PdcSetClip);
    d->s << (Q_INT8) ps->clipEnabled;
    writeCmdLength(pos, QRect(), false);
}

void QPicturePaintEngine::writeCmdLength(int pos, const QRect &r, bool corr)
{
    int newpos = pic_d->pictb.at();                // new position
    int length = newpos - pos;
    QRect br(r);

    if (length < 255) {                        // write 8-bit length
        pic_d->pictb.at(pos - 1);                        // position to right index
        d->s << (Q_UINT8)length;
    } else {                                        // write 32-bit length
        d->s << (Q_UINT32)0;                                // extend the buffer
        pic_d->pictb.at(pos - 1);                        // position to right index
        d->s << (Q_UINT8)255;                        // indicate 32-bit length
        char *p = pic_d->pictb.buffer().data();
        memmove(p+pos+4, p+pos, length);        // make room for 4 byte
        d->s << (Q_UINT32)length;
        newpos += 4;
    }
    pic_d->pictb.at(newpos);                                // set to new position

    if (br.isValid()) {
        if (corr) {                                // widen bounding rect
            int w2 = d->pt->pen().width() / 2;
            br.setCoords(br.left() - w2, br.top() - w2,
                          br.right() + w2, br.bottom() + w2);
        }
#ifndef QT_NO_TRANSFORMATIONS
        br = d->pt->worldMatrix().map(br);
#endif
        if (d->pt->hasClipping()) {
            QRect cr = d->pt->clipRegion().boundingRect();
            br &= cr;
        }
        if (br.isValid())
            pic_d->brect |= br;                        // merge with existing rect
    }
}

void QPicturePaintEngine::drawLine(const QPoint &p1, const QPoint &p2)
{
    int pos;
    SERIALIZE_CMD(PdcDrawLine);
    d->s << p1 << p2;
    writeCmdLength(pos, QRect(p1, p2).normalize(), true);
}

void QPicturePaintEngine::drawRect(const QRect &r)
{
    int pos;
    SERIALIZE_CMD(PdcDrawRect);
    d->s << r;
    writeCmdLength(pos, r, true);
}

void QPicturePaintEngine::drawPoint(const QPoint &p)
{
    int pos;
    SERIALIZE_CMD(PdcDrawPoint);
    d->s << p;
    writeCmdLength(pos, QRect(p,p), true);
}

void QPicturePaintEngine::drawPoints(const QPointArray &a, int index, int npoints)
{
    int pos;
    SERIALIZE_CMD(PdcDrawPoints);
    d->s << a << (Q_INT32) index << (Q_INT32) npoints;
    writeCmdLength(pos, a.boundingRect(), true);
}

void QPicturePaintEngine::drawRoundRect(const QRect &r, int xRnd, int yRnd)
{
    int pos;
    SERIALIZE_CMD(PdcDrawRoundRect);
    d->s << r << (Q_INT16)xRnd << (Q_INT16)yRnd;
    writeCmdLength(pos, r, true);
}

void QPicturePaintEngine::drawEllipse(const QRect &r)
{
    int pos;
    SERIALIZE_CMD(PdcDrawEllipse);
    d->s << r;
    writeCmdLength(pos, r, true);
}

void QPicturePaintEngine::drawArc(const QRect &r, int a, int alen)
{
    int pos;
    SERIALIZE_CMD(PdcDrawArc);
    d->s << r << (Q_INT16)a << (Q_INT16)alen;
    writeCmdLength(pos, r, true);
}

void QPicturePaintEngine::drawPie(const QRect &r, int _a, int alen)
{
    int pos;
    SERIALIZE_CMD(PdcDrawPie);
    d->s << r << (Q_INT16)_a << (Q_INT16)alen;
    writeCmdLength(pos, r, true);
}

void QPicturePaintEngine::drawChord(const QRect &r, int _a, int alen)
{
    int pos;
    SERIALIZE_CMD(PdcDrawChord);
    d->s << r << (Q_INT16)_a << (Q_INT16)alen;
    writeCmdLength(pos, r, true);
}

// ### Stream out index and nlines
void QPicturePaintEngine::drawLineSegments(const QPointArray &a, int /* index */, int /* nlines */)
{
    int pos;
    SERIALIZE_CMD(PdcDrawLineSegments);
    d->s << a;
    writeCmdLength(pos, a.boundingRect(), true);
}

// ### Stream out index and npoints
void QPicturePaintEngine::drawPolyline(const QPointArray &a, int /* index */, int /* npoints */)
{
    int pos;
    SERIALIZE_CMD(PdcDrawPolyline);
    d->s << a;
    writeCmdLength(pos, a.boundingRect(), true);
}

void QPicturePaintEngine::drawPolygon(const QPointArray &a, bool winding, int, int)
{
    int pos;
    SERIALIZE_CMD(PdcDrawPolygon);
    d->s << a << (Q_INT8) winding;
    writeCmdLength(pos, a.boundingRect(), true);
}

void QPicturePaintEngine::drawConvexPolygon(const QPointArray &a, int index, int npoints)
{
    drawPolygon(a, false, index, npoints);
}

// ### Stream out: index:
void QPicturePaintEngine::drawCubicBezier(const QPointArray &a, int /* index */)
{
#ifndef QT_NO_BEZIER
    int pos;
    SERIALIZE_CMD(PdcDrawCubicBezier);
    d->s << a;
    writeCmdLength(pos, a.cubicBezier().boundingRect(), true);
#else
    (void) a;
#endif
}

// ### Stream out sr
void QPicturePaintEngine::drawPixmap(const QRect &r, const QPixmap &pm, const QRect & /* sr */, bool /* imask */)
{
    int pos;
    SERIALIZE_CMD(PdcDrawPixmap);
    d->s << r << pm;
    writeCmdLength(pos, r, false);
}

void QPicturePaintEngine::drawTiledPixmap(const QRect &r, const QPixmap &pixmap, const QPoint &s, bool)
{
    int pos;
    SERIALIZE_CMD(PdcDrawTiledPixmap);
    d->s << r << pixmap << s;
    writeCmdLength(pos, r, false);
}

// ### Implementation missing?
void QPicturePaintEngine::drawTextItem(const QPoint &/* p */, const QTextItem &/* ti */, int /* textflags */)
{
//     int pos;
//     SERIALIZE_CMD(PdcDrawText2);
//     d->s << p << QString(ti.chars, ti.num_chars);
//     writeCmdLength(pos, QRect(p, p), true);
}
