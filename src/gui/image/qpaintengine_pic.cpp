/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
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
#include "qpolygon.h"
#include "qrect.h"

class QPicturePaintEnginePrivate : public QPaintEnginePrivate
{
    Q_DECLARE_PUBLIC(QPicturePaintEngine)
public:
    QDataStream s;
    QPainter *pt;
    QPicturePrivate *pic_d;
};

#define d d_func()
#define q q_func()
#define pic_d d->pic_d

QPicturePaintEngine::QPicturePaintEngine()
    : QPaintEngine(*(new QPicturePaintEnginePrivate))
{
    d->pt = 0;
}

QPicturePaintEngine::QPicturePaintEngine(QPaintEnginePrivate &dptr)
    : QPaintEngine(dptr)
{
    d->pt = 0;
}

QPicturePaintEngine::~QPicturePaintEngine()
{
}

bool QPicturePaintEngine::begin(QPaintDevice *pd)
{
    Q_ASSERT(pd);
    QPicture *pic = static_cast<QPicture *>(pd);

    d->pdev = pd;
    pic_d = pic->d;

    d->s.setDevice(&pic_d->pictb);
    d->s.setVersion(pic_d->formatMajor);

    pic_d->pictb.open(QIODevice::WriteOnly | QIODevice::Truncate);
    d->s.writeRawData(qt_mfhdr_tag, 4);
    d->s << (Q_UINT16) 0 << (Q_UINT16) pic_d->formatMajor << (Q_UINT16) pic_d->formatMinor;
    d->s << (Q_UINT8) QPicturePrivate::PdcBegin << (Q_UINT8) sizeof(Q_INT32);
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
    d->pdev = 0;
    pic_d->trecs++;
    d->s << (Q_UINT8) QPicturePrivate::PdcEnd << (Q_UINT8) 0;
    int cs_start = sizeof(Q_UINT32);                // pos of checksum word
    int data_start = cs_start + sizeof(Q_UINT16);
    int brect_start = data_start + 2*sizeof(Q_INT16) + 2*sizeof(Q_UINT8);
    int pos = pic_d->pictb.pos();
    pic_d->pictb.seek(brect_start);
    if (pic_d->formatMajor >= 4) { // bounding rectangle
        QRect r = pic_d->brect;
        d->s << (Q_INT32) r.left() << (Q_INT32) r.top() << (Q_INT32) r.width()
             << (Q_INT32) r.height();
    }
    d->s << (Q_UINT32) pic_d->trecs;                        // write number of records
    pic_d->pictb.seek(cs_start);
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
    pos = pic_d->pictb.pos()

void QPicturePaintEngine::updatePen(const QPen &pen)
{
    int pos;
    SERIALIZE_CMD(QPicturePrivate::PdcSetPen);
    d->s << pen;
    writeCmdLength(pos, QRect(), false);
}

void QPicturePaintEngine::updateBrush(const QBrush &brush, const QPointF &)
{
    int pos;
    SERIALIZE_CMD(QPicturePrivate::PdcSetBrush);
    d->s << brush;
    writeCmdLength(pos, QRect(), false);
}

void QPicturePaintEngine::updateFont(const QFont &font)
{
    int pos;
    SERIALIZE_CMD(QPicturePrivate::PdcSetFont);
    QFont fnt = font;
    // set pixel size to be device independent
    if (fnt.pointSize() > 0)
        fnt.setPixelSize(fnt.pointSize());
    d->s << font;
    writeCmdLength(pos, QRectF(), false);
}

void QPicturePaintEngine::updateBackground(Qt::BGMode bgMode, const QBrush &bgBrush)
{
    int pos;
    SERIALIZE_CMD(QPicturePrivate::PdcSetBkColor);
    d->s << bgBrush.color();
    writeCmdLength(pos, QRect(), false);

    SERIALIZE_CMD(QPicturePrivate::PdcSetBkMode);
    d->s << (Q_INT8) bgMode;
    writeCmdLength(pos, QRectF(), false);
}

void QPicturePaintEngine::updateMatrix(const QMatrix &matrix)
{
    int pos;
    SERIALIZE_CMD(QPicturePrivate::PdcSetWMatrix);
    d->s << matrix << (Q_INT8) false;
    writeCmdLength(pos, QRectF(), false);
}

void QPicturePaintEngine::updateClipRegion(const QRegion &region, Qt::ClipOperation op)
{
    Q_UNUSED(op);
    int pos;
    SERIALIZE_CMD(QPicturePrivate::PdcSetClipRegion);
    d->s << region << Q_INT8(0);
    writeCmdLength(pos, QRectF(), false);

    SERIALIZE_CMD(QPicturePrivate::PdcSetClip);
    d->s << (Q_INT8) !region.isEmpty();
    writeCmdLength(pos, QRectF(), false);
}

void QPicturePaintEngine::writeCmdLength(int pos, const QRectF &r, bool corr)
{
    int newpos = pic_d->pictb.pos();                // new position
    int length = newpos - pos;
    QRect br = r.toRect();

    if (length < 255) {                        // write 8-bit length
        pic_d->pictb.seek(pos - 1);                        // position to right index
        d->s << (Q_UINT8)length;
    } else {                                        // write 32-bit length
        d->s << (Q_UINT32)0;                                // extend the buffer
        pic_d->pictb.seek(pos - 1);                        // position to right index
        d->s << (Q_UINT8)255;                        // indicate 32-bit length
        char *p = pic_d->pictb.buffer().data();
        memmove(p+pos+4, p+pos, length);        // make room for 4 byte
        d->s << (Q_UINT32)length;
        newpos += 4;
    }
    pic_d->pictb.seek(newpos);                                // set to new position

    if (br.isValid()) {
        if (corr) {                                // widen bounding rect
            int w2 = painter()->pen().width() / 2;
            br.setCoords(br.left() - w2, br.top() - w2,
                          br.right() + w2, br.bottom() + w2);
        }
#ifndef QT_NO_TRANSFORMATIONS
        br = painter()->matrix().mapRect(br);
#endif
        if (painter()->hasClipping()) {
            QRect cr = painter()->clipRegion().boundingRect();
            br &= cr;
        }
        if (br.isValid())
            pic_d->brect |= br;                        // merge with existing rect
    }
}

void QPicturePaintEngine::drawLine(const QLineF &line)
{
    int pos;
    SERIALIZE_CMD(QPicturePrivate::PdcDrawLine);
    d->s << line.start() << line.end();
    writeCmdLength(pos, QRectF(line.startX(), line.startY(), line.vx(), line.vy()).normalize(), true);
}

void QPicturePaintEngine::drawRect(const QRectF &r)
{
    int pos;
    SERIALIZE_CMD(QPicturePrivate::PdcDrawRect);
    d->s << r;
    writeCmdLength(pos, r, true);
}

void QPicturePaintEngine::drawPoint(const QPointF &p)
{
    int pos;
    SERIALIZE_CMD(QPicturePrivate::PdcDrawPoint);
    d->s << p;
    writeCmdLength(pos, QRectF(p, QSizeF(1, 1)), true);
}

void QPicturePaintEngine::drawEllipse(const QRectF &r)
{
    int pos;
    SERIALIZE_CMD(QPicturePrivate::PdcDrawEllipse);
    d->s << r;
    writeCmdLength(pos, r, true);
}

void QPicturePaintEngine::drawPolygon(const QPointF *points, int pointCount, PolygonDrawMode mode)
{
    int pos;
    // ### fix me
    QPolygonF p;
    p.reserve(pointCount);
    for (int i=0; i<pointCount; ++i)
        p << points[i];
    if (mode == PolylineMode) {
        SERIALIZE_CMD(QPicturePrivate::PdcDrawPolyline);
        d->s << p;
        writeCmdLength(pos, p.boundingRect(), true);
    } else {
        SERIALIZE_CMD(QPicturePrivate::PdcDrawPolygon);
        d->s << p << (Q_INT8) (mode == WindingMode);
        writeCmdLength(pos, p.boundingRect(), true);
    }
}

void QPicturePaintEngine::drawPolygon(const QPoint *points, int pointCount, PolygonDrawMode mode)
{
    QPolygonF p;
    p.reserve(pointCount);
    for (int i=0; i<pointCount; ++i)
        p << points[i];
    drawPolygon(p.data(), pointCount, mode);
}

// ### Stream out sr
void QPicturePaintEngine::drawPixmap(const QRectF &r, const QPixmap &pm, const QRectF & /* sr */,
                                     Qt::PixmapDrawingMode /* mode */)
{
    int pos;
    SERIALIZE_CMD(QPicturePrivate::PdcDrawPixmap);
    d->s << r << pm;
    writeCmdLength(pos, r, false);
}

void QPicturePaintEngine::drawTiledPixmap(const QRectF &r, const QPixmap &pixmap, const QPointF &s,
					  Qt::PixmapDrawingMode)
{
    int pos;
    SERIALIZE_CMD(QPicturePrivate::PdcDrawTiledPixmap);
    d->s << r << pixmap << s;
    writeCmdLength(pos, r, false);
}

void QPicturePaintEngine::drawTextItem(const QPointF &p , const QTextItem &ti)
{
    int pos;
    SERIALIZE_CMD(QPicturePrivate::PdcDrawText2);
    d->s << p << QString(ti.chars, ti.num_chars);
    writeCmdLength(pos, QRectF(p, QSizeF(1,1)), true);
}
