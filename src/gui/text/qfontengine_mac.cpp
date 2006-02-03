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

#include <private/qapplication_p.h>
#include <private/qfontengine_p.h>
#include <private/qpainter_p.h>
#include <private/qtextengine_p.h>
#include <qbitmap.h>
#include <private/qpaintengine_mac_p.h>
#include <private/qprintengine_mac_p.h>
#include <qglobal.h>
#include <qpixmap.h>
#include <qpixmapcache.h>
#include <qvarlengtharray.h>
#include <qdebug.h>

#include <ApplicationServices/ApplicationServices.h>

/*****************************************************************************
  QFontEngine debug facilities
 *****************************************************************************/
//#define DEBUG_ADVANCES


#ifndef FixedToQFixed
#define FixedToQFixed(a) QFixed::fromFixed((a) >> 10)
#define QFixedToFixed(x) ((x).value() << 10)
#endif

class QMacFontPath
{
    float x, y;
    QPainterPath *path;
public:
    inline QMacFontPath(float _x, float _y, QPainterPath *_path) : x(_x), y(_y), path(_path) { }
    inline void setPosition(float _x, float _y) { x = _x; y = _y; }
    inline void advance(float _x) { x += _x; }
    static OSStatus lineTo(const Float32Point *, void *);
    static OSStatus cubicTo(const Float32Point *, const Float32Point *,
                            const Float32Point *, void *);
    static OSStatus moveTo(const Float32Point *, void *);
    static OSStatus closePath(void *);
};

OSStatus QMacFontPath::lineTo(const Float32Point *pt, void *data)

{
    QMacFontPath *p = static_cast<QMacFontPath*>(data);
    p->path->lineTo(p->x + pt->x, p->y + pt->y);
    return noErr;
}

OSStatus QMacFontPath::cubicTo(const Float32Point *cp1, const Float32Point *cp2,
                               const Float32Point *ep, void *data)

{
    QMacFontPath *p = static_cast<QMacFontPath*>(data);
    p->path->cubicTo(p->x + cp1->x, p->y + cp1->y,
                     p->x + cp2->x, p->y + cp2->y,
                     p->x + ep->x, p->y + ep->y);
    return noErr;
}

OSStatus QMacFontPath::moveTo(const Float32Point *pt, void *data)
{
    QMacFontPath *p = static_cast<QMacFontPath*>(data);
    p->path->moveTo(p->x + pt->x, p->y + pt->y);
    return noErr;
}

OSStatus QMacFontPath::closePath(void *data)
{
    static_cast<QMacFontPath*>(data)->path->closeSubpath();
    return noErr;
}



#if defined(Q_NEW_MAC_FONTENGINE)

QFontEngineMac::QFontEngineMac()
{
    familyref = 0;
    synthesisFlags = 0;
    fonts.resize(1);
}

void QFontEngineMac::init()
{
    OSStatus status;

    status = ATSUCreateTextLayout(&textLayout);
    Q_ASSERT(status == noErr);

    const int maxAttributeCount = 4;
    ATSUAttributeTag tags[maxAttributeCount + 1];
    ByteCount sizes[maxAttributeCount + 1];
    ATSUAttributeValuePtr values[maxAttributeCount + 1];
    int attributeCount = 0;

    Fixed size = FixRatio(fontDef.pixelSize, 1);
    tags[attributeCount] = kATSUSizeTag;
    sizes[attributeCount] = sizeof(size);
    values[attributeCount] = &size;
    ++attributeCount;

//    QCFString familyStr;
//    ATSFontFamilyGetName(familyref, kATSOptionFlagsDefault, &familyStr);
//    qDebug() << "family" << (QString)familyStr;

    FMFontFamily family = FMGetFontFamilyFromATSFontFamilyRef(familyref);
    Q_ASSERT(family != kInvalidFontFamily);

    FMFontStyle fntStyle = 0;
    if (fontDef.weight >= QFont::Bold)
        fntStyle |= ::bold;
    if (fontDef.style != QFont::StyleNormal)
        fntStyle |= ::italic;

    FMFontStyle intrinsicStyle;
    status = FMGetFontFromFontFamilyInstance(family, fntStyle, &fonts[0], &intrinsicStyle);
    Q_ASSERT(status == noErr);

    status = FMGetFontFamilyInstanceFromFont(fonts.at(0), &family, &intrinsicStyle);
    Q_ASSERT(status == noErr);

    Boolean atsuBold = false;
    Boolean atsuItalic = false;

    if ((fntStyle & ::italic)
        && (!(intrinsicStyle & ::italic))) {
            synthesisFlags |= SynthesizedItalic;

            atsuItalic = true;
            tags[attributeCount] = kATSUQDItalicTag;
            sizes[attributeCount] = sizeof(atsuItalic);
            values[attributeCount] = &atsuItalic;
            ++attributeCount;
    }
    if ((fntStyle & ::bold)
        && (!(intrinsicStyle & ::bold))) {
        synthesisFlags |= SynthesizedBold;

        atsuBold = true;
        tags[attributeCount] = kATSUQDBoldfaceTag;
        sizes[attributeCount] = sizeof(atsuBold);
        values[attributeCount] = &atsuBold;
        ++attributeCount;
   }

    tags[attributeCount] = kATSUFontTag;
    // KATSUFontID is typedef'ed to FMFont
    sizes[attributeCount] = sizeof(FMFont);
    values[attributeCount] = &fonts[0];
    ++attributeCount;

    status = ATSUCreateStyle(&style);
    Q_ASSERT(status == noErr);

    Q_ASSERT(attributeCount < maxAttributeCount + 1);
    status = ATSUSetAttributes(style, attributeCount, tags, sizes, values);
    Q_ASSERT(status == noErr);
}

QFontEngineMac::~QFontEngineMac()
{
    ATSUDisposeTextLayout(textLayout);
    ATSUDisposeStyle(style);
}

struct QGlyphLayoutInfo
{
    QGlyphLayout *glyphs;
    int *numGlyphs;
    bool callbackCalled;
    int *mappedFonts;
    QTextEngine::ShaperFlags flags;
};

static OSStatus atsuPostLayoutCallback(ATSULayoutOperationSelector selector,
                                 ATSULineRef lineRef,
                                 UInt32 refCon,
                                 void *operationExtraParameter,
                                 ATSULayoutOperationCallbackStatus *callbackStatus)
{
    Q_UNUSED(selector);
    Q_UNUSED(operationExtraParameter);

    QGlyphLayoutInfo *nfo = reinterpret_cast<QGlyphLayoutInfo *>(refCon);
    nfo->callbackCalled = true;

    ATSLayoutRecord *layoutData = 0;
    ItemCount itemCount = 0;

    OSStatus e = noErr;
    e = ATSUDirectGetLayoutDataArrayPtrFromLineRef(lineRef, kATSUDirectDataLayoutRecordATSLayoutRecordCurrent,
                                                   /*iCreate =*/ false,
                                                   (void **) &layoutData,
                                                   &itemCount);
    if (e != noErr)
        return e;

    *nfo->numGlyphs = itemCount - 1;

    Fixed *advanceDeltas = 0;
    Fixed *baselineDeltas = 0;

    e = ATSUDirectGetLayoutDataArrayPtrFromLineRef(lineRef, kATSUDirectDataAdvanceDeltaFixedArray,
                                                   /*iCreate =*/ true,
                                                   (void **) &advanceDeltas,
                                                   &itemCount);
    if (e != noErr)
        return e;

    e = ATSUDirectGetLayoutDataArrayPtrFromLineRef(lineRef, kATSUDirectDataBaselineDeltaFixedArray,
                                                   /*iCreate =*/ true,
                                                   (void **) &baselineDeltas,
                                                   &itemCount);
    if (e != noErr)
        return e;

    if (nfo->flags & QTextEngine::RightToLeft) {
        for (ItemCount i = 0; i < itemCount - 1; ++i) {
            int j = itemCount - 2 - i;

            const uint charOffset = layoutData[i].originalOffset / sizeof(UniChar);
            const int fontIdx = nfo->mappedFonts[charOffset];

            nfo->glyphs[j].glyph = (layoutData[i].glyphID & 0x00ffffff) | (fontIdx << 24);

            nfo->glyphs[j].advance.y = FixedToQFixed(baselineDeltas[i]);
            nfo->glyphs[j].advance.x = FixedToQFixed(layoutData[i + 1].realPos - layoutData[i].realPos);
        }
    } else {
        for (ItemCount i = 0; i < itemCount - 1; ++i) {

            const uint charOffset = layoutData[i].originalOffset / sizeof(UniChar);
            const int fontIdx = nfo->mappedFonts[charOffset];

            nfo->glyphs[i].glyph = (layoutData[i].glyphID & 0x00ffffff) | (fontIdx << 24);

            nfo->glyphs[i].advance.y = FixedToQFixed(baselineDeltas[i]);
            nfo->glyphs[i].advance.x = FixedToQFixed(layoutData[i + 1].realPos - layoutData[i].realPos);
        }
    }

    ATSUDirectReleaseLayoutDataArrayPtr(lineRef, kATSUDirectDataBaselineDeltaFixedArray,
                                        (void **) &baselineDeltas);

    ATSUDirectReleaseLayoutDataArrayPtr(lineRef, kATSUDirectDataAdvanceDeltaFixedArray,
                                        (void **) &advanceDeltas);

    ATSUDirectReleaseLayoutDataArrayPtr(lineRef, kATSUDirectDataLayoutRecordATSLayoutRecordCurrent,
                                        (void **) &layoutData);

    *callbackStatus = kATSULayoutOperationCallbackStatusHandled;
    return noErr;
}

int QFontEngineMac::fontIndexForFMFont(FMFont font) const
{
    for (int i = 0; i < fonts.count(); ++i)
        if (fonts.at(i) == font)
            return i;
    fonts.append(font);
    return fonts.count() - 1;
}

bool QFontEngineMac::stringToCMap(const QChar *str, int len, QGlyphLayout *glyphs, int *nglyphs, QTextEngine::ShaperFlags flags) const
{
    if (*nglyphs < len) {
        *nglyphs = len;
        return false;
    }
    //qDebug() << "stringToCMap" << QString(str, len);

    OSStatus e = noErr;

    QGlyphLayoutInfo nfo;
    nfo.glyphs = glyphs;
    nfo.numGlyphs = nglyphs;
    nfo.callbackCalled = false;
    nfo.flags = flags;

    QVarLengthArray<int> mappedFonts(len);
    for (int i = 0; i < len; ++i)
        mappedFonts[i] = 0;
    nfo.mappedFonts = mappedFonts.data();

    Q_ASSERT(sizeof(void *) <= sizeof(UInt32));
    e = ATSUSetTextLayoutRefCon(textLayout, reinterpret_cast<UInt32>(&nfo));
    if (e != noErr) {
        qWarning("Qt: internal: %ld: Error ATSUSetTextLayoutRefCon %s: %d", e, __FILE__, __LINE__);
        return false;
    }

    {
        const int maxAttributeCount = 2;
        ATSUAttributeTag tags[maxAttributeCount + 1];
        ByteCount sizes[maxAttributeCount + 1];
        ATSUAttributeValuePtr values[maxAttributeCount + 1];
        int attributeCount = 0;

        tags[attributeCount] = kATSULineLayoutOptionsTag;
        ATSLineLayoutOptions layopts = kATSLineHasNoOpticalAlignment
                                       | kATSLineIgnoreFontLeading
                                       | kATSLineNoSpecialJustification // we do kashidas ourselves
                                       | kATSLineDisableAllJustification
                                       ;

        if (!(flags & QTextEngine::DesignMetrics)) {
            layopts |= kATSLineFractDisable | kATSLineUseDeviceMetrics
                       | kATSLineDisableAutoAdjustDisplayPos;
        }

        if (fontDef.styleStrategy & QFont::NoAntialias)
            layopts |= kATSLineNoAntiAliasing;

        if (!kerning)
            layopts |= kATSLineDisableAllKerningAdjustments;

        values[attributeCount] = &layopts;
        sizes[attributeCount] = sizeof(layopts);
        ++attributeCount;

        tags[attributeCount] = kATSULayoutOperationOverrideTag;
        ATSULayoutOperationOverrideSpecifier spec;
        spec.operationSelector = kATSULayoutOperationPostLayoutAdjustment;
        spec.overrideUPP = atsuPostLayoutCallback;
        values[attributeCount] = &spec;
        sizes[attributeCount] = sizeof(spec);
        ++attributeCount;

        Q_ASSERT(attributeCount < maxAttributeCount + 1);
        e = ATSUSetLayoutControls(textLayout, attributeCount, tags, sizes, values);
        if (e != noErr) {
            qWarning("Qt: internal: %ld: Error ATSUSetLayoutControls %s: %d", e, __FILE__, __LINE__);
            return false;
        }

    }

    e = ATSUSetTextPointerLocation(textLayout, (UniChar *)(str), 0, len, len);
    if (e != noErr) {
        qWarning("Qt: internal: %ld: Error ATSUSetTextPointerLocation %s: %d", e, __FILE__, __LINE__);
        return false;
    }

    e = ATSUSetRunStyle(textLayout, style, 0, len);
    if (e != noErr) {
        qWarning("Qt: internal: %ld: Error ATSUSetRunStyle %s: %d", e, __FILE__, __LINE__);
        return false;
    }

    {
        int pos = 0;
        do {
            FMFont substFont = 0;
            UniCharArrayOffset changedOffset = 0;
            UniCharCount changeCount = 0;

            e = ATSUMatchFontsToText(textLayout, pos, len - pos,
                                     &substFont, &changedOffset,
                                     &changeCount);
            if (e == kATSUFontsMatched) {
                int fontIdx = fontIndexForFMFont(substFont);
                for (uint i = 0; i < changeCount; ++i)
                    mappedFonts[changedOffset + i] = fontIdx;
                pos = changedOffset + changeCount;

                ATSUStyle fontSubstStyle = styleForFont(fontIdx);

                ATSUSetRunStyle(textLayout, fontSubstStyle, changedOffset, changeCount);

                ATSUDisposeStyle(fontSubstStyle);

            } else if (e == kATSUFontsNotMatched) {
                pos = changedOffset + changeCount;
            }
        } while (pos < len && e != noErr);
    }

    // trigger the a layout
    {
        Rect rect;
        e = ATSUMeasureTextImage(textLayout, kATSUFromTextBeginning, kATSUToTextEnd,
                                 /*iLocationX =*/ 0, /*iLocationY =*/ 0,
                                 &rect);
        if (e != noErr) {
            qWarning("Qt: internal: %ld: Error ATSUMeasureTextImage %s: %d", e, __FILE__, __LINE__);
            return false;
        }
    }

    if (!nfo.callbackCalled) {
            qWarning("Qt: internal: %ld: Error ATSUMeasureTextImage did not trigger callback %s: %d", e, __FILE__, __LINE__);
            return false;
    }

    ATSUClearLayoutCache(textLayout, kATSUFromTextBeginning);

    return true;
}

void QFontEngineMac::recalcAdvances(int numGlyphs, QGlyphLayout *glyphs, QTextEngine::ShaperFlags flags) const
{
    Q_ASSERT(false);
    Q_UNUSED(numGlyphs);
    Q_UNUSED(glyphs);
    Q_UNUSED(flags);
}

void QFontEngineMac::doKerning(int , QGlyphLayout *, QTextEngine::ShaperFlags) const
{
    //Q_ASSERT(false);
}

void QFontEngineMac::addGlyphsToPath(glyph_t *glyphs, QFixedPoint *positions, int numGlyphs, QPainterPath *path,
                                     QTextItem::RenderFlags)
{
    if (!numGlyphs)
        return;

    OSStatus e;

    QMacFontPath fontpath(0, 0, path);
    ATSCubicMoveToUPP moveTo = NewATSCubicMoveToUPP(QMacFontPath::moveTo);
    ATSCubicLineToUPP lineTo = NewATSCubicLineToUPP(QMacFontPath::lineTo);
    ATSCubicCurveToUPP cubicTo = NewATSCubicCurveToUPP(QMacFontPath::cubicTo);
    ATSCubicClosePathUPP closePath = NewATSCubicClosePathUPP(QMacFontPath::closePath);

    int lastFontIndex = 0;
    ATSUStyle currentStyle = style;

    for (int i = 0; i < numGlyphs; ++i) {
        const int fontIdx = glyphs[i] >> 24;
        GlyphID glyph = glyphs[i] & 0x00ffffff;

        if (fontIdx != lastFontIndex) {
            if (currentStyle != style)
                ATSUDisposeStyle(currentStyle);

            currentStyle = styleForFont(fontIdx);

            lastFontIndex = fontIdx;
        }

        fontpath.setPosition(positions[i].x.toReal(), positions[i].y.toReal());
        ATSUGlyphGetCubicPaths(currentStyle, glyph, moveTo, lineTo,
                               cubicTo, closePath, &fontpath, &e);
    }

    if (currentStyle != style)
        ATSUDisposeStyle(currentStyle);

    DisposeATSCubicMoveToUPP(moveTo);
    DisposeATSCubicLineToUPP(lineTo);
    DisposeATSCubicCurveToUPP(cubicTo);
    DisposeATSCubicClosePathUPP(closePath);
}

glyph_metrics_t QFontEngineMac::boundingBox(const QGlyphLayout *glyphs, int numGlyphs)
{
    QFixed w;
    const QGlyphLayout *end = glyphs + numGlyphs;
    while(end > glyphs)
        w += (--end)->advance.x;
    return glyph_metrics_t(0, -(ascent()), w, ascent()+descent(), w, 0);
}

glyph_metrics_t QFontEngineMac::boundingBox(glyph_t glyph)
{
    GlyphID atsuGlyph = glyph & 0x00ffffff;
    ATSUStyle glyphStyle = styleForFont(glyph >> 24);

    ATSGlyphScreenMetrics metrics;

    ATSUGlyphGetScreenMetrics(glyphStyle, 1, &atsuGlyph, 0,
                              /* iForcingAntiAlias =*/ false,
                              /* iAntiAliasSwitch =*/true,
                              &metrics);

    if (glyphStyle != style)
        ATSUDisposeStyle(glyphStyle);

    // ### check again

    glyph_metrics_t gm;
    gm.width = int(metrics.width);
    gm.height = int(metrics.height);
    gm.x = QFixed::fromReal(metrics.topLeft.x);
    gm.y = QFixed::fromReal(metrics.topLeft.y);
    gm.xoff = gm.yoff = 0;

    return gm;
}

QFixed QFontEngineMac::ascent() const
{
    ATSUTextMeasurement metric;
    ATSUGetAttribute(style, kATSUAscentTag, sizeof(metric), &metric, 0);
    return FixRound(metric);
}

QFixed QFontEngineMac::descent() const
{
    ATSUTextMeasurement metric;
    ATSUGetAttribute(style, kATSUDescentTag, sizeof(metric), &metric, 0);
    return FixRound(metric);
}

QFixed QFontEngineMac::leading() const
{
    ATSUTextMeasurement metric;
    ATSUGetAttribute(style, kATSULeadingTag, sizeof(metric), &metric, 0);
    return FixRound(metric);
}

qreal QFontEngineMac::maxCharWidth() const
{
    ATSFontRef atsFont = FMGetATSFontRefFromFont(fonts.at(0));
    ATSFontMetrics metrics;
    ATSFontGetHorizontalMetrics(atsFont, kATSOptionFlagsDefault, &metrics);
    return metrics.maxAdvanceWidth * fontDef.pointSize;
}

QFixed QFontEngineMac::xHeight() const
{
    ATSFontRef atsFont = FMGetATSFontRefFromFont(fonts.at(0));
    ATSFontMetrics metrics;
    ATSFontGetHorizontalMetrics(atsFont, kATSOptionFlagsDefault, &metrics);
    return QFixed::fromReal(metrics.xHeight * fontDef.pointSize);
}

void QFontEngineMac::draw(CGContextRef ctx, qreal x, qreal y, const QTextItemInt &ti, int paintDeviceHeight)
{
    CGContextSetFontSize(ctx, fontDef.pixelSize);

    CGAffineTransform oldTextMatrix = CGContextGetTextMatrix(ctx);

    CGAffineTransform cgMatrix = CGAffineTransformMake(1, 0, 0, -1, 0, -paintDeviceHeight);

    CGAffineTransformConcat(cgMatrix, oldTextMatrix);

    if (synthesisFlags & QFontEngine::SynthesizedItalic)
        cgMatrix = CGAffineTransformConcat(cgMatrix, CGAffineTransformMake(1, 0, -tanf(14 * acosf(0) / 90), 1, 0, 0));

    CGContextSetTextMatrix(ctx, cgMatrix);

    CGContextSetTextDrawingMode(ctx, kCGTextFill);

    if (!(fontDef.styleStrategy & QFont::NoAntialias)) {
        CGContextSetShouldSmoothFonts(ctx, true);
        CGContextSetShouldAntialias(ctx, true);
    }

    QVarLengthArray<QFixedPoint> positions;
    QVarLengthArray<glyph_t> glyphs;
    QMatrix matrix;
    matrix.translate(x, y);
    ti.fontEngine->getGlyphPositions(ti.glyphs, ti.num_glyphs, matrix, ti.flags, glyphs, positions);

    CGContextSetTextPosition(ctx, positions[0].x.toReal(), positions[0].y.toReal());

    QVarLengthArray<CGSize> advances(glyphs.size());
    QVarLengthArray<CGGlyph> cgGlyphs(glyphs.size());

    for (int i = 0; i < glyphs.size() - 1; ++i) {
        advances[i].width = (positions[i + 1].x - positions[i].x).toReal();
        advances[i].height = (positions[i + 1].y - positions[i].y).toReal();
    }
    advances[glyphs.size() - 1].width = 0;
    advances[glyphs.size() - 1].height = 0;

    int currentSpan = 0;
    int currentFont = glyphs[0] >> 24;
    cgGlyphs[0] = glyphs[0] & 0x00ffffff;

    for (int i = 1; i < glyphs.size(); ++i) {
        const int nextFont = glyphs[i] >> 24;
        cgGlyphs[i] = glyphs[i] & 0x00ffffff;
        if (nextFont == currentFont)
            continue;

        FMFont fnt = fonts.at(currentFont);
        ATSFontRef atsFont = FMGetATSFontRefFromFont(fnt);
        CGFontRef cgFont = CGFontCreateWithPlatformFont(&atsFont);
        Q_ASSERT(cgFont != 0);

        CGContextSetFont(ctx, cgFont);

        CGContextShowGlyphsWithAdvances(ctx, cgGlyphs.data() + currentSpan,
                                        advances.data() + currentSpan, i - currentSpan);

        currentFont = nextFont;
        currentSpan = i;
    }

    FMFont fnt = fonts.at(currentFont);
    ATSFontRef atsFont = FMGetATSFontRefFromFont(fnt);
    CGFontRef cgFont = CGFontCreateWithPlatformFont(&atsFont);
    Q_ASSERT(cgFont != 0);

    CGContextSetFont(ctx, cgFont);

    CGContextShowGlyphsWithAdvances(ctx, cgGlyphs.data() + currentSpan,
                                    advances.data() + currentSpan, glyphs.size() - currentSpan);

    CGContextSetTextMatrix(ctx, oldTextMatrix);
}

bool QFontEngineMac::canRender(const QChar *string, int len)
{
    ATSUSetTextPointerLocation(textLayout, reinterpret_cast<const UniChar *>(string), 0, len, len);
    ATSUSetRunStyle(textLayout, style, 0, len);

    OSStatus e = noErr;
    int pos = 0;
    do {
        FMFont substFont = 0;
        UniCharArrayOffset changedOffset = 0;
        UniCharCount changeCount = 0;

        e = ATSUMatchFontsToText(textLayout, pos, len - pos,
                                 &substFont, &changedOffset,
                                 &changeCount);
        if (e == kATSUFontsMatched) {
            pos = changedOffset + changeCount;
        }
    } while (pos < len && e != noErr);

    return e == noErr || e == kATSUFontsMatched;
}

ATSUStyle QFontEngineMac::styleForFont(int fontIndex) const
{
    if (fontIndex == 0)
        return style;

    ATSUStyle result;
    FMFont font = fonts.at(fontIndex);

    ATSUCreateAndCopyStyle(style, &result);

    ATSUAttributeTag tag = kATSUFontTag;
    ATSUAttributeValuePtr value = &font;
    ByteCount dummySize = sizeof(font);

    ATSUSetAttributes(result, 1, &tag, &dummySize, &value);

    return result;
}

#else

struct QATSUStyle {
    ATSUStyle style;
    QColor rgb;
    int ascent, descent, leading, maxWidth; //slight cache
    QATSUStyle() {
        style = 0;
    }
    ~QATSUStyle() {
        if (style)
            ATSUDisposeStyle(style);

    }
};

//Mac (ATSUI) engine
QFontEngineMac::QFontEngineMac() : QFontEngine(), mTextLayout(0), internal_fi(0), familyref(0)
{
    memset(widthCache, 0, sizeof(widthCache));
    cache_cost = 0;

    glyphCache.setMaxCost(300);
}

QFontEngineMac::~QFontEngineMac()
{
    if (mTextLayout)
        ATSUDisposeTextLayout(mTextLayout);
    delete internal_fi;
}

bool QFontEngineMac::stringToCMap(const QChar *str, int len, QGlyphLayout *glyphs, int *nglyphs,
                                  QTextEngine::ShaperFlags flags) const
{
    if(*nglyphs < len) {
        *nglyphs = len;
        return false;
    }
    *nglyphs = len;
    for (int i = 0; i < len; ++i)
        glyphs[i].glyph = str[i].unicode();
    recalcAdvances(*nglyphs, glyphs, flags);
    return true;
}

void
QFontEngineMac::recalcAdvances(int numGlyphs, QGlyphLayout *glyphs,
                               QTextEngine::ShaperFlags /*flags*/) const
{
    if(numGlyphs < 1)
        return;

    QVarLengthArray<QChar> str(numGlyphs);
    QChar *data = str.data();
    for(int i = 0; i < numGlyphs; ++i)
        data[i] = ushort(glyphs[i].glyph);
    QFixed *advances = 0;
    doTextTask(str.constData(), 0, numGlyphs, numGlyphs, ADVANCES, 0, 0, 0, (void**)&advances);
    if(advances) { //try using "correct" advances
        for(int i = 0; i < numGlyphs; ++i) {
            glyphs[i].advance.x = advances[i];
            glyphs[i].advance.y = 0;
        }
    } else { //fall back to something (widths)
        for (int i = 0; i < numGlyphs; ++i) {
            bool surrogate = false;
            if(i < numGlyphs -1) {
                surrogate = (glyphs[i].glyph >= 0xd800 && glyphs[i].glyph < 0xdc00 &&
                             glyphs[i+1].glyph >= 0xdc00 && glyphs[i+1].glyph < 0xe000);
            }
            if (!surrogate && glyphs[i].glyph < widthCacheSize && widthCache[glyphs[i].glyph]) {
                int c = widthCache[glyphs[i].glyph];
                if(c == -777)
                    c = 0;
                glyphs[i].advance.x = c;
                glyphs[i].advance.y = 0;
            } else {
                QChar qc[2];
                qc[0] = glyphs[i].glyph;
                if(surrogate)
                    qc[1] = glyphs[i+1].glyph;
                glyphs[i].advance.x = doTextTask(qc, 0, surrogate ? 2 : 1, 1, WIDTH);
                glyphs[i].advance.y = 0;
            }
            if (surrogate)
                ++i;
        }
    }
}

void
QFontEngineMac::doKerning(int, QGlyphLayout *, QTextEngine::ShaperFlags) const
{
    //we do nothing here because kerning is taken into account in the recalcAdvances
}

void
QFontEngineMac::draw(QPaintEngine *p, qreal req_x, qreal req_y, const QTextItemInt &si)
{
    if(p->type() == QPaintEngine::MacPrinter)
        p = static_cast<QMacPrintEngine*>(p)->paintEngine();
    QPaintEngineState *pState = p->state;
    QFixed x = QFixed::fromReal(req_x), y = QFixed::fromReal(req_y);

#if 1
    if(p->type() == QPaintEngine::QuickDraw && !pState->matrix().isIdentity()) {
        QFixed aw = si.width, ah = si.ascent + si.descent + 1;
        if(aw == 0 || ah == 0)
            return;
        QBitmap bm(qRound(aw), qRound(ah));        // create bitmap
        {
            QPainter paint(&bm);  // draw text in bitmap
            paint.setPen(Qt::color1);
            paint.drawTextItem(QPointF(0, si.ascent.toReal()), si);
            paint.end();
        }

        QPixmap pm(bm.size());
        if(pState->backgroundMode() == Qt::OpaqueMode) {
            pm = bm;
            bm.fill(Qt::color1);
            pm.setMask(bm);
        } else {
            QPainter paint(&pm);
            paint.fillRect(QRectF(0, 0, pm.width(), pm.height()), pState->pen().color());
            paint.end();
            pm.setMask(bm);
        }
        pState->painter()->drawPixmap(QPointF(x.toReal(), (y - si.ascent).toReal()), pm);
        return;
    }
#endif
    if(p->type() == QPaintEngine::QuickDraw) {
        QQuickDrawPaintEngine *mgc = static_cast<QQuickDrawPaintEngine *>(p);
        mgc->syncState();
        mgc->setupQDPort(false, 0, 0);
        mgc->setupQDFont();
    }

    if(pState->backgroundMode() == Qt::OpaqueMode) {
        glyph_metrics_t br = boundingBox(si.glyphs, si.num_glyphs);
        pState->painter()->fillRect(QRectF((x + br.x).toReal(), (y + br.y).toReal(), br.width.toReal(),
                                    br.height.toReal()), pState->backgroundBrush().color());
    }

    bool textAA = pState->renderHints() & QPainter::TextAntialiasing;
    bool lineAA = pState->renderHints() & QPainter::Antialiasing;
    if(p->type() == QPaintEngine::CoreGraphics && textAA != lineAA)
        CGContextSetShouldAntialias(QMacCGContext(p->painter()), textAA);

    if(si.flags & QTextItem::RightToLeft) {
        for(int i = si.num_glyphs-1; i >= 0; --i) {
            const QChar glyph((ushort)si.glyphs[i].glyph);
            doTextTask(&glyph, 0, 1, 1, DRAW, x, y, p);
            x += si.glyphs[i].advance.x;
        }
    } else {
        QVarLengthArray<ushort> g(si.num_glyphs);
        for(int i = 0; i < si.num_glyphs; ++i)
            g[i] = si.glyphs[i].glyph;
        doTextTask((QChar*)g.data(), 0, si.num_glyphs, si.num_glyphs, DRAW, x, y, p);
    }
    if(si.width != 0 && si.flags != 0) {
        QPen oldPen = pState->pen();
        QBrush oldBrush = pState->brush();
        pState->painter()->setBrush(pState->pen().color());
        pState->painter()->setPen(Qt::NoPen);
        const qreal lw = lineThickness().toReal();
        if(si.flags & QTextItem::Underline) {
            QBrush anotherOldBrush = pState->painter()->brush();
            if (si.underlineColor.isValid())
                pState->painter()->setBrush(si.underlineColor);
            pState->painter()->drawRect(QRectF(req_x, req_y + underlinePosition().toReal(),
                                               si.width.toReal(), lw));
            pState->painter()->setBrush(anotherOldBrush);
        }
        if(si.flags & QTextItem::Overline)
            pState->painter()->drawRect(QRectF(req_x, req_y - (ascent() + 1).toReal(),
                                        si.width.toReal(), lw));
        if(si.flags & QTextItem::StrikeOut)
            pState->painter()->drawRect(QRectF(req_x, req_y - (ascent() / 3).toReal(), si.width.toReal(), lw));
        pState->painter()->setBrush(oldBrush);
        pState->painter()->setPen(oldPen);
    }
    if(p->type() == QPaintEngine::CoreGraphics && textAA != lineAA)
        CGContextSetShouldAntialias(QMacCGContext(pState->painter()), !textAA);
}

glyph_metrics_t
QFontEngineMac::boundingBox(const QGlyphLayout *glyphs, int numGlyphs)
{
    QFixed w;
    const QGlyphLayout *end = glyphs + numGlyphs;
    while(end > glyphs)
        w += (--end)->advance.x;
    return glyph_metrics_t(0, -(ascent()), w, ascent()+descent(), w, 0);
}

glyph_metrics_t
QFontEngineMac::boundingBox(glyph_t glyph)
{
    const QChar c(glyph);
    int w = doTextTask(&c, 0, 1, 1, WIDTH);
    return glyph_metrics_t(0, -(ascent()), w, ascent()+descent(), w, 0);
}

bool
QFontEngineMac::canRender(const QChar *string,  int len)
{
    return doTextTask(string, 0, len, len, EXISTS);
}

void
QFontEngineMac::calculateCost()
{
    // don't know how to get the number of glyphs from the font so default to 1024
    cache_cost = uint((ascent().toInt() + descent().toInt() + 1) * maxCharWidth() * 1024);
}

//Create a cacheable ATSUStyle
QATSUStyle *QFontEngineMac::getFontStyle() const
{
    if(internal_fi)
        return internal_fi;

    const int arr_guess = 7;
    int arr = 0;
    ATSUAttributeTag tags[arr_guess];
    ByteCount valueSizes[arr_guess];
    ATSUAttributeValuePtr values[arr_guess];
    tags[arr] = kATSUSizeTag; //font size
    Fixed fsize = FixRatio(fontDef.pixelSize, 1);
    valueSizes[arr] = sizeof(fsize);
    values[arr] = &fsize;
    arr++;
    tags[arr] = kATSUFontTag;  //font
    ATSUFontID fontID;
    FMFontFamily qdfamily = FMGetFontFamilyFromATSFontFamilyRef(familyref);
    ATSUFONDtoFontID(qdfamily, 0, &fontID);
    valueSizes[arr] = sizeof(fontID);
    values[arr] = &fontID;
    arr++;
    tags[arr] = kATSUQDBoldfaceTag;
    Boolean boldBool = ((fontDef.weight == QFont::Bold) ? true : false);
    valueSizes[arr] = sizeof(boldBool);
    values[arr] = &boldBool;
    arr++;
    if(arr > arr_guess) //this won't really happen, just so I will not miss the case
        qWarning("QFontEngine: Internal error: %d, WH0A %d: arr_guess overflow", __LINE__, arr);

    //create style
    QATSUStyle *ret = new QATSUStyle;
    ret->ascent = ret->descent = ret->leading = ret->maxWidth = -1;
    ATSUCreateStyle(&ret->style);
    if(OSStatus e = ATSUSetAttributes(ret->style, arr, tags, valueSizes, values)) {
        qWarning("QFontEngine: Internal error: %ld: Unexpected condition reached (%s:%d)", e, __FILE__, __LINE__);
        delete ret;
        ret = 0;
    } else {
        const ItemCount feat_guess=5;
        ItemCount feats=0;
        ATSUFontFeatureType feat_types[feat_guess];
        ATSUFontFeatureSelector feat_values[feat_guess];
        feat_types[feats] = kLigaturesType;
        feat_values[feats] = kRareLigaturesOffSelector;
        feats++;
        feat_types[feats] = kLigaturesType;
        feat_values[feats] = kCommonLigaturesOffSelector;
        feats++;
        feat_types[feats] = kLigaturesType;
        feat_values[feats] = kRequiredLigaturesOffSelector;
        feats++;
        if(feats > feat_guess) //this won't really happen, just so I will not miss the case
            qWarning("QFontEngine: Internal error: %d: WH0A feat_guess underflow %d", __LINE__, (int)feats);
#if 0
        for(ItemCount i = 0; i < feats; ++i) {
            ATSUFontFeatureSelector tmp_values[feat_guess];
            Boolean byDefault, isExclusive;
            ItemCount selectors;
            ATSUGetFontFeatureSelectors(fontID, feat_types[i], feat_guess,
                                        tmp_values, &byDefault, &selectors, &isExclusive);
            bool found = false;
            for(uint j = 0; j < selectors; ++j) {
                if(tmp_values[j] == feat_values[i]) {
                    found = true;
                    break;
                }
            }
            if(!found)
                qDebug("%d) %d/%d not found in [%s]", (int)i, feat_types[i], feat_values[i],
                       qPrintable(fontDef.family));
        }
#endif
        if(OSStatus e = ATSUSetFontFeatures(ret->style, feats, feat_types, feat_values))
            qWarning("QFontEngine: Internal error: %ld: Unexpected condition reached (%s:%d)", e, __FILE__, __LINE__);

        //set the default color
        const ATSUAttributeTag color_tag = kATSUColorTag;
        ::RGBColor fcolor;
        fcolor.red = ret->rgb.red()*256;
        fcolor.green = ret->rgb.green()*256;
        fcolor.blue = ret->rgb.blue()*256;
        ByteCount color_size = sizeof(fcolor);
        ATSUAttributeValuePtr color_value = &fcolor;
        if(OSStatus e = ATSUSetAttributes(ret->style, 1, &color_tag, &color_size, &color_value))
            qWarning("QFontEngine: Internal error: %ld: Unexpected condition reached (%s:%d)", e, __FILE__, __LINE__);
    }
    internal_fi = ret; //cache it
    const_cast<QFontEngineMac*>(this)->calculateCost(); //do this absolutely last!!
    return ret;
}

static inline int qt_mac_get_measurement(ATSUStyle style, ATSUAttributeTag tag, const QFontEngine *)
{
    ATSUTextMeasurement ret=0;
    OSStatus result = ATSUGetAttribute(style, tag, sizeof(ret), &ret, 0);
    if(result != noErr && result != kATSUNotSetErr)
        qWarning("QFontEngine: Internal error: %ld: This can't really happen! (%s:%d)", result, __FILE__, __LINE__);
    return FixRound(ret);
}

QFixed QFontEngineMac::ascent() const
{
    QATSUStyle *st = getFontStyle();
    if(st->ascent != -1)
        return st->ascent;
    return st->ascent = qt_mac_get_measurement(st->style, kATSUAscentTag, this);
}
QFixed QFontEngineMac::descent() const
{
    QATSUStyle *st = getFontStyle();
    if(st->descent != -1)
        return st->descent;
    return st->descent = qt_mac_get_measurement(st->style, kATSUDescentTag, this);
}

QFixed QFontEngineMac::leading() const
{
    QATSUStyle *st = getFontStyle();
    if(st->leading != -1)
        return st->leading;
    return st->leading = qt_mac_get_measurement(st->style, kATSULeadingTag, this);
}

qreal QFontEngineMac::maxCharWidth() const
{
    QATSUStyle *st = getFontStyle();
    if(st->maxWidth != -1)
        return st->maxWidth;
    {   // I hate doing this, but I don't see a better way just yet -
        // so I'll just take the width of the captial m 'M'
        QChar ch = 'M';
        st->maxWidth = doTextTask(&ch, 0, 1, 1, WIDTH);
    }
    return st->maxWidth;
}

static QFixed *advances_buffer = 0;
static int advances_buffer_len = 0;
static void cleanupAdvancesBuffer() {
    if(advances_buffer) {
        free(advances_buffer);
        advances_buffer = 0;
    }
    advances_buffer_len = 0;
}

struct QATSUGlyphInfo
{
    QATSUGlyphInfo()
        : glyphArray(0)
    {
        left = right = bottom = top = -1;
    }

    ~QATSUGlyphInfo() {
        delete [] glyphArray;
        glyphArray = 0;
    }
    ATSUGlyphInfoArray *glyphArray;
    ATSUTextMeasurement left,
                        right,
                        bottom,
                        top;
};


int QFontEngineMac::doTextTask(const QChar *s, int pos, int use_len, int len, uchar task,
                               QFixed x, QFixed y, QPaintEngine *p, void **data) const
{
    QATSUStyle *st = getFontStyle();
    QPaintEngineState *pState = 0;
    QPaintDevice *device = 0;
    QWidget *widget = 0;
    if(p) {
        pState = p->state;
        device = p->paintDevice();
        if(device->devType() == QInternal::Widget)
            widget = static_cast<QWidget *>(device);
    }

    if(!st) //can't really happen, but just to be sure..
        return 0;

    int ret = 0;
    if(task & DRAW) {
        Q_ASSERT(p); //really need a painter and engine to do any drawing!!!
        QColor rgb = pState->pen().color();

        if(rgb != st->rgb) {
            st->rgb = rgb;
            const ATSUAttributeTag color_tag = kATSURGBAlphaColorTag;
            ATSURGBAlphaColor fcolor;
            fcolor.red = float(rgb.redF());
            fcolor.green = float(rgb.greenF());
            fcolor.blue = float(rgb.blueF());
            fcolor.alpha = float(rgb.alphaF());
            ByteCount color_size = sizeof(fcolor);
            ATSUAttributeValuePtr color_value = &fcolor;
            if(OSStatus e = ATSUSetAttributes(st->style, 1, &color_tag, &color_size, &color_value)) {
                qWarning("QFontEngine: Internal error: %ld: This shouldn't happen! (%s:%d)", e, __FILE__, __LINE__);
                return 0;
            }
        }
    }

    { //transformations
        CGAffineTransform tf = CGAffineTransformIdentity;
        if((task & DRAW) && p && p->type() == QPaintEngine::CoreGraphics) { //flip text
#if 0
            int height = 0;
            if(widget)
                height = widget->window()->height();
            else
                height = device->metric(QPaintDevice::PdmHeight);
            tf = CGAffineTransformTranslate(tf, 0, height);
#endif
            tf = CGAffineTransformScale(tf, 1, -1);
        }

        if(fontDef.stretch != 100)
            tf = CGAffineTransformScale(tf, fontDef.stretch/100, 1);
        //we cannot do italic since ATSUI just skews the matrix
        if(fontDef.style & QFont::StyleItalic)
            tf.c = Fix2X(kATSItalicQDSkew);

        const ATSUAttributeTag tag = kATSUFontMatrixTag;
        ByteCount size = sizeof(tf);
        ATSUAttributeValuePtr value = &tf;
        if(OSStatus e = ATSUSetAttributes(st->style, 1, &tag, &size, &value)) {
            qWarning("QFontEngine: Internal error: %ld: This shouldn't happen! (%s:%d)", e, __FILE__, __LINE__);
            return 0;
        }
    }

    if(task & WIDTH) {
        bool use_cached_width = true;
        for(int i = 0; i < use_len; i++) {
            int c;
            if(s[i].unicode() >= widthCacheSize || !(c = widthCache[s[i].unicode()])) {
                use_cached_width = false;
                break;
            }
            if(c == -777) //special marker meaning 0
                c = 0;
            ret += c;
        }
        if(use_cached_width) {
            if(task == WIDTH)
                return ret;
        } else {
            ret = 0; //reset
        }
    }

    Q_UNUSED(len);

    OSStatus e;

    if (!mTextLayout) {
        e = ATSUCreateTextLayout(&mTextLayout);
        if (e != noErr) {
            qWarning("QFontEngine: Internal error: %ld: Unexpected condition reached %s: %d", e, __FILE__, __LINE__);
            return 0;
        }
    }
    const UniCharCount count = use_len;
    e = ATSUSetTextPointerLocation(mTextLayout, (UniChar *)(s) + pos, 0, count, use_len);
    if (e != noErr) {
        qWarning("QFontEngine: Internal error: %ld: Error ATSUSetTextPointerLocation %s: %d", e, __FILE__, __LINE__);
        return 0;
    }
    e = ATSUSetRunStyle(mTextLayout, st->style, 0, count);
    if (e != noErr) {
        qWarning("QQFontEngine: Internal error: %ld: Error ATSUSetRunStyle %s: %d", e, __FILE__, __LINE__);
        return 0;
    }
    const int arr_guess = 5;
    int arr = 0;
    ATSUAttributeTag tags[arr_guess];
    ByteCount valueSizes[arr_guess];
    ATSUAttributeValuePtr values[arr_guess];

    tags[arr] = kATSULineLayoutOptionsTag;
    ATSLineLayoutOptions layopts = kATSLineHasNoOpticalAlignment | kATSLineIgnoreFontLeading |
                                   kATSLineFractDisable | kATSLineDisableAutoAdjustDisplayPos |
                                   kATSLineUseDeviceMetrics;
    if(fontDef.styleStrategy & QFont::NoAntialias)
        layopts |= kATSLineNoAntiAliasing;
    if(!kerning)
        layopts |= kATSLineDisableAllKerningAdjustments;
//    layopts |=  kATSLineDisableAllLayoutOperations;
    valueSizes[arr] = sizeof(layopts);
    values[arr] = &layopts;
    arr++;

    tags[arr] = kATSUCGContextTag;
    QMacCGContext q_ctx;
    CGContextRef ctx = 0;
    CGrafPtr ctx_port = 0; //only set if the ctx is created from a port
    if(p && p->type() == QPaintEngine::CoreGraphics) {
        if(p && device) {
            q_ctx = QMacCGContext(p->painter());
        } else {
            static QPixmap *pixmap = 0;
            if(!pixmap)
                pixmap = new QPixmap(1, 1);
            q_ctx = QMacCGContext(pixmap);
        }
        ctx = static_cast<CGContextRef>(q_ctx);
    } else if(p && p->type() == QPaintEngine::QuickDraw) {
        QRegion rgn;
        QPoint pt;
        static_cast<QQuickDrawPaintEngine *>(p)->setupQDPort(false, &pt, &rgn);
        x += pt.x();
        y += pt.y();
        ATSUFontID fontID;
        FMFontFamily qdfamily = FMGetFontFamilyFromATSFontFamilyRef(familyref);
        ATSUFONDtoFontID(qdfamily, 0, &fontID);
        TextFont(fontID);

        GetGWorld(&ctx_port, 0);
        if(OSStatus err = QDBeginCGContext(ctx_port, &ctx)) {
            qWarning("QFontEngine: Internal error: WH0A, QDBeginCGContext(%ld) failed (%s:%d)", err, __FILE__, __LINE__);
            return 0;
        }
        if(task & DRAW) {
            Rect clipr;
            GetPortBounds(ctx_port, &clipr);
#if 1
            CGContextBeginPath(ctx);
            if(rgn.isEmpty()) {
                CGContextAddRect(ctx, CGRectMake(0, 0, 0, 0));
            } else {
                QVector<QRect> rects = rgn.rects();
                const int count = rects.size();
                for(int i = 0; i < count; i++) {
                    const QRect &r = rects[i];
                    CGContextAddRect(ctx, CGRectMake(r.x(), (clipr.bottom - clipr.top) - r.y() - r.height(),
                                                     r.width(), r.height()));
                }
            }
            CGContextClip(ctx);
#else
            ClipCGContextToRegion(ctx, &clipr, rgn.handle(true));
#endif
        }
    } else {
        Q_ASSERT(!(task & DRAW));
        static QPixmap *pixmap = 0;
        if(!pixmap)
            pixmap = new QPixmap(1, 1);
        QMacSavedPortInfo::setPaintDevice(pixmap);
        q_ctx = QMacCGContext(pixmap);
        ctx = static_cast<CGContextRef>(q_ctx);
    }
    valueSizes[arr] = sizeof(ctx);
    values[arr] = &ctx;
    arr++;

    if(arr > arr_guess) //this won't really happen, just so I will not miss the case
        qWarning("QFontEngine: Internal error: %d: WH0A, arr_guess underflow %d", __LINE__, arr);
    if(OSStatus e = ATSUSetLayoutControls(mTextLayout, arr, tags, valueSizes, values)) {
        qWarning("QFontEngine: Internal error: %ld: Unexpected condition reached (%s:%d)", e, __FILE__, __LINE__);
        if(ctx_port)
            QDEndCGContext(ctx_port, &ctx);
        return 0;
    }
    ATSUSetTransientFontMatching(mTextLayout, true);
    //do required task now
    if(task & EXISTS) {
        if(task != EXISTS)
            qWarning("QFontEngine: Internal error: EXISTS must appear by itself!");
        ATSUFontID fid;
        UniCharArrayOffset off;
        UniCharCount off_len;
        if (ATSUMatchFontsToText(mTextLayout, kATSUFromTextBeginning, kATSUToTextEnd, &fid,
                                 &off, &off_len) != kATSUFontsNotMatched)
            ret = 1;
    } else if((task & WIDTH) && !ret) {
        ATSUTextMeasurement left=0, right=0, bottom=0, top=0;
        ATSUGetUnjustifiedBounds(mTextLayout, kATSUFromTextBeginning, kATSUToTextEnd,
                                 &left, &right, &bottom, &top);
        ret = FixRound(right-left);
        if(use_len == 1 && s->unicode() < widthCacheSize && ret < 0x100)
            widthCache[s->unicode()] = ret ? ret : -777; //mark so that 0 is cached..
    }
#if 1
    if(task & ADVANCES) {
        Q_ASSERT(data);
        OSStatus err = noErr;

        QString checkStr = QString::fromRawData(s + pos, use_len);
        QATSUGlyphInfo *glyphInfo = glyphCache.object(checkStr);
        if (!glyphInfo) {
            glyphInfo = new QATSUGlyphInfo();
            QString copy(checkStr.unicode(), checkStr.length());
            glyphCache.insert(copy, glyphInfo);
        }

        if (!glyphInfo->glyphArray) {
            glyphInfo->glyphArray = new ATSUGlyphInfoArray[use_len];
            ByteCount size = sizeof(ATSUGlyphInfoArray) * use_len;
            err = ATSUGetGlyphInfo(mTextLayout, kATSUFromTextBeginning, kATSUToTextEnd,
                                   &size, glyphInfo->glyphArray);
        }
        if(err == noErr) {
            //sanity
            Q_ASSERT(glyphInfo->glyphArray[0].numGlyphs == (uint)use_len);
            if(!advances_buffer || use_len > advances_buffer_len) {
                advances_buffer_len = use_len;
                if(!advances_buffer) {
                    qAddPostRoutine(cleanupAdvancesBuffer);
                    advances_buffer = (QFixed*)malloc(sizeof(QFixed)*use_len);
                } else {
                    advances_buffer = (QFixed*)realloc(advances_buffer, sizeof(QFixed)*use_len);
                }
            }

            //calculate the positions
            int last = 0;
            for(int i = 0; i < use_len-1; ++i) {
                advances_buffer[i] = glyphInfo->glyphArray[0].glyphs[i+1].screenX - last;
#ifdef DEBUG_ADVANCES
                qDebug("%d [%d]) %d(%d)::%f::%d::%d -- %f", i, (s+pos+i)->latin1(), glyphInfo->glyphArray[0].glyphs[i+1].screenX, last,
                       glyphInfo->glyphArray[0].glyphs[i+1].idealX, (int)glyphInfo->glyphArray[0].glyphs[i+1].charIndex,
                       (int)glyphInfo->glyphArray[0].glyphs[i+1].glyphInfo, advances_buffer[i]);
#endif
                last = glyphInfo->glyphArray[0].glyphs[i+1].screenX;
            }

            //calculate the final width
            if (glyphInfo->left == -1) {
                if(use_len > 1 && !last)
                    ATSUGetUnjustifiedBounds(mTextLayout, use_len-2, 2,
                                             &glyphInfo->left, &glyphInfo->right,
                                             &glyphInfo->bottom,
                                             &glyphInfo->top);
                else
                    ATSUGetUnjustifiedBounds(mTextLayout, use_len-1, 1, &glyphInfo->left,
                                             &glyphInfo->right, &glyphInfo->bottom,
                                             &glyphInfo->top);
#ifdef DEBUG_ADVANCES
                qDebug("Last %d) %f-%f == %f", use_len-1, FixedToQFixed(glyphInfo->right), FixedToQFixed(glyphInfo->left), FixedToQFixed(glyphInfo->right-glyphInfo->left));
#endif
            }
            advances_buffer[use_len-1] = FixedToQFixed(glyphInfo->right-glyphInfo->left);

            //finally make sure surrogates are in Qt order..
            for(int i = 0; i < use_len-1; ++i) {
                if(!advances_buffer[i]) {
                    advances_buffer[i] = advances_buffer[i+1];
                    advances_buffer[++i] = 0;
                }
            }
            (*data) = advances_buffer;
        }
    }
#endif
    if(task & DRAW) {
        bool transform = false;
        CGAffineTransform oldMatrix = CGContextGetCTM(ctx), newMatrix;
#if 0
        if(pState &&
           (0 /*|| p->type() == QPaintEngine::CoreGraphics */
            /*|| p->type() == QPaintEngine::QuickDraw*/)) {
            newMatrix = CGAffineTransformConcat(CGAffineTransformMake(pState->matrix.m11(), pState->matrix.m12(),
                                                                      pState->matrix.m21(), pState->matrix.m22(),
                                                                      pState->matrix.dx(),  pState->matrix.dy()),
                                                oldMatrix);
            transform = true;
        } else
#endif
        {
            newMatrix = oldMatrix;
        }
        if(ctx_port) {
            int height = 0;
            if(widget)
                height = widget->window()->height();
            else
                height = device->metric(QPaintDevice::PdmHeight);
#if 0
            newMatrix = CGAffineTransformTranslate(newMatrix, 0, height);
            transform = true;
#else
            y = height - y;
#endif
        }
        while (qAbs(x) > SHRT_MAX || qAbs(y) > SHRT_MAX) { //bound to 16bit
            const float tx = newMatrix.tx, ty = newMatrix.ty;
            newMatrix = CGAffineTransformTranslate(newMatrix, -tx, ty);
            x += QFixed::fromReal(tx);
            y -= QFixed::fromReal(ty);
            transform = true;
        }
        if(transform) {
            CGContextConcatCTM(ctx, CGAffineTransformInvert(oldMatrix));
            CGContextConcatCTM(ctx, newMatrix);
            CGContextSetTextMatrix(ctx, newMatrix);
            ATSUDrawText(mTextLayout, kATSUFromTextBeginning, kATSUToTextEnd,
                         QFixedToFixed(x), QFixedToFixed(y));
            CGContextConcatCTM(ctx, CGAffineTransformInvert(CGContextGetCTM(ctx)));
            CGContextConcatCTM(ctx, oldMatrix);
            CGContextSetTextMatrix(ctx, oldMatrix);
        } else {
            ATSUDrawText(mTextLayout, kATSUFromTextBeginning, kATSUToTextEnd,
                         QFixedToFixed(x), QFixedToFixed(y));
        }
    }
    if(ctx_port)
        QDEndCGContext(ctx_port, &ctx);
    return ret;
}
#undef DoubleToFixed

void QFontEngineMac::addOutlineToPath(qreal x, qreal y, const QGlyphLayout *glyphs, int numGlyphs,
                                      QPainterPath *path, QTextItem::RenderFlags)
{

    OSStatus e;
    if (!mTextLayout) {
        e = ATSUCreateTextLayout(&mTextLayout);
        if (e != noErr) {
            qWarning("QFontEngine: Internal error: %ld: Unexpected condition reached (%s:%d)", e, __FILE__, __LINE__);
            return;
        }
    }

    QVarLengthArray<UniChar> chars(numGlyphs);
    for (int j = 0; j < numGlyphs; ++j)
        chars[j] = glyphs[j].glyph;
    e = ATSUSetTextPointerLocation(mTextLayout, chars.data(), 0, (UniCharCount)numGlyphs, numGlyphs);
    if (e != noErr) {
        qWarning("QFontEngine: Internal error: %ld: Error ATSUSetTextPointerLocation (%s:%d)", e, __FILE__, __LINE__);
        return;
    }

    QATSUStyle *st = getFontStyle();
    e = ATSUSetRunStyle(mTextLayout, st->style, 0, (UniCharCount)numGlyphs);
    if (e != noErr) {
        qWarning("QFontEngine: Internal error: %ld: Error ATSUSetRunStyle (%s:%d)", e, __FILE__, __LINE__);
        return;
    }

    const int arr_guess = 5;
    int arr = 0;
    ATSUAttributeTag tags[arr_guess];
    ByteCount valueSizes[arr_guess];
    ATSUAttributeValuePtr values[arr_guess];

    tags[arr] = kATSULineLayoutOptionsTag;
    ATSLineLayoutOptions layopts = kATSLineHasNoOpticalAlignment | kATSLineIgnoreFontLeading
                                   | kATSLineFractDisable;
    //layopts |= kATSLineDisableAutoAdjustDisplayPos | kATSLineDisableAllLayoutOperations | kATSLineUseDeviceMetrics;
    if(fontDef.styleStrategy & QFont::NoAntialias)
        layopts |= kATSLineNoAntiAliasing;
    valueSizes[arr] = sizeof(layopts);
    values[arr] = &layopts;
    arr++;

    QMacCGContext q_ctx;
    if(QSysInfo::MacintoshVersion <= QSysInfo::MV_10_2) {
        tags[arr] = kATSUCGContextTag;
        static QPixmap *pixmap = 0;
        if(!pixmap)
            pixmap = new QPixmap(1, 1);
        q_ctx = QMacCGContext(pixmap);
        CGContextRef ctx = static_cast<CGContextRef>(q_ctx);
        valueSizes[arr] = sizeof(ctx);
        values[arr] = &ctx;
        arr++;
    }

    if(arr > arr_guess) //this won't really happen, just so I will not miss the case
        qWarning("QFontEngine: Internal error: %d: WH0A, arr_guess underflow %d", __LINE__, arr);
    if(OSStatus e = ATSUSetLayoutControls(mTextLayout, arr, tags, valueSizes, values)) {
        qWarning("QFontEngine: Internal error: %ld: Unexpected condition reached (%s:%d)", e, __FILE__, __LINE__);
        return;
    }

    ItemCount numRecords;
    ATSLayoutRecord *layoutRecords;
    e = ATSUDirectGetLayoutDataArrayPtrFromTextLayout(mTextLayout,
                            kATSUFromTextBeginning,
                            kATSUDirectDataLayoutRecordATSLayoutRecordCurrent,
                            reinterpret_cast<void **>(&layoutRecords),
                            &numRecords);
    if (e != noErr) {
        qWarning("QFontEngine: Internal error: %ld: Error ATSUDirectGetLayoutDataArrayPtrFromTextLayout (%s:%d)",
                 e, __FILE__, __LINE__);
        return;
    }

    QMacFontPath fontpath(x, y, path);
    ATSCubicMoveToUPP moveTo = NewATSCubicMoveToUPP(QMacFontPath::moveTo);
    ATSCubicLineToUPP lineTo = NewATSCubicLineToUPP(QMacFontPath::lineTo);
    ATSCubicCurveToUPP cubicTo = NewATSCubicCurveToUPP(QMacFontPath::cubicTo);
    ATSCubicClosePathUPP closePath = NewATSCubicClosePathUPP(QMacFontPath::closePath);
    ATSGlyphIdealMetrics idealMetrics;
    for (ItemCount i = 0; i < numRecords; ++i) {
        ATSUGlyphGetCubicPaths(st->style, layoutRecords[i].glyphID, moveTo, lineTo,
                               cubicTo, closePath, &fontpath, &e);
        ATSUGlyphGetIdealMetrics(st->style, 1, &layoutRecords[i].glyphID, 0, &idealMetrics);
        fontpath.advance(idealMetrics.advance.x);
    }
    DisposeATSCubicMoveToUPP(moveTo);
    DisposeATSCubicLineToUPP(lineTo);
    DisposeATSCubicCurveToUPP(cubicTo);
    DisposeATSCubicClosePathUPP(closePath);

    ATSUDirectReleaseLayoutDataArrayPtr(0, kATSUDirectDataLayoutRecordATSLayoutRecordCurrent,
                                        reinterpret_cast<void **>(&layoutRecords));
}

#endif

