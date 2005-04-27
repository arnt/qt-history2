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

#include <ApplicationServices/ApplicationServices.h>

//Generic engine
QFontEngine::~QFontEngine()
{
}

qreal QFontEngine::lineThickness() const
{
  // ad hoc algorithm
  int score = fontDef.pixelSize * fontDef.weight;
  int lth = score / 700;

  // looks better with thicker line for small pointsizes
  if(lth < 2 && score >= 1050)
      lth = 2;
  else if(lth == 0)
      lth = 1;
  return lth;
}

qreal QFontEngine::underlinePosition() const
{
  return ((lineThickness() * 2) + 3) / 6;
}

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
}

QFontEngineMac::~QFontEngineMac()
{
    if (mTextLayout)
        ATSUDisposeTextLayout(mTextLayout);
    delete internal_fi;
}

bool QFontEngineMac::stringToCMap(const QChar *str, int len, QGlyphLayout *glyphs, int *nglyphs,
                                  QTextEngine::ShaperFlags /*flags*/) const
{
    if(*nglyphs < len) {
        *nglyphs = len;
        return false;
    }
    *nglyphs = len;
    for (int i = 0; i < len; ++i) {
        bool surrogate = (str[i].unicode() >= 0xd800 && str[i].unicode() < 0xdc00 && i < len-1
                          && str[i+1].unicode() >= 0xdc00 && str[i+1].unicode() < 0xe000);
        glyphs[i].glyph = str[i].unicode();
        if (!surrogate && str[i].unicode() < widthCacheSize && widthCache[str[i].unicode()]) {
            int c = widthCache[str[i].unicode()];
            if(c == -777)
                c = 0;
            glyphs[i].advance.rx() = c;
            glyphs[i].advance.ry() = 0;
        } else {
            glyphs[i].advance.rx() = doTextTask(str+i, 0, surrogate ? 2 : 1, 1, WIDTH);
            glyphs[i].advance.ry() = 0;
        }
        if (surrogate) {
            ++i;
            glyphs[i].glyph = str[i].unicode();
        }
    }
    return true;
}

void
QFontEngineMac::draw(QPaintEngine *p, int req_x, int req_y, const QTextItemInt &si)
{
    if(p->type() == QPaintEngine::MacPrinter)
        p = static_cast<QMacPrintEngine*>(p)->paintEngine();
    QPaintEngineState *pState = p->state;
    int x = req_x, y = req_y;

#if 1
    if(p->type() == QPaintEngine::QuickDraw && !pState->matrix().isIdentity()) {
        float aw = si.width, ah = si.ascent + si.descent + 1;
        if(aw == 0 || ah == 0)
            return;
        QBitmap bm(qRound(aw), qRound(ah));        // create bitmap
        {
            QPainter paint(&bm);  // draw text in bitmap
            paint.setPen(Qt::color1);
            paint.drawTextItem(QPoint(0, qRound(si.ascent)), si);
            paint.end();
        }

        QPixmap pm(bm.size());
        if(pState->backgroundMode() == Qt::OpaqueMode) {
            pm = bm;
            bm.fill(Qt::color1);
            pm.setMask(bm);
        } else {
            QPainter paint(&pm);
            paint.fillRect(0, 0, pm.width(), pm.height(), pState->pen().color());
            paint.end();
            pm.setMask(bm);
        }
        pState->painter()->drawPixmap(x, y - qRound(si.ascent), pm);
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
        pState->painter()->fillRect(x + int(br.x), y + int(br.y), int(br.width), int(br.height),
                                  pState->backgroundBrush().color());
    }

    bool textAA = pState->renderHints() & QPainter::TextAntialiasing;
    bool lineAA = pState->renderHints() & QPainter::Antialiasing;
    if(p->type() == QPaintEngine::CoreGraphics && textAA != lineAA)
        CGContextSetShouldAntialias(QMacCGContext(p->painter()), textAA);

    if(si.flags & QTextItem::RightToLeft) {
        for(int i = si.num_glyphs-1; i >= 0; --i) {
            doTextTask((QChar*)(si.glyphs+i), 0, 1, 1, DRAW, x, y, p);
            x += int(si.glyphs[i].advance.x());
        }
    } else {
        QVarLengthArray<ushort> g(si.num_glyphs);
        for(int i = 0; i < si.num_glyphs; ++i)
            g[i] = si.glyphs[i].glyph;
        doTextTask((QChar*)g.data(), 0, si.num_glyphs, si.num_glyphs, DRAW, x, y, p) + 1;
    }
    if(si.width && si.flags != 0) {
        QPen oldPen = pState->pen();
        QBrush oldBrush = pState->brush();
        pState->painter()->setBrush(pState->pen().color());
        pState->painter()->setPen(Qt::NoPen);
        const float lw = lineThickness();
        if(si.flags & QTextItem::Underline)
            pState->painter()->drawRect(QRectF(req_x, req_y + underlinePosition(), si.width, lw));
        if(si.flags & QTextItem::Overline)
            pState->painter()->drawRect(QRectF(req_x, req_y - (ascent() + 1), si.width, lw));
        if(si.flags & QTextItem::StrikeOut)
            pState->painter()->drawRect(QRectF(req_x, req_y - (ascent() / 3), si.width, lw));
        pState->painter()->setBrush(oldBrush);
        pState->painter()->setPen(oldPen);
    }
    if(p->type() == QPaintEngine::CoreGraphics && textAA != lineAA)
        CGContextSetShouldAntialias(QMacCGContext(pState->painter()), !textAA);
}

glyph_metrics_t
QFontEngineMac::boundingBox(const QGlyphLayout *glyphs, int numGlyphs)
{
    float w = 0;
    const QGlyphLayout *end = glyphs + numGlyphs;
    while(end > glyphs)
        w += (--end)->advance.x();
    return glyph_metrics_t(0, -(ascent()), w, ascent()+descent(), w, 0);
}

glyph_metrics_t
QFontEngineMac::boundingBox(glyph_t glyph)
{
    int w = doTextTask((QChar*)&glyph, 0, 1, 1, WIDTH);
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
    cache_cost = uint((ascent() + descent() + 1) * maxCharWidth() * 1024);
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
    valueSizes[arr] = sizeof(Boolean);
    Boolean boldBool = ((fontDef.weight == QFont::Bold) ? true : false);
    values[arr] = &boldBool;
    arr++;
    if(arr > arr_guess) //this won't really happen, just so I will not miss the case
        qWarning("Qt: internal: %d, WH0A %d: arr_guess overflow", __LINE__, arr);

    //create style
    QATSUStyle *ret = new QATSUStyle;
    ret->ascent = ret->descent = ret->leading = ret->maxWidth = -1;
    ATSUCreateStyle(&ret->style);
    if(OSStatus e = ATSUSetAttributes(ret->style, arr, tags, valueSizes, values)) {
        qWarning("Qt: internal: %ld: unexpected condition reached %s:%d", e, __FILE__, __LINE__);
        delete ret;
        ret = 0;
    } else {
        const int feat_guess=5;
        int feats=0;
        ATSUFontFeatureType feat_types[feat_guess];
        ATSUFontFeatureSelector feat_values[feat_guess];
        feat_types[feats] = kLigaturesType;
        feat_values[feats] = kRareLigaturesOffSelector;
        feats++;
        feat_types[feats] = kLigaturesType;
        feat_values[feats] = kCommonLigaturesOffSelector;
        feats++;
        if(feats > feat_guess) //this won't really happen, just so I will not miss the case
            qWarning("Qt: internal: %d: WH0A feat_guess underflow %d", __LINE__, feats);
        if(OSStatus e = ATSUSetFontFeatures(ret->style, feats, feat_types, feat_values))
            qWarning("Qt: internal: %ld: unexpected condition reached %s:%d", e, __FILE__, __LINE__);

        //set the default color
        const ATSUAttributeTag color_tag = kATSUColorTag;
        ::RGBColor fcolor;
        fcolor.red = ret->rgb.red()*256;
        fcolor.green = ret->rgb.green()*256;
        fcolor.blue = ret->rgb.blue()*256;
        ByteCount color_size = sizeof(fcolor);
        ATSUAttributeValuePtr color_value = &fcolor;
        if(OSStatus e = ATSUSetAttributes(ret->style, 1, &color_tag, &color_size, &color_value))
            qWarning("Qt: internal: %ld: unexpected condition reached %s:%d", e, __FILE__, __LINE__);
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
        qWarning("%s:%d -- This can't really happen!! %ld", __FILE__, __LINE__, result);
    return FixRound(ret);
}

qreal QFontEngineMac::ascent() const
{
    QATSUStyle *st = getFontStyle();
    if(st->ascent != -1)
        return st->ascent;
    return st->ascent = qt_mac_get_measurement(st->style, kATSUAscentTag, this);
}
qreal QFontEngineMac::descent() const
{
    QATSUStyle *st = getFontStyle();
    if(st->descent != -1)
        return st->descent;
    return st->descent = qt_mac_get_measurement(st->style, kATSUDescentTag, this);
}

qreal QFontEngineMac::leading() const
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

int QFontEngineMac::doTextTask(const QChar *s, int pos, int use_len, int len, uchar task,
                               int x, int y, QPaintEngine *p) const
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
            const ATSUAttributeTag color_tag = kATSUColorTag;
            ::RGBColor fcolor;
            fcolor.red = rgb.red()*256;
            fcolor.green = rgb.green()*256;
            fcolor.blue = rgb.blue()*256;
            ByteCount color_size = sizeof(fcolor);
            ATSUAttributeValuePtr color_value = &fcolor;
            if(OSStatus e = ATSUSetAttributes(st->style, 1, &color_tag, &color_size, &color_value)) {
                qWarning("Qt: internal: %ld: This shouldn't happen %s:%d", e, __FILE__, __LINE__);
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
            qWarning("Qt: internal: %ld: This shouldn't happen %s:%d", e, __FILE__, __LINE__);
            return 0;
        }
    }

    if((task & WIDTH)) {
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
            qWarning("Qt: internal: %ld: Unexpected condition reached %s: %d", e, __FILE__, __LINE__);
            return 0;
        }
    }
    const UniCharCount count = use_len;
    e = ATSUSetTextPointerLocation(mTextLayout, (UniChar *)(s) + pos, 0, count, use_len);
    if (e != noErr) {
        qWarning("Qt: internal: %ld: Error ATSUSetTextPointerLocation %s: %d", e, __FILE__, __LINE__);
        return 0;
    }
    e = ATSUSetRunStyle(mTextLayout, st->style, 0, count);
    if (e != noErr) {
        qWarning("Qt: internal: %ld: Error ATSUSetRunStyle %s: %d", e, __FILE__, __LINE__);
        return 0;
    }
    const int arr_guess = 5;
    int arr = 0;
    ATSUAttributeTag tags[arr_guess];
    ByteCount valueSizes[arr_guess];
    ATSUAttributeValuePtr values[arr_guess];

    tags[arr] = kATSULineLayoutOptionsTag;
    ATSLineLayoutOptions layopts = kATSLineHasNoOpticalAlignment | kATSLineIgnoreFontLeading
                                   | kATSLineFractDisable;
    if(task & WIDTH)
        layopts |= kATSLineDisableAutoAdjustDisplayPos | kATSLineDisableAllLayoutOperations |
                   kATSLineUseDeviceMetrics;
    if(fontDef.styleStrategy & QFont::NoAntialias)
        layopts |= kATSLineNoAntiAliasing;
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
            qWarning("Qt: internal: WH0A, QDBeginCGContext(%ld) failed. %s:%d", err, __FILE__, __LINE__);
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
        qWarning("Qt: internal: %d: WH0A, arr_guess underflow %d", __LINE__, arr);
    if(OSStatus e = ATSUSetLayoutControls(mTextLayout, arr, tags, valueSizes, values)) {
        qWarning("Qt: internal: %ld: Unexpected condition reached %s:%d", e, __FILE__, __LINE__);
        if(ctx_port)
            QDEndCGContext(ctx_port, &ctx);
        return 0;
    }
    ATSUSetTransientFontMatching(mTextLayout, true);
    //do required task now
    if(task & EXISTS) {
        if(task != EXISTS)
            qWarning("Qt: EXISTS must appear by itself!");
        ATSUFontID fid;
        UniCharArrayOffset off;
        UniCharCount off_len;
        if (ATSUMatchFontsToText(mTextLayout, kATSUFromTextBeginning, kATSUToTextEnd, &fid, &off,
                                 &off_len) != kATSUFontsNotMatched)
            ret = 1;
    } else if((task & WIDTH) && !ret) {
        ATSUTextMeasurement left=0, right=0, bottom=0, top=0;
        ATSUGetUnjustifiedBounds(mTextLayout, kATSUFromTextBeginning, kATSUToTextEnd, &left, &right, &bottom, &top);
        ret = FixRound(right-left);
        if(use_len == 1 && s->unicode() < widthCacheSize && ret < 0x100)
            widthCache[s->unicode()] = ret ? ret : -777; //mark so that 0 is cached..
    }
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
        if (qAbs(x) > SHRT_MAX || qAbs(y) > SHRT_MAX) { //bound to 16bit
            const float tx = newMatrix.tx, ty = newMatrix.ty;
            newMatrix = CGAffineTransformTranslate(newMatrix, -tx, ty);
            x += qRound(tx);
            y -= qRound(ty);
            transform = true;
        }
        if(transform) {
            CGContextConcatCTM(ctx, CGAffineTransformInvert(oldMatrix));
            CGContextConcatCTM(ctx, newMatrix);
            CGContextSetTextMatrix(ctx, newMatrix);
            ATSUDrawText(mTextLayout, kATSUFromTextBeginning, kATSUToTextEnd,
                         FixRatio(x, 1), FixRatio(y, 1));
            CGContextConcatCTM(ctx, CGAffineTransformInvert(CGContextGetCTM(ctx)));
            CGContextConcatCTM(ctx, oldMatrix);
            CGContextSetTextMatrix(ctx, oldMatrix);
        } else {
            ATSUDrawText(mTextLayout, kATSUFromTextBeginning, kATSUToTextEnd,
                         FixRatio(x, 1), FixRatio(y, 1));
        }
    }
    if(ctx_port)
        QDEndCGContext(ctx_port, &ctx);
    return ret;
}

class QMacFontPath
{
    float x, y;
    QPainterPath *path;
public:
    inline QMacFontPath(float _x, float _y, QPainterPath *_path) : x(_x), y(_y), path(_path) { }
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


void QFontEngineMac::addOutlineToPath(qreal x, qreal y, const QGlyphLayout *glyphs, int numGlyphs,
                                      QPainterPath *path, QTextItem::RenderFlags)
{

    OSStatus e;
    if (!mTextLayout) {
        e = ATSUCreateTextLayout(&mTextLayout);
        if (e != noErr) {
            qWarning("Qt: internal: %ld: Unexpected condition reached %s: %d", e, __FILE__, __LINE__);
            return;
        }
    }

    QVarLengthArray<UniChar> chars(numGlyphs);
    for (int j = 0; j < numGlyphs; ++j)
        chars[j] = glyphs[j].glyph;
    e = ATSUSetTextPointerLocation(mTextLayout, chars.data(), 0, (UniCharCount)numGlyphs, numGlyphs);
    if (e != noErr) {
        qWarning("Qt: internal: %ld: Error ATSUSetTextPointerLocation %s: %d", e, __FILE__, __LINE__);
        return;
    }

    QATSUStyle *st = getFontStyle();
    e = ATSUSetRunStyle(mTextLayout, st->style, 0, (UniCharCount)numGlyphs);
    if (e != noErr) {
        qWarning("Qt: internal: %ld: Error ATSUSetRunStyle %s: %d", e, __FILE__, __LINE__);
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
        qWarning("Qt: internal: %d: WH0A, arr_guess underflow %d", __LINE__, arr);
    if(OSStatus e = ATSUSetLayoutControls(mTextLayout, arr, tags, valueSizes, values)) {
        qWarning("Qt: internal: %ld: Unexpected condition reached %s:%d", e, __FILE__, __LINE__);
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
        qWarning("Qt: internal: %ld: Error ATSUDirectGetLayoutDataArrayPtrFromTextLayout %s: %d", e,
                 __FILE__, __LINE__);
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
