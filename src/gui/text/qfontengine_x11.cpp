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

#include "qbitmap.h"

// #define FONTENGINE_DEBUG

#include <qbytearray.h>
#include <qtextcodec.h>

#include "qfontdatabase.h"
#include "qpaintdevice.h"
#include "qpainter.h"
#include "qvarlengtharray.h"
#include "qwidget.h"

#include <private/qpaintengine_x11_p.h>
#include "qfont.h"
#include "qfont_p.h"
#include "qfontengine_p.h"
#include "qopentype_p.h"

#include <private/qpainter_p.h>
#include <private/qunicodetables_p.h>

#include <private/qt_x11_p.h>


#include <math.h>
#include <limits.h>
#define d d_func()
#define q q_func()


QFontEngine::~QFontEngine()
{
}

qreal QFontEngine::lineThickness() const
{
    // ad hoc algorithm
    int score = fontDef.weight * fontDef.pixelSize;
    int lw = score / 700;

    // looks better with thicker line for small pointsizes
    if (lw < 2 && score >= 1050) lw = 2;
    if (lw == 0) lw = 1;

    return lw;
}

qreal QFontEngine::underlinePosition() const
{
    return ((lineThickness() * 2) + 3) / 6;
}


// ------------------------------------------------------------------
// Multi XLFD engine
// ------------------------------------------------------------------

QFontEngineMultiXLFD::QFontEngineMultiXLFD(const QFontDef &r, const QList<int> &l, int s)
    : QFontEngineMulti(l.size()), encodings(l), screen(s)
{
    fontDef = r;
    loadEngine(0);
}

QFontEngineMultiXLFD::~QFontEngineMultiXLFD()
{ }

void QFontEngineMultiXLFD::loadEngine(int at)
{
    Q_ASSERT(at < engines.size());
    Q_ASSERT(engines.at(at) == 0);
    const int encoding = encodings.at(at);
    QFontDef req = fontDef;
    QFontEngine *fontEngine = QFontDatabase::findFont(QUnicodeTables::Common, 0, req, encoding);
    if (!fontEngine) {
        req.family = QString::null;
        fontEngine = QFontDatabase::findFont(QUnicodeTables::Common, 0, req, encoding);
    }
    Q_ASSERT(fontEngine != 0);
    fontEngine->ref.ref();
    engines[at] = fontEngine;
}

// ------------------------------------------------------------------
// Xlfd cont engine
// ------------------------------------------------------------------

// defined in qfontdatbase_x11.cpp
extern int qt_mib_for_xlfd_encoding(const char *encoding);
extern int qt_xlfd_encoding_id(const char *encoding);

static inline XCharStruct *charStruct(XFontStruct *xfs, uint ch)
{
    XCharStruct *xcs = 0;
    unsigned char r = ch>>8;
    unsigned char c = ch&0xff;
    if (r >= xfs->min_byte1 &&
         r <= xfs->max_byte1 &&
         c >= xfs->min_char_or_byte2 &&
         c <= xfs->max_char_or_byte2) {
        if (!xfs->per_char)
            xcs = &(xfs->min_bounds);
        else {
            xcs = xfs->per_char + ((r - xfs->min_byte1) *
                                   (xfs->max_char_or_byte2 -
                                    xfs->min_char_or_byte2 + 1)) +
                  (c - xfs->min_char_or_byte2);
            if (xcs->width == 0 && xcs->ascent == 0 &&  xcs->descent == 0)
                xcs = 0;
        }
    }
    return xcs;
}

QFontEngineXLFD::QFontEngineXLFD(XFontStruct *fs, const char *name, int mib)
    : _fs(fs), _name(name), _codec(0), _cmap(mib)
{
    if (_cmap) _codec = QTextCodec::codecForMib(_cmap);

    cache_cost = (((fs->max_byte1 - fs->min_byte1) *
                   (fs->max_char_or_byte2 - fs->min_char_or_byte2 + 1)) +
                  fs->max_char_or_byte2 - fs->min_char_or_byte2);
    cache_cost = ((fs->max_bounds.ascent + fs->max_bounds.descent) *
                  (fs->max_bounds.width * cache_cost / 8));
    lbearing = SHRT_MIN;
    rbearing = SHRT_MIN;
}

QFontEngineXLFD::~QFontEngineXLFD()
{
    XFreeFont(QX11Info::display(), _fs);
    _fs = 0;
}

QFontEngine::FECaps QFontEngineXLFD::capabilites() const
{
    return FullTransformations;
}

bool QFontEngineXLFD::stringToCMap(const QChar *str, int len, QGlyphLayout *glyphs, int *nglyphs, QTextEngine::ShaperFlags flags) const
{
    if (*nglyphs < len) {
        *nglyphs = len;
        return false;
    }

    bool mirrored = flags & QTextEngine::RightToLeft;
    if (_codec) {
        bool haveNbsp = false;
        for (int i = 0; i < len; i++)
            if (str[i].unicode() == 0xa0) {
                haveNbsp = true;
                break;
            }

        QVarLengthArray<unsigned short> ch(len);
        QChar *chars = (QChar *)ch.data();
        if (haveNbsp || mirrored) {
            for (int i = 0; i < len; i++)
                chars[i] = (str[i].unicode() == 0xa0 ? 0x20 :
                            (mirrored ? ::mirroredChar(str[i]).unicode() : str[i].unicode()));
        } else {
            for (int i = 0; i < len; i++)
                chars[i] = (str[i].unicode() == 0xa0 ? 0x20 : str[i].unicode());
        }
        QTextCodec::ConverterState state;
        state.flags = QTextCodec::ConvertInvalidToNull;
        QByteArray ba = _codec->fromUnicode(chars, len, &state);
        if (ba.length() == 2*len) {
            // double byte encoding
            const uchar *data = (const uchar *)ba.constData();
            for (int i = 0; i < len; i++) {
                glyphs[i].glyph = ((ushort)data[0] << 8) + data[1];
                data += 2;
            }
        } else {
            const uchar *data = (const uchar *)ba.constData();
            for (int i = 0; i < len; i++)
                glyphs[i].glyph = (ushort)data[i];
        }
    } else {
        QGlyphLayout *g = glyphs + len;
        const QChar *c = str + len;
        if (mirrored) {
            while (c != str)
                (--g)->glyph = (--c)->unicode() == 0xa0 ? 0x20 : ::mirroredChar(*c).unicode();
        } else {
            while (c != str)
                (--g)->glyph = (--c)->unicode() == 0xa0 ? 0x20 : c->unicode();
        }
    }
    *nglyphs = len;

    QGlyphLayout *g = glyphs + len;
    XCharStruct *xcs;
    // inlined for better perfomance
    if (!_fs->per_char) {
        xcs = &_fs->min_bounds;
        while (g != glyphs) {
            --g;
            g->advance.rx() = xcs->width;
            g->advance.ry() = 0;
        }
    }
    else if (!_fs->max_byte1) {
        XCharStruct *base = _fs->per_char - _fs->min_char_or_byte2;
        while (g != glyphs) {
            unsigned int gl = (--g)->glyph;
            xcs = (gl >= _fs->min_char_or_byte2 && gl <= _fs->max_char_or_byte2) ?
                  base + gl : 0;
            g->advance.rx() = (!xcs || (!xcs->width && !xcs->ascent && !xcs->descent)) ? _fs->ascent : xcs->width;
            g->advance.ry() = 0;
        }
    }
    else {
        while (g != glyphs) {
            xcs = charStruct(_fs, (--g)->glyph);
            g->advance.rx() = xcs ? xcs->width : _fs->ascent;
            g->advance.ry() = 0;
        }
    }
    if (_scale != 1.) {
        for (int i = 0; i < len; i++)
            glyphs[i].advance.rx() *= _scale;
    }
    return true;
}

glyph_metrics_t QFontEngineXLFD::boundingBox(const QGlyphLayout *glyphs, int numGlyphs)
{
    int i;

    glyph_metrics_t overall;
    qreal ymax = 0.;
    qreal xmax = 0.;
    for (i = 0; i < numGlyphs; i++) {
        XCharStruct *xcs = charStruct(_fs, glyphs[i].glyph);
        if (xcs) {
            qreal x = overall.xoff + glyphs[i].offset.x() - xcs->lbearing;
            qreal y = overall.yoff + glyphs[i].offset.y() - xcs->ascent;
            overall.x = qMin(overall.x, x);
            overall.y = qMin(overall.y, y);
            xmax = qMax(xmax, overall.xoff + glyphs[i].offset.x() + xcs->rbearing);
            ymax = qMax(ymax, y + xcs->ascent + xcs->descent);
            overall.xoff += glyphs[i].advance.x()/_scale;
        } else {
            qreal size = _fs->ascent;
            overall.x = qMin(overall.x, overall.xoff);
            overall.y = qMin(overall.y, overall.yoff - size);
            ymax = qMax(ymax, overall.yoff);
            overall.xoff += size;
            xmax = qMax(xmax, overall.xoff);
        }
    }
    overall.height = ymax - overall.y;
    overall.width = xmax - overall.x;

    if (_scale != qreal(1)) {
        overall.x *= _scale;
        overall.y *= _scale;
        overall.height *= _scale;
        overall.width *= _scale;
        overall.xoff *= _scale;
        overall.yoff *= _scale;
    }
    return overall;
}

glyph_metrics_t QFontEngineXLFD::boundingBox(glyph_t glyph)
{
    glyph_metrics_t gm;
    // ### scale missing!
    XCharStruct *xcs = charStruct(_fs, glyph);
    if (xcs) {
        gm = glyph_metrics_t(xcs->lbearing, -xcs->ascent, xcs->rbearing- xcs->lbearing, xcs->ascent + xcs->descent,
                              xcs->width, 0);
    } else {
        qreal size = ascent();
        gm = glyph_metrics_t(0, size, size, size, size, 0);
    }
    if (_scale != 1.) {
        gm.x *= _scale;
        gm.y *= _scale;
        gm.height *= _scale;
        gm.width *= _scale;
        gm.xoff *= _scale;
        gm.yoff *= _scale;
    }
    return gm;
}


qreal QFontEngineXLFD::ascent() const
{
    return _fs->ascent*_scale;
}

qreal QFontEngineXLFD::descent() const
{
    return (_fs->descent-1)*_scale;
}

qreal QFontEngineXLFD::leading() const
{
    qreal l = (qMin<int>(_fs->ascent, _fs->max_bounds.ascent)
                 + qMin<int>(_fs->descent, _fs->max_bounds.descent)) * _scale * qreal(0.15);
    return ceil(l);
}

qreal QFontEngineXLFD::maxCharWidth() const
{
    return _fs->max_bounds.width*_scale;
}


// Loads the font for the specified script
static inline int maxIndex(XFontStruct *f) {
    return (((f->max_byte1 - f->min_byte1) *
             (f->max_char_or_byte2 - f->min_char_or_byte2 + 1)) +
            f->max_char_or_byte2 - f->min_char_or_byte2);
}

qreal QFontEngineXLFD::minLeftBearing() const
{
    if (lbearing == SHRT_MIN) {
        if (_fs->per_char) {
            XCharStruct *cs = _fs->per_char;
            int nc = maxIndex(_fs) + 1;
            int mx = cs->lbearing;

            for (int c = 1; c < nc; c++) {
                // ignore the bearings for characters whose ink is
                // completely outside the normal bounding box
                if ((cs[c].lbearing <= 0 && cs[c].rbearing <= 0) ||
                    (cs[c].lbearing >= cs[c].width && cs[c].rbearing >= cs[c].width))
                    continue;

                int nmx = cs[c].lbearing;

                if (nmx < mx)
                    mx = nmx;
            }

            ((QFontEngineXLFD *)this)->lbearing = mx;
        } else
            ((QFontEngineXLFD *)this)->lbearing = _fs->min_bounds.lbearing;
    }
    return lbearing*_scale;
}

qreal QFontEngineXLFD::minRightBearing() const
{
    if (rbearing == SHRT_MIN) {
        if (_fs->per_char) {
            XCharStruct *cs = _fs->per_char;
            int nc = maxIndex(_fs) + 1;
            int mx = cs->rbearing;

            for (int c = 1; c < nc; c++) {
                // ignore the bearings for characters whose ink is
                // completely outside the normal bounding box
                if ((cs[c].lbearing <= 0 && cs[c].rbearing <= 0) ||
                    (cs[c].lbearing >= cs[c].width && cs[c].rbearing >= cs[c].width))
                    continue;

                int nmx = cs[c].rbearing;

                if (nmx < mx)
                    mx = nmx;
            }

            ((QFontEngineXLFD *)this)->rbearing = mx;
        } else
            ((QFontEngineXLFD *)this)->rbearing = _fs->min_bounds.rbearing;
    }
    return rbearing*_scale;
}

int QFontEngineXLFD::cmap() const
{
    return _cmap;
}

const char *QFontEngineXLFD::name() const
{
    return _name;
}

bool QFontEngineXLFD::canRender(const QChar *string, int len)
{
    QVarLengthArray<QGlyphLayout, 256> glyphs(len);
    int nglyphs = len;
    if (stringToCMap(string, len, glyphs.data(), &nglyphs, 0) == false) {
        glyphs.resize(nglyphs);
        stringToCMap(string, len, glyphs.data(), &nglyphs, 0);
    }

    bool allExist = true;
    for (int i = 0; i < nglyphs; i++) {
        if (!glyphs[i].glyph || !charStruct(_fs, glyphs[i].glyph)) {
            allExist = false;
            break;
        }
    }

    return allExist;
}


#ifndef QT_NO_XFT

// ------------------------------------------------------------------
// Multi Xft engine
// ------------------------------------------------------------------

QFontEngineMultiXft::QFontEngineMultiXft(FcFontSet *fs, int s)
    : QFontEngineMulti(fs->nfont), fontSet(fs), screen(s)
{ loadEngine(0); }

QFontEngineMultiXft::~QFontEngineMultiXft()
{ FcFontSetDestroy(fontSet); }

void QFontEngineMultiXft::loadEngine(int at)
{
    Q_ASSERT(at < engines.size());
    Q_ASSERT(engines.at(at) == 0);
    FcPattern *pattern = fontSet->fonts[at];
    extern QFontDef FcPatternToQFontDef(FcPattern *pattern);
    QFontDef fontDef = FcPatternToQFontDef(fontSet->fonts[at]);
    // note: we use -1 for the script to make sure that we keep real
    // Xft engines separate from Multi engines in the font cache
    QFontCache::Key key(fontDef, -1, screen);
    QFontEngine *fontEngine = QFontCache::instance->findEngine(key);
    if (!fontEngine) {
        FcResult res;
        FcPattern *p = XftFontMatch(QX11Info::display(), screen, pattern, &res);
        XftFont *f = XftFontOpenPattern(QX11Info::display(), p);
        if (f) {
            fontEngine = new QFontEngineXft(f);
        } else {
            fontEngine = new QFontEngineBox(fontDef.pixelSize);
        }
        fontEngine->fontDef = fontDef;
        QFontCache::instance->insertEngine(key, fontEngine);
    }
    fontEngine->ref.ref();
    engines[at] = fontEngine;
}


// ------------------------------------------------------------------
// Xft cont engine
// ------------------------------------------------------------------

QFontEngineXft::QFontEngineXft(XftFont *font)
    : _font(font), _openType(0), _cmap(-1), transformed_fonts(0)
{
    _face = XftLockFace(_font);

    // Xft maps Unicode and adobe roman for us.
    for (int i = 0; i < _face->num_charmaps; ++i) {
        FT_CharMap cm = _face->charmaps[i];
//         qDebug("font has charmap %x", cm->encoding);
        if (cm->encoding == ft_encoding_adobe_custom
            || cm->encoding == ft_encoding_symbol) {
//             qDebug("font has adobe custom or ms symbol charmap");
            _cmap = i;
            break;
        }
    }

    cache_cost = _font->height * _font->max_advance_width *
                 (_face ? _face->num_glyphs : 1024);

    // if the Xft font is not antialiased, it uses bitmaps instead of
    // 8-bit alpha maps... adjust the cache_cost to reflect this
    Bool antialiased = true;
    if (XftPatternGetBool(_font->pattern, XFT_ANTIALIAS,
			  0, &antialiased) == XftResultMatch &&
	! antialiased) {
        cache_cost /= 8;
    }
    lbearing = SHRT_MIN;
    rbearing = SHRT_MIN;

    memset(cmapCache, 0, sizeof(cmapCache));
    advanceCache = (float *)malloc(256*sizeof(float));
    advanceCacheSize = 256;
    for (uint i = 0; i < advanceCacheSize; ++i)
        advanceCache[i] = -1000000.;
    designAdvanceCacheSize = 0;
    designAdvanceCache = 0;
}

QFontEngineXft::~QFontEngineXft()
{
    delete _openType;
    XftUnlockFace(_font);

    XftFontClose(QX11Info::display(),_font);
    _font = 0;
    TransformedFont *trf = transformed_fonts;
    while (trf) {
        XftFontClose(QX11Info::display(), trf->xft_font);
        TransformedFont *tmp = trf;
        trf = trf->next;
        delete tmp;
    }
    free(advanceCache);
    if (designAdvanceCache)
        free(designAdvanceCache);
}

QFontEngine::FECaps QFontEngineXft::capabilites() const
{
    return (_face->face_flags & FT_FACE_FLAG_SCALABLE) ? FullTransformations : NoTransformations;
}

static glyph_t getAdobeCharIndex(XftFont *font, int cmap, uint ucs4)
{
    FT_Face _face = XftLockFace( font );
    FT_Set_Charmap(_face, _face->charmaps[cmap]);
    glyph_t g = FT_Get_Char_Index(_face, ucs4);
    XftUnlockFace(font);
    return g;
}

inline unsigned int getChar(const QChar *str, int &i, const int len)
{
    unsigned int uc = str[i].unicode();
    if (uc >= 0xd800 && uc < 0xdc00 && i < len-1) {
        uint low = str[i+1].unicode();
       if (low >= 0xdc00 && low < 0xe000) {
            uc = (uc - 0xd800)*0x400 + (low - 0xdc00) + 0x10000;
            ++i;
        }
    }
    return uc;
}

bool QFontEngineXft::stringToCMap(const QChar *str, int len, QGlyphLayout *glyphs, int *nglyphs, QTextEngine::ShaperFlags flags) const
{
    if (*nglyphs < len) {
        *nglyphs = len;
        return false;
    }

    bool mirrored = flags & QTextEngine::RightToLeft;
    int glyph_pos = 0;
    if (_cmap != -1) {
       for ( int i = 0; i < len; ++i ) {
           unsigned int uc = getChar(str, i, len);
           glyphs[glyph_pos].glyph = uc < cmapCacheSize ? cmapCache[uc] : 0;
           if ( !glyphs[glyph_pos].glyph ) {
               glyph_t glyph = XftCharIndex(0, _font, uc);
               if (!glyph)
                   glyph = getAdobeCharIndex(_font, _cmap, uc);
              glyphs[glyph_pos].glyph = glyph;
               if ( uc < cmapCacheSize )
                   ((QFontEngineXft *)this)->cmapCache[uc] = glyph;
           }
           ++glyph_pos;
       }
    } else if ( mirrored ) {
        for (int i = 0; i < len; ++i) {
            unsigned int uc = QUnicodeTables::mirroredChar(getChar(str, i, len));
            glyphs[glyph_pos].glyph = uc < cmapCacheSize ? cmapCache[uc] : 0;
            if (!glyphs[glyph_pos].glyph) {
                if (uc == 0xa0)
                    uc = 0x20;
                glyph_t glyph = XftCharIndex(0, _font, uc);
                glyphs[glyph_pos].glyph = glyph;
                if (uc < cmapCacheSize)
                    ((QFontEngineXft *)this)->cmapCache[uc] = glyph;
            }
            ++glyph_pos;
        }
    } else {
        for (int i = 0; i < len; ++i) {
            unsigned int uc = getChar(str, i, len);
            glyphs[glyph_pos].glyph = uc < cmapCacheSize ? cmapCache[uc] : 0;
            if (!glyphs[glyph_pos].glyph) {
                if (uc == 0xa0)
                    uc = 0x20;
                glyph_t glyph = XftCharIndex(0, _font, uc);
                glyphs[glyph_pos].glyph = glyph;
                if (uc < cmapCacheSize)
                    ((QFontEngineXft *)this)->cmapCache[uc] = glyph;
            }
            ++glyph_pos;
        }
    }

    *nglyphs = glyph_pos;
    recalcAdvances(*nglyphs, glyphs, flags);

    return true;
}

void QFontEngineXft::recalcAdvances(int len, QGlyphLayout *glyphs, QTextEngine::ShaperFlags flags) const
{
    if (flags & QTextEngine::DesignMetrics) {
        FT_Face face = XftLockFace(_font);
        for (int i = 0; i < len; i++) {
            FT_UInt glyph = glyphs[i].glyph;
            if (glyph >= designAdvanceCacheSize) {
                int newSize = ((glyph+255)>>8 << 8);
                designAdvanceCache = (float *)realloc(designAdvanceCache, newSize*sizeof(float));
                for (int i = designAdvanceCacheSize; i < newSize; ++i)
                    designAdvanceCache[i] = -1000000.;
                designAdvanceCacheSize = newSize;
            }
            if (designAdvanceCache[glyph] < -999999.) {
                FT_Load_Glyph(face, glyph, FT_LOAD_NO_HINTING);
                designAdvanceCache[glyph] = face->glyph->metrics.horiAdvance/64.;
            }
            glyphs[i].advance.rx() = designAdvanceCache[glyph];
            glyphs[i].advance.ry() = 0.;
        }
        XftUnlockFace(_font);
        if (_scale != 1.) {
            for (int i = 0; i < len; i++)
                glyphs[i].advance.rx() *= _scale;
        }
    } else {
        for (int i = 0; i < len; i++) {
            FT_UInt glyph = glyphs[i].glyph;
            if (glyph >= advanceCacheSize) {
                int newSize = ((glyph+256)>>8 << 8);
                advanceCache = (float *)realloc(advanceCache, newSize*sizeof(float));
                for (int i = advanceCacheSize; i < newSize; ++i)
                    advanceCache[i] = -1000000.;
                advanceCacheSize = newSize;
            }
            if (advanceCache[glyph] < -999999.) {
                XGlyphInfo gi;
                XftGlyphExtents(X11->display, _font, &glyph, 1, &gi);
                advanceCache[glyph] = gi.xOff;
            }
            glyphs[i].advance.rx() = advanceCache[glyph];
            glyphs[i].advance.ry() = 0;
        }
        if (_scale != 1.) {
            for (int i = 0; i < len; i++)
                glyphs[i].advance.rx() *= _scale;
        }
    }
}

void QFontEngineXft::doKerning(int num_glyphs, QGlyphLayout *g, QTextEngine::ShaperFlags flags) const
{
    FT_Face face = XftLockFace(_font);
    if (FT_HAS_KERNING(face)) {
        uint f = (flags == QTextEngine::DesignMetrics ? FT_KERNING_UNFITTED : FT_KERNING_DEFAULT);
        for (int i = 0; i < num_glyphs-1; ++i) {
            FT_Vector kerning;
            FT_Get_Kerning(face, g[i].glyph, g[i+1].glyph, f, &kerning);
            g[i].advance.rx() += kerning.x / 64.;
            g[i].advance.ry() += kerning.y / 64.;
        }
    }
    XftUnlockFace(_font);
}


XftFont *QFontEngineXft::transformedFont(const QMatrix &matrix)
{
    Q_ASSERT(_face->face_flags & FT_FACE_FLAG_SCALABLE);

    XftFont *fnt = 0;
    XftMatrix *mat = 0;
    XftPatternGetMatrix(_font->pattern, XFT_MATRIX, 0, &mat);
    XftMatrix m2;
    qreal scale = matrix.det();
    scale = sqrt(qAbs(scale));
#ifdef QT_USE_FIXED_POINT
    m2.xx = (matrix.m11()*_scale).toDouble();
    m2.xy = (-matrix.m21()*_scale).toDouble();
    m2.yx = (-matrix.m12()*_scale).toDouble();
    m2.yy = (matrix.m22()*_scale).toDouble();
#else
    m2.xx = matrix.m11()*_scale;
    m2.xy = -matrix.m21()*_scale;
    m2.yx = -matrix.m12()*_scale;
    m2.yy = matrix.m22()*_scale;
#endif

    // check if we have it cached
    TransformedFont *trf = transformed_fonts;
    TransformedFont *prev = 0;
    int i = 0;
    while (trf) {
        if (trf->xx == (qreal)m2.xx &&
            trf->xy == (qreal)m2.xy &&
            trf->yx == (qreal)m2.yx &&
            trf->yy == (qreal)m2.yy)
            break;
        TransformedFont *tmp = trf;
        trf = trf->next;
        if (i > 10) {
            XftFontClose(QX11Info::display(), tmp->xft_font);
            delete tmp;
            prev->next = trf;
        } else {
            prev = tmp;
        }
        ++i;
    }
    if (trf) {
        if (prev) {
            // move to beginning of list
            prev->next = trf->next;
            trf->next = transformed_fonts;
            transformed_fonts = trf;
        }
        fnt = trf->xft_font;
    } else {
        if (mat)
            XftMatrixMultiply(&m2, &m2, mat);
        if (scale > 0) {
#ifdef QT_USE_FIXED_POINT
            XftMatrixScale(&m2, (1/scale).toDouble(), (1/scale).toDouble());
#else
            XftMatrixScale(&m2, 1/scale, 1/scale);
#endif
        }
        XftPattern *pattern = XftPatternDuplicate(_font->pattern);
        XftPatternDel(pattern, XFT_MATRIX);
        XftPatternAddMatrix(pattern, XFT_MATRIX, &m2);
        double size;
        XftPatternGetDouble(_font->pattern, XFT_PIXEL_SIZE, 0, &size);
        XftPatternDel(pattern, XFT_SIZE);
        XftPatternDel(pattern, XFT_PIXEL_SIZE);
//             qDebug("setting new size: orig=%f, scale=%f, new=%f", size, scale, size*scale);
#ifdef QT_USE_FIXED_POINT
        XftPatternAddDouble(pattern, XFT_PIXEL_SIZE, (size*scale).toDouble());
#else
        XftPatternAddDouble(pattern, XFT_PIXEL_SIZE, size*scale);
#endif
        fnt = XftFontOpenPattern(QX11Info::display(), pattern);
        TransformedFont *trf = new TransformedFont;
        trf->xx = (qreal)m2.xx;
        trf->xy = (qreal)m2.xy;
        trf->yx = (qreal)m2.yx;
        trf->yy = (qreal)m2.yy;
        trf->xft_font = fnt;
        trf->next = transformed_fonts;
        transformed_fonts = trf;
    }
    return fnt;
}

glyph_metrics_t QFontEngineXft::boundingBox(const QGlyphLayout *glyphs, int numGlyphs)
{
    XGlyphInfo xgi;

    glyph_metrics_t overall;
    qreal ymax = 0;
    qreal xmax = 0;
    for (int i = 0; i < numGlyphs; i++) {
        FT_UInt gl = glyphs[i].glyph;
        XftGlyphExtents(QX11Info::display(), _font, &gl, 1, &xgi);
        qreal x = overall.xoff + glyphs[i].offset.x() - xgi.x;
        qreal y = overall.yoff + glyphs[i].offset.y() - xgi.y;
        overall.x = qMin(overall.x, x);
        overall.y = qMin(overall.y, y);
        xmax = qMax(xmax, x + xgi.width);
        ymax = qMax(ymax, y + xgi.height);
        overall.xoff += qRound(glyphs[i].advance.x()/_scale);
    }
    overall.height = ymax - overall.y;
    overall.width = xmax - overall.x;

    if (_scale != 1.) {
        overall.x *= _scale;
        overall.y *= _scale;
        overall.height *= _scale;
        overall.width *= _scale;
        overall.xoff *= _scale;
        overall.yoff *= _scale;
    }
    return overall;
}

glyph_metrics_t QFontEngineXft::boundingBox(glyph_t glyph)
{
    XGlyphInfo xgi;
    FT_UInt x = glyph;
    XftGlyphExtents(QX11Info::display(), _font, &x, 1, &xgi);
    glyph_metrics_t gm = glyph_metrics_t(-xgi.x, -xgi.y, xgi.width, xgi.height, xgi.xOff, -xgi.yOff);
    if (_scale != 1.) {
        gm.x *= _scale;
        gm.y *= _scale;
        gm.height *= _scale;
        gm.width *= _scale;
        gm.xoff *= _scale;
        gm.yoff *= _scale;
    }
    return gm;
}

static void addCurve(QPainterPath *path, const QPointF &cp, const QPointF &endPoint,
                     int startOff, int nOff, FT_GlyphSlot g)
{
    int j;
    QPointF c0 = QPointF(g->outline.points[startOff-1].x/64., -g->outline.points[startOff-1].y/64.);
    QPointF current = QPointF(g->outline.points[startOff].x/64., -g->outline.points[startOff].y/64.);
    for(j = 1; j <= nOff; j++) {
        QPointF next = (j == nOff)
                       ? endPoint
                       : QPointF(g->outline.points[startOff + j].x/64., -g->outline.points[startOff + j].y/64.);
        QPointF c3 = (j == nOff) ? next : (next + current)/2;
        QPointF c1 = (2*current + c0)/3;
        QPointF c2 = (2*current + c3)/3;
//         qDebug("cubicTo %f/%f %f/%f %f/%f", (cp + c1).x(),  (cp + c1).y(),
//                (cp + c2).x(),  (cp + c2).y(), (cp + c3).x(),  (cp + c3).y());
        path->cubicTo(cp + c1, cp + c2, cp + c3);
        c0 = c3;
        current = next;
    }
}

void QFontEngineXft::addOutlineToPath(qreal x, qreal y, const QGlyphLayout *glyphs, int numGlyphs, QPainterPath *path)
{
    FT_Face face = XftLockFace(_font);
    if (FT_IS_SCALABLE(face)) {
        QPointF point = QPointF(x, y);
        for (int i = 0; i < numGlyphs; i++) {
            FT_UInt glyph = glyphs[i].glyph;
            QPointF cp = point + glyphs[i].offset;
            point += glyphs[i].advance;

            FT_Load_Glyph(face, glyph, FT_LOAD_NO_HINTING|FT_LOAD_NO_BITMAP);

            FT_GlyphSlot g = face->glyph;
            if (g->format != FT_GLYPH_FORMAT_OUTLINE)
                continue;

            // convert the outline to a painter path
            int i = 0;
            for (int c = 0; c < g->outline.n_contours; ++c) {
                QPointF p = cp + QPointF(g->outline.points[i].x/64., -g->outline.points[i].y/64.);
//                 qDebug("contour: %d -- %d", i, g->outline.contours[c]);
//                 qDebug("first point at %f %f", p.x(), p.y());
                path->moveTo(p);

                int first = i;
                int startOff = 0;
                int nOff = 0;
                ++i;
                while (i <= g->outline.contours[c]) {
                    QPointF p = cp + QPointF(g->outline.points[i].x/64., -g->outline.points[i].y/64.);
//                     qDebug("     point at %f %f, on curve=%d", p.x(), p.y(), g->outline.tags[i] & 1);
                    if (!(g->outline.tags[i] & 1)) {
                        /* Off curve */
                        if (!startOff) {
                            startOff = i;
                            nOff = 1;
                        } else {
                            ++nOff;
                        }
                    } else {
                        /* On Curve */
                        if (startOff) {
                            // ###### fix 3rd order beziers
                            addCurve(path, cp, QPointF(g->outline.points[i].x/64., -g->outline.points[i].y/64.),
                                     startOff, nOff, g);
                            startOff = 0;
                        } else {
                            p = cp + QPointF(g->outline.points[i].x/64., -g->outline.points[i].y/64.);
                            path->lineTo(p);
                        }
                    }
                    ++i;
                }
                QPointF end(g->outline.points[first].x/64., -g->outline.points[first].y/64.);
                if (startOff)
                    addCurve(path, cp, end, startOff, nOff, g);
                else
                    path->lineTo(end + cp);
            }
        }
    } else {
        addBitmapFontToPath(x, y, glyphs, numGlyphs, path);
    }
    XftUnlockFace(_font);
}


qreal QFontEngineXft::ascent() const
{
    return (_font->height - _font->descent)*_scale;
}

qreal QFontEngineXft::descent() const
{
    return (_font->descent)*_scale;
}

// #### use Freetype to determine this
qreal QFontEngineXft::leading() const
{
    qreal l = qMin(_font->height - (_font->ascent + _font->descent),
                     int(((_font->ascent + _font->descent) >> 4)))*_scale;
    return (l > 0) ? l : 1.;
}

// #### use Freetype to determine this
qreal QFontEngineXft::lineThickness() const
{
    // ad hoc algorithm
    int score = fontDef.weight * fontDef.pixelSize;
    int lw = score / 700;

    // looks better with thicker line for small pointsizes
    if (lw < 2 && score >= 1050)
        lw = 2;
    if (lw == 0)
        lw = 1;

    return lw;
}

qreal QFontEngineXft::maxCharWidth() const
{
    return _font->max_advance_width*_scale;
}

static const ushort char_table[] = {
        40,
        67,
        70,
        75,
        86,
        88,
        89,
        91,
        102,
        114,
        124,
        127,
        205,
        645,
        884,
        922,
        1070,
        12386
};

static const int char_table_entries = sizeof(char_table)/sizeof(ushort);


qreal QFontEngineXft::minLeftBearing() const
{
    if (lbearing == SHRT_MIN)
        (void) minRightBearing(); // calculates both
    return lbearing;
}

qreal QFontEngineXft::minRightBearing() const
{
    if (rbearing == SHRT_MIN) {
        QFontEngineXft *that = (QFontEngineXft *)this;
        that->lbearing = that->rbearing = 0;
        QChar *ch = (QChar *)char_table;
        QGlyphLayout glyphs[char_table_entries];
        int ng = char_table_entries;
        stringToCMap(ch, char_table_entries, glyphs, &ng, 0);
        while (--ng) {
            if (glyphs[ng].glyph) {
                glyph_metrics_t gi = that->boundingBox(glyphs[ng].glyph);
                that->lbearing = qMin(lbearing, gi.x);
                that->rbearing = qMin(rbearing, (gi.xoff - gi.x - gi.width));
            }
        }
    }
    return rbearing;
}

bool QFontEngineXft::canRender(const QChar *string, int len)
{
    bool allExist = true;
    if (_cmap != -1) {
        for ( int i = 0; i < len; i++ ) {
            unsigned int uc = getChar(string, i, len);
            if (!XftCharExists(0, _font, uc) && getAdobeCharIndex(_font, _cmap, uc) == 0) {
                allExist = false;
                break;
            }
        }
    } else {
        for ( int i = 0; i < len; i++ ) {
            unsigned int uc = getChar(string, i, len);
            if (!XftCharExists(0, _font, uc)) {
                allExist = false;
                break;
            }
        }
    }
    return allExist;
}

QOpenType *QFontEngineXft::openType() const
{
//     qDebug("openTypeIface requested!");
    if (_openType)
        return _openType;

    if (!_face || !FT_IS_SFNT(_face))
        return 0;

    QFontEngineXft *that = const_cast<QFontEngineXft *>(this);
    that->_openType = new QOpenType(that, that->_face);
    return _openType;
}

#endif // QT_NO_XFT
