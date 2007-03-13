/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qabstractfontengine_qws.h"
#include "qabstractfontengine_p.h"

#include <private/qtextengine_p.h>
#include <private/qpaintengine_raster_p.h>

class QFontEngineInfoPrivate
{
public:
    inline QFontEngineInfoPrivate()
        : pixelSize(0), weight(QFont::Normal), style(QFont::StyleNormal)
    {}

    QString family;
    qreal pixelSize;
    int weight;
    QFont::Style style;
};

QFontEngineInfo::QFontEngineInfo()
{
    d = new QFontEngineInfoPrivate;
}

QFontEngineInfo::QFontEngineInfo(const QString &family)
{
    d = new QFontEngineInfoPrivate;
    d->family = family;
}

QFontEngineInfo::QFontEngineInfo(const QFontEngineInfo &other)
    : d(new QFontEngineInfoPrivate(*other.d))
{
}

QFontEngineInfo &QFontEngineInfo::operator=(const QFontEngineInfo &other)
{
    *d = *other.d;
    return *this;
}

QFontEngineInfo::~QFontEngineInfo()
{
    delete d;
}

void QFontEngineInfo::setFamily(const QString &family)
{
    d->family = family;
}

QString QFontEngineInfo::family() const
{
    return d->family;
}

void QFontEngineInfo::setPixelSize(qreal size)
{
    d->pixelSize = size;
}

qreal QFontEngineInfo::pixelSize() const
{
    return d->pixelSize;
}

void QFontEngineInfo::setWeight(int weight)
{
    d->weight = weight;
}

int QFontEngineInfo::weight() const
{
    return d->weight;
}

void QFontEngineInfo::setStyle(QFont::Style style)
{
    d->style = style;
}

QFont::Style QFontEngineInfo::style() const
{
    return d->style;
}

class QFontEnginePluginPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QFontEnginePlugin)

    QString foundry;
};

QFontEnginePlugin::QFontEnginePlugin(const QString &foundry, QObject *parent)
    : QObject(*new QFontEnginePluginPrivate, parent)
{
    Q_D(QFontEnginePlugin);
    d->foundry = foundry;
}

QFontEnginePlugin::~QFontEnginePlugin()
{
}

QStringList QFontEnginePlugin::keys() const
{
    Q_D(const QFontEnginePlugin);
    return QStringList(d->foundry);
}

class QAbstractFontEnginePrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QAbstractFontEngine)
public:
    QAbstractFontEngine::Capabilities capabilities;
};

QAbstractFontEngine::QAbstractFontEngine(Capabilities capabilities, QObject *parent)
    : QObject(*new QAbstractFontEnginePrivate, parent)
{
    Q_D(QAbstractFontEngine);
    d->capabilities = capabilities;
}

QAbstractFontEngine::~QAbstractFontEngine()
{
}

QAbstractFontEngine::Capabilities QAbstractFontEngine::capabilities() const
{
    Q_D(const QAbstractFontEngine);
    return d->capabilities;
}

QImage QAbstractFontEngine::renderGlyph(uint glyph)
{
    Q_UNUSED(glyph)
    qWarning("QAbstractFontEngine: renderGlyph is not implemented in font plugin!");
    return QImage();
}

void QAbstractFontEngine::addGlyphOutlinesToPath(uint *glyphs, int numGlyphs, FixedPoint *positions, QPainterPath *path, QTextItem::RenderFlags flags)
{
    Q_UNUSED(glyphs)
    Q_UNUSED(numGlyphs)
    Q_UNUSED(positions)
    Q_UNUSED(path)
    Q_UNUSED(flags)
    qWarning("QAbstractFontEngine: addGlyphOutlinesToPath is not implemented in font plugin!");
}

bool QAbstractFontEngine::supportsExtension(Extension extension) const
{
    Q_UNUSED(extension)
    return false;
}

QVariant QAbstractFontEngine::extension(Extension extension, const QVariant &argument)
{
    Q_UNUSED(argument)
    Q_UNUSED(extension)
    return QVariant();
}

QProxyFontEngine::QProxyFontEngine(QAbstractFontEngine *customEngine, const QFontDef &def)
    : engine(customEngine)
{
    fontDef = def;
}

QProxyFontEngine::~QProxyFontEngine()
{
    delete engine;
}

bool QProxyFontEngine::stringToCMap(const QChar *str, int len, QGlyphLayout *glyphs, int *nglyphs, QTextEngine::ShaperFlags flags) const
{
    if (*nglyphs < len) {
        *nglyphs = len;
        return false;
    }

    QVarLengthArray<uint> glyphIndicies(*nglyphs);
    if (!engine->convertStringToGlyphIndices(str, len, glyphIndicies.data(), nglyphs, QAbstractFontEngine::TextShapingFlags(int(flags))))
        return false;

    QVarLengthArray<QAbstractFontEngine::Fixed> advances(*nglyphs);
    engine->getGlyphAdvances(glyphIndicies.data(), *nglyphs, advances.data(), QAbstractFontEngine::TextShapingFlags(int(flags)));

    for (int i = 0; i < *nglyphs; ++i) {
        glyphs[i].glyph = glyphIndicies[i];
        glyphs[i].advance.x = QFixed::fromFixed(advances[i]);
        glyphs[i].advance.y = 0;
    }
    return true;
}

QImage QProxyFontEngine::alphaMapForGlyph(glyph_t glyph)
{
    if (engine->capabilities() & QAbstractFontEngine::RenderGlyphs)
        return engine->renderGlyph(glyph);
    return QFontEngine::alphaMapForGlyph(glyph);
}

void QProxyFontEngine::addGlyphsToPath(glyph_t *glyphs, QFixedPoint *positions, int nglyphs, QPainterPath *path, QTextItem::RenderFlags flags)
{
    if (engine->capabilities() & QAbstractFontEngine::OutlineGlyphs)
        engine->addGlyphOutlinesToPath(glyphs, nglyphs, reinterpret_cast<QAbstractFontEngine::FixedPoint *>(positions), path, flags);
    else
        QFontEngine::addGlyphsToPath(glyphs, positions, nglyphs, path, flags);
}

glyph_metrics_t QProxyFontEngine::boundingBox(const QGlyphLayout *glyphs, int numGlyphs)
{
    if (numGlyphs == 0)
        return glyph_metrics_t();

    QFixed w = 0;
    const QGlyphLayout *end = glyphs + numGlyphs;
    while(end > glyphs) {
        --end;
        w += (end->advance.x + end->space_18d6) * !end->attributes.dontPrint;
    }

    return glyph_metrics_t(0, -ascent(), w, ascent() + descent(), w, 0);
}

glyph_metrics_t QProxyFontEngine::boundingBox(glyph_t glyph)
{
    glyph_metrics_t m;

    QAbstractFontEngine::GlyphMetrics metrics = engine->glyphMetrics(glyph);
    m.x = QFixed::fromFixed(metrics.x);
    m.y = QFixed::fromFixed(metrics.y);
    m.width = QFixed::fromFixed(metrics.width);
    m.height = QFixed::fromFixed(metrics.height);
    m.xoff = QFixed::fromFixed(metrics.advance);

    return m;
}

QFixed QProxyFontEngine::ascent() const
{
    return QFixed::fromFixed(engine->fontProperty(QAbstractFontEngine::Ascent).toInt());
}

QFixed QProxyFontEngine::descent() const
{
    return QFixed::fromFixed(engine->fontProperty(QAbstractFontEngine::Descent).toInt());
}

QFixed QProxyFontEngine::leading() const
{
    return QFixed::fromFixed(engine->fontProperty(QAbstractFontEngine::Leading).toInt());
}

QFixed QProxyFontEngine::xHeight() const
{
    return QFixed::fromFixed(engine->fontProperty(QAbstractFontEngine::XHeight).toInt());
}

QFixed QProxyFontEngine::averageCharWidth() const
{
    return QFixed::fromFixed(engine->fontProperty(QAbstractFontEngine::AverageCharWidth).toInt());
}

QFixed QProxyFontEngine::lineThickness() const
{
    return QFixed::fromFixed(engine->fontProperty(QAbstractFontEngine::LineThickness).toInt());
}

QFixed QProxyFontEngine::underlinePosition() const
{
    return QFixed::fromFixed(engine->fontProperty(QAbstractFontEngine::UnderlinePosition).toInt());
}

qreal QProxyFontEngine::maxCharWidth() const
{
    return QFixed::fromFixed(engine->fontProperty(QAbstractFontEngine::MaxCharWidth).toInt()).toReal();
}

qreal QProxyFontEngine::minLeftBearing() const
{
    return QFixed::fromFixed(engine->fontProperty(QAbstractFontEngine::MinLeftBearing).toInt()).toReal();
}

qreal QProxyFontEngine::minRightBearing() const
{
    return QFixed::fromFixed(engine->fontProperty(QAbstractFontEngine::MinRightBearing).toInt()).toReal();
}

int QProxyFontEngine::glyphCount() const
{
    return engine->fontProperty(QAbstractFontEngine::GlyphCount).toInt();
}

bool QProxyFontEngine::canRender(const QChar *string, int len)
{
    QVarLengthArray<uint> glyphs(len);
    int numGlyphs = len;

    if (!engine->convertStringToGlyphIndices(string, len, glyphs.data(), &numGlyphs, /*flags*/0))
        return false;

    for (int i = 0; i < numGlyphs; ++i)
        if (!glyphs[i])
            return false;

    return true;
}

void QProxyFontEngine::draw(QPaintEngine *p, qreal _x, qreal _y, const QTextItemInt &si)
{
    QPaintEngineState *pState = p->state;
    QRasterPaintEngine *paintEngine = static_cast<QRasterPaintEngine*>(p);

    QTransform matrix = pState->transform();
    matrix.translate(_x, _y);
    QFixed x = QFixed::fromReal(matrix.dx());
    QFixed y = QFixed::fromReal(matrix.dy());

    QVarLengthArray<QFixedPoint> positions;
    QVarLengthArray<glyph_t> glyphs;
    getGlyphPositions(si.glyphs, si.num_glyphs, matrix, si.flags, glyphs, positions);
    if (glyphs.size() == 0)
        return;

    for(int i = 0; i < glyphs.size(); i++) {
        QImage glyph = engine->renderGlyph(glyphs[i]);
        if (glyph.isNull())
            continue;

        if (glyph.format() != QImage::Format_Indexed8
            && glyph.format() != QImage::Format_Mono)
            continue;

        QAbstractFontEngine::GlyphMetrics metrics = engine->glyphMetrics(glyphs[i]);


        paintEngine->alphaPenBlt(glyph.bits(), glyph.bytesPerLine(), glyph.format() == QImage::Format_Mono,
                                 qRound(positions[i].x + QFixed::fromFixed(metrics.x)),
                                 qRound(positions[i].y + QFixed::fromFixed(metrics.y)),
                                 glyph.width(), glyph.height());
    }
}

/*
 * This is only called when we use the proxy fontengine directly (without sharing the rendered
 * glyphs). So we prefer outline rendering over rendering of unshared glyphs. That decision is
 * done in qfontdatabase_qws.cpp by looking at the ShareGlyphsHint and the pixel size of the font.
 */
bool QProxyFontEngine::drawAsOutline() const
{
    if (!(engine->capabilities() & QAbstractFontEngine::OutlineGlyphs))
        return false;

    QVariant outlineHint = engine->fontProperty(QAbstractFontEngine::OutlineGlyphsHint);
    return !outlineHint.isValid() || outlineHint.toBool();
}

