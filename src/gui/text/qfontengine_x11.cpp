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

#include "qbitmap.h"

// #define FONTENGINE_DEBUG

#include <qbytearray.h>
#include <qtextcodec.h>

#include "qfontdatabase.h"
#include "qpaintdevice.h"
#include "qpainter.h"
#include "qvarlengtharray.h"
#include "qwidget.h"

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

extern void qt_draw_transformed_rect(QPaintEngine *p, int x, int y, int w, int h, bool fill);

static void drawLines(QPaintEngine *p, QFontEngine *fe, int baseline, int x1, int w, int textFlags)
{
    int lw = fe->lineThickness().toInt();
    if (textFlags & Qt::Underline) {
        int pos = fe->underlinePosition().toInt();
        qt_draw_transformed_rect(p, x1, baseline+pos, w, lw, true);
    }
    if (textFlags & Qt::Overline) {
        int pos = fe->ascent().toInt()+1;
        if (!pos) pos = 1;
        qt_draw_transformed_rect(p, x1, baseline-pos, w, lw, true);
    }
    if (textFlags & Qt::StrikeOut) {
        int pos = fe->ascent().toInt()/3;
        if (!pos) pos = 1;
        qt_draw_transformed_rect(p, x1, baseline-pos, w, lw, true);
    }
}

QFontEngine::~QFontEngine()
{
}

Q26Dot6 QFontEngine::lineThickness() const
{
    // ad hoc algorithm
    int score = fontDef.weight * fontDef.pixelSize;
    int lw = score / 700;

    // looks better with thicker line for small pointsizes
    if (lw < 2 && score >= 1050) lw = 2;
    if (lw == 0) lw = 1;

    return lw;
}

Q26Dot6 QFontEngine::underlinePosition() const
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
        glyphs[i].advance.x = _size;
        glyphs[i].advance.y = 0;
    }

    *nglyphs = len;
    return true;
}

void QFontEngineBox::draw(QPaintEngine *p, int x, int y, const QTextItem &si, int textFlags)
{
    Display *dpy = QX11Info::appDisplay();
    Qt::HANDLE hd = p->handle();
    GC gc = static_cast<QX11PaintEngine *>(p)->d->gc;

    if (p->painterState()->txop > QPainter::TxTranslate) {
        int xp = x;
        int yp = _size + 2;
        int s = _size - 3;
        for (int k = 0; k < si.num_glyphs; k++) {
            qt_draw_transformed_rect(p, xp, yp, s, s, false);
            xp += _size;
        }
    } else {
        if (p->painterState()->txop == QPainter::TxTranslate)
            p->painter()->map(x, y, &x, &y);
        XRectangle rects[64];

        int gl = 0;
        while (gl < si.num_glyphs) {
            int toDraw = qMin(64, si.num_glyphs-gl);
            int adv = toDraw*_size;
            if (x + adv < SHRT_MAX && x > SHRT_MIN) {
                for (int k = 0; k < toDraw; k++) {
                    rects[k].x = x + (k * _size);
                    rects[k].y = y - _size + 2;
                    rects[k].width = rects[k].height = _size - 3;
                }
                XDrawRectangles(dpy, hd, gc, rects, toDraw);
            }
            gl += toDraw;
            x += adv;
        }
    }

    if (textFlags != 0)
        drawLines(p, this, y, x, si.num_glyphs*_size, textFlags);
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
    Q26Dot6 l = Q26Dot6(_size * 0.15);
    return l.ceil();
}

Q26Dot6 QFontEngineBox::maxCharWidth() const
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
    XFreeFont(QX11Info::appDisplay(), _fs);
    _fs = 0;
}

QFontEngine::FECaps QFontEngineXLFD::capabilites() const
{
    return NoTransformations;
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
            g->advance.x = xcs->width;
            g->advance.y = 0;
        }
    }
    else if (!_fs->max_byte1) {
        XCharStruct *base = _fs->per_char - _fs->min_char_or_byte2;
        while (g != glyphs) {
            unsigned int gl = (--g)->glyph;
            xcs = (gl >= _fs->min_char_or_byte2 && gl <= _fs->max_char_or_byte2) ?
                  base + gl : 0;
            g->advance.x = (!xcs || (!xcs->width && !xcs->ascent && !xcs->descent)) ? _fs->ascent : xcs->width;
            g->advance.y = 0;
        }
    }
    else {
        while (g != glyphs) {
            xcs = charStruct(_fs, (--g)->glyph);
            g->advance.x = xcs ? xcs->width : _fs->ascent;
            g->advance.y = 0;
        }
    }
    if (_scale != Q26Dot6(1)) {
        for (int i = 0; i < len; i++)
            glyphs[i].advance.x *= _scale;
    }
    return true;
}

void QFontEngineXLFD::draw(QPaintEngine *p, int xpos, int ypos, const QTextItem &si, int textFlags)
{
    if (!si.num_glyphs)
        return;

    // since we advocate we can't do translations this should hold.
    Q_ASSERT(p->painterState()->txop <= QPainter::TxTranslate);

//     qDebug("QFontEngineXLFD::draw(%d, %d, numglyphs=%d", x, y, si.num_glyphs);

    Display *dpy = QX11Info::appDisplay();
    Qt::HANDLE hd = p->handle();
    GC gc = static_cast<QX11PaintEngine *>(p)->d->gc;

    int xorig = xpos;
    int yorig = ypos;

    Qt::HANDLE font_id = _fs->fid;
    if (p->painterState()->txop == QPainter::TxTranslate)
        p->painter()->map(xpos, ypos, &xpos, &ypos);

    Q26Dot6 x(xpos);
    Q26Dot6 y(ypos);

    XSetFont(dpy, gc, font_id);

#ifdef FONTENGINE_DEBUG
    p->save();
    p->setBrush(Qt::white);
    glyph_metrics_t ci = boundingBox(glyphs, si.num_glyphs);
    p->drawRect(x + ci.x, y + ci.y, ci.width, ci.height);
    p->drawRect(x + ci.x, y + 100 + ci.y, ci.width, ci.height);
    qDebug("bounding rect=%d %d (%d/%d)", ci.x, ci.y, ci.width, ci.height);
    p->restore();
    int xp = x;
    int yp = y;
#endif

    QGlyphLayout *glyphs = si.glyphs;

    QVarLengthArray<XChar2b> chars(si.num_glyphs);

    for (int i = 0; i < si.num_glyphs; i++) {
        chars[i].byte1 = glyphs[i].glyph >> 8;
        chars[i].byte2 = glyphs[i].glyph & 0xff;
    }

    if (si.right_to_left) {
        int i = si.num_glyphs;
        while(i--) {
            x += glyphs[i].advance.x + Q26Dot6(glyphs[i].space_18d6, F26Dot6);
            y += glyphs[i].advance.y;
        }
        i = 0;
        while(i < si.num_glyphs) {
            x -= glyphs[i].advance.x;
            y -= glyphs[i].advance.y;

            int xp = (x+glyphs[i].offset.x).toInt();
            int yp = (y+glyphs[i].offset.y).toInt();
            if (xp < SHRT_MAX && xp > SHRT_MIN)
                XDrawString16(dpy, hd, gc, xp, yp, chars.data()+i, 1);

            if (glyphs[i].nKashidas) {
                QChar ch(0x640); // Kashida character
                QGlyphLayout g[8];
                int nglyphs = 7;
                stringToCMap(&ch, 1, g, &nglyphs, 0);
                for (uint k = 0; k < glyphs[i].nKashidas; ++k) {
                    x -= g[0].advance.x;
                    y -= g[0].advance.y;

                    int xp = (x+g[0].offset.x).toInt();
                    int yp = (y+g[0].offset.y).toInt();
                    if (xp < SHRT_MAX && xp > SHRT_MIN)
                        XDrawString16(dpy, hd, gc, xp, yp, chars.data()+i, 1);
                }
            } else {
                x -= Q26Dot6(glyphs[i].space_18d6, F26Dot6);
            }
            ++i;
        }
    } else {
        int i = 0;
        while(i < si.num_glyphs) {
            int xp = (x+glyphs[i].offset.x).toInt();
            int yp = (y+glyphs[i].offset.y).toInt();
            if (xp < SHRT_MAX && xp > SHRT_MIN)
                XDrawString16(dpy, hd, gc, xp, yp, chars.data()+i, 1);
            x += glyphs[i].advance.x + Q26Dot6(glyphs[i].space_18d6, F26Dot6);
            y += glyphs[i].advance.y;
            i++;
        }
    }

    if (textFlags != 0)
        drawLines(p, this, yorig, xorig, x.toInt()-xpos, textFlags);

#ifdef FONTENGINE_DEBUG
    x = xp;
    y = yp;
    p->save();
    p->setPen(Qt::red);
    for (int i = 0; i < si.num_glyphs; i++) {
        glyph_metrics_t ci = boundingBox(glyphs[i].glyph);
        p->drawRect(x + ci.x + glyphs[i].offset.x, y + 100 + ci.y + glyphs[i].offset.y, ci.width, ci.height);
        qDebug("bounding ci[%d]=%d %d (%d/%d) / %d %d   offs=(%d/%d) advance=(%d/%d)", i, ci.x, ci.y, ci.width, ci.height,
               ci.xoff, ci.yoff, glyphs[i].offset.x, glyphs[i].offset.y,
               glyphs[i].advance.x, glyphs[i].advance.y);
        x += glyphs[i].advance.x;
        y += glyphs[i].advance.y;
    }
    p->restore();
#endif
}

glyph_metrics_t QFontEngineXLFD::boundingBox(const QGlyphLayout *glyphs, int numGlyphs)
{
    int i;

    glyph_metrics_t overall;
    Q26Dot6 ymax;
    Q26Dot6 xmax;
    for (i = 0; i < numGlyphs; i++) {
        XCharStruct *xcs = charStruct(_fs, glyphs[i].glyph);
        if (xcs) {
            Q26Dot6 x = overall.xoff + glyphs[i].offset.x - xcs->lbearing;
            Q26Dot6 y = overall.yoff + glyphs[i].offset.y - xcs->ascent;
            overall.x = qMin(overall.x, x);
            overall.y = qMin(overall.y, y);
            xmax = qMax(xmax, overall.xoff + glyphs[i].offset.x + xcs->rbearing);
            ymax = qMax(ymax, y + xcs->ascent + xcs->descent);
            overall.xoff += glyphs[i].advance.x;
        } else {
            Q26Dot6 size = ascent();
            overall.x = qMin(overall.x, overall.xoff);
            overall.y = qMin(overall.y, overall.yoff - size);
            ymax = qMax(ymax, overall.yoff);
            overall.xoff += size;
            xmax = qMax(xmax, overall.xoff);
        }
    }
    overall.height = ymax - overall.y;
    overall.width = xmax - overall.x;

    if (_scale != Q26Dot6(1)) {
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
        Q26Dot6 size = ascent();
        gm = glyph_metrics_t(0, size, size, size, size, 0);
    }
    if (_scale != Q26Dot6(1)) {
        gm.x *= _scale;
        gm.y *= _scale;
        gm.height *= _scale;
        gm.width *= _scale;
        gm.xoff *= _scale;
        gm.yoff *= _scale;
    }
    return gm;
}


Q26Dot6 QFontEngineXLFD::ascent() const
{
    return _fs->ascent*_scale;
}

Q26Dot6 QFontEngineXLFD::descent() const
{
    return (_fs->descent-1)*_scale;
}

Q26Dot6 QFontEngineXLFD::leading() const
{
    Q26Dot6 l = (qMin(_fs->ascent, _fs->max_bounds.ascent)
                 + qMin(_fs->descent, _fs->max_bounds.descent)) * _scale * Q26Dot6(0.15);
    return l.ceil();
}

Q26Dot6 QFontEngineXLFD::maxCharWidth() const
{
    return _fs->max_bounds.width*_scale;
}


// Loads the font for the specified script
static inline int maxIndex(XFontStruct *f) {
    return (((f->max_byte1 - f->min_byte1) *
             (f->max_char_or_byte2 - f->min_char_or_byte2 + 1)) +
            f->max_char_or_byte2 - f->min_char_or_byte2);
}

Q26Dot6 QFontEngineXLFD::minLeftBearing() const
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

Q26Dot6 QFontEngineXLFD::minRightBearing() const
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
    euroIndex = 0;
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
    return NoTransformations;
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
        glyphAdvances[i] = glyphs[i].advance.x;
    }
    if (!euroIndex && glyphs[0x200].glyph) {
        euroIndex = hi | glyphs[0x200].glyph;
        euroAdvance = glyphs[0x200].advance.x;
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
    Q26Dot6 asc = ascent();
    if (mirrored) {
        while (c != str) {
            --c;
            --g;
            g->advance.y = 0;
            if (c->unicode() < 0x200) {
                unsigned short ch = ::mirroredChar(*c).unicode();
                g->glyph = glyphIndices[ch];
                g->advance.x = glyphAdvances[ch];
            } else {
                if (c->unicode() == 0x20ac) {
                    g->glyph = euroIndex;
                    g->advance.x = euroAdvance;
                } else {
                    g->glyph = 0;
                    g->advance.x = asc;
                }
            }
            missing = (missing || (g->glyph == 0));
        }
    } else {
        while (c != str) {
            --c;
            --g;
            g->advance.y = 0;
            if (c->unicode() < 0x200) {
                g->glyph = glyphIndices[c->unicode()];
                g->advance.x = glyphAdvances[c->unicode()];
            } else {
                if (c->unicode() == 0x20ac) {
                    g->glyph = euroIndex;
                    g->advance.x = euroAdvance;
                } else {
                    g->glyph = 0;
                    g->advance.x = asc;
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
            glyphs[i].advance.x = (uc == 0x20ac ? euroAdvance : glyphAdvances[uc]);
            glyphs[i].advance.y = 0;
        }
    }

    *nglyphs = len;
    return true;
}

void QFontEngineLatinXLFD::draw(QPaintEngine *p, int xpos, int y, const QTextItem &si, int textFlags)
{
    if (!si.num_glyphs) return;

    QGlyphLayout *glyphs = si.glyphs;
    int which = glyphs[0].glyph >> 8;

    Q26Dot6 x(xpos);

    int start = 0;
    int end, i;
    for (end = 0; end < si.num_glyphs; ++end) {
        const int e = glyphs[end].glyph >> 8;
        if (e == which) continue;

        // set the high byte to zero
        for (i = start; i < end; ++i)
            glyphs[i].glyph = glyphs[i].glyph & 0xff;

        // draw the text
        QTextItem si2 = si;
        si2.glyphs = si.glyphs + start;
        si2.num_glyphs = end - start;
        _engines[which]->draw(p, x.toInt(), y, si2, textFlags);

        // reset the high byte for all glyphs and advance to the next sub-string
        const int hi = which << 8;
        for (i = start; i < end; ++i) {
            glyphs[i].glyph = hi | glyphs[i].glyph;
            x += glyphs[i].advance.x;
        }

        // change engine
        start = end;
        which = e;
    }

    // set the high byte to zero
    for (i = start; i < end; ++i)
        glyphs[i].glyph = glyphs[i].glyph & 0xff;

    // draw the text
    QTextItem si2 = si;
    si2.glyphs = si.glyphs + start;
    si2.num_glyphs = end - start;
    _engines[which]->draw(p, x.toInt(), y, si2, textFlags);

    // reset the high byte for all glyphs
    const int hi = which << 8;
    for (i = start; i < end; ++i)
        glyphs[i].glyph = hi | glyphs[i].glyph;
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

Q26Dot6 QFontEngineLatinXLFD::ascent() const
{
    return _engines[0]->ascent();
}

Q26Dot6 QFontEngineLatinXLFD::descent() const
{
    return _engines[0]->descent();
}

Q26Dot6 QFontEngineLatinXLFD::leading() const
{
    return _engines[0]->leading();
}

Q26Dot6 QFontEngineLatinXLFD::maxCharWidth() const
{
    return _engines[0]->maxCharWidth();
}

Q26Dot6 QFontEngineLatinXLFD::minLeftBearing() const
{
    return _engines[0]->minLeftBearing();
}

Q26Dot6 QFontEngineLatinXLFD::minRightBearing() const
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
        glyphAdvances[i] = glyphs[i].advance.x;
    }
    euroIndex = glyphs[0x200].glyph;
    euroAdvance = glyphs[0x200].advance.x;
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
}

QFontEngineXft::~QFontEngineXft()
{
    delete _openType;
    XftUnlockFace(_font);

    XftFontClose(QX11Info::appDisplay(),_font);
    XftPatternDestroy(_pattern);
    _font = 0;
    _pattern = 0;
    TransformedFont *trf = transformed_fonts;
    while (trf) {
        XftFontClose(QX11Info::appDisplay(), trf->xft_font);
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

static Q26Dot6Offset map(const QWMatrix &m, Q26Dot6Offset &pos)
{
    Q26Dot6Offset ret;
    ret.x = m.m11()*pos.x + m.m21()*pos.y;
    ret.y = m.m12()*pos.x + m.m22()*pos.y;
    return ret;
}

void QFontEngineXft::recalcAdvances(int len, QGlyphLayout *glyphs, QTextEngine::ShaperFlags flags) const
{
    if (flags & QTextEngine::DesignMetrics) {
        FT_Face face = XftLockFace(_font);
        for (int i = 0; i < len; i++) {
            FT_UInt glyph = glyphs[i].glyph;
            FT_Load_Glyph(face, glyph, FT_LOAD_NO_HINTING);
            glyphs[i].advance.x.setValue(face->glyph->metrics.horiAdvance);
            glyphs[i].advance.y.setValue(0);
        }
        XftUnlockFace(_font);
        if (_scale != Q26Dot6(1)) {
            for (int i = 0; i < len; i++)
                glyphs[i].advance.x *= _scale;
        }
    } else {
        for (int i = 0; i < len; i++) {
            FT_UInt glyph = glyphs[i].glyph;
            if (glyph < widthCacheSize) {
                glyphs[i].advance.x = widthCache[glyph].x;
                glyphs[i].advance.y = widthCache[glyph].y;
            }
            if (!glyphs[i].advance.x) {
                XGlyphInfo gi;
                XftGlyphExtents(QX11Info::appDisplay(), _font, &glyph, 1, &gi);
                glyphs[i].advance.x = gi.xOff;
                glyphs[i].advance.y = gi.yOff;
                if (glyph < widthCacheSize) {
                    widthCache[glyph].x = gi.xOff;
                    widthCache[glyph].y = gi.yOff;
                }
            }
        }
        if (_scale != Q26Dot6(1)) {
            for (int i = 0; i < len; i++)
                glyphs[i].advance.x *= _scale;
        }
    }
}


//#define FONTENGINE_DEBUG
void QFontEngineXft::draw(QPaintEngine *p, int xpos, int ypos, const QTextItem &si, int textFlags)
{
    if (!si.num_glyphs)
        return;

    Display *dpy = QX11Info::appDisplay();

    int xorig = xpos;
    int yorig = ypos;

    XftFont *fnt = _font;
    bool transform = false;
    if (p->painterState()->txop >= QPainter::TxScale) {
        Q_ASSERT(_face->face_flags & FT_FACE_FLAG_SCALABLE);

        XftMatrix *mat = 0;
        XftPatternGetMatrix(_pattern, XFT_MATRIX, 0, &mat);
        XftMatrix m2;
        double scale = p->painterState()->worldMatrix.det();
        scale = sqrt(QABS(scale));
        m2.xx = p->painterState()->worldMatrix.m11()*_scale.toDouble();
        m2.xy = -p->painterState()->worldMatrix.m21()*_scale.toDouble();
        m2.yx = -p->painterState()->worldMatrix.m12()*_scale.toDouble();
        m2.yy = p->painterState()->worldMatrix.m22()*_scale.toDouble();

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
                XftFontClose(QX11Info::appDisplay(), tmp->xft_font);
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

            fnt = XftFontOpenPattern(dpy, pattern);
            TransformedFont *trf = new TransformedFont;
            trf->xx = (float)m2.xx;
            trf->xy = (float)m2.xy;
            trf->yx = (float)m2.yx;
            trf->yy = (float)m2.yy;
            trf->xft_font = fnt;
            trf->next = transformed_fonts;
            transformed_fonts = trf;
        }
        transform = true;
    }

    if (p->painterState()->txop == QPainter::TxTranslate)
        p->painter()->map(xpos, ypos, &xpos, &ypos);

    Q26Dot6Offset pos;
    pos.x = xpos;
    pos.y = ypos;

    QGlyphLayout *glyphs = si.glyphs;

    const QColor &pen = static_cast<QX11PaintEngine *>(p)->d->cpen.color();


    XftDraw *draw = 0;
    QPaintDevice *pd = p->painter()->device();
    if (pd->devType() == QInternal::Widget)
	draw = reinterpret_cast<XftDraw *>(static_cast<const QWidget *>(pd)->xftDrawHandle());
    else
	draw = reinterpret_cast<XftDraw *>(static_cast<const QPixmap *>(pd)->xftDrawHandle());
    XftColor col;
    col.color.red = pen.red () | pen.red() << 8;
    col.color.green = pen.green () | pen.green() << 8;
    col.color.blue = pen.blue () | pen.blue() << 8;
    col.color.alpha = 0xffff;
    col.pixel = pen.pixel();
#ifdef FONTENGINE_DEBUG
    qDebug("===== drawing %d glyphs reverse=%s ======", si.num_glyphs, si.right_to_left?"true":"false");
    p->painter()->save();
    p->painter()->setBrush(Qt::white);
    glyph_metrics_t ci = boundingBox(glyphs, si.num_glyphs);
    p->painter()->drawRect(x + ci.x, y + ci.y, ci.width, ci.height);
    p->painter()->drawLine(x + ci.x, y, ci.width, y);
    p->painter()->drawRect(x + ci.x, y + 100 + ci.y, ci.width, ci.height);
    p->painter()->drawLine(x + ci.x, y + 100, ci.width, y + 100);
    qDebug("bounding rect=%d %d (%d/%d)", ci.x, ci.y, ci.width, ci.height);
    p->painter()->restore();
#endif

    if (textFlags != 0)
        drawLines(p, this, yorig, xorig, si.width, textFlags);

    QVarLengthArray<XftGlyphSpec,256> glyphSpec(si.num_glyphs);

#ifdef FONTENGINE_DEBUG
    p->painter()->save();
    p->painter()->setPen(Qt::red);
#endif

    int nGlyphs = 0;

    if (si.right_to_left) {
        int i = si.num_glyphs;
        while(i--) {
            pos.x += glyphs[i].advance.x + Q26Dot6(glyphs[i].space_18d6, F26Dot6);
            pos.y += glyphs[i].advance.y;
        }
        i = 0;
        while(i < si.num_glyphs) {
            pos.x -= glyphs[i].advance.x;
            pos.y -= glyphs[i].advance.y;

            Q26Dot6Offset gpos = pos;
            gpos.x += glyphs[i].offset.x;
            gpos.y += glyphs[i].offset.y;
            if (transform)
                gpos = map(p->painterState()->worldMatrix, gpos);
            int xp = gpos.x.toInt();
            int yp = gpos.y.toInt();
            if (xp > SHRT_MIN && xp < SHRT_MAX) {
                glyphSpec[nGlyphs].x = xp;
                glyphSpec[nGlyphs].y = yp;
                glyphSpec[nGlyphs].glyph = glyphs[i].glyph;
                ++nGlyphs;
            }
            if (glyphs[i].nKashidas) {
                glyphSpec.resize(glyphSpec.size() + glyphs[i].nKashidas);
                QChar ch(0x640); // Kashida character
                QGlyphLayout g[8];
                int nglyphs = 7;
                stringToCMap(&ch, 1, g, &nglyphs, 0);
                for (uint k = 0; k < glyphs[i].nKashidas; ++k) {
                    pos.x -= g[0].advance.x;
                    pos.y -= g[0].advance.y;

                    Q26Dot6Offset gpos = pos;
                    if (transform)
                        gpos = map(p->painterState()->worldMatrix, gpos);
                    int xp = gpos.x.toInt();
                    int yp = gpos.y.toInt();
                    if (xp > SHRT_MIN && xp < SHRT_MAX) {
                        glyphSpec[nGlyphs].x = xp;
                        glyphSpec[nGlyphs].y = yp;
                        glyphSpec[nGlyphs].glyph = g[0].glyph;
                    }
                    ++nGlyphs;
                }
            } else {
                pos.x -= Q26Dot6(glyphs[i].space_18d6, F26Dot6);
            }
#ifdef FONTENGINE_DEBUG
            glyph_metrics_t ci = boundingBox(glyphs[i].glyph);
            p->painter()->drawRect(x + ci.x + glyphs[i].offset.x, y + 100 + ci.y + glyphs[i].offset.y, ci.width, ci.height);
            qDebug("bounding ci[%d]=%d %d (%d/%d) / %d %d   offs=(%d/%d) advance=(%d/%d)", i, ci.x, ci.y, ci.width, ci.height,
                   ci.xoff, ci.yoff, glyphs[i].offset.x, glyphs[i].offset.y, glyphs[i].advance.x, glyphs[i].advance.y);
#endif
            ++i;
        }
    } else {
        int i = 0;
        while (i < si.num_glyphs) {
            Q26Dot6Offset gpos = pos;
            gpos.x += glyphs[i].offset.x;
            gpos.y += glyphs[i].offset.y;
            if (transform)
                gpos = map(p->painterState()->worldMatrix, gpos);
            int xp = gpos.x.toInt();
            int yp = gpos.y.toInt();
            if (xp > SHRT_MIN && xp < SHRT_MAX) {
                glyphSpec[i].x = xp;
                glyphSpec[i].y = yp;
                glyphSpec[i].glyph = glyphs[i].glyph;
                ++nGlyphs;
            }
#ifdef FONTENGINE_DEBUG
            glyph_metrics_t ci = boundingBox(glyphs[i].glyph);
            qDebug("bounding %d ci[%x]=%d %d (%d/%d) / %d %d   offs=(%d/%d) advance=(%d/%d)", i, glyphs[i].glyph,
                   ci.x.toInt(), ci.y.toInt(), ci.width.toInt(), ci.height.toInt(), ci.xoff.toInt(), ci.yoff.toInt(),
                   glyphs[i].offset.x.toInt(), glyphs[i].offset.y.toInt(), glyphs[i].advance.x.toInt(), glyphs[i].advance.y.toInt());
#endif

            pos.x += glyphs[i].advance.x + Q26Dot6(glyphs[i].space_18d6, F26Dot6);
            pos.y += glyphs[i].advance.y;
            ++i;
        }
    }

#ifdef FONTENGINE_DEBUG
    p->painter()->restore();
#endif

    int i = 0;
    while (i < nGlyphs) {
        int toDraw = qMin(64, nGlyphs-i);
        XftDrawGlyphSpec(draw, &col, fnt, glyphSpec.data()+i, toDraw);
        i += toDraw;
    }

}

glyph_metrics_t QFontEngineXft::boundingBox(const QGlyphLayout *glyphs, int numGlyphs)
{
    XGlyphInfo xgi;

    glyph_metrics_t overall;
    Q26Dot6 ymax;
    Q26Dot6 xmax;
    for (int i = 0; i < numGlyphs; i++) {
        FT_UInt gl = glyphs[i].glyph;
        XftGlyphExtents(QX11Info::appDisplay(), _font, &gl, 1, &xgi);
        Q26Dot6 x = overall.xoff + glyphs[i].offset.x - xgi.x;
        Q26Dot6 y = overall.yoff + glyphs[i].offset.y - xgi.y;
        overall.x = qMin(overall.x, x);
        overall.y = qMin(overall.y, y);
        xmax = qMax(xmax, x + xgi.width);
        ymax = qMax(ymax, y + xgi.height);
        overall.xoff += glyphs[i].advance.x;
    }
    overall.height = ymax - overall.y;
    overall.width = xmax - overall.x;

    if (_scale != Q26Dot6(1)) {
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
    XftGlyphExtents(QX11Info::appDisplay(), _font, &x, 1, &xgi);
    glyph_metrics_t gm = glyph_metrics_t(-xgi.x, -xgi.y, xgi.width, xgi.height, xgi.xOff, -xgi.yOff);
    if (_scale != Q26Dot6(1)) {
        gm.x *= _scale;
        gm.y *= _scale;
        gm.height *= _scale;
        gm.width *= _scale;
        gm.xoff *= _scale;
        gm.yoff *= _scale;
    }
    return gm;
}



Q26Dot6 QFontEngineXft::ascent() const
{
    return (_font->ascent)*_scale;
}

Q26Dot6 QFontEngineXft::descent() const
{
    return (_font->descent-1)*_scale;
}

// #### use Freetype to determine this
Q26Dot6 QFontEngineXft::leading() const
{
    Q26Dot6 l = qMin(_font->height - (_font->ascent + _font->descent),
                     int(((_font->ascent + _font->descent) >> 4)))*_scale;
    return (l > 0) ? l : Q26Dot6(1);
}

// #### use Freetype to determine this
Q26Dot6 QFontEngineXft::lineThickness() const
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
Q26Dot6 QFontEngineXft::underlinePosition() const
{
    return Q26Dot6((lineThickness() * 2) + 3) / 6;
}

Q26Dot6 QFontEngineXft::maxCharWidth() const
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


Q26Dot6 QFontEngineXft::minLeftBearing() const
{
    if (lbearing == SHRT_MIN)
        minRightBearing(); // calculates both

    return lbearing;
}

Q26Dot6 QFontEngineXft::minRightBearing() const
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
                that->lbearing = qMin(lbearing, gi.x.toInt());
                that->rbearing = qMin(rbearing, (gi.xoff - gi.x - gi.width).toInt());
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

