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
#include <qhash.h>

#include <private/qpainter_p.h>
#include <private/qunicodetables_p.h>

#include <private/qt_x11_p.h>

#include <qdebug.h>

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
                chars[i] = str[i].unicode();
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
            overall.xoff += glyphs[i].advance.x();
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

    return overall;
}

glyph_metrics_t QFontEngineXLFD::boundingBox(glyph_t glyph)
{
    glyph_metrics_t gm;
    XCharStruct *xcs = charStruct(_fs, glyph);
    if (xcs) {
        gm = glyph_metrics_t(xcs->lbearing, -xcs->ascent, xcs->rbearing- xcs->lbearing, xcs->ascent + xcs->descent,
                              xcs->width, 0);
    } else {
        qreal size = ascent();
        gm = glyph_metrics_t(0, size, size, size, size, 0);
    }
    return gm;
}


qreal QFontEngineXLFD::ascent() const
{
    return _fs->ascent;
}

qreal QFontEngineXLFD::descent() const
{
    return (_fs->descent-1);
}

qreal QFontEngineXLFD::leading() const
{
    qreal l = (qMin<int>(_fs->ascent, _fs->max_bounds.ascent)
                 + qMin<int>(_fs->descent, _fs->max_bounds.descent)) * qreal(0.15);
    return ceil(l);
}

qreal QFontEngineXLFD::maxCharWidth() const
{
    return _fs->max_bounds.width;
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
    return lbearing;
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
    return rbearing;
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


#ifndef QT_NO_FONTCONFIG

// ------------------------------------------------------------------
// Multi FT engine
// ------------------------------------------------------------------

#include FT_OUTLINE_H

QFontEngineMultiFT::QFontEngineMultiFT(FcFontSet *fs, int s)
    : QFontEngineMulti(fs->nfont), fontSet(fs), screen(s)
{
    loadEngine(0);
    cache_cost = 100;
}

QFontEngineMultiFT::~QFontEngineMultiFT()
{ FcFontSetDestroy(fontSet); }

void QFontEngineMultiFT::loadEngine(int at)
{
    Q_ASSERT(at < engines.size());
    Q_ASSERT(engines.at(at) == 0);
    FcPattern *pattern = fontSet->fonts[at];
    extern QFontDef FcPatternToQFontDef(FcPattern *pattern);
    QFontDef fontDef = FcPatternToQFontDef(fontSet->fonts[at]);
    // note: we use -1 for the script to make sure that we keep real
    // FT engines separate from Multi engines in the font cache
    QFontCache::Key key(fontDef, -1, screen);
    QFontEngine *fontEngine = QFontCache::instance->findEngine(key);
    if (!fontEngine) {
        FcConfigSubstitute(0, pattern, FcMatchPattern);
        FcDefaultSubstitute(pattern);
        FcResult res;
        FcPattern *match = FcFontMatch(0, pattern, &res);
        QFontEngineFT *engine = new QFontEngineFT(match, fontDef, screen);
        if (engine->invalid())
            delete engine;
        else
            fontEngine = engine;
        if (!fontEngine) {
            fontEngine = new QFontEngineBox(fontDef.pixelSize);
            fontEngine->fontDef = fontDef;
        }
        QFontCache::instance->insertEngine(key, fontEngine);
    }
    fontEngine->ref.ref();
    engines[at] = fontEngine;
}


// ------------------------------------------------------------------
// FT font engine
// ------------------------------------------------------------------

/*
 * Freetype 2.1.7 and earlier used width/height
 * for matching sizes in the BDF and PCF loaders.
 * This has been fixed for 2.1.8.
 */
#if HAVE_FT_BITMAP_SIZE_Y_PPEM
#define X_SIZE(face,i) ((face)->available_sizes[i].x_ppem)
#define Y_SIZE(face,i) ((face)->available_sizes[i].y_ppem)
#else
#define X_SIZE(face,i) ((face)->available_sizes[i].width << 6)
#define Y_SIZE(face,i) ((face)->available_sizes[i].height << 6)
#endif

void QFontEngineFT::computeSize()
{
    ysize = fontDef.pixelSize << 6;
    xsize = ysize * fontDef.stretch / 100;

    FT_Face face = freetype->face;
    /*
     * Bitmap only faces must match exactly, so find the closest
     * one (height dominant search)
     */
    if (!(face->face_flags & FT_FACE_FLAG_SCALABLE)) {
        int best = 0;
        for (int i = 1; i < face->num_fixed_sizes; i++) {
            if (qAbs(ysize -  Y_SIZE(face,i)) <
                qAbs (ysize - Y_SIZE(face, best)) ||
                (qAbs (ysize - Y_SIZE(face, i)) ==
                 qAbs (ysize - Y_SIZE(face, best)) &&
                 qAbs (xsize - X_SIZE(face, i)) <
                 qAbs (xsize - X_SIZE(face, best)))) {
                best = i;
            }
        }
        if (FT_Set_Char_Size (face, X_SIZE(face,  best), Y_SIZE(face, best), 0, 0) == 0) {
            xsize = X_SIZE(face, best);
            ysize = Y_SIZE(face, best);
        } else
            xsize = ysize = 0;
    }
}


static FT_Library library = 0;
QHash<QFreetypeFaceId, QFreetypeFace *> *QFontEngineFT::freetypeFaces = 0;

QFontEngineFT::Glyph::~Glyph()
{
    if (data) free(data);
}

static QFreetypeFaceId face_id(FcPattern *pattern)
{
    char *file_name;
    FcPatternGetString(pattern, FC_FILE, 0, (FcChar8 **)&file_name);
    int face_index;
    if (!FcPatternGetInteger(pattern, FC_INDEX, 0, &face_index))
        face_index = 0;
    QFreetypeFaceId face_id;
    face_id.filename = file_name;
    face_id.index = face_index;
    return face_id;
}

QFontEngineFT::QFontEngineFT(FcPattern *pattern, const QFontDef &fd, int screen)
{
    cache_cost = 100;
    fontDef = fd;
    _pattern = FcPatternDuplicate(pattern);
//     FcPatternPrint(pattern);

    antialias = X11->fc_antialias;
    FcBool b;
    if (FcPatternGetBool(pattern, FC_ANTIALIAS, 0, &b) == FcResultMatch)
        antialias = b;
    if (FcPatternGetInteger(pattern, FC_RGBA, 0, &subpixel) == FcResultNoMatch)
        subpixel = X11->screens[screen].subpixel;
    if (!antialias || subpixel == FC_RGBA_UNKNOWN)
        subpixel = FC_RGBA_NONE;

    if (!library)
        FT_Init_FreeType(&library);

    if (!freetypeFaces)
        freetypeFaces = new QHash<QFreetypeFaceId, QFreetypeFace *>();

    QFreetypeFaceId face_id = ::face_id(pattern);

    freetype = freetypeFaces->value(face_id, 0);
    if (!freetype) {
        freetype = new QFreetypeFace;
        freetype->ref = 0;
        freetype->lock = 0;
        freetype->xsize = 0;
        freetype->ysize = 0;
        FT_New_Face(library, face_id.filename, face_id.index, &freetype->face);
        freetypeFaces->insert(face_id, freetype);
    }
    freetype->ref.ref();

    lbearing = rbearing = SHRT_MIN;
    computeSize();
    outline_drawing = xsize > (64<<6) || ysize > (64<<6);

    lockFace();
    // #####
    line_thickness = underline_position = 1.;
    metrics = freetype->face->size->metrics;

    int load_flags = FT_LOAD_DEFAULT;
    int format = PictStandardA8;
    if (!antialias) {
        load_flags |= FT_LOAD_TARGET_MONO;
        format = PictStandardA1;
    } else {
        if (subpixel == FC_RGBA_RGB || subpixel == FC_RGBA_BGR) {
            load_flags |= FT_LOAD_TARGET_LCD;
            format = PictStandardARGB32;
        } else if (subpixel == FC_RGBA_VRGB || subpixel == FC_RGBA_VBGR) {
            load_flags |= FT_LOAD_TARGET_LCD_V;
            format = PictStandardARGB32;
        }
    }

    unlockFace();
    glyphSet = XRenderCreateGlyphSet(X11->display, XRenderFindStandardFormat(X11->display, format));
    _openType = 0;
}

QFontEngineFT::~QFontEngineFT()
{
    if (!freetype->ref.deref()) {
        FT_Done_Face(freetype->face);
        delete freetype;
        freetypeFaces->take(::face_id(_pattern));
    }
    if (!freetypeFaces->size()) {
        delete freetypeFaces;
        freetypeFaces = 0;
        FT_Done_FreeType(library);
        library = 0;
    }
    FcPatternDestroy(_pattern);
    _pattern = 0;

    XRenderFreeGlyphSet(X11->display, glyphSet);
    delete _openType;
}

FT_Face QFontEngineFT::lockFace() const
{
    Q_ASSERT(freetype->lock == 0);
    while (!freetype->lock.testAndSet(0, 1))
        usleep(100);
    FT_Face face = freetype->face;
    if (freetype->xsize != xsize && freetype->ysize != ysize) {
        FT_Set_Char_Size(face, xsize, ysize, 0, 0);
        freetype->xsize = xsize;
        freetype->ysize = ysize;
    }
    return face;
}

void QFontEngineFT::unlockFace() const
{
    if (!freetype->lock.testAndSet(1, 0))
        Q_ASSERT(false);
}

#define FLOOR(x)    ((x) & -64)
#define CEIL(x)	    (((x)+63) & -64)
#define TRUNC(x)    ((x) >> 6)
#define ROUND(x)    (((x)+32) & -64)

QFontEngineFT::Glyph *QFontEngineFT::loadGlyph(uint glyph, GlyphFormat format) const
{
    Q_ASSERT(freetype->lock == 1);

    bool add_to_glyphset = false;
    if (format == Format_None) {
        format = Format_Mono;
        if (X11->use_xrender) {
            add_to_glyphset = true;
            if (subpixel != FC_RGBA_NONE)
                format = Format_A32;
            else if (antialias)
                format = Format_A8;
        }
    }
    Q_ASSERT(format != Format_None);
    int hfactor = 1;
    int vfactor = 1;
    int load_flags = FT_LOAD_DEFAULT;
    if (outline_drawing) {
        load_flags = FT_LOAD_NO_BITMAP|FT_LOAD_NO_HINTING;
    } else if (format == Format_Mono) {
        load_flags |= FT_LOAD_TARGET_MONO;
    } else if (format == Format_A32) {
        if (subpixel == FC_RGBA_RGB || subpixel == FC_RGBA_BGR) {
            load_flags |= FT_LOAD_TARGET_LCD;
            hfactor = 3;
        } else if (subpixel == FC_RGBA_VRGB || subpixel == FC_RGBA_VBGR) {
            load_flags |= FT_LOAD_TARGET_LCD_V;
            vfactor = 3;
        }

    }

    {
        Glyph *g = glyph_data.value(glyph);
        if (g && g->format == format)
            return g;
    }

    FT_Face face = freetype->face;
    FT_Load_Glyph(face, glyph, load_flags);

    if (outline_drawing)
        return 0;

    FT_GlyphSlot slot = face->glyph;

    int left  = FLOOR(slot->metrics.horiBearingX);
    int right = CEIL(slot->metrics.horiBearingX + slot->metrics.width);
    int top    = CEIL(slot->metrics.horiBearingY);
    int bottom = FLOOR(slot->metrics.horiBearingY - slot->metrics.height);

    XGlyphInfo info;
    info.width = TRUNC(right - left);
    info.height = TRUNC(top - bottom);
    info.x = -TRUNC(left);
    info.y = TRUNC(top);
    info.xOff = TRUNC(ROUND(slot->advance.x));
    info.yOff = 0;

    int pitch = format == Format_Mono ? ((info.width + 31) & ~31) >> 3 : (info.width * hfactor + 3) & ~3;
    int size = pitch * info.height * vfactor;
    uchar *buffer = new uchar[size];
    memset (buffer, 0, size);

    if (slot->format == FT_GLYPH_FORMAT_OUTLINE) {
        FT_Bitmap bitmap;
        bitmap.rows = info.height*vfactor;
        bitmap.width = info.width*hfactor;
        bitmap.pitch = pitch;
        bitmap.buffer = buffer;
        bitmap.pixel_mode = format == Format_Mono ? ft_pixel_mode_mono : ft_pixel_mode_grays;
        FT_Matrix matrix;
        matrix.xx = hfactor << 16;
        matrix.yy = vfactor << 16;
        matrix.yx = matrix.xy = 0;

        FT_Outline_Transform(&slot->outline, &matrix);
        FT_Outline_Translate (&slot->outline, -left*hfactor, -bottom*vfactor);
        FT_Outline_Get_Bitmap(library, &slot->outline, &bitmap);
        if (hfactor != 1) {
            Q_ASSERT (bitmap.pixel_mode == FT_PIXEL_MODE_GRAY);
            Q_ASSERT(antialias);
            uchar *src = buffer;
            size = info.width * 4 * info.height;
            uchar *newBuf = new uchar[size];
            uint *dst = (uint *)newBuf;
            int h = info.height;
            const uint r = (subpixel == FC_RGBA_RGB || subpixel == FC_RGBA_VRGB) ? 16 : 0;
            const uint b = 16 - r;
            while (h--) {
                uint *dd = dst;
                for (int x = 0; x < bitmap.width; x += 3) {
                    // ############# filter
                    uint res = (src[x] << r) + (src[x+1] << 8) + (src[x+2] << b);
                    *dd = res;
                    ++dd;
                }
                dst += info.width;
                src += bitmap.pitch;
            }
            delete [] buffer;
            buffer = newBuf;
        } else if (vfactor != 1) {
            uchar *src = buffer;
            size = info.width * 4 * info.height;
            uchar *newBuf = new uchar[size];
            uint *dst = (uint *)newBuf;
            int h = info.height;
            const uint r = (subpixel == FC_RGBA_RGB || subpixel == FC_RGBA_VRGB) ? 16 : 0;
            const uint b = 16 - r;
            while (h--) {
                for (int x = 0; x < info.width; x++) {
                    // ############# filter
                    uint res = src[x] << r + src[x+bitmap.pitch] << 8 + src[x+2*bitmap.pitch] << b;
                    dst[x] = res;
                }
                dst += info.width;
                src += 3*bitmap.pitch;
            }
            delete [] buffer;
            buffer = newBuf;
        }
    } else if (slot->format == FT_GLYPH_FORMAT_BITMAP) {
        Q_ASSERT(slot->bitmap.pixel_mode == FT_PIXEL_MODE_MONO);
        uchar *src = slot->bitmap.buffer;
        uchar *dst = buffer;
        int h = slot->bitmap.rows;
        if (format == Format_Mono) {
            int bytes = ((info.width + 7) & ~7) >> 3;
            while (h--) {
                memcpy (dst, src, bytes);
                dst += pitch;
                src += slot->bitmap.pitch;
            }
        } else {
            if (hfactor != 1) {
                while (h--) {
                    uchar *dd = dst;
                    for (int x = 0; x < slot->bitmap.width; x++) {
                        unsigned char a = ((src[x >> 3] & (0x80 >> (x & 7))) ? 0xff : 0x00);
                        *dd++ = a;
                        *dd++ = a;
                        *dd++ = a;
                    }
                    dst += pitch * vfactor;
                    src += slot->bitmap.pitch;
                }

            } else if (vfactor != 1) {
                while (h--) {
                    for (int x = 0; x < slot->bitmap.width; x++) {
                        unsigned char a = ((src[x >> 3] & (0x80 >> (x & 7))) ? 0xff : 0x00);
                        dst[x] = a;
                    }
                    memcpy(dst + pitch, dst, pitch);
                    dst += pitch;
                    memcpy(dst + pitch, dst, pitch);
                    dst += pitch;
                    src += slot->bitmap.pitch;
                }
            } else {
                while (h--) {
                    for (int x = 0; x < slot->bitmap.width; x++) {
                        unsigned char a = ((src[x >> 3] & (0x80 >> (x & 7))) ? 0xff : 0x00);
                        dst[x] = a;
                    }
                    dst += pitch;
                    src += slot->bitmap.pitch;
                }
            }
        }
    } else {
        qWarning("glyph neither outline nor bitmap");
        return 0;
    }

#ifndef QT_NO_XRENDER
    if (add_to_glyphset) {
        if (format == Format_Mono) {
            /*
             * swap bit order around; FreeType is always MSBFirst
             */
            if (BitmapBitOrder(X11->display) != MSBFirst) {
                unsigned char *line = (unsigned char *) buffer;
                int i = size;
                i = size;
                while (i--) {
                    unsigned char c;
                    c = *line;
                    c = ((c << 1) & 0xaa) | ((c >> 1) & 0x55);
                    c = ((c << 2) & 0xcc) | ((c >> 2) & 0x33);
                    c = ((c << 4) & 0xf0) | ((c >> 4) & 0x0f);
                    *line++ = c;
                }
            }
        }

        ::Glyph xglyph = glyph;
        XRenderAddGlyphs (X11->display, glyphSet, &xglyph, &info, 1, (const char *)buffer, size);
        delete [] buffer;
        buffer = 0;
    }
#endif


    bool large_glyph = (((signed char)(slot->linearHoriAdvance>>16) != slot->linearHoriAdvance>>16)
                        || ((uchar)(info.width) != info.width)
                        || ((uchar)(info.height) != info.height)
                        || ((signed char)(info.x) != info.x)
                        || ((signed char)(info.y) != info.y)
                        || ((signed char)(info.xOff) != info.xOff));

    if (large_glyph) {
        qDebug("got a large glyph!");
        return 0;
    }

    Glyph *g = new Glyph;

    g->linearAdvance = slot->linearHoriAdvance >> 10;
    g->width = TRUNC(right - left);
    g->height = TRUNC(top - bottom);
    g->x = -TRUNC(left);
    g->y = TRUNC(top);
    g->advance = TRUNC(ROUND(slot->advance.x));
    g->format = add_to_glyphset ? Format_None : format;
    g->data = buffer;
    glyph_data[glyph] = g;

    return g;
}

#if 0
static inline glyph_t getAdobeCharIndex(FT_Face _face, int cmap, uint ucs4)
{
    // ############## save and restore charmap
    FT_Set_Charmap(_face, _face->charmaps[cmap]);
    glyph_t g = FT_Get_Char_Index(_face, ucs4);
    return g;
}
#endif

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

bool QFontEngineFT::stringToCMap(const QChar *str, int len, QGlyphLayout *glyphs, int *nglyphs,
                                 QTextEngine::ShaperFlags flags) const
{
    if (*nglyphs < len) {
        *nglyphs = len;
        return false;
    }

    bool mirrored = flags & QTextEngine::RightToLeft;
    int glyph_pos = 0;
#if 0
    if (_cmap != -1) {
        FT_Face _face = lockFace();
       for ( int i = 0; i < len; ++i ) {
           unsigned int uc = getChar(str, i, len);
           glyphs[glyph_pos].glyph = uc < cmapCacheSize ? cmapCache[uc] : 0;
           if ( !glyphs[glyph_pos].glyph ) {
               glyph_t glyph = FT_Get_Char_Index(_face, uc);
               if (!glyph)
                   glyph = getAdobeCharIndex(_face, _cmap, uc);
              glyphs[glyph_pos].glyph = glyph;
               if ( uc < cmapCacheSize )
                    ((QFontEngineFT *)this)->cmapCache[uc] = glyph;
           }
           ++glyph_pos;
       }
        unlockFace();
    } else
#endif
    {
        FT_Face face = freetype->face;
        for (int i = 0; i < len; ++i) {
            unsigned int uc = getChar(str, i, len);
            if (mirrored)
                uc = QUnicodeTables::mirroredChar(uc);
            redo:
            glyph_t glyph = FT_Get_Char_Index(face, uc);
                if (!glyph && (uc == 0xa0 || uc == 0x9)) {
                    uc = 0x20;
                    goto redo;
                }
                glyphs[glyph_pos].glyph = glyph;
            ++glyph_pos;
        }
    }

    *nglyphs = glyph_pos;
    recalcAdvances(*nglyphs, glyphs, flags);

    return true;
}

void QFontEngineFT::recalcAdvances(int len, QGlyphLayout *glyphs, QTextEngine::ShaperFlags flags) const
{
    FT_Face face = 0;
    if (flags & QTextEngine::DesignMetrics) {
        for (int i = 0; i < len; i++) {
            Glyph *g = glyph_data.value(glyphs[i].glyph);
            if (!g) {
                if (!face)
                    face = lockFace();
                g = loadGlyph(glyphs[i].glyph);
            }
            // for uncachable glyph, get advance from glyphslot
            glyphs[i].advance.rx() = g ? g->linearAdvance/65536. : face->glyph->linearHoriAdvance/65536.;
            glyphs[i].advance.ry() = 0.;
        }
    } else {
        for (int i = 0; i < len; i++) {
            Glyph *g = glyph_data.value(glyphs[i].glyph);
            if (!g) {
                if (!face)
                    face = lockFace();
                g = loadGlyph(glyphs[i].glyph);
    }
            // for uncachable glyph, get advance from glyphslot
            glyphs[i].advance.rx() = g ? g->advance : face->glyph->metrics.horiAdvance/64.;
            glyphs[i].advance.ry() = 0.;
        }
    }
    if (face)
        unlockFace();
}

glyph_metrics_t QFontEngineFT::boundingBox(const QGlyphLayout *glyphs,  int numGlyphs)
{

    FT_Face face = 0;

    glyph_metrics_t overall;
    qreal ymax = 0;
    qreal xmax = 0;
    for (int i = 0; i < numGlyphs; i++) {
        Glyph *g = glyph_data.value(glyphs[i].glyph);
        if (!g) {
            if (!face)
                face = lockFace();
            g = loadGlyph(glyphs[i].glyph);
        }
        if (g) {
            qreal x = overall.xoff + glyphs[i].offset.x() - g->x;
            qreal y = overall.yoff + glyphs[i].offset.y() - g->y;
            overall.x = qMin(overall.x, x);
            overall.y = qMin(overall.y, y);
            xmax = qMax(xmax, x + g->width);
            ymax = qMax(ymax, y + g->height);
            overall.xoff += qRound(g->advance);
        } else {
            int left  = FLOOR(face->glyph->metrics.horiBearingX);
            int right = CEIL(face->glyph->metrics.horiBearingX + face->glyph->metrics.width);
            int top    = CEIL(face->glyph->metrics.horiBearingY);
            int bottom = FLOOR(face->glyph->metrics.horiBearingY - face->glyph->metrics.height);

            qreal x = overall.xoff + glyphs[i].offset.x() - (-TRUNC(left));
            qreal y = overall.yoff + glyphs[i].offset.y() - TRUNC(top);
            overall.x = qMin(overall.x, x);
            overall.y = qMin(overall.y, y);
            xmax = qMax(xmax, x + TRUNC(right - left));
            ymax = qMax(ymax, y + TRUNC(top - bottom));
            overall.xoff += qRound(TRUNC(ROUND(face->glyph->advance.x)));
        }
    }
    overall.height = ymax - overall.y;
    overall.width = xmax - overall.x;

    if (face)
        unlockFace();

    return overall;
}

glyph_metrics_t QFontEngineFT::boundingBox(glyph_t glyph)
{
    FT_Face face = 0;
    glyph_metrics_t overall;
    Glyph *g = glyph_data.value(glyph);
    if (!g) {
        face = lockFace();
        g = loadGlyph(glyph);
    }
    if (g) {
        overall.x = g->x;
        overall.y = g->y;
        overall.width = g->width;
        overall.height = g->height;
        overall.xoff = g->advance;
    } else {
        int left  = FLOOR(face->glyph->metrics.horiBearingX);
        int right = CEIL(face->glyph->metrics.horiBearingX + face->glyph->metrics.width);
        int top    = CEIL(face->glyph->metrics.horiBearingY);
        int bottom = FLOOR(face->glyph->metrics.horiBearingY - face->glyph->metrics.height);

        overall.width = TRUNC(right-left);
        overall.height = TRUNC(top-bottom);
        overall.x = -TRUNC(left);
        overall.y = TRUNC(top);
        overall.xoff = TRUNC(ROUND(face->glyph->advance.x));
    }
    if (face)
        unlockFace();
    return overall;
}

bool QFontEngineFT::canRender(const QChar *string,  int len)
{
    FT_Face face = freetype->face;
#if 0
    if (_cmap != -1) {
        lockFace();
        for ( int i = 0; i < len; i++ ) {
            unsigned int uc = getChar(string, i, len);
            if (!FcCharSetHasChar (_font->charset, uc) && getAdobeCharIndex(face, _cmap, uc) == 0) {
                allExist = false;
                break;
            }
        }
        unlockFace();
    } else
#endif
    {
        for ( int i = 0; i < len; i++ ) {
            unsigned int uc = getChar(string, i, len);
            if (!FT_Get_Char_Index(face, uc))
                    return false;
        }
    }
    return true;
}

void QFontEngineFT::doKerning(int num_glyphs, QGlyphLayout *g, QTextEngine::ShaperFlags flags) const
{
    if (!FT_HAS_KERNING(freetype->face))
        return;
    FT_Face face = lockFace();
    uint f = (flags == QTextEngine::DesignMetrics ? FT_KERNING_UNFITTED : FT_KERNING_DEFAULT);
    for (int i = 0; i < num_glyphs-1; ++i) {
        FT_Vector kerning;
        FT_Get_Kerning(face, g[i].glyph, g[i+1].glyph, f, &kerning);
        g[i].advance.rx() += kerning.x / 64.;
        g[i].advance.ry() += kerning.y / 64.;
    }
    unlockFace();
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

void QFontEngineFT::addOutlineToPath(qreal x, qreal y, const QGlyphLayout *glyphs, int numGlyphs, QPainterPath *path)
{
    if (FT_IS_SCALABLE(freetype->face)) {
        FT_Face face = lockFace();
        QPointF point = QPointF(x, y);
        for (int gl = 0; gl < numGlyphs; gl++) {
            FT_UInt glyph = glyphs[gl].glyph;
            QPointF cp = point + glyphs[gl].offset;
            point += glyphs[gl].advance;

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
        unlockFace();
    } else {
        addBitmapFontToPath(x, y, glyphs, numGlyphs, path);
    }
}

qreal QFontEngineFT::ascent() const
{
    return TRUNC(ROUND(metrics.ascender));
}

qreal QFontEngineFT::descent() const
{
    return -TRUNC(ROUND(metrics.descender)) + 1;
}

qreal QFontEngineFT::leading() const
{
    return (metrics.height - metrics.ascender + metrics.descender) >> 6;
}

qreal QFontEngineFT::maxCharWidth() const
{
    return metrics.max_advance >> 6;
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


qreal QFontEngineFT::minLeftBearing() const
{
    if (lbearing == SHRT_MIN)
        (void) minRightBearing(); // calculates both
    return lbearing;
}

qreal QFontEngineFT::minRightBearing() const
{
    if (rbearing == SHRT_MIN) {
        lbearing = rbearing = 0;
        const QChar *ch = (const QChar *)char_table;
        QGlyphLayout glyphs[char_table_entries];
        int ng = char_table_entries;
        stringToCMap(ch, char_table_entries, glyphs, &ng, 0);
        while (--ng) {
            if (glyphs[ng].glyph) {
                glyph_metrics_t gi = ((QFontEngineFT *)this)->boundingBox(glyphs[ng].glyph);
                lbearing = qMin(lbearing, gi.x);
                rbearing = qMin(rbearing, (gi.xoff - gi.x - gi.width));
            }
        }
    }
    return rbearing;
}

qreal QFontEngineFT::lineThickness() const
{
    return line_thickness;
}


QOpenType *QFontEngineFT::openType() const
{
    if (_openType)
         return _openType;

    FT_Face face = lockFace();
    if (!face || !FT_IS_SFNT(face))
        return 0;

    _openType = new QOpenType(const_cast<QFontEngineFT *>(this), face);
    unlockFace();
    return _openType;
}



#endif // QT_NO_FONTCONFIG
