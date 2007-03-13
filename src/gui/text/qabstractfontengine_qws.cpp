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

/*!
    \class QFontEngineInfo
    \preliminary
    \brief The QFontEngineInfo class describes a specific font provided by a font engine plugin.

    \ingroup qws

    \tableofcontents

    QFontEngineInfo is used to describe a request of a font to a font engine plugin as well as to
    describe the actual fonts a plugin provides.

    \sa QAbstractFontEngine, QFontEnginePlugin
*/

/*!
   Constructs a new empty QFontEngineInfo.
*/
QFontEngineInfo::QFontEngineInfo()
{
    d = new QFontEngineInfoPrivate;
}

/*!
   Constructs a new QFontEngineInfo with the specified \a family.
   The resulting object represents a freely scalable font with normal
   weight and style.
*/
QFontEngineInfo::QFontEngineInfo(const QString &family)
{
    d = new QFontEngineInfoPrivate;
    d->family = family;
}

/*!
   Creates a new font engine info object with the same attributes as \a other.
*/
QFontEngineInfo::QFontEngineInfo(const QFontEngineInfo &other)
    : d(new QFontEngineInfoPrivate(*other.d))
{
}

/*!
   Assigns \a other to this font engine info object, and returns a reference
   to this.
*/
QFontEngineInfo &QFontEngineInfo::operator=(const QFontEngineInfo &other)
{
    *d = *other.d;
    return *this;
}

/*!
   Destroys this QFontEngineInfo object.
*/
QFontEngineInfo::~QFontEngineInfo()
{
    delete d;
}

/*!
   \property QFontEngineInfo::family
   the family name of the font
*/

void QFontEngineInfo::setFamily(const QString &family)
{
    d->family = family;
}

QString QFontEngineInfo::family() const
{
    return d->family;
}

/*!
   \property QFontEngineInfo::pixelSize
   the pixel size of the font

   A pixel size of 0 represents a freely scalable font.
*/

void QFontEngineInfo::setPixelSize(qreal size)
{
    d->pixelSize = size;
}

qreal QFontEngineInfo::pixelSize() const
{
    return d->pixelSize;
}

/*!
   \property QFontEngineInfo::weight
   the weight of the font

   The value should be from the \l{QFont::Weight} enumeration.
*/

void QFontEngineInfo::setWeight(int weight)
{
    d->weight = weight;
}

int QFontEngineInfo::weight() const
{
    return d->weight;
}

/*!
   \property QFontEngineInfo::style
   the style of the font
*/

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

/*!
    \class QFontEnginePlugin
    \preliminary
    \brief QFontEnginePlugin is the base class for font engine factory plugins in Qtopia Core.

    \ingroup qws

    \tableofcontents

    QFontEnginePlugin is provided by font engine plugins to create instances of subclasses of
    QAbstractFontEngine.

    The following functions need to be implemented: create() and availableFontEngines().

    \sa QAbstractFontEngine, QFontEngineInfo
*/

/*!
   Creates a font engine plugin that creates font engines with the specified \a foundry
   and \a parent.
*/
QFontEnginePlugin::QFontEnginePlugin(const QString &foundry, QObject *parent)
    : QObject(*new QFontEnginePluginPrivate, parent)
{
    Q_D(QFontEnginePlugin);
    d->foundry = foundry;
}

/*!
   Destroys this font engine plugin.
*/
QFontEnginePlugin::~QFontEnginePlugin()
{
}

/*!
   Returns a list of foundries the font engine plugin provides.
   The default implementation returns the foundry specified with the constructor.
*/
QStringList QFontEnginePlugin::keys() const
{
    Q_D(const QFontEnginePlugin);
    return QStringList(d->foundry);
}

/*!
    \fn QAbstractFontEngine *QFontEnginePlugin::create(const QFontEngineInfo &info)

    Implemented in subclasses to create a new font engine that provides a font that
    matches \a info.
*/

/*!
    \fn QList<QFontEngineInfo> QFontEnginePlugin::availableFontEngines() const

    Implemented in subclasses to return a list of QFontEngineInfo objects that represents all font
    engines the plugin can create.
*/

class QAbstractFontEnginePrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QAbstractFontEngine)
public:
};

/*!
    \class QAbstractFontEngine
    \preliminary
    \brief QAbstractFontEngine is the base class for font engine plugins in Qtopia Core.

    \ingroup qws

    \tableofcontents

    QAbstractFontEngine is implemented by font engine plugins through QFontEnginePlugin.

    \sa QFontEnginePlugin, QFontEngineInfo
*/

/*!
   \enum QAbstractFontEngine::Capability

   This enum describes the capabilities of a font engine.

   \value CanRenderGlyphs_Gray The font engine can render individual glyphs into 8 bpp images.
   \value CanRenderGlyphs_Mono The font engine can render individual glyphs into 1 bpp images.
   \value CanRenderGlyphs The font engine can render individual glyphs into images.
   \value CanOutlineGlyphs The font engine can convert glyphs to painter paths.
*/

/*!
   \enum QAbstractFontEngine::FontProperty

   This enum describes the properties of a font provided by a font engine.

   \value Ascent The ascent of the font, specified as a 26.6 fixed point value.
   \value Descent The descent of the font, specified as a 26.6 fixed point value.
   \value Leading The leading of the font, specified as a 26.6 fixed point value.
   \value XHeight The 'x' height of the font, specified as a 26.6 fixed point value.
   \value AverageCharWidth The average character width of the font, specified as a 26.6 fixed point value.
   \value LineThickness The thickness of the underline and strikeout lines for the font, specified as a 26.6 fixed point value.
   \value UnderlinePosition The distance from the base line to the underline position for the font, specified as a 26.6 fixed point value.
   \value MaxCharWidth The width of the widest character in the font, specified as a 26.6 fixed point value.
   \value MinLeftBearing The minimum left bearing of the font, specified as a 26.6 fixed point value.
   \value MinRightBearing The maximum right bearing of the font, specified as a 26.6 fixed point value.
   \value GlyphCount The number of glyphs in the font, specified as an integer value.
   \value CacheGlyphsHint A boolean value specifying whether rendered glyphs should be cached by Qt.
   \value OutlineGlyphsHint A boolean value specifying whether the font engine prefers outline drawing over image rendering for uncached glyphs.
*/

/*!
   \enum QAbstractFontEngine::TextShapingFlag

   This enum describes flags controlling conversion of characters to glyphs and their metrics.

   \value RightToLeft The text is used in a right-to-left context.
   \value ReturnDesignMetrics Return font design metrics instead of pixel metrics.
*/

/*!
   Constructs a new QAbstractFontEngine with the given \a parent.
*/
QAbstractFontEngine::QAbstractFontEngine(QObject *parent)
    : QObject(*new QAbstractFontEnginePrivate, parent)
{
}

/*!
   Destroys this QAbstractFontEngine object.
*/
QAbstractFontEngine::~QAbstractFontEngine()
{
}

/*!
    \fn QAbstractFontEngine::Capabilities QAbstractFontEngine::capabilities() const

    Implemented in subclasses to specify the font engine's capabilities. The return value
    may be cached by the caller and is expected not to change during the lifetime of the
    font engine.
*/

/*!
    \fn QVariant QAbstractFontEngine::fontProperty(FontProperty property) const

    Implemented in subclasses to return the value of the font attribute \a property. The return
    value may be cached by the caller and is expected not to change during the lifetime of the font
    engine.
*/

/*!
    \fn bool QAbstractFontEngine::convertStringToGlyphIndices(const QChar *string, int length, uint *glyphs, int *numGlyphs, TextShapingFlags flags) const

    Implemented in subclasses to convert the characters specified by \a string and \a length to
    glyph indicies, using \a flags. The glyph indicies should be returned in the \a glyphs array
    provided by the caller. The maximum size of \a glyphs is specified by the value pointed to by \a
    numGlyphs. If successful, the subclass implementation sets the value pointed to by \a numGlyphs
    to the actual number of glyph indices generated, and returns true. Otherwise, e.g. if there is
    not enough space in the provided \a glyphs array, it should set \a numGlyphs to the number of
    glyphs needed for the conversion and return false.
*/

/*!
    \fn void QAbstractFontEngine::getGlyphAdvances(const uint *glyphs, int numGlyphs, Fixed *advances, TextShapingFlags flags) const

    Implemented in subclasses to retrieve the advances of the array specified by \a glyphs and \a
    numGlyphs, using \a flags. The result is returned in \a advances, which is allocated by the
    caller and contains \a numGlyphs elements.
*/

/*!
    \fn QAbstractFontEngine::GlyphMetrics QAbstractFontEngine::glyphMetrics(uint glyph) const

    Implemented in subclass to return the metrics for \a glyph.
*/

/*!
   Implemented in subclasses to render the specified \a glyph into a \a buffer with the given \a depth ,
   \a bytesPerLine and \a height.

   Returns true if rendering succeeded, false otherwise.
*/
bool QAbstractFontEngine::renderGlyph(uint glyph, int depth, int bytesPerLine, int height, uchar *buffer)
{
    Q_UNUSED(glyph)
    Q_UNUSED(depth)
    Q_UNUSED(bytesPerLine)
    Q_UNUSED(height)
    Q_UNUSED(buffer)
    qWarning("QAbstractFontEngine: renderGlyph is not implemented in font plugin!");
    return false;
}

/*!
   Implemented in subclasses to add the outline of the glyphs specified by \a glyphs and \a
   numGlyphs at the specified \a positions to the painter path \a path.
*/
void QAbstractFontEngine::addGlyphOutlinesToPath(uint *glyphs, int numGlyphs, FixedPoint *positions, QPainterPath *path)
{
    Q_UNUSED(glyphs)
    Q_UNUSED(numGlyphs)
    Q_UNUSED(positions)
    Q_UNUSED(path)
    qWarning("QAbstractFontEngine: addGlyphOutlinesToPath is not implemented in font plugin!");
}

/*
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
*/

QProxyFontEngine::QProxyFontEngine(QAbstractFontEngine *customEngine, const QFontDef &def)
    : engine(customEngine)
{
    fontDef = def;
    engineCapabilities = engine->capabilities();
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
    if (!(engineCapabilities & QAbstractFontEngine::CanRenderGlyphs_Gray))
        return QFontEngine::alphaMapForGlyph(glyph);

    QAbstractFontEngine::GlyphMetrics metrics = engine->glyphMetrics(glyph);
    if (metrics.width <= 0 || metrics.height <= 0)
        return QImage();

    QImage img(metrics.width >> 6, metrics.height >> 6, QImage::Format_Indexed8);

    // ### we should have QImage::Format_GrayScale8
    static QVector<QRgb> colorMap;
    if (colorMap.isEmpty()) {
        colorMap.resize(256);
        for (int i=0; i<256; ++i)
            colorMap[i] = qRgba(0, 0, 0, i);
    }

    img.setColorTable(colorMap);

    engine->renderGlyph(glyph, /*depth*/8, img.bytesPerLine(), img.height(), img.bits());

    return img;
}

void QProxyFontEngine::addGlyphsToPath(glyph_t *glyphs, QFixedPoint *positions, int nglyphs, QPainterPath *path, QTextItem::RenderFlags flags)
{
    if (engineCapabilities & QAbstractFontEngine::CanOutlineGlyphs)
        engine->addGlyphOutlinesToPath(glyphs, nglyphs, reinterpret_cast<QAbstractFontEngine::FixedPoint *>(positions), path);
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
        QImage glyph = alphaMapForGlyph(glyphs[i]);
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
    if (!(engineCapabilities & QAbstractFontEngine::CanOutlineGlyphs))
        return false;

    QVariant outlineHint = engine->fontProperty(QAbstractFontEngine::OutlineGlyphsHint);
    return !outlineHint.isValid() || outlineHint.toBool();
}

