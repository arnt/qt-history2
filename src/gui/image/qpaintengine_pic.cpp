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

// #define QT_PICTURE_DEBUG
#ifdef QT_PICTURE_DEBUG
#include <qdebug.h>
#endif

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
    : QPaintEngine(*(new QPicturePaintEnginePrivate),
                   CoordTransform
                   | PenWidthTransform
                   | PatternBrush
                   | PixmapTransform
                   | LinearGradientFill
                   | LinearGradientFillPolygon
                   | PixmapScale
                   | AlphaFill
                   | AlphaFillPolygon
                   | AlphaStroke
                   | AlphaPixmap
                   | PainterPaths
                   | ClipTransform
                   | LineAntialiasing
                   | FillAntialiasing
                   | PaintOutsidePaintEvent)
{
    d->pt = 0;
}

QPicturePaintEngine::QPicturePaintEngine(QPaintEnginePrivate &dptr)
    : QPaintEngine(dptr,
                   CoordTransform
                   | PenWidthTransform
                   | PatternBrush
                   | PixmapTransform
                   | LinearGradientFill
                   | LinearGradientFillPolygon
                   | PixmapScale
                   | AlphaFill
                   | AlphaFillPolygon
                   | AlphaStroke
                   | AlphaPixmap
                   | PainterPaths
                   | ClipTransform
                   | LineAntialiasing
                   | FillAntialiasing
                   | PaintOutsidePaintEvent)
{
    d->pt = 0;
}

QPicturePaintEngine::~QPicturePaintEngine()
{
}

bool QPicturePaintEngine::begin(QPaintDevice *pd)
{
#ifdef QT_PICTURE_DEBUG
    qDebug() << "QPicturePaintEngine::begin()";
#endif
    Q_ASSERT(pd);
    QPicture *pic = static_cast<QPicture *>(pd);

    d->pdev = pd;
    pic_d = pic->d;
    Q_ASSERT(pic_d);

    d->s.setDevice(&pic_d->pictb);
    d->s.setVersion(pic_d->formatMajor);

    pic_d->pictb.open(QIODevice::WriteOnly | QIODevice::Truncate);
    d->s.writeRawData(qt_mfhdr_tag, 4);
    d->s << (quint16) 0 << (quint16) pic_d->formatMajor << (quint16) pic_d->formatMinor;
    d->s << (quint8) QPicturePrivate::PdcBegin << (quint8) sizeof(qint32);
    pic_d->brect = QRect();
    if (pic_d->formatMajor >= 4) {
        QRect r = pic_d->brect;
        d->s << (qint32) r.left() << (qint32) r.top() << (qint32) r.width()
             << (qint32) r.height();
    }
    pic_d->trecs = 0;
    d->s << (quint32)pic_d->trecs; // total number of records
    pic_d->formatOk = false;
    setActive(true);
    return true;
}

bool QPicturePaintEngine::end()
{
#ifdef QT_PICTURE_DEBUG
    qDebug() << "QPicturePaintEngine::end()";
#endif
    d->pdev = 0;
    pic_d->trecs++;
    d->s << (quint8) QPicturePrivate::PdcEnd << (quint8) 0;
    int cs_start = sizeof(quint32);                // pos of checksum word
    int data_start = cs_start + sizeof(quint16);
    int brect_start = data_start + 2*sizeof(qint16) + 2*sizeof(quint8);
    int pos = pic_d->pictb.pos();
    pic_d->pictb.seek(brect_start);
    if (pic_d->formatMajor >= 4) { // bounding rectangle
        QRect r = pic_d->brect;
        d->s << (qint32) r.left() << (qint32) r.top() << (qint32) r.width()
             << (qint32) r.height();
    }
    d->s << (quint32) pic_d->trecs;                        // write number of records
    pic_d->pictb.seek(cs_start);
    QByteArray buf = pic_d->pictb.buffer();
    quint16 cs = (quint16) qChecksum(buf.constData() + data_start, pos - data_start);
    d->s << cs;                                // write checksum
    pic_d->pictb.close();
    setActive(false);
    return true;
}

#define SERIALIZE_CMD(c) \
    pic_d->trecs++; \
    d->s << (quint8) c; \
    d->s << (quint8) 0; \
    pos = pic_d->pictb.pos()

void QPicturePaintEngine::updatePen(const QPen &pen)
{
#ifdef QT_PICTURE_DEBUG
    qDebug() << " -> updatePen(): width:" << pen.width() << "style:"
             << pen.style() << "color:" << pen.color();
#endif
    int pos;
    SERIALIZE_CMD(QPicturePrivate::PdcSetPen);
    d->s << pen;
    writeCmdLength(pos, QRect(), false);
}

void QPicturePaintEngine::updateBrush(const QBrush &brush, const QPointF &)
{
#ifdef QT_PICTURE_DEBUG
    qDebug() << " -> updateBrush(): style:" << brush.style();
#endif
    int pos;
    SERIALIZE_CMD(QPicturePrivate::PdcSetBrush);
    d->s << brush;
    writeCmdLength(pos, QRect(), false);
}

void QPicturePaintEngine::updateFont(const QFont &font)
{
#ifdef QT_PICTURE_DEBUG
    qDebug() << " -> updateFont(): pt sz:" << font.pointSize();
#endif
    int pos;
    SERIALIZE_CMD(QPicturePrivate::PdcSetFont);
    QFont fnt = font;
    d->s << fnt;
    writeCmdLength(pos, QRectF(), false);
}

void QPicturePaintEngine::updateBackground(Qt::BGMode bgMode, const QBrush &bgBrush)
{
#ifdef QT_PICTURE_DEBUG
    qDebug() << " -> updateBackground(): mode:" << bgMode << "style:" << bgBrush.style();
#endif
    int pos;
    SERIALIZE_CMD(QPicturePrivate::PdcSetBkColor);
    d->s << bgBrush.color();
    writeCmdLength(pos, QRect(), false);

    SERIALIZE_CMD(QPicturePrivate::PdcSetBkMode);
    d->s << (qint8) bgMode;
    writeCmdLength(pos, QRectF(), false);
}

void QPicturePaintEngine::updateMatrix(const QMatrix &matrix)
{
#ifdef QT_PICTURE_DEBUG
    qDebug() << " -> updateMatrix():" << matrix;
#endif
    int pos;
    SERIALIZE_CMD(QPicturePrivate::PdcSetWMatrix);
    d->s << matrix << (qint8) false;
    writeCmdLength(pos, QRectF(), false);
}

void QPicturePaintEngine::updateClipRegion(const QRegion &region, Qt::ClipOperation op)
{
#ifdef QT_PICTURE_DEBUG
    qDebug() << " -> updateClipRegion(): op:" << op
             << "bounding rect:" << region.boundingRect();
#endif
    Q_UNUSED(op);
    int pos;
    SERIALIZE_CMD(QPicturePrivate::PdcSetClipRegion);
    d->s << region << qint8(0);
    writeCmdLength(pos, QRectF(), false);

    SERIALIZE_CMD(QPicturePrivate::PdcSetClip);
    d->s << (qint8) !region.isEmpty();
    writeCmdLength(pos, QRectF(), false);
}

void QPicturePaintEngine::updateRenderHints(QPainter::RenderHints hints)
{
#ifdef QT_PICTURE_DEBUG
    qDebug() << " -> updateRenderHints(): " << hints;
#endif
    int pos;
    SERIALIZE_CMD(QPicturePrivate::PdcSetRenderHint);
    d->s << (quint32) hints;
    writeCmdLength(pos, QRect(), false);
}

void QPicturePaintEngine::writeCmdLength(int pos, const QRectF &r, bool corr)
{
    int newpos = pic_d->pictb.pos();            // new position
    int length = newpos - pos;
    QRect br = r.toRect();

    if (length < 255) {                         // write 8-bit length
        pic_d->pictb.seek(pos - 1);             // position to right index
        d->s << (quint8)length;
    } else {                                    // write 32-bit length
        d->s << (quint32)0;                    // extend the buffer
        pic_d->pictb.seek(pos - 1);             // position to right index
        d->s << (quint8)255;                   // indicate 32-bit length
        char *p = pic_d->pictb.buffer().data();
        memmove(p+pos+4, p+pos, length);        // make room for 4 byte
        d->s << (quint32)length;
        newpos += 4;
    }
    pic_d->pictb.seek(newpos);                  // set to new position

    if (br.isValid()) {
        if (corr) {                             // widen bounding rect
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
            pic_d->brect |= br;                 // merge with existing rect
    }
}

void QPicturePaintEngine::drawPath(const QPainterPath &path)
{
#ifdef QT_PICTURE_DEBUG
    qDebug() << " -> drawPath():" << path.boundingRect();
#endif
    int pos;
    SERIALIZE_CMD(QPicturePrivate::PdcDrawPath);
    d->s << path;
    writeCmdLength(pos, path.boundingRect(), true);
}

// ### Stream out sr
void QPicturePaintEngine::drawPixmap(const QRectF &r, const QPixmap &pm, const QRectF & /* sr */,
                                     Qt::PixmapDrawingMode /* mode */)
{
#ifdef QT_PICTURE_DEBUG
    qDebug() << " -> drawPixmap():" << r;
#endif
    int pos;
    SERIALIZE_CMD(QPicturePrivate::PdcDrawPixmap);
    d->s << r << pm;
    writeCmdLength(pos, r, false);
}

void QPicturePaintEngine::drawTiledPixmap(const QRectF &r, const QPixmap &pixmap, const QPointF &s,
					  Qt::PixmapDrawingMode)
{
#ifdef QT_PICTURE_DEBUG
    qDebug() << " -> drawTiledPixmap():" << r << s;
#endif
    int pos;
    SERIALIZE_CMD(QPicturePrivate::PdcDrawTiledPixmap);
    d->s << r << pixmap << s;
    writeCmdLength(pos, r, false);
}

void QPicturePaintEngine::drawTextItem(const QPointF &p , const QTextItem &ti)
{
#ifdef QT_PICTURE_DEBUG
    qDebug() << " -> drawTextItem():" << p << QString(ti.chars, ti.num_chars);
#endif
    int pos;
    SERIALIZE_CMD(QPicturePrivate::PdcDrawText2);
    d->s << p << ti.text();
    writeCmdLength(pos, QRectF(p, QSizeF(1,1)), true);
}
