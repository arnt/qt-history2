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


void QFontEngine::addOutlineToPath(qreal x, qreal y, const QGlyphLayout *glyphs, int numGlyphs, QPainterPath *path,
                                   QTextItem::RenderFlags flags)
{
    addBitmapFontToPath(x, y, glyphs, numGlyphs, path, flags);
}

void QFontEngine::addBitmapFontToPath(qreal x, qreal y, const QGlyphLayout *glyphs, int numGlyphs,
                                      QPainterPath *path, QTextItem::RenderFlags flags)
{
    glyph_metrics_t metrics = boundingBox(glyphs, numGlyphs);
    QBitmap bm(qRound(metrics.width), qRound(metrics.height));
    QPainter p(&bm);
    p.fillRect(0, 0, qRound(metrics.width), qRound(metrics.height), Qt::color0);
    p.setPen(Qt::color1);

    QTextItemInt item;
    item.flags = flags;
    item.descent = descent();
    item.ascent = ascent();
    item.width = metrics.width;
    item.chars = 0;
    item.num_chars = 0;
    item.glyphs = const_cast<QGlyphLayout *>(glyphs);
    item.num_glyphs = numGlyphs;
    item.fontEngine = this;
    item.f = 0;

    p.drawTextItem(QPointF(0, ascent()), item);
    p.end();

    QRegion region(bm);
    region.translate(qRound(x), qRound(y - ascent()));
    path->addRegion(region);
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

QFontEngine::FECaps QFontEngineBox::capabilites() const
{
    return FullTransformations;
}

bool QFontEngineBox::stringToCMap(const QChar *, int len, QGlyphLayout *glyphs, int *nglyphs, QTextEngine::ShaperFlags) const
{
    if (*nglyphs < len) {
        *nglyphs = len;
        return false;
    }

    memset(glyphs, 0, len * sizeof(QGlyphLayout));

    for (int i = 0; i < len; i++) {
        glyphs[i].advance.rx() = _size;
        glyphs[i].advance.ry() = 0;
    }

    *nglyphs = len;
    return true;
}

glyph_metrics_t QFontEngineBox::boundingBox(const QGlyphLayout *, int numGlyphs)
{
    glyph_metrics_t overall;
    overall.width = _size*numGlyphs;
    overall.height = _size;
    overall.xoff = overall.width;
    return overall;
}

#if !defined(Q_WS_X11) && !defined(Q_WS_WIN)
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



qreal QFontEngineBox::ascent() const
{
    return _size;
}

qreal QFontEngineBox::descent() const
{
    return 0;
}

qreal QFontEngineBox::leading() const
{
    qreal l = _size * 0.15;
    return ceil(l);
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

uchar highByte(glyph_t glyph)
{ return glyph >> 24; }

// strip high byte from glyph
glyph_t stripped(glyph_t glyph)
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

QFontEngine::FECaps QFontEngineMulti::capabilites() const
{ return engine(0)->capabilites(); }

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
                glyphs[i].advance = glyphs[i].offset = QPointF();
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
            x += glyphs[gl].advance.x();
            y += glyphs[gl].advance.x();
        }
    }
    for (end = 0; end < numGlyphs; ++end) {
        const int e = highByte(glyphs[end].glyph);
        if (e == which)
            continue;

        if (flags & QTextItem::RightToLeft) {
            for (i = start; i < end; ++i) {
                x -= glyphs[i].advance.x();
                y -= glyphs[i].advance.y();
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
                x += glyphs[i].advance.x();
                y += glyphs[i].advance.y();
            }
        }

        // change engine
        start = end;
        which = e;
    }

    if (flags & QTextItem::RightToLeft) {
        for (i = start; i < end; ++i) {
            x -= glyphs[i].advance.x();
            y -= glyphs[i].advance.y();
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

qreal QFontEngineMulti::ascent() const
{ return engine(0)->ascent(); }

qreal QFontEngineMulti::descent() const
{ return engine(0)->descent(); }

qreal QFontEngineMulti::leading() const
{
    return engine(0)->leading();
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
