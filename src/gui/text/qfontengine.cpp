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
