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

#include <private/qfontengine_p.h>

#include "qbitmap.h"
#include "qpainter.h"
#include "qpainterpath.h"
#include "qvarlengtharray.h"

#include <math.h>


#ifndef Q_WS_WIN
QFontEngine::~QFontEngine()
{
}

QFixed QFontEngine::lineThickness() const
{
    // ad hoc algorithm
    int score = fontDef.weight * fontDef.pixelSize;
    int lw = score / 700;

    // looks better with thicker line for small pointsizes
    if (lw < 2 && score >= 1050) lw = 2;
    if (lw == 0) lw = 1;

    return lw;
}

QFixed QFontEngine::underlinePosition() const
{
    return ((lineThickness() * 2) + 3) / 6;
}
#endif

QFixed QFontEngine::xHeight() const
{
    QGlyphLayout glyphs[8];
    int nglyphs = 7;
    QChar x((ushort)'x');
    stringToCMap(&x, 1, glyphs, &nglyphs, 0);

    glyph_metrics_t bb = const_cast<QFontEngine *>(this)->boundingBox(glyphs[0].glyph);
    return bb.height;
}

void QFontEngine::addGlyphsToPath(glyph_t *, QFixedPoint *, int ,
                                  QPainterPath *, QTextItem::RenderFlags)
{
}

void QFontEngine::getGlyphPositions(const QGlyphLayout *glyphs, int nglyphs, const QMatrix &matrix, QTextItem::RenderFlags flags,
                                    QVarLengthArray<glyph_t> &glyphs_out, QVarLengthArray<QFixedPoint> &positions)
{
    QFixed xpos = QFixed::fromReal(matrix.dx());
    QFixed ypos = QFixed::fromReal(matrix.dy());

    bool transform = matrix.m11() != 1.
                     || matrix.m12() != 0.
                     || matrix.m21() != 0.
                     || matrix.m22() != 1.;

    int current = 0;
    if (flags & QTextItem::RightToLeft) {
        int i = nglyphs;
        int totalKashidas = 0;
        while(i--) {
            xpos += glyphs[i].advance.x + QFixed::fromFixed(glyphs[i].space_18d6);
            ypos += glyphs[i].advance.y;
            totalKashidas += glyphs[i].nKashidas;
        }
        positions.resize(nglyphs+totalKashidas);
        glyphs_out.resize(nglyphs+totalKashidas);

        i = 0;
        while(i < nglyphs) {
            if (glyphs[i].attributes.dontPrint) {
                ++i;
                continue;
            }
            xpos -= glyphs[i].advance.x;
            ypos -= glyphs[i].advance.y;

            QFixed gpos_x = xpos + glyphs[i].offset.x;
            QFixed gpos_y = ypos + glyphs[i].offset.y;
            if (transform) {
                QPointF gpos(gpos_x.toReal(), gpos_y.toReal());
                gpos = gpos * matrix;
                gpos_x = QFixed::fromReal(gpos.x());
                gpos_y = QFixed::fromReal(gpos.y());
            }
            positions[current].x = gpos_x;
            positions[current].y = gpos_y;
            glyphs_out[current] = glyphs[i].glyph;
            ++current;
            if (glyphs[i].nKashidas) {
                QChar ch(0x640); // Kashida character
                QGlyphLayout g[8];
                int nglyphs = 7;
                stringToCMap(&ch, 1, g, &nglyphs, 0);
                for (uint k = 0; k < glyphs[i].nKashidas; ++k) {
                    xpos -= g[0].advance.x;
                    ypos -= g[0].advance.y;

                    QFixed gpos_x = xpos + glyphs[i].offset.x;
                    QFixed gpos_y = ypos + glyphs[i].offset.y;
                    if (transform) {
                        QPointF gpos(gpos_x.toReal(), gpos_y.toReal());
                        gpos = gpos * matrix;
                        gpos_x = QFixed::fromReal(gpos.x());
                        gpos_y = QFixed::fromReal(gpos.y());
                    }
                    positions[current].x = gpos_x;
                    positions[current].y = gpos_y;
                    glyphs_out[current] = g[0].glyph;
                    ++current;
                }
            } else {
                xpos -= QFixed::fromFixed(glyphs[i].space_18d6);
            }
            ++i;
        }
    } else {
        positions.resize(nglyphs);
        glyphs_out.resize(nglyphs);
        int i = 0;
        while (i < nglyphs) {
            if (glyphs[i].attributes.dontPrint) {
                ++i;
                continue;
            }
            QFixed gpos_x = xpos + glyphs[i].offset.x;
            QFixed gpos_y = ypos + glyphs[i].offset.y;
            if (transform) {
                QPointF gpos(gpos_x.toReal(), gpos_y.toReal());
                gpos = gpos * matrix;
                gpos_x = QFixed::fromReal(gpos.x());
                gpos_y = QFixed::fromReal(gpos.y());
            }
            positions[current].x = gpos_x;
            positions[current].y = gpos_y;
            glyphs_out[current] = glyphs[i].glyph;
            xpos += glyphs[i].advance.x + QFixed::fromFixed(glyphs[i].space_18d6);
            ypos += glyphs[i].advance.y;
            ++i;
            ++current;
        }
    }
    positions.resize(current);
    glyphs_out.resize(current);
    Q_ASSERT(positions.size() == glyphs_out.size());
}


void QFontEngine::addOutlineToPath(qreal x, qreal y, const QGlyphLayout *glyphs, int numGlyphs, QPainterPath *path,
                                   QTextItem::RenderFlags flags)
{
    if (!numGlyphs)
        return;

    QVarLengthArray<QFixedPoint> positions;
    QVarLengthArray<glyph_t> positioned_glyphs;
    QMatrix matrix;
    matrix.translate(x, y);
    getGlyphPositions(glyphs, numGlyphs, matrix, flags, positioned_glyphs, positions);
    addGlyphsToPath(positioned_glyphs.data(), positions.data(), positioned_glyphs.size(), path, flags);
}

void QFontEngine::addBitmapFontToPath(qreal x, qreal y, const QGlyphLayout *glyphs, int numGlyphs,
                                      QPainterPath *path, QTextItem::RenderFlags flags)
{
    glyph_metrics_t metrics = boundingBox(glyphs, numGlyphs);
    QBitmap bm(metrics.width.toInt(), metrics.height.toInt());
    QPainter p(&bm);
    p.fillRect(0, 0, metrics.width.toInt(), metrics.height.toInt(), Qt::color0);
    p.setPen(Qt::color1);

    QTextItemInt item;
    item.flags = flags;
    item.ascent = -metrics.y;
    item.descent = metrics.height - item.ascent;
    item.width = metrics.width;
    item.chars = 0;
    item.num_chars = 0;
    item.logClusters = 0;
    item.glyphs = const_cast<QGlyphLayout *>(glyphs);
    item.num_glyphs = numGlyphs;
    item.fontEngine = this;
    item.f = 0;

    p.drawTextItem(QPointF(0, item.ascent.toReal()), item);
    p.end();

    QRegion region(bm);
    region.translate(qRound(x), qRound(y - item.ascent.toReal()));
    path->addRegion(region);
}

QImage QFontEngine::alphaMapForGlyph(glyph_t glyph)
{
    glyph_metrics_t gm = boundingBox(glyph);
    int glyph_width = qRound(gm.width)+2;
    int glyph_height = qRound(ascent() + descent())+2;

    QFixedPoint pt;
    pt.y = ascent();
    QPainterPath path;
    QPixmap pm(glyph_width, glyph_height);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);
    addGlyphsToPath(&glyph, &pt, 1, &path, 0);
    p.setPen(Qt::NoPen);
    p.setBrush(Qt::black);
    p.drawPath(path);
    p.end();

    return pm.toImage().convertToFormat(QImage::Format_ARGB32);
}

QFontEngine::Properties QFontEngine::properties() const
{
    Properties p;
    QByteArray psname = fontDef.family.toUtf8();
    psname.replace(" ", "");
    p.postscriptName = psname;
    p.ascent = ascent();
    p.descent = descent();
    p.leading = leading();
    p.emSquare = p.ascent;
    p.boundingBox = QRectF(0, p.ascent.toReal(), maxCharWidth(), (p.ascent + p.descent).toReal());
    p.italicAngle = 0;
    p.capHeight = p.ascent;
    p.lineWidth = lineThickness();
    return p;
}

void QFontEngine::getUnscaledGlyph(glyph_t glyph, QPainterPath *path, glyph_metrics_t *metrics)
{
    *metrics = boundingBox(glyph);
    QFixedPoint p;
    p.x = 0;
    p.y = 0;
    addGlyphsToPath(&glyph, &p, 1, path, QFlag(0));
}


// ------------------------------------------------------------------
// The box font engine
// ------------------------------------------------------------------

#ifdef Q_WS_WIN
#include "qt_windows.h"
#endif

QFontEngineBox::QFontEngineBox(int size)
    : _size(size)
{
    cache_cost = sizeof(QFontEngineBox);

#ifdef Q_WS_WIN
#ifndef Q_OS_TEMP
    hfont = (HFONT)GetStockObject(ANSI_VAR_FONT);
#endif
    stockFont = true;
    ttf = false;

    cmap = 0;
    script_cache = 0;
#endif
}

QFontEngineBox::~QFontEngineBox()
{
}

bool QFontEngineBox::stringToCMap(const QChar *, int len, QGlyphLayout *glyphs, int *nglyphs, QTextEngine::ShaperFlags) const
{
    if (*nglyphs < len) {
        *nglyphs = len;
        return false;
    }

    for (int i = 0; i < len; i++) {
        glyphs[i].glyph = 0;
        glyphs[i].advance.x = _size;
        glyphs[i].advance.y = 0;
    }

    *nglyphs = len;
    return true;
}

void QFontEngineBox::addOutlineToPath(qreal x, qreal y, const QGlyphLayout *glyphs, int numGlyphs, QPainterPath *path, QTextItem::RenderFlags flags)
{
    if (!numGlyphs)
        return;

    QVarLengthArray<QFixedPoint> positions;
    QVarLengthArray<glyph_t> positioned_glyphs;
    QMatrix matrix;
    matrix.translate(x, y);
    getGlyphPositions(glyphs, numGlyphs, matrix, flags, positioned_glyphs, positions);
    addGlyphsToPath(positioned_glyphs.data(), positions.data(), positioned_glyphs.size(), path, flags);

    int size = qRound(ascent());
    QSize s(size - 3, size - 3);
    for (int k = 0; k < positions.size(); k++)
        path->addRect(QRectF(positions[k].toPointF(), s));
}

glyph_metrics_t QFontEngineBox::boundingBox(const QGlyphLayout *, int numGlyphs)
{
    glyph_metrics_t overall;
    overall.width = _size*numGlyphs;
    overall.height = _size;
    overall.xoff = overall.width;
    return overall;
}

#if !defined(Q_WS_X11) && !defined(Q_WS_WIN) && !defined(Q_NEW_MAC_FONTENGINE)
void QFontEngineBox::draw(QPaintEngine *p, qreal x, qreal y, const QTextItemInt &si)
{
    Q_UNUSED(p);
    Q_UNUSED(x);
    Q_UNUSED(y);
    Q_UNUSED(si);
    //qDebug("QFontEngineBox::draw(%d, %d, numglyphs=%d", x, y, numGlyphs);
}
#endif

glyph_metrics_t QFontEngineBox::boundingBox(glyph_t)
{
    return glyph_metrics_t(0, _size, _size, _size, _size, 0);
}



QFixed QFontEngineBox::ascent() const
{
    return _size;
}

QFixed QFontEngineBox::descent() const
{
    return 0;
}

QFixed QFontEngineBox::leading() const
{
    QFixed l = _size * QFixed::fromReal(0.15);
    return l.ceil();
}

qreal QFontEngineBox::maxCharWidth() const
{
    return _size;
}

#ifdef Q_WS_X11
int QFontEngineBox::cmap() const
{
    return -1;
}
#endif

const char *QFontEngineBox::name() const
{
    return "null";
}

bool QFontEngineBox::canRender(const QChar *, int)
{
    return true;
}

QFontEngine::Type QFontEngineBox::type() const
{
    return Box;
}


// ------------------------------------------------------------------
// Multi engine
// ------------------------------------------------------------------

static inline uchar highByte(glyph_t glyph)
{ return glyph >> 24; }

// strip high byte from glyph
static inline glyph_t stripped(glyph_t glyph)
{ return glyph & 0x00ffffff; }

QFontEngineMulti::QFontEngineMulti(int engineCount)
{
    engines.fill(0, engineCount);
    cache_cost = 0;
}

QFontEngineMulti::~QFontEngineMulti()
{
    for (int i = 0; i < engines.size(); ++i) {
        QFontEngine *fontEngine = engines.at(i);
        if (fontEngine) {
            fontEngine->ref.deref();
            if (fontEngine->cache_count == 0 && fontEngine->ref == 0)
                delete fontEngine;
        }
    }
}

bool QFontEngineMulti::stringToCMap(const QChar *str, int len,
                                    QGlyphLayout *glyphs, int *nglyphs,
                                    QTextEngine::ShaperFlags flags) const
{
    int ng = *nglyphs;
    if (!engine(0)->stringToCMap(str, len, glyphs, &ng, flags))
        return false;

    int glyph_pos = 0;
    for (int i = 0; i < len; ++i) {
        bool surrogate = (str[i].unicode() >= 0xd800 && str[i].unicode() < 0xdc00 && i < len-1
                          && str[i+1].unicode() >= 0xdc00 && str[i+1].unicode() < 0xe000);
        if (glyphs[glyph_pos].glyph == 0) {
            for (int x = 1; x < engines.size(); ++x) {
                QFontEngine *engine = engines.at(x);
                if (!engine) {
                    const_cast<QFontEngineMulti *>(this)->loadEngine(x);
                    engine = engines.at(x);
                }
                Q_ASSERT(engine != 0);
                if (engine->type() == Box)
                    continue;
                glyphs[i].advance = glyphs[i].offset = QFixedPoint();
                int num = 2;
                engine->stringToCMap(str + i, surrogate ? 2 : 1, glyphs + glyph_pos, &num, flags);
                Q_ASSERT(num == 1); // surrogates only give 1 glyph
                if (glyphs[glyph_pos].glyph) {
                    // set the high byte to indicate which engine the glyph came from
                    glyphs[glyph_pos].glyph |= (x << 24);
                    break;
                }
            }
        }
        if (surrogate)
            ++i;
        ++glyph_pos;
    }

    *nglyphs = ng;
    return true;
}

glyph_metrics_t QFontEngineMulti::boundingBox(const QGlyphLayout *glyphs_const, int numGlyphs)
{
    if (numGlyphs <= 0)
        return glyph_metrics_t();

    glyph_metrics_t overall;

    QGlyphLayout *glyphs = const_cast<QGlyphLayout *>(glyphs_const);
    int which = highByte(glyphs[0].glyph);
    int start = 0;
    int end, i;
    for (end = 0; end < numGlyphs; ++end) {
        const int e = highByte(glyphs[end].glyph);
        if (e == which)
            continue;

        // set the high byte to zero
        for (i = start; i < end; ++i)
            glyphs[i].glyph = stripped(glyphs[i].glyph);

        // merge the bounding box for this run
        const glyph_metrics_t gm = engine(which)->boundingBox(glyphs + start, end - start);

        overall.x = qMin(overall.x, gm.x);
        overall.y = qMin(overall.y, gm.y);
        overall.width = overall.xoff + gm.width;
        overall.height = qMax(overall.height + overall.y, gm.height + gm.y) -
                         qMin(overall.y, gm.y);
        overall.xoff += gm.xoff;
        overall.yoff += gm.yoff;

        // reset the high byte for all glyphs
        const int hi = which << 24;
        for (i = start; i < end; ++i)
            glyphs[i].glyph = hi | glyphs[i].glyph;

        // change engine
        start = end;
        which = e;
    }

    // set the high byte to zero
    for (i = start; i < end; ++i)
        glyphs[i].glyph = stripped(glyphs[i].glyph);

    // merge the bounding box for this run
    const glyph_metrics_t gm = engine(which)->boundingBox(glyphs + start, end - start);

    overall.x = qMin(overall.x, gm.x);
    overall.y = qMin(overall.y, gm.y);
    overall.width = overall.xoff + gm.width;
    overall.height = qMax(overall.height + overall.y, gm.height + gm.y) -
                     qMin(overall.y, gm.y);
    overall.xoff += gm.xoff;
    overall.yoff += gm.yoff;

    // reset the high byte for all glyphs
    const int hi = which << 24;
    for (i = start; i < end; ++i)
        glyphs[i].glyph = hi | glyphs[i].glyph;

    return overall;
}

void QFontEngineMulti::addOutlineToPath(qreal x, qreal y, const QGlyphLayout *glyphs_const, int numGlyphs,
                                        QPainterPath *path, QTextItem::RenderFlags flags)
{
    if (numGlyphs <= 0)
        return;

    QGlyphLayout *glyphs = const_cast<QGlyphLayout *>(glyphs_const);
    int which = highByte(glyphs[0].glyph);
    int start = 0;
    int end, i;
    if (flags & QTextItem::RightToLeft) {
        for (int gl = 0; gl < numGlyphs; gl++) {
            x += glyphs[gl].advance.x.toReal();
            y += glyphs[gl].advance.x.toReal();
        }
    }
    for (end = 0; end < numGlyphs; ++end) {
        const int e = highByte(glyphs[end].glyph);
        if (e == which)
            continue;

        if (flags & QTextItem::RightToLeft) {
            for (i = start; i < end; ++i) {
                x -= glyphs[i].advance.x.toReal();
                y -= glyphs[i].advance.y.toReal();
            }
        }

        // set the high byte to zero
        for (i = start; i < end; ++i)
            glyphs[i].glyph = stripped(glyphs[i].glyph);
        engine(which)->addOutlineToPath(x, y, glyphs + start, end - start, path, flags);
        // reset the high byte for all glyphs and update x and y
        const int hi = which << 24;
        for (i = start; i < end; ++i)
            glyphs[i].glyph = hi | glyphs[i].glyph;

        if (!(flags & QTextItem::RightToLeft)) {
            for (i = start; i < end; ++i) {
                x += glyphs[i].advance.x.toReal();
                y += glyphs[i].advance.y.toReal();
            }
        }

        // change engine
        start = end;
        which = e;
    }

    if (flags & QTextItem::RightToLeft) {
        for (i = start; i < end; ++i) {
            x -= glyphs[i].advance.x.toReal();
            y -= glyphs[i].advance.y.toReal();
        }
    }

    // set the high byte to zero
    for (i = start; i < end; ++i)
        glyphs[i].glyph = stripped(glyphs[i].glyph);

    engine(which)->addOutlineToPath(x, y, glyphs + start, end - start, path, flags);

    // reset the high byte for all glyphs
    const int hi = which << 24;
    for (i = start; i < end; ++i)
        glyphs[i].glyph = hi | glyphs[i].glyph;
}

void QFontEngineMulti::recalcAdvances(int numGlyphs, QGlyphLayout *glyphs, QTextEngine::ShaperFlags flags) const
{
    if (numGlyphs <= 0)
        return;

    int which = highByte(glyphs[0].glyph);
    int start = 0;
    int end, i;
    for (end = 0; end < numGlyphs; ++end) {
        const int e = highByte(glyphs[end].glyph);
        if (e == which)
            continue;

        // set the high byte to zero
        for (i = start; i < end; ++i)
            glyphs[i].glyph = stripped(glyphs[i].glyph);

        engine(which)->recalcAdvances(end - start, glyphs + start, flags);

        // reset the high byte for all glyphs and update x and y
        const int hi = which << 24;
        for (i = start; i < end; ++i)
            glyphs[i].glyph = hi | glyphs[i].glyph;

        // change engine
        start = end;
        which = e;
    }

    // set the high byte to zero
    for (i = start; i < end; ++i)
        glyphs[i].glyph = stripped(glyphs[i].glyph);

    engine(which)->recalcAdvances(end - start, glyphs + start, flags);

    // reset the high byte for all glyphs
    const int hi = which << 24;
    for (i = start; i < end; ++i)
        glyphs[i].glyph = hi | glyphs[i].glyph;
}

void QFontEngineMulti::doKerning(int numGlyphs, QGlyphLayout *glyphs, QTextEngine::ShaperFlags flags) const
{
    if (numGlyphs <= 0)
        return;

    int which = highByte(glyphs[0].glyph);
    int start = 0;
    int end, i;
    for (end = 0; end < numGlyphs; ++end) {
        const int e = highByte(glyphs[end].glyph);
        if (e == which)
            continue;

        // set the high byte to zero
        for (i = start; i < end; ++i)
            glyphs[i].glyph = stripped(glyphs[i].glyph);

        engine(which)->doKerning(end - start, glyphs + start, flags);

        // reset the high byte for all glyphs and update x and y
        const int hi = which << 24;
        for (i = start; i < end; ++i)
            glyphs[i].glyph = hi | glyphs[i].glyph;

        // change engine
        start = end;
        which = e;
    }

    // set the high byte to zero
    for (i = start; i < end; ++i)
        glyphs[i].glyph = stripped(glyphs[i].glyph);

    engine(which)->doKerning(end - start, glyphs + start, flags);

    // reset the high byte for all glyphs
    const int hi = which << 24;
    for (i = start; i < end; ++i)
        glyphs[i].glyph = hi | glyphs[i].glyph;
}

glyph_metrics_t QFontEngineMulti::boundingBox(glyph_t glyph)
{
    const int which = highByte(glyph);
    Q_ASSERT(which < engines.size());
    return engine(which)->boundingBox(stripped(glyph));
}

QFixed QFontEngineMulti::ascent() const
{ return engine(0)->ascent(); }

QFixed QFontEngineMulti::descent() const
{ return engine(0)->descent(); }

QFixed QFontEngineMulti::leading() const
{
    return engine(0)->leading();
}

QFixed QFontEngineMulti::xHeight() const
{
    return engine(0)->xHeight();
}

QFixed QFontEngineMulti::lineThickness() const
{
    return engine(0)->lineThickness();
}

QFixed QFontEngineMulti::underlinePosition() const
{
    return engine(0)->underlinePosition();
}

qreal QFontEngineMulti::maxCharWidth() const
{
    return engine(0)->maxCharWidth();
}

qreal QFontEngineMulti::minLeftBearing() const
{
    return engine(0)->minLeftBearing();
}

qreal QFontEngineMulti::minRightBearing() const
{
    return engine(0)->minRightBearing();
}

bool QFontEngineMulti::canRender(const QChar *string, int len)
{
    if (engine(0)->canRender(string, len))
        return true;

    QVarLengthArray<QGlyphLayout, 256> glyphs(len);
    int nglyphs = len;
    if (stringToCMap(string, len, glyphs.data(), &nglyphs, 0) == false) {
        glyphs.resize(nglyphs);
        stringToCMap(string, len, glyphs.data(), &nglyphs, 0);
    }

    bool allExist = true;
    for (int i = 0; i < nglyphs; i++) {
        if (!glyphs[i].glyph) {
            allExist = false;
            break;
        }
    }

    return allExist;
}

QFontEngine *QFontEngineMulti::engine(int at) const
{
    Q_ASSERT(at < engines.size());
    return engines.at(at);
}
