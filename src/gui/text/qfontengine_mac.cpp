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

#include "qscriptengine_p.h"

QFontEngineMac::QFontEngineMac(const ATSFontFamilyRef &atsFamily, const QFontDef &fontDef, bool kerning)
{
    this->fontDef = fontDef;
    this->kerning = kerning;
    familyref = atsFamily;
    synthesisFlags = 0;
    fonts.resize(1);

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
    FMFont fnt;
    status = FMGetFontFromFontFamilyInstance(family, fntStyle, &fnt, &intrinsicStyle);
    fonts[0] = FontInfo(fnt);
    Q_ASSERT(status == noErr);

    status = FMGetFontFamilyInstanceFromFont(fonts.at(0).fmFont, &family, &intrinsicStyle);
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
    values[attributeCount] = &fonts[0].fmFont;
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

QFontEngineMac::FontInfo::FontInfo(FMFont fnt)
{
    fmFont = fnt;
    ATSFontRef atsFont = FMGetATSFontRefFromFont(fmFont);
    cgFont = CGFontCreateWithPlatformFont(&atsFont);
}

struct QGlyphLayoutInfo
{
    QGlyphLayout *glyphs;
    int *numGlyphs;
    bool callbackCalled;
    int *mappedFonts;
    QTextEngine::ShaperFlags flags;
    QShaperItem *shaperItem;
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

    Fixed *baselineDeltas = 0;

    e = ATSUDirectGetLayoutDataArrayPtrFromLineRef(lineRef, kATSUDirectDataBaselineDeltaFixedArray,
                                                   /*iCreate =*/ true,
                                                   (void **) &baselineDeltas,
                                                   &itemCount);
    if (e != noErr)
        return e;

    int nextCharStop = -1;
    int currentClusterGlyph = -1; // first glyph in log cluster
    QShaperItem *item = 0;
    if (nfo->shaperItem) {
        item = nfo->shaperItem;
#if !defined(QT_NO_DEBUG)
        int surrogates = 0;
        const QString &str = *item->string;
        for (int i = item->from; i < item->from + item->length - 1; ++i) {
            surrogates += (str[i].unicode() >= 0xd800 && str[i].unicode() < 0xdc00
                           && str[i+1].unicode() >= 0xdc00 && str[i+1].unicode() < 0xe000);
        }
        Q_ASSERT(*nfo->numGlyphs == item->length - surrogates);
#endif
        for (nextCharStop = item->from; nextCharStop < item->from + item->length; ++nextCharStop)
            if (item->charAttributes[nextCharStop].charStop)
                break;
        nextCharStop -= item->from;
    }

    int glyphIdx = 0;
    int glyphIncrement = 1;
    if (nfo->flags & QTextEngine::RightToLeft) {
        glyphIdx  = itemCount - 2;
        glyphIncrement = -1;
    }
    for (int i = 0; i < *nfo->numGlyphs; ++i, glyphIdx += glyphIncrement) {

        int charOffset = layoutData[glyphIdx].originalOffset / sizeof(UniChar);
        const int fontIdx = nfo->mappedFonts[charOffset];

        ATSGlyphRef glyphId = layoutData[glyphIdx].glyphID;

        QFixed yAdvance = FixedToQFixed(baselineDeltas[glyphIdx]);
        QFixed xAdvance = FixedToQFixed(layoutData[glyphIdx + 1].realPos - layoutData[glyphIdx].realPos);

        if (glyphId != 0xffff || i == 0) {
            nfo->glyphs[i].glyph = (glyphId & 0x00ffffff) | (fontIdx << 24);

            nfo->glyphs[i].advance.y = yAdvance;
            nfo->glyphs[i].advance.x = xAdvance;
        } else {
            // ATSUI gives us 0xffff as glyph id at the index in the glyph array for
            // a character position that maps to a ligtature. Such a glyph id does not
            // result in any visual glyph, but it may have an advance, which is why we
            // sum up the glyph advances.
            --i;
            nfo->glyphs[i].advance.y += yAdvance;
            nfo->glyphs[i].advance.x += xAdvance;
            *nfo->numGlyphs -= 1;
        }

        if (item) {
            if (charOffset >= nextCharStop) {
                nfo->glyphs[i].attributes.clusterStart = true;
                currentClusterGlyph = i;

                ++nextCharStop;
                for (; nextCharStop < item->length; ++nextCharStop)
                    if (item->charAttributes[item->from + nextCharStop].charStop)
                        break;
            } else {
                if (currentClusterGlyph == -1)
                    currentClusterGlyph = i;
            }
            item->log_clusters[charOffset] = currentClusterGlyph;

            // surrogate handling
            if (charOffset < item->length - 1) {
                QChar current = item->string->at(item->from + charOffset);
                QChar next = item->string->at(item->from + charOffset + 1);
                if (current.unicode() >= 0xd800 && current.unicode() < 0xdc00
                    && next.unicode() >= 0xdc00 && next.unicode() < 0xe000) {
                    item->log_clusters[charOffset + 1] = currentClusterGlyph;
                }
            }
        }
    }

    /*
    if (item) {
        qDebug() << "resulting logclusters:";
        for (int i = 0; i < item->length; ++i)
            qDebug() << "logClusters[" << i << "] =" << item->log_clusters[i];
        qDebug() << "clusterstarts:";
        for (int i = 0; i < *nfo->numGlyphs; ++i)
            qDebug() << "clusterStart[" << i << "] =" << nfo->glyphs[i].attributes.clusterStart;
    }
    */

    ATSUDirectReleaseLayoutDataArrayPtr(lineRef, kATSUDirectDataBaselineDeltaFixedArray,
                                        (void **) &baselineDeltas);

    ATSUDirectReleaseLayoutDataArrayPtr(lineRef, kATSUDirectDataLayoutRecordATSLayoutRecordCurrent,
                                        (void **) &layoutData);

    *callbackStatus = kATSULayoutOperationCallbackStatusHandled;
    return noErr;
}

int QFontEngineMac::fontIndexForFMFont(FMFont font) const
{
    for (int i = 0; i < fonts.count(); ++i)
        if (fonts.at(i).fmFont == font)
            return i;
    fonts.append(FontInfo(font));
    return fonts.count() - 1;
}

bool QFontEngineMac::stringToCMap(const QChar *str, int len, QGlyphLayout *glyphs, int *nglyphs, QTextEngine::ShaperFlags flags) const
{
    return stringToCMap(str, len, glyphs, nglyphs, flags, /*shaperItem=*/0);
}

bool QFontEngineMac::stringToCMap(const QChar *str, int len, QGlyphLayout *glyphs, int *nglyphs, QTextEngine::ShaperFlags flags,
                                  QShaperItem *shaperItem) const
{
    if (*nglyphs < len) {
        *nglyphs = len;
        return false;
    }
    //qDebug() << "stringToCMap" << QString(str, len);

    OSStatus e = noErr;

    e = ATSUSetTextPointerLocation(textLayout, (UniChar *)(str), 0, len, len);
    if (e != noErr) {
        qWarning("Qt: internal: %ld: Error ATSUSetTextPointerLocation %s: %d", e, __FILE__, __LINE__);
        return false;
    }

    QGlyphLayoutInfo nfo;
    nfo.glyphs = glyphs;
    nfo.numGlyphs = nglyphs;
    nfo.callbackCalled = false;
    nfo.flags = flags;
    nfo.shaperItem = shaperItem;

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
        const int maxAttributeCount = 3;
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

        Boolean direction;
        if (flags & QTextEngine::RightToLeft)
            direction = kATSURightToLeftBaseDirection;
        else
            direction = kATSULeftToRightBaseDirection;
        tags[attributeCount] = kATSULineDirectionTag;
        values[attributeCount] = &direction;
        sizes[attributeCount] = sizeof(direction);
        ++attributeCount;

        Q_ASSERT(attributeCount < maxAttributeCount + 1);
        e = ATSUSetLayoutControls(textLayout, attributeCount, tags, sizes, values);
        if (e != noErr) {
            qWarning("Qt: internal: %ld: Error ATSUSetLayoutControls %s: %d", e, __FILE__, __LINE__);
            return false;
        }

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
    ATSFontRef atsFont = FMGetATSFontRefFromFont(fonts.at(0).fmFont);
    ATSFontMetrics metrics;
    ATSFontGetHorizontalMetrics(atsFont, kATSOptionFlagsDefault, &metrics);
    return metrics.maxAdvanceWidth * fontDef.pointSize;
}

QFixed QFontEngineMac::xHeight() const
{
    ATSFontRef atsFont = FMGetATSFontRefFromFont(fonts.at(0).fmFont);
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

    QVarLengthArray<QFixedPoint> positions;
    QVarLengthArray<glyph_t> glyphs;
    QMatrix matrix;
    matrix.translate(x, y);
    ti.fontEngine->getGlyphPositions(ti.glyphs, ti.num_glyphs, matrix, ti.flags, glyphs, positions);

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

	QCFType<CGFontRef> cgFont = fonts.at(currentFont).cgFont;
        CGContextSetFont(ctx, cgFont);

        CGContextSetTextPosition(ctx, positions[currentSpan].x.toReal(), positions[currentSpan].y.toReal());

        CGContextShowGlyphsWithAdvances(ctx, cgGlyphs.data() + currentSpan,
                                        advances.data() + currentSpan, i - currentSpan);

        if (synthesisFlags & QFontEngine::SynthesizedBold) {
            CGContextSetTextPosition(ctx, positions[currentSpan].x.toReal() + 0.5 * lineThickness().toReal(),
                                          positions[currentSpan].y.toReal());

            CGContextShowGlyphsWithAdvances(ctx, cgGlyphs.data() + currentSpan,
                                           advances.data() + currentSpan, i - currentSpan);
        }

        currentFont = nextFont;
        currentSpan = i;
    }

    QCFType<CGFontRef> cgFont = fonts.at(currentFont).cgFont;
    CGContextSetFont(ctx, cgFont);

    CGContextSetTextPosition(ctx, positions[currentSpan].x.toReal(), positions[currentSpan].y.toReal());

    CGContextShowGlyphsWithAdvances(ctx, cgGlyphs.data() + currentSpan,
                                    advances.data() + currentSpan, glyphs.size() - currentSpan);

    if (synthesisFlags & QFontEngine::SynthesizedBold) {
        CGContextSetTextPosition(ctx, positions[currentSpan].x.toReal() + 0.5 * lineThickness().toReal(),
                                      positions[currentSpan].y.toReal());

        CGContextShowGlyphsWithAdvances(ctx, cgGlyphs.data() + currentSpan,
                                        advances.data() + currentSpan, glyphs.size() - currentSpan);
    }

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
    FMFont font = fonts.at(fontIndex).fmFont;

    ATSUCreateAndCopyStyle(style, &result);

    ATSUAttributeTag tag = kATSUFontTag;
    ATSUAttributeValuePtr value = &font;
    ByteCount dummySize = sizeof(font);

    ATSUSetAttributes(result, 1, &tag, &dummySize, &value);

    return result;
}

