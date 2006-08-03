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

#include "qfontengine_p.h"
#include "qtextengine_p.h"
#include <qglobal.h>
#include "qt_windows.h"
#include <private/qapplication_p.h>

#include <qpaintdevice.h>
#include <qpainter.h>
#include <limits.h>
#include <math.h>

#include <qendian.h>

#include <private/qunicodetables_p.h>
#include <qbitmap.h>

#include <private/qpainter_p.h>
#include <private/qpdf_p.h>
#include "qpaintengine.h"
#include "qvarlengtharray.h"

#ifndef M_PI
#define M_PI 3.14159265358979
#endif

//### mingw needed define
#ifndef TT_PRIM_CSPLINE
#define TT_PRIM_CSPLINE 3
#endif

// Copy a LOGFONTW struct into a LOGFONTA by converting the face name to an 8 bit value.
// This is needed when calling CreateFontIndirect on non-unicode windowses.
inline static void wa_copy_logfont(LOGFONTW *lfw, LOGFONTA *lfa)
{
    lfa->lfHeight = lfw->lfHeight;
    lfa->lfWidth = lfw->lfWidth;
    lfa->lfEscapement = lfw->lfEscapement;
    lfa->lfOrientation = lfw->lfOrientation;
    lfa->lfWeight = lfw->lfWeight;
    lfa->lfItalic = lfw->lfItalic;
    lfa->lfUnderline = lfw->lfUnderline;
    lfa->lfCharSet = lfw->lfCharSet;
    lfa->lfOutPrecision = lfw->lfOutPrecision;
    lfa->lfClipPrecision = lfw->lfClipPrecision;
    lfa->lfQuality = lfw->lfQuality;
    lfa->lfPitchAndFamily = lfw->lfPitchAndFamily;

    QString fam = QString::fromUtf16((const ushort*)lfw->lfFaceName);
    memcpy(lfa->lfFaceName, fam.toLocal8Bit().constData(), fam.length() + 1);
}

// defined in qtextengine_win.cpp
typedef void *SCRIPT_CACHE;
typedef HRESULT (WINAPI *fScriptFreeCache)(SCRIPT_CACHE *);
extern fScriptFreeCache ScriptFreeCache;

static QVector<QFontEngineWin::KernPair> getKerning(HDC hdc, QFixed factor);
static unsigned char *getCMap(HDC hdc, bool &);
static quint32 getGlyphIndex(unsigned char *table, unsigned int unicode);

#define MAKE_TAG(ch1, ch2, ch3, ch4) (\
    (((DWORD)(ch4)) << 24) | \
    (((DWORD)(ch3)) << 16) | \
    (((DWORD)(ch2)) << 8) | \
    ((DWORD)(ch1)) \
   )

static inline quint32 getUInt(unsigned char *p)
{
    quint32 val;
    val = *p++ << 24;
    val |= *p++ << 16;
    val |= *p++ << 8;
    val |= *p;

    return val;
}

static inline quint16 getUShort(unsigned char *p)
{
    quint16 val;
    val = *p++ << 8;
    val |= *p;

    return val;
}


HDC   shared_dc            = 0;                // common dc for all fonts
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
    // make sure we aren't by accident still selected
    SelectObject(shared_dc, systemFont());
    // for Uniscribe
    if (ScriptFreeCache && script_cache)
        ScriptFreeCache(&script_cache);

    if (cmap)
        delete [] cmap;
}

QFixed QFontEngine::lineThickness() const
{
    if(lineWidth > 0)
        return lineWidth;

    // ad hoc algorithm
    int score = fontDef.weight * fontDef.pixelSize;
    int lw = score / 700;

    // looks better with thicker line for small pointsizes
    if (lw < 2 && score >= 1050) lw = 2;
    if (lw == 0) lw = 1;

    return lw;
}

// ##### get these from windows
QFixed QFontEngine::underlinePosition() const
{
    return (lineThickness() * 2 + 3) / 6;
}

static OUTLINETEXTMETRICA *getOutlineTextMetric(HDC hdc)
{
    int size = GetOutlineTextMetricsA(hdc, 0, 0);
    OUTLINETEXTMETRICA *otm = (OUTLINETEXTMETRICA *)malloc(size);
    GetOutlineTextMetricsA(hdc, size, otm);
    return otm;
}

void QFontEngine::getCMap()
{
    QT_WA({
        ttf = (bool)(tm.w.tmPitchAndFamily & TMPF_TRUETYPE);
    } , {
        ttf = (bool)(tm.a.tmPitchAndFamily & TMPF_TRUETYPE);
    });
    HDC hdc = shared_dc;
    SelectObject(hdc, hfont);
    bool symb = false;
    cmap = ttf ? ::getCMap(hdc, symb) : 0;
    if (!cmap) {
        ttf = false;
        symb = false;
    }
    symbol = symb;
    script_cache = 0;
    designToDevice = 1;
    _faceId.index = 0;
    if(cmap) {
        OUTLINETEXTMETRICA *otm = getOutlineTextMetric(hdc);
        designToDevice = QFixed((int)otm->otmEMSquare)/int(otm->otmTextMetrics.tmHeight);
        unitsPerEm = otm->otmEMSquare;
        kerning_pairs = getKerning(hdc, designToDevice);
        _faceId.filename = (char *)otm + (int)otm->otmpFullName;
        lineWidth = otm->otmsUnderscoreSize;
        fsType = otm->otmfsType;
        free(otm);
    } else {
        unitsPerEm = tm.w.tmHeight;
    }
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

int QFontEngine::getGlyphIndexes(const QChar *str, int numChars, QGlyphLayout *glyphs, bool mirrored) const
{
    QGlyphLayout *g = glyphs;
    if (mirrored) {
        if (symbol) {
            for (int i = 0; i < numChars; ++i) {
                unsigned int uc = getChar(str, i, numChars);
                glyphs->glyph = getGlyphIndex(cmap, uc);
                if(!glyphs->glyph && uc < 0x100)
                    glyphs->glyph = getGlyphIndex(cmap, uc + 0xf000);
                glyphs++;
            }
        } else if (ttf) {
            for (int i = 0; i < numChars; ++i) {
                unsigned int uc = getChar(str, i, numChars);
                glyphs->glyph = getGlyphIndex(cmap, QUnicodeTables::mirroredChar(uc));
                glyphs++;
            }
        } else {
            ushort first, last;
            QT_WA({
                first = tm.w.tmFirstChar;
                last = tm.w.tmLastChar;
            }, {
                first = tm.a.tmFirstChar;
                last = tm.a.tmLastChar;
            });
            for (int i = 0; i < numChars; ++i) {
                uint ucs = QUnicodeTables::mirroredChar(getChar(str, i, numChars));
                if (ucs >= first && ucs <= last)
                    glyphs->glyph = ucs;
                else
                    glyphs->glyph = 0;
                glyphs++;
            }
        }
    } else {
        if (symbol) {
            for (int i = 0; i < numChars; ++i) {
                unsigned int uc = getChar(str, i, numChars);
                glyphs->glyph = getGlyphIndex(cmap, uc);
                if(!glyphs->glyph && uc < 0x100)
                    glyphs->glyph = getGlyphIndex(cmap, uc + 0xf000);
                glyphs++;
            }
        } else if (ttf) {
            for (int i = 0; i < numChars; ++i) {
                unsigned int uc = getChar(str, i, numChars);
                glyphs->glyph = getGlyphIndex(cmap, uc);
                glyphs++;
            }
        } else {
            ushort first, last;
            QT_WA({
                first = tm.w.tmFirstChar;
                last = tm.w.tmLastChar;
            }, {
                first = tm.a.tmFirstChar;
                last = tm.a.tmLastChar;
            });
            for (int i = 0; i < numChars; ++i) {
                uint uc = getChar(str, i, numChars);
                if (uc >= first && uc <= last)
                    glyphs->glyph = uc;
                else
                    glyphs->glyph = 0;
                glyphs++;
            }
        }
    }
    return glyphs - g;
}


QFontEngineWin::QFontEngineWin(const QString &name, HFONT _hfont, bool stockFont, LOGFONT lf)
{
    //qDebug("regular windows font engine created: font='%s', size=%d", name, lf.lfHeight);

    _name = name;

    hfont = _hfont;
    logfont = lf;
    SelectObject(shared_dc, hfont);
    this->stockFont = stockFont;
    fontDef.pixelSize = -lf.lfHeight;

    lbearing = SHRT_MIN;
    rbearing = SHRT_MIN;
    synthesized_flags = -1;
    lineWidth = -1;
    x_height = -1;

    BOOL res;
    QT_WA({
        res = GetTextMetricsW(shared_dc, &tm.w);
    } , {
        res = GetTextMetricsA(shared_dc, &tm.a);
    });
    fontDef.fixedPitch = !(tm.w.tmPitchAndFamily & TMPF_FIXED_PITCH);
    if (!res)
        qErrnoWarning("QFontEngineWin: GetTextMetrics failed");

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
    designAdvances = 0;
    designAdvancesSize = 0;
}

QFontEngineWin::~QFontEngineWin()
{
    if(designAdvances)
        free(designAdvances);
}

HGDIOBJ QFontEngineWin::selectDesignFont(QFixed *overhang) const
{
    LOGFONT f = logfont;
    f.lfHeight = unitsPerEm;
    HFONT designFont;
    QT_WA({
        designFont = CreateFontIndirectW(&f);
    }, {
        LOGFONTA fa;
        wa_copy_logfont(&f, &fa);
        designFont = CreateFontIndirectA(&fa);
    });
    HGDIOBJ oldFont = SelectObject(shared_dc, designFont);

    if (QSysInfo::WindowsVersion & QSysInfo::WV_DOS_based) {
        TEXTMETRICA tm;
        BOOL res = GetTextMetricsA(shared_dc, &tm);
        if (!res)
            qErrnoWarning("QFontEngineWin: GetTextMetrics failed");
        *overhang = QFixed((int)tm.tmOverhang) / designToDevice;
    } else {
        *overhang = 0;
    }
    return oldFont;
}

bool QFontEngineWin::stringToCMap(const QChar *str, int len, QGlyphLayout *glyphs, int *nglyphs, QTextEngine::ShaperFlags flags) const
{
    if (*nglyphs < len) {
        *nglyphs = len;
        return false;
    }

    *nglyphs = getGlyphIndexes(str, len, glyphs, flags & QTextEngine::RightToLeft);

    HDC hdc = shared_dc;
    if (flags & QTextEngine::DesignMetrics) {
        HGDIOBJ oldFont = 0;
        QFixed overhang = 0;

        int glyph_pos = 0;
        for(register int i = 0; i < len; i++) {
            bool surrogate = (str[i].unicode() >= 0xd800 && str[i].unicode() < 0xdc00 && i < len-1
                              && str[i+1].unicode() >= 0xdc00 && str[i+1].unicode() < 0xe000);
            unsigned int glyph = glyphs[glyph_pos].glyph;
            if(int(glyph) >= designAdvancesSize) {
                int newSize = (glyph + 256) >> 8 << 8;
                designAdvances = (QFixed *)realloc(designAdvances, newSize*sizeof(QFixed));
                for(int i = designAdvancesSize; i < newSize; ++i)
                    designAdvances[i] = -1000000;
                designAdvancesSize = newSize;
            }
            if(designAdvances[glyph] < -999999) {
                if(!oldFont)
                    oldFont = selectDesignFont(&overhang);
                SIZE size = {0, 0};
                GetTextExtentPoint32W(hdc, (wchar_t *)(str+i), surrogate ? 2 : 1, &size);
                designAdvances[glyph] = QFixed((int)size.cx)/designToDevice;
            }
            glyphs[glyph_pos].advance.x = designAdvances[glyph];
            glyphs[glyph_pos].advance.y = 0;
            if (surrogate)
                ++i;
            ++glyph_pos;
        }
        if(oldFont)
            DeleteObject(SelectObject(hdc, oldFont));
    } else {
        int overhang = (QSysInfo::WindowsVersion & QSysInfo::WV_DOS_based) ? tm.a.tmOverhang : 0;
        int glyph_pos = 0;
        HGDIOBJ oldFont = 0;

        for(register int i = 0; i < len; i++) {
            bool surrogate = (str[i].unicode() >= 0xd800 && str[i].unicode() < 0xdc00 && i < len-1
                              && str[i+1].unicode() >= 0xdc00 && str[i+1].unicode() < 0xe000);
            unsigned int glyph = glyphs[i].glyph;
            glyphs[glyph_pos].advance.x = (glyph < widthCacheSize) ? widthCache[glyph] : 0;
            glyphs[glyph_pos].advance.y = 0;
            // font-width cache failed
            if (glyphs[glyph_pos].advance.x == 0) {
                SIZE size = {0, 0};
                if (!oldFont)
                    oldFont = SelectObject(hdc, hfont);
                GetTextExtentPoint32W(hdc, (wchar_t *)str + i, surrogate ? 2 : 1, &size);
                size.cx -= overhang;
                glyphs[glyph_pos].advance.x = size.cx;
                // if glyph's within cache range, store it for later
                if (glyph < widthCacheSize && size.cx > 0 && size.cx < 0x100)
                    widthCache[glyph] = size.cx;
            }
            if (surrogate)
                ++i;
            ++glyph_pos;
        }

        if (oldFont)
            SelectObject(hdc, oldFont);
    }
    return true;
}

glyph_metrics_t QFontEngineWin::boundingBox(const QGlyphLayout *glyphs, int numGlyphs)
{
    if (numGlyphs == 0)
        return glyph_metrics_t();

    QFixed w = 0;
    const QGlyphLayout *end = glyphs + numGlyphs;
    while(end > glyphs)
        w += ((--end)->advance.x + end->space_18d6) * !end->attributes.dontPrint;

    return glyph_metrics_t(0, -tm.w.tmAscent, w, tm.w.tmHeight, w, 0);
}

glyph_metrics_t QFontEngineWin::boundingBox(glyph_t glyph)
{
#ifndef Q_OS_TEMP
    GLYPHMETRICS gm;

    HDC hdc = shared_dc;
    SelectObject(hdc, hfont);
    if(!ttf) {
        SIZE s = {0, 0};
        WCHAR ch = glyph;
        BOOL res = GetTextExtentPoint32W(hdc, &ch, 1, &s);
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
            res = GetGlyphOutlineW(hdc, glyph, GGO_METRICS|GGO_GLYPH_INDEX, &gm, 0, 0, &mat);
        } , {
            res = GetGlyphOutlineA(hdc, glyph, GGO_METRICS|GGO_GLYPH_INDEX, &gm, 0, 0, &mat);
        });
        if (res != GDI_ERROR)
            return glyph_metrics_t(gm.gmptGlyphOrigin.x, -gm.gmptGlyphOrigin.y,
                                  (int)gm.gmBlackBoxX, (int)gm.gmBlackBoxY, gm.gmCellIncX, gm.gmCellIncY);
    }
#endif
    return glyph_metrics_t();
}

static inline QFixed kerning(int left, int right, const QFontEngineWin::KernPair *pairs, int numPairs)
{
    int left_right = (left << 16) + right;

    left = 0, right = numPairs - 1;
    while (left <= right) {
        int middle = left + ( ( right - left ) >> 1 );

	if(pairs[middle].left_right == left_right)
            return pairs[middle].adjust;

        if (int(pairs[middle].left_right) < left_right)
            left = middle + 1;
        else
            right = middle - 1;
    }
    return 0;
}

void QFontEngineWin::doKerning(int num_glyphs, QGlyphLayout *glyphs, QTextEngine::ShaperFlags flags) const
{
    int numPairs = kerning_pairs.size();
    if(!numPairs)
        return;

    const KernPair *pairs = kerning_pairs.constData();

    if(flags & QTextEngine::DesignMetrics) {
        for(int i = 0; i < num_glyphs - 1; ++i)
            glyphs[i].advance.x += kerning(glyphs[i].glyph, glyphs[i+1].glyph , pairs, numPairs);
    } else {
        for(int i = 0; i < num_glyphs - 1; ++i)
            glyphs[i].advance.x += qRound(kerning(glyphs[i].glyph, glyphs[i+1].glyph , pairs, numPairs));
    }
}


QFixed QFontEngineWin::ascent() const
{
    return tm.w.tmAscent;
}

QFixed QFontEngineWin::descent() const
{
    return tm.w.tmDescent;
}

QFixed QFontEngineWin::leading() const
{
    return tm.w.tmExternalLeading;
}


QFixed QFontEngineWin::xHeight() const
{
    if (x_height < 0) {
        if(ttf) {
#if 0
            const DWORD HEAD = MAKE_TAG('h', 'e', 'a', 'd');
            SelectObject(shared_dc, hfont);
            uchar data[4];
            GetFontData(shared_dc, HEAD, 44, &data, 4);
            USHORT macStyle = getUShort(data);
            if (tm.w.tmItalic && !(macStyle & 2))
                synthesized_flags = SynthesizedItalic;
            if (fontDef.stretch != 100 && ttf)
                synthesized_flags |= SynthesizedStretch;
            if (tm.w.tmWeight >= 500 && !(macStyle & 1))
                synthesized_flags |= SynthesizedBold;
            //qDebug() << "font is" << _name <<
            //    "it=" << (macStyle & 2) << fontDef.style << "flags=" << synthesized_flags;
#endif
        }
        if (x_height < 0)
            x_height = QFontEngine::xHeight();
    }
    return x_height;
}

QFixed QFontEngineWin::averageCharWidth() const
{
    return tm.w.tmAveCharWidth;
}

qreal QFontEngineWin::maxCharWidth() const
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


qreal QFontEngineWin::minLeftBearing() const
{
    if (lbearing == SHRT_MIN)
        minRightBearing(); // calculates both

    return lbearing;
}

qreal QFontEngineWin::minRightBearing() const
{
#ifdef Q_OS_TEMP
        return 0;
#else
    if (rbearing == SHRT_MIN) {
        int ml = 0;
        int mr = 0;
        HDC hdc = shared_dc;
        SelectObject(hdc, hfont);
        if (ttf) {
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
        for (int i = 0; i < len; ++i) {
            unsigned int uc = getChar(string, i, len);
            if (getGlyphIndex(cmap, uc) == 0) {
                if (uc < 0x100) {
                    if (getGlyphIndex(cmap, uc + 0xf000) == 0)
                        return false;
                } else {
                    return false;
                }
            }
        }
    } else if (ttf) {
        for (int i = 0; i < len; ++i) {
            unsigned int uc = getChar(string, i, len);
            if (getGlyphIndex(cmap, uc) == 0)
                return false;
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

static inline double qt_fixed_to_double(const FIXED &p) {
    return ((p.value << 16) + p.fract) / 65536.0;
}

static inline QPointF qt_to_qpointf(const POINTFX &pt) {
    return QPointF(qt_fixed_to_double(pt.x), -qt_fixed_to_double(pt.y));
}

#ifndef GGO_UNHINTED
#define GGO_UNHINTED 0x0100
#endif

static void addGlyphToPath(glyph_t glyph, const QFixedPoint &position, HDC hdc,
                           QPainterPath *path, bool ttf, glyph_metrics_t *metric = 0)
{
    MAT2 mat;
    mat.eM11.value = mat.eM22.value = 1;
    mat.eM11.fract = mat.eM22.fract = 0;
    mat.eM21.value = mat.eM12.value = 0;
    mat.eM21.fract = mat.eM12.fract = 0;
    uint glyphFormat = GGO_NATIVE;
	if ((QSysInfo::WindowsVersion & QSysInfo::WV_NT_based)
        && QSysInfo::WindowsVersion != QSysInfo::WV_NT) {
		glyphFormat |= GGO_UNHINTED;
	}

    if (ttf)
        glyphFormat |= GGO_GLYPH_INDEX;

    GLYPHMETRICS gMetric;
    memset(&gMetric, 0, sizeof(GLYPHMETRICS));
    int bufferSize;
    QT_WA( {
        bufferSize = GetGlyphOutlineW(hdc, glyph, glyphFormat, &gMetric, 0, 0, &mat);
    }, {
        bufferSize = GetGlyphOutlineA(hdc, glyph, glyphFormat, &gMetric, 0, 0, &mat);
    });
    if ((DWORD)bufferSize == GDI_ERROR) {
        qErrnoWarning("QFontEngineWin::addOutlineToPath: GetGlyphOutline(1) failed");
        return;
    }

    void *dataBuffer = new char[bufferSize];
    DWORD ret;
    QT_WA( {
	ret = GetGlyphOutlineW(hdc, glyph, glyphFormat, &gMetric, bufferSize,
			    dataBuffer, &mat);
    }, {
        ret = GetGlyphOutlineA(hdc, glyph, glyphFormat, &gMetric, bufferSize,
                                dataBuffer, &mat);
    } );
    if (ret == GDI_ERROR) {
        qErrnoWarning("QFontEngineWin::addOutlineToPath: GetGlyphOutline(2) failed");
        delete dataBuffer;
        return;
    }

    if(metric) {
        *metric = glyph_metrics_t(gMetric.gmptGlyphOrigin.x, -gMetric.gmptGlyphOrigin.y,
                                  (int)gMetric.gmBlackBoxX, (int)gMetric.gmBlackBoxY,
                                  gMetric.gmCellIncX, gMetric.gmCellIncY);
    }

    int offset = 0;
    int headerOffset = 0;
    TTPOLYGONHEADER *ttph = 0;

    QPointF oset = position.toPointF();
    while (headerOffset < bufferSize) {
        ttph = (TTPOLYGONHEADER*)((char *)dataBuffer + headerOffset);

        QPointF lastPoint(qt_to_qpointf(ttph->pfxStart));
        path->moveTo(lastPoint + oset);
        offset += sizeof(TTPOLYGONHEADER);
        TTPOLYCURVE *curve;
        while (offset<int(headerOffset + ttph->cb)) {
            curve = (TTPOLYCURVE*)((char*)(dataBuffer) + offset);
            switch (curve->wType) {
            case TT_PRIM_LINE: {
                for (int i=0; i<curve->cpfx; ++i) {
                    QPointF p = qt_to_qpointf(curve->apfx[i]) + oset;
                    path->lineTo(p);
                }
                break;
            }
            case TT_PRIM_QSPLINE: {
                const QPainterPath::Element &elm = path->elementAt(path->elementCount()-1);
                QPointF prev(elm.x, elm.y);
                QPointF endPoint;
                for (int i=0; i<curve->cpfx - 1; ++i) {
                    QPointF p1 = qt_to_qpointf(curve->apfx[i]) + oset;
                    QPointF p2 = qt_to_qpointf(curve->apfx[i+1]) + oset;
                    if (i < curve->cpfx - 2) {
                        endPoint = QPointF((p1.x() + p2.x()) / 2, (p1.y() + p2.y()) / 2);
                    } else {
                        endPoint = p2;
                    }

                    path->quadTo(p1, endPoint);
                    prev = endPoint;
                }

                break;
            }
            case TT_PRIM_CSPLINE: {
                for (int i=0; i<curve->cpfx; ) {
                    QPointF p2 = qt_to_qpointf(curve->apfx[i++]) + oset;
                    QPointF p3 = qt_to_qpointf(curve->apfx[i++]) + oset;
                    QPointF p4 = qt_to_qpointf(curve->apfx[i++]) + oset;
                    path->cubicTo(p2, p3, p4);
                }
                break;
            }
            default:
                qWarning("QFontEngineWin::addOutlineToPath, unhandled switch case");
            }
            offset += sizeof(TTPOLYCURVE) + (curve->cpfx-1) * sizeof(POINTFX);
        }
        path->closeSubpath();
        headerOffset += ttph->cb;
    }
    delete [] (char*)dataBuffer;
}

void QFontEngineWin::addGlyphsToPath(glyph_t *glyphs, QFixedPoint *positions, int nglyphs,
				     QPainterPath *path, QTextItem::RenderFlags)
{
    HDC hdc = shared_dc;
    SelectObject(hdc, hfont);
    Q_ASSERT(hdc);

    for(int i = 0; i < nglyphs; ++i)
        addGlyphToPath(glyphs[i], positions[i], hdc, path, ttf);
}

void QFontEngineWin::addOutlineToPath(qreal x, qreal y, const QGlyphLayout *glyphs, int numGlyphs,
                                      QPainterPath *path, QTextItem::RenderFlags flags)
{
    if(tm.w.tmPitchAndFamily & (TMPF_TRUETYPE)) {
	QFontEngine::addOutlineToPath(x, y, glyphs, numGlyphs, path, flags);
        return;
    }
    QFontEngine::addBitmapFontToPath(x, y, glyphs, numGlyphs, path, flags);
}

QFontEngine::FaceId QFontEngineWin::faceId() const
{
    return _faceId;
}
#include <qdebug.h>

int QFontEngineWin::synthesized() const
{
    if(synthesized_flags == -1) {
        synthesized_flags = 0;
        if(ttf) {
            const DWORD HEAD = MAKE_TAG('h', 'e', 'a', 'd');
            SelectObject(shared_dc, hfont);
            uchar data[4];
            GetFontData(shared_dc, HEAD, 44, &data, 4);
            USHORT macStyle = getUShort(data);
            if (tm.w.tmItalic && !(macStyle & 2))
                synthesized_flags = SynthesizedItalic;
            if (fontDef.stretch != 100 && ttf)
                synthesized_flags |= SynthesizedStretch;
            if (tm.w.tmWeight >= 500 && !(macStyle & 1))
                synthesized_flags |= SynthesizedBold;
            //qDebug() << "font is" << _name <<
            //    "it=" << (macStyle & 2) << fontDef.style << "flags=" << synthesized_flags;
        }
    }
    return synthesized_flags;
}

QFontEngine::Properties QFontEngineWin::properties() const
{

    LOGFONT lf = logfont;
    lf.lfHeight = unitsPerEm;
    HFONT hf;
    QT_WA({
        hf = CreateFontIndirectW(&lf);
    }, {
        LOGFONTA lfa;
        wa_copy_logfont(&lf, &lfa);
        hf = CreateFontIndirectA(&lfa);
    });
    HGDIOBJ oldfont = SelectObject(shared_dc, hf);
    OUTLINETEXTMETRICA *otm = getOutlineTextMetric(shared_dc);
    Properties p;
    p.emSquare = unitsPerEm;
    p.italicAngle = otm->otmItalicAngle;
    p.postscriptName = (char *)otm + (int)otm->otmpFamilyName;
    p.postscriptName += (char *)otm + (int)otm->otmpStyleName;
    p.postscriptName = QPdf::stripSpecialCharacters(p.postscriptName);
    p.boundingBox = QRectF(otm->otmrcFontBox.left, -otm->otmrcFontBox.top,
                           otm->otmrcFontBox.right - otm->otmrcFontBox.left,
                           otm->otmrcFontBox.top - otm->otmrcFontBox.bottom);
    p.ascent = otm->otmAscent;
    p.descent = -otm->otmDescent;
    p.leading = (int)otm->otmLineGap;
    p.capHeight = 0;
    p.lineWidth = otm->otmsUnderscoreSize;
    free(otm);
    DeleteObject(SelectObject(shared_dc, oldfont));
    return p;
}

void QFontEngineWin::getUnscaledGlyph(glyph_t glyph, QPainterPath *path, glyph_metrics_t *metrics)
{
    LOGFONT lf = logfont;
    lf.lfHeight = unitsPerEm;
    int flags = synthesized();
    if(flags & SynthesizedItalic)
        lf.lfItalic = false;
    lf.lfWidth = 0;
    HFONT hf;
    QT_WA({
        hf = CreateFontIndirectW(&lf);
    }, {
        LOGFONTA lfa;
        wa_copy_logfont(&lf, &lfa);
        hf = CreateFontIndirectA(&lfa);
    });
    HGDIOBJ oldfont = SelectObject(shared_dc, hf);
    QFixedPoint p;
    p.x = 0;
    p.y = 0;
    addGlyphToPath(glyph, p, shared_dc, path, ttf, metrics);
    DeleteObject(SelectObject(shared_dc, oldfont));
}

QByteArray QFontEngineWin::getSfntTable(uint tag) const
{
    if (!ttf)
        return QByteArray();
    SelectObject(shared_dc, hfont);
    DWORD t = qbswap(tag);
    int length = GetFontData(shared_dc, t, 0, NULL, 0);
    QByteArray table;
    if(length > 0) {
        table.resize(length);
        GetFontData(shared_dc, t, 0, table.data(), length);
    }
    return table;
}



// -------------------------------------- Multi font engine

QFontEngineMultiWin::QFontEngineMultiWin(QFontEngineWin *first, const QStringList &fallbacks)
        : QFontEngineMulti(fallbacks.size()+1),
          fallbacks(fallbacks)
{
    ttf = false;
    engines[0] = first;
    first->ref.ref();
    fontDef = engines[0]->fontDef;

    // ### initialize so that the handle() function returns something sensible...
    hfont = (HFONT) GetStockObject(ANSI_VAR_FONT);
}

void QFontEngineMultiWin::loadEngine(int at)
{
    Q_ASSERT(at < engines.size());
    Q_ASSERT(engines.at(at) == 0);

    QString fam = fallbacks.at(at-1);

    LOGFONT lf = engines.at(0)->logfont;
    HFONT hfont;
    QT_WA({
        memcpy(lf.lfFaceName, fam.utf16(), sizeof(TCHAR)*qMin(fam.length()+1,32));  // 32 = Windows hard-coded
        hfont = CreateFontIndirectW(&lf);
    } , {
        // LOGFONTA and LOGFONTW are binary compatible
        QByteArray lname = fam.toLocal8Bit();
        memcpy(lf.lfFaceName,lname.data(),
            qMin(lname.length()+1,32));  // 32 = Windows hard-coded
        hfont = CreateFontIndirectA((LOGFONTA*)&lf);
    });
    bool stockFont = false;
    if (hfont == 0) {
        hfont = (HFONT)GetStockObject(ANSI_VAR_FONT);
        stockFont = true;
    }
    engines[at] = new QFontEngineWin(fam, hfont, stockFont, lf);
    engines[at]->ref.ref();
    engines[at]->fontDef = fontDef;
}


// ----------------------------------------------------------------------------
// True type support methods
// ----------------------------------------------------------------------------

static inline void tag_to_string(char *string, quint32 tag)
{
    string[0] = (tag >> 24)&0xff;
    string[1] = (tag >> 16)&0xff;
    string[2] = (tag >> 8)&0xff;
    string[3] = tag&0xff;
    string[4] = 0;
}

static quint32 getGlyphIndex(unsigned char *table, unsigned int unicode)
{
    unsigned short format = getUShort(table);
    if (format == 0) {
        if (unicode < 256)
            return (int) *(table+6+unicode);
    } else if (format == 4) {
        /* some fonts come with invalid cmap tables, where the last segment
           specified end = start = rangeoffset = 0xffff, delta = 0x0001
           Since 0xffff is never a valid Unicode char anyway, we just get rid of the issue
           by returning 0 for 0xffff
        */
        if(unicode >= 0xffff)
            return 0;
        quint16 segCountX2 = getUShort(table + 6);
        unsigned char *ends = table + 14;
        quint16 endIndex = 0;
        int i = 0;
        for (; i < segCountX2/2 && (endIndex = getUShort(ends + 2*i)) < unicode; i++);

        unsigned char *idx = ends + segCountX2 + 2 + 2*i;
        quint16 startIndex = getUShort(idx);

        if (startIndex > unicode)
            return 0;

        idx += segCountX2;
        qint16 idDelta = (qint16)getUShort(idx);
        idx += segCountX2;
        quint16 idRangeoffset_t = (quint16)getUShort(idx);

        quint16 glyphIndex;
        if (idRangeoffset_t) {
            quint16 id = getUShort(idRangeoffset_t + 2*(unicode - startIndex) + idx);
            if (id)
                glyphIndex = (idDelta + id) % 0x10000;
            else
                glyphIndex = 0;
        } else {
            glyphIndex = (idDelta + unicode) % 0x10000;
        }
        return glyphIndex;
    } else if (format == 12) {
        quint32 nGroups = getUInt(table + 12);

        table += 16; // move to start of groups

        int left = 0, right = nGroups - 1;
        while (left <= right) {
            int middle = left + ( ( right - left ) >> 1 );

            quint32 startCharCode = getUInt(table + 12*middle);
            if(unicode < startCharCode)
                right = middle - 1;
            else {
                quint32 endCharCode = getUInt(table + 12*middle + 4);
                if(unicode <= endCharCode)
                    return getUInt(table + 12*middle + 8) + unicode - startCharCode;
                left = middle + 1;
            }
        }
    } else {
        qDebug("QFontEngineWin::cmap table of format %d not implemented", format);
    }

    return 0;
}


static unsigned char *getCMap(HDC hdc, bool &symbol)
{
    const DWORD CMAP = MAKE_TAG('c', 'm', 'a', 'p');

    unsigned char header[8];

    // get the CMAP header and the number of encoding tables
    DWORD bytes =
#ifndef Q_OS_TEMP
        GetFontData(hdc, CMAP, 0, &header, 4);
#else
        0;
#endif
    if (bytes == GDI_ERROR)
        return 0;
    {
        unsigned short version = getUShort(header);
        if (version != 0)
            return 0;
    }

    unsigned short numTables = getUShort(header+2);
    unsigned char *maps = new unsigned char[8*numTables];

    // get the encoding table and look for Unicode
#ifndef Q_OS_TEMP
    bytes = GetFontData(hdc, CMAP, 4, maps, 8*numTables);
#endif
    if (bytes == GDI_ERROR)
        return 0;

    quint32 version = 0;
    unsigned int unicode_table = 0;
    for (int n = 0; n < numTables; n++) {
        quint32 v = getUInt(maps + 8*n);
        // accept both symbol and Unicode encodings. prefer unicode.
        if(v == 0x00030001 || v == 0x00030000 || v == 0x0003000a) {
            if (v > version) {
                version = v;
                unicode_table = getUInt(maps + 8*n + 4);
            }
        }
    }
    symbol = version == 0x00030000;

    if (!unicode_table) {
        // qDebug("no unicode table found");
        return 0;
    }

    delete [] maps;

    // get the header of the unicode table
#ifndef Q_OS_TEMP
    bytes = GetFontData(hdc, CMAP, unicode_table, &header, 8);
#endif
    if (bytes == GDI_ERROR)
        return 0;

    unsigned short format = getUShort(header);
    unsigned int length;
    if(format < 8)
        length = getUShort(header+2);
    else
        length = getUInt(header+4);
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

bool operator<(const QFontEngineWin::KernPair &p1, const QFontEngineWin::KernPair &p2)
{
    return p1.left_right < p2.left_right;
}

static QVector<QFontEngineWin::KernPair> getKerning(HDC hdc, QFixed factor)
{
    const DWORD KERN = MAKE_TAG('k', 'e', 'r', 'n');

    QVector<QFontEngineWin::KernPair> pairs;
    unsigned short numTables;

    {
        unsigned char header[4];

        // get the KERN header and the number of encoding tables
        DWORD bytes = GetFontData(hdc, KERN, 0, &header, 4);
        if (bytes == GDI_ERROR) {
//            qDebug("table doesn't exist");
            goto end;
        }
        unsigned short version = getUShort(header);
        if (version != 0) {
//            qDebug("wrong version");
            goto end;
        }

        numTables = getUShort(header+2);
    }
    {
        int offset = 4;
        for(int i = 0; i < numTables; ++i) {
            unsigned char header[6];
            DWORD bytes = GetFontData(hdc, KERN, offset, &header, 6);
            if (bytes == GDI_ERROR) {
//                qDebug("GDI_ERROR 2");
                goto end;
            }

            ushort version = getUShort(header);
            ushort length = getUShort(header+2);
            ushort coverage = getUShort(header+4);
//            qDebug("subtable: version=%d, coverage=%x",version, coverage);
            if(version == 0 && coverage == 0x0001) {
                QVarLengthArray<uchar, 4096> data(length - 6);
                bytes = GetFontData(hdc, KERN, offset+6, data.data(), length - 6);
                if (bytes == GDI_ERROR) {
//                    qDebug("GDI_ERROR 3");
                    goto end;
                }

                ushort nPairs = getUShort(data.data());
                if(nPairs * 6 + 8 > length - 6) {
//                    qDebug("corrupt table!");
                    // corrupt table
                    goto end;
                }

                int off = 8;
                for(int i = 0; i < nPairs; ++i) {
                    QFontEngineWin::KernPair p;
                    p.left_right = (((uint)getUShort(data.data()+off)) << 16) + getUShort(data.data()+off+2);
                    p.adjust = QFixed((int)(short)getUShort(data.data()+off+4)) / factor;
                    pairs.append(p);
                    off += 6;
                }
            }
            offset += length;
        }
    }
end:
    qSort(pairs);
    return pairs;
}

