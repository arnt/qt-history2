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
#include "qcolormap.h"

#include "qpaintengine_x11.h"

#include "qfont.h"
#include "qfontengine_p.h"
#include "qopentype_p.h"

#include <private/qpaintengine_x11_p.h>
#include <private/qpainter_p.h>
#include <private/qunicodetables_p.h>

#include <private/qt_x11_p.h>


#include <math.h>
#include <limits.h>
#define d d_func()
#define q q_func()

// defined in qfontdatbase_x11.cpp
extern int qt_mib_for_xlfd_encoding(const char *encoding);
extern int qt_xlfd_encoding_id(const char *encoding);

QFontEngine::~QFontEngine()
{
}

float QFontEngine::lineThickness() const
{
    // ad hoc algorithm
    int score = fontDef.weight * fontDef.pixelSize;
    int lw = score / 700;

    // looks better with thicker line for small pointsizes
    if (lw < 2 && score >= 1050) lw = 2;
    if (lw == 0) lw = 1;

    return lw;
}

float QFontEngine::underlinePosition() const
{
    return ((lineThickness() * 2) + 3) / 6;
}

// ------------------------------------------------------------------
// The box font engine
// ------------------------------------------------------------------


QFontEngineBox::QFontEngineBox(int size)
    : _size(size)
{
    cache_cost = sizeof(QFontEngineBox);
}

QFontEngineBox::~QFontEngineBox()
{
}

QFontEngine::FECaps QFontEngineBox::capabilites() const
{
    return FullTransformations;
}

bool QFontEngineBox::stringToCMap(const QChar *, int len, QGlyphLayout *glyphs, int *nglyphs, QTextEngine::ShaperFlags) const
{
    if (*nglyphs < len) {
        *nglyphs = len;
        return false;
    }

    memset(glyphs, 0, len * sizeof(QGlyphLayout));

    for (int i = 0; i < len; i++) {
        glyphs[i].advance.rx() = _size;
        glyphs[i].advance.ry() = 0;
    }

    *nglyphs = len;
    return true;
}

glyph_metrics_t QFontEngineBox::boundingBox(const QGlyphLayout *, int numGlyphs)
{
    glyph_metrics_t overall;
    overall.width = _size*numGlyphs;
    overall.height = _size;
    overall.xoff = overall.width;
    return overall;
}

glyph_metrics_t QFontEngineBox::boundingBox(glyph_t)
{
    return glyph_metrics_t(0, _size, _size, _size, _size, 0);
}



float QFontEngineBox::ascent() const
{
    return _size;
}

float QFontEngineBox::descent() const
{
    return 0;
}

float QFontEngineBox::leading() const
{
    float l = _size * 0.15;
    return ceil(l);
}

float QFontEngineBox::maxCharWidth() const
{
    return _size;
}

int QFontEngineBox::cmap() const
{
    return -1;
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




// ------------------------------------------------------------------
// Xlfd cont engine
// ------------------------------------------------------------------

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
        QVarLengthArray<glyph_t> g(len);
        _codec->fromUnicode(chars, g.data(), len);
        for (int i = 0; i < len; i++)
            glyphs[i].glyph = g[i];
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
    float ymax = 0.;
    float xmax = 0.;
    for (i = 0; i < numGlyphs; i++) {
        XCharStruct *xcs = charStruct(_fs, glyphs[i].glyph);
        if (xcs) {
            float x = overall.xoff + glyphs[i].offset.x() - xcs->lbearing;
            float y = overall.yoff + glyphs[i].offset.y() - xcs->ascent;
            overall.x = qMin(overall.x, x);
            overall.y = qMin(overall.y, y);
            xmax = qMax(xmax, overall.xoff + glyphs[i].offset.x() + xcs->rbearing);
            ymax = qMax(ymax, y + xcs->ascent + xcs->descent);
            overall.xoff += glyphs[i].advance.x()/_scale;
        } else {
            float size = _fs->ascent;
            overall.x = qMin(overall.x, overall.xoff);
            overall.y = qMin(overall.y, overall.yoff - size);
            ymax = qMax(ymax, overall.yoff);
            overall.xoff += size;
            xmax = qMax(xmax, overall.xoff);
        }
    }
    overall.height = ymax - overall.y;
    overall.width = xmax - overall.x;

    if (_scale != float(1)) {
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
        float size = ascent();
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


float QFontEngineXLFD::ascent() const
{
    return _fs->ascent*_scale;
}

float QFontEngineXLFD::descent() const
{
    return (_fs->descent-1)*_scale;
}

float QFontEngineXLFD::leading() const
{
    float l = (qMin(_fs->ascent, _fs->max_bounds.ascent)
                 + qMin(_fs->descent, _fs->max_bounds.descent)) * _scale * float(0.15);
    return ceil(l);
}

float QFontEngineXLFD::maxCharWidth() const
{
    return _fs->max_bounds.width*_scale;
}


// Loads the font for the specified script
static inline int maxIndex(XFontStruct *f) {
    return (((f->max_byte1 - f->min_byte1) *
             (f->max_char_or_byte2 - f->min_char_or_byte2 + 1)) +
            f->max_char_or_byte2 - f->min_char_or_byte2);
}

float QFontEngineXLFD::minLeftBearing() const
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

float QFontEngineXLFD::minRightBearing() const
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


QFontEngine::Type QFontEngineXLFD::type() const
{
    return XLFD;
}


// ------------------------------------------------------------------
// LatinXLFD engine
// ------------------------------------------------------------------

static const int engine_array_inc = 4;

QFontEngineLatinXLFD::QFontEngineLatinXLFD(XFontStruct *xfs, const char *name,
                                            int mib)
{
    _engines = new QFontEngine*[engine_array_inc];
    _engines[0] = new QFontEngineXLFD(xfs, name, mib);
    _count = 1;

    cache_cost = _engines[0]->cache_cost;

    memset(glyphIndices, 0, sizeof(glyphIndices));
    memset(glyphAdvances, 0, sizeof(glyphAdvances));
    euroIndex = 0;
    euroAdvance = 0;
}

QFontEngineLatinXLFD::~QFontEngineLatinXLFD()
{
    for (int i = 0; i < _count; ++i) {
        delete _engines[i];
        _engines[i] = 0;
    }
    delete [] _engines;
    _engines = 0;
}

QFontEngine::FECaps QFontEngineLatinXLFD::capabilites() const
{
    return FullTransformations;
}

void QFontEngineLatinXLFD::findEngine(const QChar &ch)
{
    if (ch.unicode() == 0) return;

    static const char *alternate_encodings[] = {
        "iso8859-1",
        "iso8859-2",
        "iso8859-3",
        "iso8859-4",
        "iso8859-9",
        "iso8859-10",
        "iso8859-13",
        "iso8859-14",
        "iso8859-15",
        "hp-roman8"
    };
    static const int mib_count = sizeof(alternate_encodings) / sizeof(const char *);

    // see if one of the above mibs can map the char we want
    QTextCodec *codec = 0;
    int which = -1;
    int i;
    for (i = 0; i < mib_count; ++i) {
        const int mib = qt_mib_for_xlfd_encoding(alternate_encodings[i]);
        bool skip = false;
        for (int e = 0; e < _count; ++e) {
            if (_engines[e]->cmap() == mib) {
                skip = true;
                break;
            }
        }
        if (skip) continue;

        codec = QTextCodec::codecForMib(mib);
        if (codec && codec->canEncode(ch)) {
            which = i;
            break;
        }
    }

    if (! codec || which == -1)
        return;

    const int enc_id = qt_xlfd_encoding_id(alternate_encodings[which]);
    QFontDef req = fontDef;
    QFontEngine *engine = QFontDatabase::findFont(QFont::Latin, 0, req, enc_id);
    if (! engine) {
        req.family = QString::null;
        engine = QFontDatabase::findFont(QFont::Latin, 0, req, enc_id);
        if (! engine) return;
    }
    engine->setScale(scale());

    if (! (_count % engine_array_inc)) {
        // grow the engines array
        QFontEngine **old = _engines;
        int new_size =
            (((_count+engine_array_inc) / engine_array_inc) * engine_array_inc);
        _engines = new QFontEngine*[new_size];
        for (i = 0; i < _count; ++i)
            _engines[i] = old[i];
        delete [] old;
    }

    _engines[_count] = engine;
    const int hi = _count << 8;
    ++_count;

    unsigned short chars[0x201];
    QGlyphLayout glyphs[0x201];
    for (i = 0; i < 0x200; ++i)
        chars[i] = i;
    chars[0x200] = 0x20ac;
    int glyphCount = 0x201;
    engine->stringToCMap((const QChar *) chars, 0x201, glyphs, &glyphCount, 0);

    // merge member data with the above
    for (i = 0; i < 0x200; ++i) {
        if (glyphIndices[i] != 0 || glyphs[i].glyph == 0) continue;
        glyphIndices[i] = hi | glyphs[i].glyph;
        glyphAdvances[i] = glyphs[i].advance.x();
    }
    if (!euroIndex && glyphs[0x200].glyph) {
        euroIndex = hi | glyphs[0x200].glyph;
        euroAdvance = glyphs[0x200].advance.x();
    }
}

bool
QFontEngineLatinXLFD::stringToCMap(const QChar *str, int len, QGlyphLayout *glyphs, int *nglyphs, QTextEngine::ShaperFlags flags) const
{
    if (*nglyphs < len) {
        *nglyphs = len;
        return false;
    }

    bool mirrored = flags & QTextEngine::RightToLeft;
    int i;
    bool missing = false;
    const QChar *c = str+len;
    QGlyphLayout *g = glyphs+len;
    float asc = ascent();
    if (mirrored) {
        while (c != str) {
            --c;
            --g;
            g->advance.ry() = 0;
            if (c->unicode() < 0x200) {
                unsigned short ch = ::mirroredChar(*c).unicode();
                g->glyph = glyphIndices[ch];
                g->advance.rx() = glyphAdvances[ch];
            } else {
                if (c->unicode() == 0x20ac) {
                    g->glyph = euroIndex;
                    g->advance.rx() = euroAdvance;
                } else {
                    g->glyph = 0;
                    g->advance.rx() = asc;
                }
            }
            missing = (missing || (g->glyph == 0));
        }
    } else {
        while (c != str) {
            --c;
            --g;
            g->advance.ry() = 0;
            if (c->unicode() < 0x200) {
                g->glyph = glyphIndices[c->unicode()];
                g->advance.rx() = glyphAdvances[c->unicode()];
            } else {
                if (c->unicode() == 0x20ac) {
                    g->glyph = euroIndex;
                    g->advance.rx() = euroAdvance;
                } else {
                    g->glyph = 0;
                    g->advance.rx() = asc;
                }
            }
            missing = (missing || (g->glyph == 0));
        }
    }

    if (missing) {
        for (i = 0; i < len; ++i) {
            unsigned short uc = str[i].unicode();
            if (glyphs[i].glyph != 0 || (uc >= 0x200 && uc != 0x20ac))
                continue;

            QFontEngineLatinXLFD *that = (QFontEngineLatinXLFD *) this;
            that->findEngine(str[i]);
            glyphs[i].glyph = (uc == 0x20ac ? euroIndex : that->glyphIndices[uc]);
            glyphs[i].advance.rx() = (uc == 0x20ac ? euroAdvance : glyphAdvances[uc]);
            glyphs[i].advance.ry() = 0;
        }
    }

    *nglyphs = len;
    return true;
}

glyph_metrics_t QFontEngineLatinXLFD::boundingBox(const QGlyphLayout *glyphs_const, int numGlyphs)
{
    if (numGlyphs <= 0) return glyph_metrics_t();

    glyph_metrics_t overall;

    QGlyphLayout *glyphs = const_cast<QGlyphLayout *>(glyphs_const);
    int which = glyphs[0].glyph >> 8;

    int start = 0;
    int end, i;
    for (end = 0; end < numGlyphs; ++end) {
        const int e = glyphs[end].glyph >> 8;
        if (e == which) continue;

        // set the high byte to zero
        for (i = start; i < end; ++i)
            glyphs[i].glyph = glyphs[i].glyph & 0xff;

        // merge the bounding box for this run
        const glyph_metrics_t gm =
            _engines[which]->boundingBox(glyphs + start, end - start);

        overall.x = qMin(overall.x, gm.x);
        overall.y = qMin(overall.y, gm.y);
        overall.width = overall.xoff + gm.width;
        overall.height = qMax(overall.height + overall.y, gm.height + gm.y) -
                         qMin(overall.y, gm.y);
        overall.xoff += gm.xoff;
        overall.yoff += gm.yoff;

        // reset the high byte for all glyphs
        const int hi = which << 8;
        for (i = start; i < end; ++i)
            glyphs[i].glyph = hi | glyphs[i].glyph;

        // change engine
        start = end;
        which = e;
    }

    // set the high byte to zero
    for (i = start; i < end; ++i)
        glyphs[i].glyph = glyphs[i].glyph & 0xff;

    // merge the bounding box for this run
    const glyph_metrics_t gm = _engines[which]->boundingBox(glyphs + start, end - start);

    overall.x = qMin(overall.x, gm.x);
    overall.y = qMin(overall.y, gm.y);
    overall.width = overall.xoff + gm.width;
    overall.height = qMax(overall.height + overall.y, gm.height + gm.y) -
                     qMin(overall.y, gm.y);
    overall.xoff += gm.xoff;
    overall.yoff += gm.yoff;

    // reset the high byte for all glyphs
    const int hi = which << 8;
    for (i = start; i < end; ++i)
        glyphs[i].glyph = hi | glyphs[i].glyph;

    return overall;
}

glyph_metrics_t QFontEngineLatinXLFD::boundingBox(glyph_t glyph)
{
    const int engine = glyph >> 8;
    Q_ASSERT(engine < _count);
    return _engines[engine]->boundingBox(glyph & 0xff);
}

float QFontEngineLatinXLFD::ascent() const
{
    return _engines[0]->ascent();
}

float QFontEngineLatinXLFD::descent() const
{
    return _engines[0]->descent();
}

float QFontEngineLatinXLFD::leading() const
{
    return _engines[0]->leading();
}

float QFontEngineLatinXLFD::maxCharWidth() const
{
    return _engines[0]->maxCharWidth();
}

float QFontEngineLatinXLFD::minLeftBearing() const
{
    return _engines[0]->minLeftBearing();
}

float QFontEngineLatinXLFD::minRightBearing() const
{
    return _engines[0]->minRightBearing();
}

const char *QFontEngineLatinXLFD::name() const
{
    return _engines[0]->name();
}

bool QFontEngineLatinXLFD::canRender(const QChar *string, int len)
{
    bool all = true;
    int i;
    for (i = 0; i < len; ++i) {
        if (string[i].unicode() >= 0x200 ||
             glyphIndices[string[i].unicode()] == 0) {
            if (string[i].unicode() != 0x20ac || euroIndex == 0)
                all = false;
            break;
        }
    }

    if (all)
        return true;

    all = true;
    for (i = 0; i < len; ++i) {
        if (string[i].unicode() >= 0x200) {
            if (string[i].unicode() == 0x20ac) {
                if (euroIndex)
                    continue;

                findEngine(string[i]);
                if (euroIndex)
                    continue;
            }
            all = false;
            break;
        }
        if (glyphIndices[string[i].unicode()] != 0) continue;

        findEngine(string[i]);
        if (glyphIndices[string[i].unicode()] == 0) {
            all = false;
            break;
        }
    }

    return all;
}

void QFontEngineLatinXLFD::setScale(double scale)
{
    QFontEngine::setScale(scale);
    int i;
    for (i = 0; i < _count; ++i)
        _engines[i]->setScale(scale);
    unsigned short chars[0x201];
    for (i = 0; i < 0x200; ++i)
        chars[i] = i;
    chars[0x200] = 0x20ac;
    int glyphCount = 0x201;
    QGlyphLayout glyphs[0x201];
    _engines[0]->stringToCMap((const QChar *)chars, 0x200, glyphs, &glyphCount, 0);
    for (int i = 0; i < 0x200; ++i) {
        glyphIndices[i] = glyphs[i].glyph;
        glyphAdvances[i] = glyphs[i].advance.x();
    }
    euroIndex = glyphs[0x200].glyph;
    euroAdvance = glyphs[0x200].advance.x();
}


// ------------------------------------------------------------------
// Xft cont engine
// ------------------------------------------------------------------
// #define FONTENGINE_DEBUG

#ifndef QT_NO_XFT

QFontEngineXft::QFontEngineXft(XftFont *font, XftPattern *pattern, int cmap)
    : _font(font), _pattern(pattern), _openType(0), _cmap(cmap), transformed_fonts(0)
{
    _face = XftLockFace(_font);

    _cmap = -1;
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
    if (XftPatternGetBool(pattern, XFT_ANTIALIAS,
			  0, &antialiased) == XftResultMatch &&
	! antialiased) {
        cache_cost /= 8;
    }
    lbearing = SHRT_MIN;
    rbearing = SHRT_MIN;

    memset(cmapCache, 0, sizeof(cmapCache));
    memset(widthCache, 0, sizeof(widthCache));
}

QFontEngineXft::~QFontEngineXft()
{
    delete _openType;
    XftUnlockFace(_font);

    XftFontClose(QX11Info::display(),_font);
    XftPatternDestroy(_pattern);
    _font = 0;
    _pattern = 0;
    TransformedFont *trf = transformed_fonts;
    while (trf) {
        XftFontClose(QX11Info::display(), trf->xft_font);
        TransformedFont *tmp = trf;
        trf = trf->next;
        delete tmp;
    }
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

bool QFontEngineXft::stringToCMap(const QChar *str, int len, QGlyphLayout *glyphs, int *nglyphs, QTextEngine::ShaperFlags flags) const
{
    if (*nglyphs < len) {
        *nglyphs = len;
        return false;
    }

    bool mirrored = flags & QTextEngine::RightToLeft;
    if (_cmap != -1) {
       for ( int i = 0; i < len; ++i ) {
           unsigned short uc = str[i].unicode();
           glyphs[i].glyph = uc < cmapCacheSize ? cmapCache[uc] : 0;
           if ( !glyphs[i].glyph ) {
               glyph_t glyph = XftCharIndex(0, _font, uc);
                if (!glyph)
                    glyph = getAdobeCharIndex(_font, _cmap, uc);
              glyphs[i].glyph = glyph;
               if ( uc < cmapCacheSize )
                   ((QFontEngineXft *)this)->cmapCache[uc] = glyph;
           }
       }
    } else if ( mirrored ) {
        for (int i = 0; i < len; ++i) {
            unsigned short uc = ::mirroredChar(str[i]).unicode();
            glyphs[i].glyph = uc < cmapCacheSize ? cmapCache[uc] : 0;
            if (!glyphs[i].glyph) {
                if (uc == 0xa0)
                    uc = 0x20;
                glyph_t glyph = XftCharIndex(0, _font, uc);
                glyphs[i].glyph = glyph;
                if (uc < cmapCacheSize)
                    ((QFontEngineXft *)this)->cmapCache[uc] = glyph;
            }
        }
    } else {
        for (int i = 0; i < len; ++i) {
            unsigned short uc = str[i].unicode();
            glyphs[i].glyph = uc < cmapCacheSize ? cmapCache[uc] : 0;
            if (!glyphs[i].glyph) {
                if (uc == 0xa0)
                    uc = 0x20;
                glyph_t glyph = XftCharIndex(0, _font, uc);
                glyphs[i].glyph = glyph;
                if (uc < cmapCacheSize)
                    ((QFontEngineXft *)this)->cmapCache[uc] = glyph;
            }
        }
    }

    recalcAdvances(len, glyphs, flags);

    *nglyphs = len;
    return true;
}

void QFontEngineXft::recalcAdvances(int len, QGlyphLayout *glyphs, QTextEngine::ShaperFlags flags) const
{
    if (flags & QTextEngine::DesignMetrics) {
        FT_Face face = XftLockFace(_font);
        for (int i = 0; i < len; i++) {
            FT_UInt glyph = glyphs[i].glyph;
            FT_Load_Glyph(face, glyph, FT_LOAD_NO_HINTING);
            glyphs[i].advance.rx() = face->glyph->metrics.horiAdvance/64.;
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
            if (glyph < widthCacheSize) {
                glyphs[i].advance.rx() = widthCache[glyph].x;
                glyphs[i].advance.ry() = widthCache[glyph].y;
            }
            if (!glyphs[i].advance.x()) {
                XGlyphInfo gi;
                XftGlyphExtents(QX11Info::display(), _font, &glyph, 1, &gi);
                glyphs[i].advance.rx() = gi.xOff;
                glyphs[i].advance.ry() = gi.yOff;
                if (glyph < widthCacheSize) {
                    widthCache[glyph].x = gi.xOff;
                    widthCache[glyph].y = gi.yOff;
                }
            }
        }
        if (_scale != 1.) {
            for (int i = 0; i < len; i++)
                glyphs[i].advance.rx() *= _scale;
        }
    }
}


XftFont *QFontEngineXft::transformedFont(const QMatrix &matrix)
{
    Q_ASSERT(_face->face_flags & FT_FACE_FLAG_SCALABLE);

    XftFont *fnt = 0;
    XftMatrix *mat = 0;
    XftPatternGetMatrix(_pattern, XFT_MATRIX, 0, &mat);
    XftMatrix m2;
    double scale = matrix.det();
    scale = sqrt(QABS(scale));
    m2.xx = matrix.m11()*_scale;
    m2.xy = -matrix.m21()*_scale;
    m2.yx = -matrix.m12()*_scale;
    m2.yy = matrix.m22()*_scale;

    // check if we have it cached
    TransformedFont *trf = transformed_fonts;
    TransformedFont *prev = 0;
    int i = 0;
    while (trf) {
        if (trf->xx == (float)m2.xx &&
            trf->xy == (float)m2.xy &&
            trf->yx == (float)m2.yx &&
            trf->yy == (float)m2.yy)
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
        if (scale > 0)
            XftMatrixScale(&m2, 1/scale, 1/scale);

        XftPattern *pattern = XftPatternDuplicate(_pattern);
        XftPatternDel(pattern, XFT_MATRIX);
        XftPatternAddMatrix(pattern, XFT_MATRIX, &m2);
        double size;
        XftPatternGetDouble(_pattern, XFT_PIXEL_SIZE, 0, &size);
        XftPatternDel(pattern, XFT_SIZE);
        XftPatternDel(pattern, XFT_PIXEL_SIZE);
//             qDebug("setting new size: orig=%f, scale=%f, new=%f", size, scale, size*scale);
        XftPatternAddDouble(pattern, XFT_PIXEL_SIZE, size*scale);

        fnt = XftFontOpenPattern(QX11Info::display(), pattern);
        TransformedFont *trf = new TransformedFont;
        trf->xx = (float)m2.xx;
        trf->xy = (float)m2.xy;
        trf->yx = (float)m2.yx;
        trf->yy = (float)m2.yy;
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
    float ymax = 0;
    float xmax = 0;
    for (int i = 0; i < numGlyphs; i++) {
        FT_UInt gl = glyphs[i].glyph;
        XftGlyphExtents(QX11Info::display(), _font, &gl, 1, &xgi);
        float x = overall.xoff + glyphs[i].offset.x() - xgi.x;
        float y = overall.yoff + glyphs[i].offset.y() - xgi.y;
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



float QFontEngineXft::ascent() const
{
    return (_font->ascent)*_scale;
}

float QFontEngineXft::descent() const
{
    return (_font->descent-1)*_scale;
}

// #### use Freetype to determine this
float QFontEngineXft::leading() const
{
    float l = qMin(_font->height - (_font->ascent + _font->descent),
                     int(((_font->ascent + _font->descent) >> 4)))*_scale;
    return (l > 0) ? l : 1.;
}

// #### use Freetype to determine this
float QFontEngineXft::lineThickness() const
{
    // ad hoc algorithm
    int score = fontDef.weight * fontDef.pixelSize;
    int lw = score / 700;

    // looks better with thicker line for small pointsizes
    if (lw < 2 && score >= 1050) lw = 2;
    if (lw == 0) lw = 1;

    return lw;
}

// #### use Freetype to determine this
float QFontEngineXft::underlinePosition() const
{
    return ((lineThickness() * 2) + 3) / 6;
}

float QFontEngineXft::maxCharWidth() const
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


float QFontEngineXft::minLeftBearing() const
{
    if (lbearing == SHRT_MIN)
        minRightBearing(); // calculates both

    return lbearing;
}

float QFontEngineXft::minRightBearing() const
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

int QFontEngineXft::cmap() const
{
    return _cmap;
}

const char *QFontEngineXft::name() const
{
    return "xft";
}

bool QFontEngineXft::canRender(const QChar *string, int len)
{
    bool allExist = true;

    if (_cmap != -1) {
        for ( int i = 0; i < len; i++ ) {
            if (!XftCharExists(0, _font, string[i].unicode())
                && getAdobeCharIndex(_font, _cmap, string[i].unicode()) == 0) {
                allExist = FALSE;
                break;
            }
        }
    } else {
        for ( int i = 0; i < len; i++ ) {
            if (!XftCharExists(0, _font, string[i].unicode())) {
                allExist = FALSE;
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

QFontEngine::Type QFontEngineXft::type() const
{
    return Xft;
}

#endif // QT_NO_XFT

