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

#include <math.h>

void QFontEngine::addOutlineToPath(qReal x, qReal y, const QGlyphLayout *glyphs, int numGlyphs, QPainterPath *path)
{
    addBitmapFontToPath(x, y, glyphs, numGlyphs, path);
}

void QFontEngine::addBitmapFontToPath(qReal x, qReal y, const QGlyphLayout *glyphs, int numGlyphs,
                                 QPainterPath *path)
{
    glyph_metrics_t metrics = boundingBox(glyphs, numGlyphs);
    QBitmap bm(qRound(metrics.width), qRound(metrics.height));
    QPainter p(&bm);
    p.fillRect(0, 0, qRound(metrics.width), qRound(metrics.height), Qt::color0);
    p.setPen(Qt::color1);

    QTextItem item;
    item.flags = 0;
    item.descent = descent();
    item.ascent = ascent();
    item.width = metrics.width;
    item.chars = 0;
    item.num_chars = 0;
    item.glyphs = const_cast<QGlyphLayout *>(glyphs);
    item.num_glyphs = numGlyphs;
    item.fontEngine = this;

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
void QFontEngineBox::draw(QPaintEngine *p, int x, int y, const QTextItem &si)
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



qReal QFontEngineBox::ascent() const
{
    return _size;
}

qReal QFontEngineBox::descent() const
{
    return 0;
}

qReal QFontEngineBox::leading() const
{
    qReal l = _size * 0.15;
    return ceil(l);
}

qReal QFontEngineBox::maxCharWidth() const
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
