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

#include "qfontengine_p.h"
#include "qtextengine_p.h"
#include <qglobal.h>
#include "qt_windows.h"
#include <private/qapplication_p.h>

#include <qpaintdevice.h>
#include <qpainter.h>
#include <limits.h>
#include <math.h>

#include <private/qunicodetables_p.h>
#include <qbitmap.h>

#include <private/qpainter_p.h>
#include "qpaintengine.h"
#include "qvarlengtharray.h"

#ifndef M_PI
#define M_PI 3.14159265358979
#endif

// defined in qtextengine_win.cpp
typedef void *SCRIPT_CACHE;
#if 0 // ##### Uniscribe is disabled for now
typedef HRESULT (WINAPI *fScriptFreeCache)(SCRIPT_CACHE *);
extern fScriptFreeCache ScriptFreeCache;
#endif

static unsigned char *getCMap(HDC hdc, bool &);
static Q_UINT16 getGlyphIndex(unsigned char *table, unsigned short unicode);


HDC   shared_dc            = 0;                // common dc for all fonts
static HFONT shared_dc_font = 0;                // used by Windows 95/98
static HFONT stock_sysfont  = 0;

static inline HFONT systemFont()
{
    if (stock_sysfont == 0)
        stock_sysfont = (HFONT)GetStockObject(SYSTEM_FONT);
    return stock_sysfont;
}

// general font engine

QFontEngine::~QFontEngine()
{
    QT_WA({
        if (hdc) {                                // one DC per font (Win NT)
            //SelectObject(hdc, systemFont());
            if (!stockFont)
                DeleteObject(hfont);
            if (!paintDevice)
                ReleaseDC(0, hdc);
            hdc = 0;
            hfont = 0;
        }
    } , {
        if (hfont) {                                // shared DC (Windows 95/98)
            if (shared_dc_font == hfont) {        // this is the current font
                Q_ASSERT(shared_dc != 0);
                SelectObject(shared_dc, systemFont());
                shared_dc_font = 0;
            }
            if (!stockFont)
                DeleteObject(hfont);
            hfont = 0;
        }
    });
    delete [] cmap;

#if 0
    // for Uniscribe
    if (ScriptFreeCache)
        ScriptFreeCache(&script_cache);
#endif
}

// ##### get these from windows
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

// ##### get these from windows
Q26Dot6 QFontEngine::underlinePosition() const
{
    return ((lineThickness() * 2) + 3) / 6;
}


HDC QFontEngine::dc() const
{
    if (hdc || (QSysInfo::WindowsVersion & QSysInfo::WV_NT_based)) // either NT_based or Printer
        return hdc;
    Q_ASSERT(shared_dc != 0 && hfont != 0);
    if (shared_dc_font != hfont) {
        SelectObject(shared_dc, hfont);
        shared_dc_font = hfont;
    }
    return shared_dc;
}

void QFontEngine::getCMap()
{
    QT_WA({
        ttf = (bool)(tm.w.tmPitchAndFamily & TMPF_TRUETYPE);
    } , {
        ttf = (bool)(tm.a.tmPitchAndFamily & TMPF_TRUETYPE);
    });
    HDC hdc = dc();
    SelectObject(hdc, hfont);
    bool symb = false;
    cmap = ttf ? ::getCMap(hdc, symb) : 0;
    if (!cmap) {
        ttf = false;
        symb = false;
    }
    symbol = symb;
    script_cache = 0;
}

void QFontEngine::getGlyphIndexes(const QChar *ch, int numChars, QGlyphLayout *glyphs, bool mirrored) const
{
    if (mirrored) {
        if (symbol) {
            while(numChars--) {
                glyphs->glyph = getGlyphIndex(cmap, ch->unicode());
                if(!glyphs->glyph && ch->unicode() < 0x100)
                    glyphs->glyph = getGlyphIndex(cmap, ch->unicode()+0xf000);
                glyphs++;
                ch++;
            }
        } else if (ttf) {
            while(numChars--) {
                glyphs->glyph = getGlyphIndex(cmap, ::mirroredChar(*ch).unicode());
                glyphs++;
                ch++;
            }
        } else {
            while(numChars--) {
                glyphs->glyph = ::mirroredChar(*ch).unicode();
                glyphs++;
                ch++;
            }
        }
    } else {
        if (symbol) {
            while(numChars--) {
                glyphs->glyph = getGlyphIndex(cmap, ch->unicode());
                if(!glyphs->glyph && ch->unicode() < 0x100)
                    glyphs->glyph = getGlyphIndex(cmap, ch->unicode()+0xf000);
                glyphs++;
                ch++;
            }
        } else if (ttf) {
            while(numChars--) {
                glyphs->glyph = getGlyphIndex(cmap, ch->unicode());
                glyphs++;
                ch++;
            }
        } else {
            while(numChars--) {
                glyphs->glyph = ch->unicode();
                glyphs++;
                ch++;
            }
        }
    }
}


// non Uniscribe engine

QFontEngineWin::QFontEngineWin(const QString &name, HDC _hdc, HFONT _hfont, bool stockFont, LOGFONT lf)
{
    paintDevice = false;
    //qDebug("regular windows font engine created: font='%s', size=%d", name, lf.lfHeight);

    _name = name;

    hdc = _hdc;
    hfont = _hfont;
    logfont = lf;
    SelectObject(dc(), hfont);
    this->stockFont = stockFont;

    lbearing = SHRT_MIN;
    rbearing = SHRT_MIN;

    BOOL res;
    QT_WA({
        res = GetTextMetricsW(dc(), &tm.w);
    } , {
        res = GetTextMetricsA(dc(), &tm.a);
    });
#ifndef QT_NO_DEBUG
    if (!res)
        qSystemWarning("QFontPrivate: GetTextMetrics failed");
#endif
    cache_cost = tm.w.tmHeight * tm.w.tmAveCharWidth * 2000;
    getCMap();

    useTextOutA = false;
#ifndef Q_OS_TEMP
    // TextOutW doesn't work for symbol fonts on Windows 95!
    // since we're using glyph indices we don't care for ttfs about this!
    if (QSysInfo::WindowsVersion == QSysInfo::WV_95 && !ttf &&
         (_name == QLatin1String("Marlett") || _name == QLatin1String("Symbol") ||
           _name == QLatin1String("Webdings") || _name == QLatin1String("Wingdings")))
            useTextOutA = true;
#endif
    memset(widthCache, 0, sizeof(widthCache));
}

QFontEngine::FECaps QFontEngineWin::capabilites() const
{
    return QT_WA_INLINE(
        (tm.w.tmPitchAndFamily & (TMPF_VECTOR|TMPF_TRUETYPE) ? FullTransformations : NoTransformations),
        (tm.a.tmPitchAndFamily & (TMPF_VECTOR|TMPF_TRUETYPE) ? RotScale : NoTransformations)
       );
}

bool QFontEngineWin::stringToCMap(const QChar *str, int len, QGlyphLayout *glyphs, int *nglyphs, QTextEngine::ShaperFlags flags) const
{
    if (*nglyphs < len) {
        *nglyphs = len;
        return false;
    }

    getGlyphIndexes(str, len, glyphs, flags & QTextEngine::RightToLeft);

    HDC hdc = dc();
    unsigned int glyph;
    int overhang = (QSysInfo::WindowsVersion & QSysInfo::WV_DOS_based) ? tm.a.tmOverhang : 0;
    for(register int i = 0; i < len; i++) {
        glyph = glyphs[i].glyph;
        glyphs[i].advance.x = (glyph < widthCacheSize) ? widthCache[glyph] : 0;
        glyphs[i].advance.y = 0;
        // font-width cache failed
        if (!glyphs[i].advance.x) {
            SIZE size;
            GetTextExtentPoint32W(hdc, (wchar_t *)str, 1, &size);
            glyphs[i].advance.x = size.cx - overhang;
            // if glyph's within cache range, store it for later
            if (glyph < widthCacheSize && glyphs[i].advance.x > 0 && glyphs[i].advance.x < 0x100)
                widthCache[glyph] = size.cx - overhang;
        }
        str++;
    }

    *nglyphs = len;
    return true;
}
// ### Port properly
// #define COLOR_VALUE(c) ((p->flags & QPainter::RGBColor) ? RGB(c.red(),c.green(),c.blue()) : c.pixel())
#define COLOR_VALUE(c) c.pixel()

void QFontEngineWin::draw(QPaintEngine *p, int x, int y, const QTextItem &si, int textFlags)
{
    QPainterState *state = p->painterState();

    double scale = 1.;
    int angle = 0;
    bool transform = false;

    if (state->txop >= QPainter::TxScale && !(QSysInfo::WindowsVersion & QSysInfo::WV_NT_based)) {
        // Draw rotated and sheared text on Windows 95, 98

        // All versions can draw rotated text natively. Scaling can be done with window/viewport transformations.
        // Shearing transformations are done by QPainter.

        // rotation + scale + translation
        scale = sqrt(state->matrix.m11()*state->matrix.m22()
                      - state->matrix.m12()*state->matrix.m21());
        angle = 1800*acos(state->matrix.m11()/scale)/M_PI;
        if (state->matrix.m12() < 0)
            angle = 3600 - angle;

        transform = true;
    } else if (!p->hasFeature(QPaintEngine::CoordTransform) && state->txop == QPainter::TxTranslate) {
        state->painter->map(x, y, &x, &y);
    }

    if (textFlags & Qt::TextUnderline || textFlags & Qt::TextStrikeOut || scale != 1. || angle) {
        LOGFONT lf = logfont;
        lf.lfUnderline = (bool)(textFlags & Qt::TextUnderline);
        lf.lfStrikeOut = (bool)(textFlags & Qt::TextStrikeOut);
        if (angle) {
            lf.lfOrientation = -angle;
            lf.lfEscapement = -angle;
        }
        if (scale != 1.) {
            lf.lfHeight = (int) (lf.lfHeight*scale);
            lf.lfWidth = (int) (lf.lfWidth*scale);
        }
        HFONT hf = QT_WA_INLINE(CreateFontIndirectW(&lf), CreateFontIndirectA((LOGFONTA*)&lf));
        SelectObject(hdc, hf);
    } else {
        SelectObject(hdc, hfont);
    }

    unsigned int options =  ttf ? ETO_GLYPH_INDEX : 0;

    QGlyphLayout *glyphs = si.glyphs;

#if 0
    // ###### should be moved to the printer GC
    if(p->pdev->devType() == QInternal::Printer) {
        // some buggy printer drivers can't handle glyph indices correctly for latin1
        // If the string is pure latin1, we output the string directly, not the glyph indices.
        // There must be a better way to get this working, but currently I can't think of one.
        const QChar *uc = engine->string.unicode() + si.position;
        int l = engine->length(si - &engine->items[0]);
        int i = 0;
        bool latin = (l == si.num_glyphs);
        while (latin && i < l) {
            if(uc[i].unicode() >= 0x100)
                latin = false;
            ++i;
        }
        if(latin) {
            glyphs = (glyph_t *)uc;
            options = 0;
        }
    }
#endif

    int xo = x;

    if (!(si.right_to_left)) {
        // hack to get symbol fonts working on Win95. See also QFontEngine constructor
        if (useTextOutA) {
            // can only happen if !ttf
            for(int i = 0; i < si.num_glyphs; i++) {
                QString str(QChar(glyphs->glyph));
                QByteArray cstr = str.toLocal8Bit();
                TextOutA(hdc, x + glyphs->offset.x.toInt(), y + glyphs->offset.y.toInt(), cstr.data(), cstr.length());
                x += glyphs->advance.x.toInt();
                glyphs++;
            }
        } else {
            bool haveOffsets = false;
            int w = 0;
            for(int i = 0; i < si.num_glyphs; i++) {
                if (glyphs[i].offset.x != 0 || glyphs[i].offset.y != 0) {
                    haveOffsets = true;
                    break;
                }
                w += glyphs[i].advance.x.toInt();
            }

            if (haveOffsets || transform) {
                for(int i = 0; i < si.num_glyphs; i++) {
                    wchar_t chr = glyphs->glyph;
                    int xp = x + glyphs->offset.x.toInt();
                    int yp = y + glyphs->offset.y.toInt();
                    if (transform)
                        state->painter->map(xp, yp, &xp, &yp);
                    ExtTextOutW(hdc, xp, yp, options, 0, &chr, 1, 0);
                    x += glyphs->advance.x.toInt();
                    glyphs++;
                }
            } else {
                // fast path
                QVarLengthArray<wchar_t> g(si.num_glyphs);
                for (int i = 0; i < si.num_glyphs; ++i)
                    g[i] = glyphs[i].glyph;
                // fast path
                ExtTextOutW(hdc, x + glyphs->offset.x.toInt(), y + glyphs->offset.y.toInt(), options, 0, g.data(), si.num_glyphs, 0);
                x += w;
            }
        }
    } else {
        glyphs += si.num_glyphs;
        for(int i = 0; i < si.num_glyphs; i++) {
            glyphs--;
            wchar_t chr = glyphs->glyph;
            int xp = x + glyphs->offset.x.toInt();
            int yp = y + glyphs->offset.y.toInt();
            if (transform)
                state->painter->map(xp, yp, &xp, &yp);
            ExtTextOutW(hdc, xp, yp, options, 0, &chr, 1, 0);
            x += glyphs->advance.x.toInt();
        }
    }

    if (textFlags & Qt::TextUnderline || textFlags & Qt::TextStrikeOut || scale != 1. || angle)
        DeleteObject(SelectObject(hdc, hfont));

    if (textFlags & Qt::TextOverline) {
        int lw = lineThickness().toInt();
        int yp = y - ascent().toInt() -1;
        Rectangle(hdc, xo, yp, x, yp + lw);

    }

#if 0
    if (nat_xf)
        p->nativeXForm(false);
#endif
}

glyph_metrics_t QFontEngineWin::boundingBox(const QGlyphLayout *glyphs, int numGlyphs)
{
    if (numGlyphs == 0)
        return glyph_metrics_t();

    int w = 0;
    const QGlyphLayout *end = glyphs + numGlyphs;
    while(end > glyphs)
        w += (--end)->advance.x.toInt();

    return glyph_metrics_t(0, -tm.w.tmAscent, w, tm.w.tmHeight, w, 0);
}

glyph_metrics_t QFontEngineWin::boundingBox(glyph_t glyph)
{
#ifndef Q_OS_TEMP
    GLYPHMETRICS gm;

    if(!ttf) {
        SIZE s;
        WCHAR ch = glyph;
        BOOL res = GetTextExtentPoint32W(dc(), &ch, 1, &s);
        Q_UNUSED(res);
        int overhang = (QSysInfo::WindowsVersion & QSysInfo::WV_DOS_based) ? tm.a.tmOverhang : 0;
        return glyph_metrics_t(0, -tm.a.tmAscent, s.cx, tm.a.tmHeight, s.cx-overhang, 0);
    } else {
        DWORD res = 0;
        MAT2 mat;
        mat.eM11.value = mat.eM22.value = 1;
        mat.eM11.fract = mat.eM22.fract = 0;
        mat.eM21.value = mat.eM12.value = 0;
        mat.eM21.fract = mat.eM12.fract = 0;
        QT_WA({
            res = GetGlyphOutlineW(dc(), glyph, GGO_METRICS|GGO_GLYPH_INDEX, &gm, 0, 0, &mat);
        } , {
            res = GetGlyphOutlineA(dc(), glyph, GGO_METRICS|GGO_GLYPH_INDEX, &gm, 0, 0, &mat);
        });
        if (res != GDI_ERROR)
            return glyph_metrics_t(gm.gmptGlyphOrigin.x, -gm.gmptGlyphOrigin.y,
                                  gm.gmBlackBoxX, gm.gmBlackBoxY, gm.gmCellIncX, gm.gmCellIncY);
    }
#endif
    return glyph_metrics_t();
}

Q26Dot6 QFontEngineWin::ascent() const
{
    return tm.w.tmAscent;
}

Q26Dot6 QFontEngineWin::descent() const
{
    return tm.w.tmDescent;
}

Q26Dot6 QFontEngineWin::leading() const
{
    return tm.w.tmExternalLeading;
}

Q26Dot6 QFontEngineWin::maxCharWidth() const
{
    return tm.w.tmMaxCharWidth;
}

enum { max_font_count = 256 };
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
        12386,
        0
};

static const int char_table_entries = sizeof(char_table)/sizeof(ushort);


Q26Dot6 QFontEngineWin::minLeftBearing() const
{
    if (lbearing == SHRT_MIN)
        minRightBearing(); // calculates both

    return lbearing;
}

Q26Dot6 QFontEngineWin::minRightBearing() const
{
#ifdef Q_OS_TEMP
        return 0;
#else
    if (rbearing == SHRT_MIN) {
        int ml = 0;
        int mr = 0;
        if (ttf) {
            HDC hdc = dc();
            SelectObject(hdc, hfont);
            ABC *abc = 0;
            int n = QT_WA_INLINE(tm.w.tmLastChar - tm.w.tmFirstChar, tm.a.tmLastChar - tm.a.tmFirstChar);
            if (n <= max_font_count) {
                abc = new ABC[n+1];
                QT_WA({
                    GetCharABCWidths(hdc, tm.w.tmFirstChar, tm.w.tmLastChar, abc);
                }, {
                    GetCharABCWidthsA(hdc,tm.a.tmFirstChar,tm.a.tmLastChar,abc);
                });
            } else {
                abc = new ABC[char_table_entries+1];
                QT_WA({
                    for(int i = 0; i < char_table_entries; i++)
                        GetCharABCWidths(hdc, char_table[i], char_table[i], abc+i);
                }, {
                    for(int i = 0; i < char_table_entries; i++) {
                        QByteArray w = QString(QChar(char_table[i])).toLocal8Bit();
                        if (w.length() == 1) {
                            uint ch8 = (uchar)w[0];
                            GetCharABCWidthsA(hdc, ch8, ch8, abc+i);
                        }
                    }
                });
                n = char_table_entries;
            }
            ml = abc[0].abcA;
            mr = abc[0].abcC;
            for (int i = 1; i < n; i++) {
                if (abc[i].abcA + abc[i].abcB + abc[i].abcC != 0) {
                    ml = qMin(ml,abc[i].abcA);
                    mr = qMin(mr,abc[i].abcC);
                }
            }
            delete [] abc;
        } else {
            QT_WA({
                ABCFLOAT *abc = 0;
                int n = tm.w.tmLastChar - tm.w.tmFirstChar+1;
                if (n <= max_font_count) {
                    abc = new ABCFLOAT[n];
                    GetCharABCWidthsFloat(hdc, tm.w.tmFirstChar, tm.w.tmLastChar, abc);
                } else {
                    abc = new ABCFLOAT[char_table_entries];
                    for(int i = 0; i < char_table_entries; i++)
                        GetCharABCWidthsFloat(hdc, char_table[i], char_table[i], abc+i);
                    n = char_table_entries;
                }
                float fml = abc[0].abcfA;
                float fmr = abc[0].abcfC;
                for (int i=1; i<n; i++) {
                    if (abc[i].abcfA + abc[i].abcfB + abc[i].abcfC != 0) {
                        fml = qMin(fml,abc[i].abcfA);
                        fmr = qMin(fmr,abc[i].abcfC);
                    }
                }
                ml = int(fml-0.9999);
                mr = int(fmr-0.9999);
                delete [] abc;
            } , {
                ml = 0;
                mr = -tm.a.tmOverhang;
            });
        }
        ((QFontEngine *)this)->lbearing = ml;
        ((QFontEngine *)this)->rbearing = mr;
    }

    return rbearing;
#endif
}


const char *QFontEngineWin::name() const
{
    return 0;
}

bool QFontEngineWin::canRender(const QChar *string,  int len)
{
    if (symbol) {
        while(len--) {
            if (getGlyphIndex(cmap, string->unicode()) == 0) {
                if(string->unicode() < 0x100) {
                    if(getGlyphIndex(cmap, string->unicode()+0xf000) == 0)
                        return false;
                } else {
                    return false;
                }
            }
            string++;
        }
    } else if (ttf) {
        while(len--) {
            if (getGlyphIndex(cmap, string->unicode()) == 0)
                return false;
            string++;
        }
    } else {
        QT_WA({
            while(len--) {
                if (tm.w.tmFirstChar > string->unicode() || tm.w.tmLastChar < string->unicode())
                    return false;
            }
        }, {
            while(len--) {
                if (tm.a.tmFirstChar > string->unicode() || tm.a.tmLastChar < string->unicode())
                    return false;
            }
        });
    }
    return true;
}

QFontEngine::Type QFontEngineWin::type() const
{
    return QFontEngine::Win;
}



// box font engine

QFontEngineBox::QFontEngineBox(int size)
    : _size(size)
{
    cache_cost = 1;
    hdc = (QSysInfo::WindowsVersion & QSysInfo::WV_NT_based) ? GetDC(0) : shared_dc;
#ifndef Q_OS_TEMP
    hfont = (HFONT)GetStockObject(ANSI_VAR_FONT);
#endif
    stockFont = true;
    paintDevice = false;
    ttf = false;

    cmap = 0;
    script_cache = 0;

    //qDebug("box font engine created!");
}

QFontEngineBox::~QFontEngineBox()
{
}

QFontEngine::FECaps QFontEngineBox::capabilites() const
{
    return FullTransformations;
}

bool QFontEngineBox::stringToCMap(const QChar *,  int len, QGlyphLayout *glyphs, int *nglyphs, QTextEngine::ShaperFlags) const
{
    if (*nglyphs < len) {
        *nglyphs = len;
        return false;
    }

    for (int i = 0; i < len; i++)
        glyphs[i].glyph = 0;
    *nglyphs = len;

    for (int i = 0; i < len; i++)
        (glyphs++)->advance.x = _size;

    return true;
}

void QFontEngineBox::draw(QPaintEngine *p, int x, int y, const QTextItem &si, int textFlags)
{
    Q_UNUSED(p);
    Q_UNUSED(x);
    Q_UNUSED(y);
    Q_UNUSED(si);
    Q_UNUSED(textFlags);
//     qDebug("QFontEngineXLFD::draw(%d, %d, numglyphs=%d", x, y, numGlyphs);

    // ########
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

bool QFontEngineBox::canRender(const QChar *,  int)
{
    return true;
}

QFontEngine::Type QFontEngineBox::type() const
{
    return Box;
}





// ----------------------------------------------------------------------------
// True type support methods
// ----------------------------------------------------------------------------




#define MAKE_TAG(ch1, ch2, ch3, ch4) (\
    (((DWORD)(ch4)) << 24) | \
    (((DWORD)(ch3)) << 16) | \
    (((DWORD)(ch2)) << 8) | \
    ((DWORD)(ch1)) \
   )

static inline Q_UINT32 getUInt(unsigned char *p)
{
    Q_UINT32 val;
    val = *p++ << 24;
    val |= *p++ << 16;
    val |= *p++ << 8;
    val |= *p;

    return val;
}

static inline Q_UINT16 getUShort(unsigned char *p)
{
    Q_UINT16 val;
    val = *p++ << 8;
    val |= *p;

    return val;
}

static inline void tag_to_string(char *string, Q_UINT32 tag)
{
    string[0] = (tag >> 24)&0xff;
    string[1] = (tag >> 16)&0xff;
    string[2] = (tag >> 8)&0xff;
    string[3] = tag&0xff;
    string[4] = 0;
}

static Q_UINT16 getGlyphIndex(unsigned char *table, unsigned short unicode)
{
    unsigned short format = getUShort(table);
    if (format == 0) {
        if (unicode < 256)
            return (int) *(table+6+unicode);
    } else if (format == 2) {
        qWarning("format 2 encoding table for Unicode, not implemented!");
    } else if (format == 4) {
        Q_UINT16 segCountX2 = getUShort(table + 6);
        unsigned char *ends = table + 14;
        Q_UINT16 endIndex = 0;
        int i = 0;
        for (; i < segCountX2/2 && (endIndex = getUShort(ends + 2*i)) < unicode; i++);

        unsigned char *idx = ends + segCountX2 + 2 + 2*i;
        Q_UINT16 startIndex = getUShort(idx);

        if (startIndex > unicode)
            return 0;

        idx += segCountX2;
        Q_INT16 idDelta = (Q_INT16)getUShort(idx);
        idx += segCountX2;
        Q_UINT16 idRangeoffset_t = (Q_UINT16)getUShort(idx);

        Q_UINT16 glyphIndex;
        if (idRangeoffset_t) {
            Q_UINT16 id = getUShort(idRangeoffset_t + 2*(unicode - startIndex) + idx);
            if (id)
                glyphIndex = (idDelta + id) % 0x10000;
            else
                glyphIndex = 0;
        } else {
            glyphIndex = (idDelta + unicode) % 0x10000;
        }
        return glyphIndex;
    }

    return 0;
}


static unsigned char *getCMap(HDC hdc, bool &symbol)
{
    const DWORD CMAP = MAKE_TAG('c', 'm', 'a', 'p');

    unsigned char header[4];

    // get the CMAP header and the number of encoding tables
    DWORD bytes =
#ifndef Q_OS_TEMP
        GetFontData(hdc, CMAP, 0, &header, 4);
#else
        0;
#endif
    if (bytes == GDI_ERROR)
        return 0;
    unsigned short version = getUShort(header);
    if (version != 0)
        return 0;

    unsigned short numTables = getUShort(header+2);
    unsigned char *maps = new unsigned char[8*numTables];

    // get the encoding table and look for Unicode
#ifndef Q_OS_TEMP
    bytes = GetFontData(hdc, CMAP, 4, maps, 8*numTables);
#endif
    if (bytes == GDI_ERROR)
        return 0;

    symbol = true;
    unsigned int unicode_table = 0;
    for (int n = 0; n < numTables; n++) {
        Q_UINT32 version = getUInt(maps + 8*n);
        // accept both symbol and Unicode encodings. prefer unicode.
        if (version == 0x00030001 || version == 0x00030000) {
            unicode_table = getUInt(maps + 8*n + 4);
            if (version == 0x00030001) {
                symbol = false;
                break;
            }
        }
    }

    if (!unicode_table) {
        // qDebug("no unicode table found");
        return 0;
    }

    delete [] maps;

    // get the header of the unicode table
#ifndef Q_OS_TEMP
    bytes = GetFontData(hdc, CMAP, unicode_table, &header, 4);
#endif
    if (bytes == GDI_ERROR)
        return 0;

    unsigned short length = getUShort(header+2);
    unsigned char *unicode_data = new unsigned char[length];

    // get the cmap table itself
#ifndef Q_OS_TEMP
    bytes = GetFontData(hdc, CMAP, unicode_table, unicode_data, length);
#endif
    if (bytes == GDI_ERROR) {
        delete [] unicode_data;
        return 0;
    }
    return unicode_data;
}



