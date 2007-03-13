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

#include "qcustomfontengine_qws.h"
#include "qcustomfontengine_p.h"

#include <private/qtextengine_p.h>

QCustomFontInfo::QCustomFontInfo()
{
}

QCustomFontInfo::QCustomFontInfo(const QString &family)
{
    setFamily(family);
}

QCustomFontInfo::QCustomFontInfo(const QCustomFontInfo &other)
    : QHash<int, QVariant>(other)
{
}

QCustomFontInfo &QCustomFontInfo::operator=(const QCustomFontInfo &other)
{
    QHash<int, QVariant>::operator=(other);
    return *this;
}

void QCustomFontInfo::setPixelSize(qreal size)
{
    insert(PixelSize, QFixed::fromReal(size).value());
}

qreal QCustomFontInfo::pixelSize() const
{
    return QFixed::fromFixed(value(PixelSize).toInt()).toReal();
}

class QCustomFontEnginePluginPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QCustomFontEnginePlugin)

    QString foundry;
};

QCustomFontEnginePlugin::QCustomFontEnginePlugin(const QString &foundry, QObject *parent)
    : QObject(*new QCustomFontEnginePluginPrivate, parent)
{
    Q_D(QCustomFontEnginePlugin);
    d->foundry = foundry;
}

QCustomFontEnginePlugin::~QCustomFontEnginePlugin()
{
}

QStringList QCustomFontEnginePlugin::keys() const
{
    Q_D(const QCustomFontEnginePlugin);
    return QStringList(d->foundry);
}

QCustomFontEngine::QCustomFontEngine(QObject *parent)
    : QObject(parent)
{
}

QCustomFontEngine::~QCustomFontEngine()
{
}

QImage QCustomFontEngine::renderGlyph(uint glyph)
{
    qWarning("QCustomFontEngine::renderGlyph not implemented!");
    return QImage();
}

void QCustomFontEngine::addGlyphsToPath(uint *glyphs, int numGlyphs, Fixed *positions, QPainterPath *path, QTextItem::RenderFlags flags)
{
    qWarning("QCustomFontEngine::addGlyphsToPath not implemented!");
}

QVariant QCustomFontEngine::extension(Extension extension, const QVariant &argument)
{
    Q_UNUSED(argument)
    Q_UNUSED(extension)
    return QVariant();
}

QProxyFontEngine::QProxyFontEngine(QCustomFontEngine *customEngine, const QFontDef &def)
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
    if (!engine->stringToGlyphIndices(str, len, glyphIndicies.data(), nglyphs, /*flags ######### */0))
        return false;

    QVarLengthArray<QCustomFontEngine::Fixed> advances(*nglyphs);
    engine->getGlyphAdvances(glyphIndicies.data(), *nglyphs, advances.data(), /*flags ###### */0);

    for (int i = 0; i < *nglyphs; ++i) {
        glyphs[i].glyph = glyphIndicies[i];
        glyphs[i].advance.x = QFixed::fromFixed(advances[i]);
        glyphs[i].advance.y = 0;
    }
    return true;
}

QImage QProxyFontEngine::alphaMapForGlyph(glyph_t glyph)
{
    return engine->renderGlyph(glyph);
}

void QProxyFontEngine::addGlyphsToPath(glyph_t *glyphs, QFixedPoint *positions, int nglyphs, QPainterPath *path, QTextItem::RenderFlags flags)
{
    QVarLengthArray<int> newPositions(nglyphs * 2);
    for (int i = 0; i < nglyphs; ++i) {
        newPositions[i * 2] = positions[i].x.value();
        newPositions[i * 2 + 1] = positions[i].y.value();
    }

    engine->addGlyphsToPath(glyphs, nglyphs, newPositions.data(), path, flags);
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

    QCustomFontEngine::GlyphMetrics metrics = engine->getGlyphMetrics(glyph);
    m.x = QFixed::fromFixed(metrics.x);
    m.y = QFixed::fromFixed(metrics.y);
    m.width = QFixed::fromFixed(metrics.width);
    m.height = QFixed::fromFixed(metrics.height);
    m.xoff = QFixed::fromFixed(metrics.xOffset);
    m.yoff = QFixed::fromFixed(metrics.yOffset);

    return m;
}

QFixed QProxyFontEngine::ascent() const
{
    return QFixed::fromFixed(engine->fontProperty(QCustomFontEngine::Ascent).toInt());
}

QFixed QProxyFontEngine::descent() const
{
    return QFixed::fromFixed(engine->fontProperty(QCustomFontEngine::Descent).toInt());
}

QFixed QProxyFontEngine::leading() const
{
    return QFixed::fromFixed(engine->fontProperty(QCustomFontEngine::Leading).toInt());
}

QFixed QProxyFontEngine::xHeight() const
{
    return QFixed::fromFixed(engine->fontProperty(QCustomFontEngine::XHeight).toInt());
}

QFixed QProxyFontEngine::averageCharWidth() const
{
    return QFixed::fromFixed(engine->fontProperty(QCustomFontEngine::AverageCharWidth).toInt());
}

QFixed QProxyFontEngine::lineThickness() const
{
    return QFixed::fromFixed(engine->fontProperty(QCustomFontEngine::LineThickness).toInt());
}

QFixed QProxyFontEngine::underlinePosition() const
{
    return QFixed::fromFixed(engine->fontProperty(QCustomFontEngine::UnderlinePosition).toInt());
}

qreal QProxyFontEngine::maxCharWidth() const
{
    return QFixed::fromFixed(engine->fontProperty(QCustomFontEngine::MaxCharWidth).toInt()).toReal();
}

qreal QProxyFontEngine::minLeftBearing() const
{
    return QFixed::fromFixed(engine->fontProperty(QCustomFontEngine::MinLeftBearing).toInt()).toReal();
}

qreal QProxyFontEngine::minRightBearing() const
{
    return QFixed::fromFixed(engine->fontProperty(QCustomFontEngine::MinRightBearing).toInt()).toReal();
}

int QProxyFontEngine::glyphCount() const
{
    return engine->fontProperty(QCustomFontEngine::TotalGlyphCount).toInt();
}

bool QProxyFontEngine::canRender(const QChar *string, int len)
{
    QVarLengthArray<uint> glyphs(len);
    int numGlyphs = len;

    if (!engine->stringToGlyphIndices(string, len, glyphs.data(), &numGlyphs, /*flags*/0))
        return false;

    for (int i = 0; i < numGlyphs; ++i)
        if (!glyphs[i])
            return false;

    return true;
}

