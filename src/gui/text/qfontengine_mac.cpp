/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <private/qapplication_p.h>
#include <private/qfontengine_p.h>
#include <private/qpainter_p.h>
#include <private/qpainter_p.h>
#include <private/qtextengine_p.h>
#include <qbitmap.h>
#include <qpaintengine_mac.h>
#include <qglobal.h>
#include <qpaintdevicemetrics.h>
#include <qpixmap.h>
#include <qpixmapcache.h>
#include <qvarlengtharray.h>

#include <ApplicationServices/ApplicationServices.h>

//Externals
void qstring2pstring(QString s, Str255 str, TextEncoding encoding=0, int len=-1); //qglobal.cpp
QByteArray pstring2qstring(const unsigned char *c); //qglobal.cpp
unsigned char * p_str(const QString &); //qglobal.cpp

//Generic engine
QFontEngine::~QFontEngine()
{
}

Q26Dot6 QFontEngine::lineThickness() const
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

Q26Dot6 QFontEngine::underlinePosition() const
{
  return ((lineThickness() * 2) + 3) / 6;
}

struct QATSUStyle {
    ATSUStyle style;
    QColor rgb;
    int ascent, descent, leading, maxWidth; //slight cache
};

//Mac (ATSUI) engine
QFontEngineMac::QFontEngineMac() : QFontEngine(), mTextLayout(0), internal_fi(0), fontref(0)
{
    memset(widthCache, '\0', widthCacheSize);
}

QFontEngineMac::~QFontEngineMac()
{
    delete internal_fi;
    if (mTextLayout)
        ATSUDisposeTextLayout(mTextLayout);
}

bool QFontEngineMac::stringToCMap(const QChar *str, int len, QGlyphLayout *glyphs, int *nglyphs,
                                  QTextEngine::ShaperFlags /*flags*/) const
{
    if(*nglyphs < len) {
        *nglyphs = len;
        return false;
    }
    *nglyphs = len;
    for(int i = 0; i < len; i++) {
        glyphs[i].glyph = str[i].unicode();
        if(str[i].unicode() < widthCacheSize && widthCache[str[i].unicode()]) {
            glyphs[i].advance.x = widthCache[str[i].unicode()];
            glyphs[i].advance.y = 0;
        } else {
            glyphs[i].advance.x = doTextTask(str+i, 0, 1, 1, WIDTH);
            glyphs[i].advance.y = 0;
        }
    }
    return true;
}

void
QFontEngineMac::draw(QPaintEngine *p, int x, int y, const QTextItem &si, int textFlags)
{
    QPainterState *pState = p->painterState();
    int txop = pState->txop;
    QWMatrix xmat = pState->matrix;

    if(!p->hasFeature(QPaintEngine::CoordTransform)) {
        if(txop >= QPainter::TxScale) {
            int aw = si.width, ah = si.ascent + si.descent + 1;
            if(aw == 0 || ah == 0)
                return;
            QBitmap bm(aw, ah, true);        // create bitmap
            {
                QPainter paint(&bm);  // draw text in bitmap
                paint.setPen(Qt::color1);
                paint.drawTextItem(QPoint(0, si.ascent), si, textFlags);
                paint.end();
            }

            QPixmap pm(bm.size());
            if(pState->painter->backgroundMode() == QPainter::OpaqueMode) {
                pm = bm;
                bm.fill(Qt::color1);
                pm.setMask(bm);
            } else {
                QPainter paint(&pm);
                paint.fillRect(0, 0, pm.width(), pm.height(), pState->painter->pen().color());
                paint.end();
                pm.setMask(bm);
            }

            pState->painter->drawPixmap(x, y - si.ascent, pm);
            return;
        }
        if(txop == QPainter::TxTranslate)
            pState->painter->map(x, y, &x, &y);
    }
    if(p->type() == QPaintEngine::QuickDraw) {
        QQuickDrawPaintEngine *mgc = static_cast<QQuickDrawPaintEngine *>(p);
        mgc->updateState(pState);
        mgc->setupQDPort(false, 0, 0);
        mgc->setupQDFont();
    }

    QGlyphLayout *glyphs = si.glyphs;
    if(pState->painter->backgroundMode() == Qt::OpaqueMode) {
        glyph_metrics_t br = boundingBox(glyphs, si.num_glyphs);
        pState->painter->fillRect(x+br.x.toInt(), y+br.y.toInt(), br.width.toInt(), br.height.toInt(),
                                  pState->painter->background().color());
    }

    bool textAA = p->renderHints() & QPainter::TextAntialiasing;
    bool lineAA = p->renderHints() & QPainter::LineAntialiasing;
    if (textAA != lineAA) {
        if (textAA)
            p->setRenderHints(QPainter::LineAntialiasing);
        else
            p->clearRenderHints(QPainter::LineAntialiasing);
    }

    int w = 0;
    uchar task = DRAW;
    if(textFlags != 0)
        task |= WIDTH; //I need the width for these..
    if(si.right_to_left) {
        glyphs += si.num_glyphs;
        for(int i = 0; i < si.num_glyphs; i++) {
            glyphs--;
            w += doTextTask((QChar*)&glyphs->glyph, 0, 1, 1, task, x, y, p);
            x += glyphs->advance.x.toInt();
        }
    } else {
        QVarLengthArray<ushort> g(si.num_glyphs);
        for(int i = 0; i < si.num_glyphs; ++i)
            g[i] = glyphs[i].glyph;
        w = doTextTask((QChar*)g.data(), 0, si.num_glyphs, si.num_glyphs, task, x, y, p);
    }
    if(w && textFlags != 0) {
        int lw = lineThickness().toInt();
        if(textFlags & Qt::Underline)
            p->drawRect(QRect(x, y+underlinePosition().toInt(), si.right_to_left ? -w : w, lw));
        if(textFlags & Qt::Overline)
            p->drawRect(QRect(x, y + (ascent().toInt() + 1), si.right_to_left ? -w : w, lw));
        if(textFlags & Qt::StrikeOut)
            p->drawRect(QRect(x, y + (ascent().toInt() / 3), si.right_to_left ? -w : w, lw));
    }
    if (textAA != lineAA) {
        if (textAA)
            p->clearRenderHints(QPainter::LineAntialiasing);
        else
            p->setRenderHints(QPainter::LineAntialiasing);
    }
}

glyph_metrics_t
QFontEngineMac::boundingBox(const QGlyphLayout *glyphs, int numGlyphs)
{
    Q26Dot6 w;
    const QGlyphLayout *end = glyphs + numGlyphs;
    while(end > glyphs)
        w += (--end)->advance.x;
    return glyph_metrics_t(0, -(ascent()), w, ascent()+descent()+1, w, 0);
}

glyph_metrics_t
QFontEngineMac::boundingBox(glyph_t glyph)
{
    int w = doTextTask((QChar*)&glyph, 0, 1, 1, WIDTH);
    return glyph_metrics_t(0, -(ascent()), w, ascent()+descent()+1, w, 0);
}

bool
QFontEngineMac::canRender(const QChar *string,  int len)
{
    return doTextTask(string, 0, len, len, EXISTS);
}

void
QFontEngineMac::calculateCost()
{
    // ### don't know how to get the number of glyphs from the font,
    // ### so default to 1024
    cache_cost = (ascent() + descent() + 1).toInt() * maxCharWidth().toInt() * 1024;
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
    ATSUFontID fond = fontref;
    valueSizes[arr] = sizeof(fond);
    values[arr] = &fond;
    arr++;
    tags[arr] = kATSUQDItalicTag;
    valueSizes[arr] = sizeof(Boolean);
    Boolean italicBool = fontDef.italic ? true : false;
    values[arr] = &italicBool;
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
        ret = NULL;
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
    }
    internal_fi = ret; //cache it
    const_cast<QFontEngineMac*>(this)->calculateCost(); //do this absolutely last!!
    return ret;
}

static inline int qt_mac_get_measurement(ATSUStyle style, ATSUAttributeTag tag, const QFontEngine *) //the engine is passed for debugging only
{
    ATSUTextMeasurement ret=0;
    OSStatus result = ATSUGetAttribute(style, tag, sizeof(ret), &ret, 0);
    if(result != noErr && result != kATSUNotSetErr)
        qWarning("%s:%d -- This can't really happen!! %ld", __FILE__, __LINE__, result);
    return FixRound(ret);
}

Q26Dot6 QFontEngineMac::ascent() const
{
    QATSUStyle *st = getFontStyle();
    if(st->ascent != -1)
        return st->ascent;
    return st->ascent = qt_mac_get_measurement(st->style, kATSUAscentTag, this);
}
Q26Dot6 QFontEngineMac::descent() const
{
    QATSUStyle *st = getFontStyle();
    if(st->descent != -1)
        return st->descent;
    return st->descent = qt_mac_get_measurement(st->style, kATSUDescentTag, this);
}

Q26Dot6 QFontEngineMac::leading() const
{
    QATSUStyle *st = getFontStyle();
    if(st->leading != -1)
        return st->leading;
    return st->leading = qt_mac_get_measurement(st->style, kATSULeadingTag, this);
}

Q26Dot6 QFontEngineMac::maxCharWidth() const
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
    QPainterState *pState = 0;
    QPaintDevice *device = 0;
    QWidget *widget = 0;
    if(p) {
        pState = p->painterState();
        device = p->painter()->device();
        if(device->devType() == QInternal::Widget)
            widget = static_cast<QWidget *>(device);
    }

    if(!st) //can't really happen, but just to be sure..
        return 0;

    int ret = 0;
    if(task & DRAW) {
        Q_ASSERT(p); //really need a painter and engine to do any drawing!!!
        QColor rgb = pState->painter->pen().color();
        if(rgb != st->rgb) {
            st->rgb = rgb;
            const ATSUAttributeTag tag = kATSUColorTag;
            ::RGBColor fcolor;
            fcolor.red = rgb.red()*256;
            fcolor.green = rgb.green()*256;
            fcolor.blue = rgb.blue()*256;
            ByteCount size = sizeof(fcolor);
            ATSUAttributeValuePtr value = &fcolor;
            if(OSStatus e = ATSUSetAttributes(st->style, 1, &tag, &size, &value)) {
                qWarning("Qt: internal: %ld: This shouldn't happen %s:%d", e, __FILE__, __LINE__);
                return 0;
            }
        }
    }

    { //transformations
        CGAffineTransform tf = CGAffineTransformIdentity;
        if((task & DRAW) && p && p->type() == QPaintEngine::CoreGraphics) { //flip text
            int height = 0;
            if(widget)
                height = widget->topLevelWidget()->height();
            else
                height = device->metric(QPaintDeviceMetrics::PdmHeight);
            tf = CGAffineTransformTranslate(tf, 0, height);
            tf = CGAffineTransformScale(tf, 1, -1);
        }
        if(fontDef.stretch != 100)
            tf = CGAffineTransformScale(tf, fontDef.stretch/100, 1);
#if 0 //grr this just doesn't work, how frustrating..
        if(pState) {
            CGAffineTransform xf = CGAffineTransformMake(pState->matrix.m11(), pState->matrix.m12(),
                                                         pState->matrix.m21(), pState->matrix.m22(),
                                                         pState->matrix.dx(),  pState->matrix.dy());
            tf = CGAffineTransformConcat(xf, tf);
        }
#endif


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
            if(c == -666) //special marker meaning 0
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
            qDebug("Qt: internal: %ld: Unexpected condition reached %s:%d", e, __FILE__, __LINE__);
            return 0;
        }
    }
    const UniCharCount count = use_len;
    e = ATSUSetTextPointerLocation(mTextLayout, (UniChar *)(s) + pos, 0, count, use_len);
    if (e != noErr) {
        qDebug("Qt: internal: %ld: Error ATSUSetTextPointerLocation %s: %d", e, __FILE__, __LINE__);
        return 0;
    }
    e = ATSUSetRunStyle(mTextLayout, st->style, 0, count);
    if (e != noErr) {
        qDebug("Qt: internal: %ld: Error ATSUSetRunStyle %s: %d", e, __FILE__, __LINE__);
        return 0;
    }
    const int arr_guess = 5;
    int arr = 0;
    ATSUAttributeTag tags[arr_guess];
    ByteCount valueSizes[arr_guess];
    ATSUAttributeValuePtr values[arr_guess];

    tags[arr] = kATSULineLayoutOptionsTag;
    ATSLineLayoutOptions layopts = kATSLineHasNoOpticalAlignment | kATSLineIgnoreFontLeading
                                   | kATSLineFractDisable | kATSLineDisableAutoAdjustDisplayPos |
                                   kATSLineDisableAllLayoutOperations | kATSLineUseDeviceMetrics;
    if(fontDef.styleStrategy & QFont::NoAntialias)
        layopts |= kATSLineNoAntiAliasing;
    valueSizes[arr] = sizeof(layopts);
    values[arr] = &layopts;
    arr++;

    tags[arr] = kATSUCGContextTag;
    CGContextRef ctx = NULL;
    CGrafPtr ctx_port = 0; //only set if the ctx is a ctx created from a port
    if(p && p->type() == QPaintEngine::CoreGraphics) {
        if(p && device) {
            ctx = static_cast<CGContextRef>(p->handle());
        } else {
            static QPixmap *pixmap = NULL;
            if(!pixmap)
                pixmap = new QPixmap(1, 1, 32);
            ctx = static_cast<CGContextRef>(pixmap->macCGHandle());
        }
    } else {
        QRegion rgn;
        if(p && p->type() == QPaintEngine::QuickDraw) {
            QPoint pt;
            static_cast<QQuickDrawPaintEngine *>(p)->setupQDPort(false, &pt, &rgn);
            x += pt.x();
            y += pt.y();
            ATSUFontID fond;
            ATSUFONDtoFontID(fontref, NULL, &fond);
            TextFont(fond);
        } else {
            static QPixmap *pixmap = 0;
            if(!pixmap)
                pixmap = new QPixmap(1, 1, 32);
            QMacSavedPortInfo::setPaintDevice(pixmap);
        }

        GetGWorld(&ctx_port, 0);
        if(OSStatus err = QDBeginCGContext(ctx_port, &ctx)) {
            qWarning("Qt: internal: WH0A, QDBeginCGContext(%ld) failed. %s:%d", err, __FILE__, __LINE__);
            return 0;
        }
        if(task & DRAW) {
            Rect clipr;
            GetPortBounds(ctx_port, &clipr);
            ClipCGContextToRegion(ctx, &clipr, rgn.handle(true));
        }
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
            widthCache[s->unicode()] = ret ? ret : -666; //mark so that 0 is cached..
    }
    if(task & DRAW) {
        int drawy = y;
        if(ctx_port) {
            int height = 0;
            if(widget)
                height = widget->topLevelWidget()->height();
            else
                height = device->metric(QPaintDeviceMetrics::PdmHeight);
            drawy = height-drawy;
        }
        ATSUDrawText(mTextLayout, kATSUFromTextBeginning, kATSUToTextEnd, FixRatio(x, 1),
                      FixRatio(drawy, 1));
    }
    if(ctx_port)
        QDEndCGContext(ctx_port, &ctx);
    return ret;
}

// box font engine
QFontEngineBox::QFontEngineBox(int size) : _size(size)
{
    //qDebug("box font engine created!");
}

QFontEngineBox::~QFontEngineBox()
{
}

QFontEngine::FECaps QFontEngineBox::capabilites() const
{
    return FullTransformations;
}

bool QFontEngineBox::stringToCMap(const QChar *,  int len, QGlyphLayout *glyphs,
                                                int *nglyphs, QTextEngine::ShaperFlags) const
{
    if(*nglyphs < len) {
        *nglyphs = len;
        return false;
    }

    for(int i = 0; i < len; i++)
        glyphs[i].glyph = 0;
    *nglyphs = len;

    for(int i = 0; i < len; i++) {
        (glyphs++)->advance.x = _size;
        (glyphs++)->advance.y = 0;
    }

    return true;
}

void QFontEngineBox::draw(QPaintEngine *p, int x, int y, const QTextItem &si, int textFlags)
{
    Q_UNUSED(p);
    Q_UNUSED(x);
    Q_UNUSED(y);
    Q_UNUSED(si);
    Q_UNUSED(textFlags);
    //qDebug("QFontEngineBox::draw(%d, %d, numglyphs=%d", x, y, numGlyphs);
}

glyph_metrics_t QFontEngineBox::boundingBox(const QGlyphLayout *, int numGlyphs)
{
    glyph_metrics_t overall;
    overall.x = overall.y = 0;
    overall.width = _size*numGlyphs;
    overall.height = _size;
    overall.xoff = overall.width;
    overall.yoff = 0;
    return overall;
}

glyph_metrics_t QFontEngineBox::boundingBox(glyph_t)
{
    return glyph_metrics_t(0, _size, _size, _size, _size, 0);
}

Q26Dot6 QFontEngineBox::ascent() const
{
    return _size;
}

Q26Dot6 QFontEngineBox::descent() const
{
    return 0;
}

Q26Dot6 QFontEngineBox::leading() const
{
    int l = qRound(_size * 0.15);
    return (l > 0) ? l : 1;
}

Q26Dot6 QFontEngineBox::maxCharWidth() const
{
    return _size;
}

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
