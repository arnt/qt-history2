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

#ifndef QFONTENGINE_P_H
#define QFONTENGINE_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of other Qt classes.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "qatomic.h"
#include "qglobal.h"
#include "qtextengine_p.h"
#include "qfontdata_p.h"

#ifdef Q_WS_WIN
#include "qt_windows.h"
#endif

struct glyph_metrics_t;
class QChar;
typedef unsigned short glyph_t;
class QOpenType;
class QPainterPath;

class QTextEngine;
struct QGlyphLayout;

class QFontEngine
{
public:

    enum Type {
        // X11 types
        Box,
        XLFD,
        LatinXLFD,
        Xft,

        // MS Windows types
        Win,

        // Apple MacOS types
        Mac,

        // Trolltech QWS types
        Freetype,
        QPF,
        TestFontEngine = 0x1000
    };

    enum FECap {
        NoTransformations = 0x00,
        Scale = 0x01,
        Rotate = 0x02,
        RotScale = 0x03,
        Shear = 0x04,
        FullTransformations = 0x0f
    };
    Q_DECLARE_FLAGS(FECaps, FECap)

    inline QFontEngine() {
        ref = 0;
        cache_count = 0;
        _scale = 1;
    }

    virtual ~QFontEngine();

    virtual FECaps capabilites() const = 0;

    /* returns 0 as glyph index for non existant glyphs */
    virtual bool stringToCMap(const QChar *str, int len, QGlyphLayout *glyphs, int *nglyphs, QTextEngine::ShaperFlags flags) const = 0;

#if defined(Q_WS_X11)
    virtual int cmap() const { return -1; }
#endif

    virtual QOpenType *openType() const { return 0; }
    virtual void recalcAdvances(int , QGlyphLayout *, QTextEngine::ShaperFlags) const {}

#if !defined(Q_WS_X11) && !defined(Q_WS_WIN)
    virtual void draw(QPaintEngine *p, int x, int y, const QTextItem &si) = 0;
#endif
    virtual void addOutlineToPath(qReal, qReal, const QGlyphLayout *, int, QPainterPath *) { }
    virtual void addBitmapFontToPath(qReal x, qReal y, const QGlyphLayout *, int, QPainterPath *);

    virtual glyph_metrics_t boundingBox(const QGlyphLayout *glyphs,  int numGlyphs) = 0;
    virtual glyph_metrics_t boundingBox(glyph_t glyph) = 0;

    virtual qReal ascent() const = 0;
    virtual qReal descent() const = 0;
    virtual qReal leading() const = 0;

    virtual qReal lineThickness() const;
    virtual qReal underlinePosition() const;

    virtual qReal maxCharWidth() const = 0;
    virtual qReal minLeftBearing() const { return qReal(); }
    virtual qReal minRightBearing() const { return qReal(); }

    virtual const char *name() const = 0;

    virtual bool canRender(const QChar *string,  int len) = 0;

    virtual void setScale(qReal s) { _scale = s; }
    virtual qReal scale() const { return _scale; }

    virtual Type type() const = 0;

    QAtomic     ref;
    QFontDef fontDef;
    uint cache_cost; // amount of mem used in kb by the font
    int cache_count;

#ifdef Q_WS_WIN
    void getGlyphIndexes(const QChar *ch, int numChars, QGlyphLayout *glyphs, bool mirrored) const;
    void getCMap();

    QString        _name;
    HFONT        hfont;
    LOGFONT     logfont;
    uint        stockFont   : 1;
    uint        useTextOutA : 1;
    uint        ttf         : 1;
    uint        symbol      : 1;
    union {
        TEXTMETRICW        w;
        TEXTMETRICA        a;
    } tm;
    int                lw;
    unsigned char *cmap;
    void *script_cache;
    qReal lbearing;
    qReal rbearing;
#endif // Q_WS_WIN
    qReal _scale;
};


#if defined(Q_WS_QWS)

#include <ft2build.h>
#include FT_FREETYPE_H

class QGlyph;

class QFontEngineFT : public QFontEngine
{
public:
    QFontEngineFT(const QFontDef&, FT_Face face);
   ~QFontEngineFT();
    FT_Face handle() const;

    FECaps capabilites() const;

    QOpenType *openType() const;
    void recalcAdvances(int len, QGlyphLayout *glyphs, QTextEngine::ShaperFlags flags) const;

    /* returns 0 as glyph index for non existant glyphs */
    bool stringToCMap(const QChar *str, int len, QGlyphLayout *glyphs, int *nglyphs, QTextEngine::ShaperFlags flags) const;

    void draw(QPaintEngine *p, int x, int y, const QTextItem &si);
    void addOutlineToPath(qReal x, qReal y, const QGlyphLayout *glyphs, int numGlyphs, QPainterPath *path);

    glyph_metrics_t boundingBox(const QGlyphLayout *glyphs,  int numGlyphs);
    glyph_metrics_t boundingBox(glyph_t glyph);

    qReal ascent() const;
    qReal descent() const;
    qReal leading() const;
    qReal maxCharWidth() const;
    qReal minLeftBearing() const;
    qReal minRightBearing() const;
    qReal underlinePosition() const;
    qReal lineThickness() const;

    Type type() const;

    bool canRender(const QChar *string,  int len);
    inline const char *name() const { return 0; }

    FT_Face face;
    bool smooth;
    QGlyph **rendered_glyphs;
    QOpenType *_openType;

    friend class QFontDatabase;
    static FT_Library ft_library;
};

class QFontEngineQPFData;

class QFontEngineQPF : public QFontEngine
{
public:
    QFontEngineQPF(const QFontDef&, const QString &fn);
   ~QFontEngineQPF();

    FECaps capabilites() const;
    bool stringToCMap(const QChar *str, int len, QGlyphLayout *glyphs, int *nglyphs, QTextEngine::ShaperFlags flags) const;

    void draw(QPaintEngine *p, int x, int y, const QTextItem &si);

    glyph_metrics_t boundingBox(const QGlyphLayout *glyphs,  int numGlyphs);
    glyph_metrics_t boundingBox(glyph_t glyph);

    qReal ascent() const;
    qReal descent() const;
    qReal leading() const;
    qReal maxCharWidth() const;
    qReal minLeftBearing() const;
    qReal minRightBearing() const;
    qReal underlinePosition() const;
    qReal lineThickness() const;

    Type type() const;

    bool canRender(const QChar *string,  int len);
    inline const char *name() const { return 0; }


    QFontEngineQPFData *d;
};

#endif // QWS


class QFontEngineBox : public QFontEngine
{
public:
    QFontEngineBox(int size);
    ~QFontEngineBox();

    FECaps capabilites() const;

    bool stringToCMap(const QChar *str, int len, QGlyphLayout *glyphs, int *nglyphs, QTextEngine::ShaperFlags flags) const;

#if !defined(Q_WS_X11) && !defined(Q_WS_WIN)
    void draw(QPaintEngine *p, int x, int y, const QTextItem &si);
#endif

    glyph_metrics_t boundingBox(const QGlyphLayout *glyphs,  int numGlyphs);
    glyph_metrics_t boundingBox(glyph_t glyph);

    qReal ascent() const;
    qReal descent() const;
    qReal leading() const;
    qReal maxCharWidth() const;
    qReal minLeftBearing() const { return 0; }
    qReal minRightBearing() const { return 0; }

#ifdef Q_WS_X11
    int cmap() const;
#endif
    const char *name() const;

    bool canRender(const QChar *string,  int len);

    Type type() const;
    inline int size() const { return _size; }

private:
    friend class QFontPrivate;
    int _size;
};

#ifdef Q_WS_X11
#include <private/qt_x11_p.h>

class QTextCodec;

#ifndef QT_NO_XFT
#include <ft2build.h>
#include FT_FREETYPE_H

struct TransformedFont
{
    qReal xx;
    qReal xy;
    qReal yx;
    qReal yy;
    union {
        XftFont *xft_font;
    };
    TransformedFont *next;
};

class QFontEngineXft : public QFontEngine
{
public:
    QFontEngineXft(XftFont *font, XftPattern *pattern, int cmap);
    ~QFontEngineXft();

    FECaps capabilites() const;

    QOpenType *openType() const;

    bool stringToCMap(const QChar *str, int len, QGlyphLayout *glyphs, int *nglyphs, QTextEngine::ShaperFlags flags) const;

    virtual void addOutlineToPath(qReal x, qReal y, const QGlyphLayout *glyphs, int numGlyphs, QPainterPath *path);

    glyph_metrics_t boundingBox(const QGlyphLayout *glyphs,  int numGlyphs);
    glyph_metrics_t boundingBox(glyph_t glyph);

    qReal ascent() const;
    qReal descent() const;
    qReal leading() const;
    qReal lineThickness() const;
    qReal underlinePosition() const;
    qReal maxCharWidth() const;
    qReal minLeftBearing() const;
    qReal minRightBearing() const;

    int cmap() const;
    const char *name() const;

    bool canRender(const QChar *string,  int len);

    Type type() const;
    XftPattern *pattern() const { return _pattern; }

    void recalcAdvances(int len, QGlyphLayout *glyphs, QTextEngine::ShaperFlags flags) const;

    FT_Face freetypeFace() const { return _face; }
    XftFont *xftFont() const { return _font; }
    XftFont *transformedFont(const QMatrix &matrix);
private:
    friend class QFontPrivate;
    friend class QOpenType;
    XftFont *_font;
    XftPattern *_pattern;
    FT_Face _face;
    QOpenType *_openType;
    int _cmap;
    qReal lbearing;
    qReal rbearing;
    enum { widthCacheSize = 0x800, cmapCacheSize = 0x500 };
    mutable struct { qReal x; qReal y; } widthCache[widthCacheSize];
    glyph_t cmapCache[cmapCacheSize];

    TransformedFont *transformed_fonts;
};
#endif

class QFontEngineLatinXLFD;

class QFontEngineXLFD : public QFontEngine
{
public:
    QFontEngineXLFD(XFontStruct *fs, const char *name, int cmap);
    ~QFontEngineXLFD();

    FECaps capabilites() const;

    bool stringToCMap(const QChar *str, int len, QGlyphLayout *glyphs, int *nglyphs, QTextEngine::ShaperFlags flags) const;

    glyph_metrics_t boundingBox(const QGlyphLayout *glyphs,  int numGlyphs);
    glyph_metrics_t boundingBox(glyph_t glyph);

    qReal ascent() const;
    qReal descent() const;
    qReal leading() const;
    qReal maxCharWidth() const;
    qReal minLeftBearing() const;
    qReal minRightBearing() const;

    int cmap() const;
    const char *name() const;

    bool canRender(const QChar *string,  int len);

    Type type() const;

    Qt::HANDLE handle() const { return (Qt::HANDLE) _fs->fid; }

private:
    friend class QFontPrivate;
    XFontStruct *_fs;
    QByteArray _name;
    QTextCodec *_codec;
    int _cmap;
    qReal lbearing;
    qReal rbearing;

    friend class QFontEngineLatinXLFD;
};

class QFontEngineLatinXLFD : public QFontEngine
{
public:
    QFontEngineLatinXLFD(XFontStruct *xfs, const char *name, int cmap);
    ~QFontEngineLatinXLFD();

    FECaps capabilites() const;

    bool stringToCMap(const QChar *str, int len, QGlyphLayout *glyphs, int *nglyphs, QTextEngine::ShaperFlags flags) const;

    glyph_metrics_t boundingBox(const QGlyphLayout *glyphs,  int numGlyphs);
    glyph_metrics_t boundingBox(glyph_t glyph);

    qReal ascent() const;
    qReal descent() const;
    qReal leading() const;
    qReal maxCharWidth() const;
    qReal minLeftBearing() const;
    qReal minRightBearing() const;

    int cmap() const { return -1; } // ###
    const char *name() const;

    bool canRender(const QChar *string,  int len);

    void setScale(qReal scale);
    qReal scale() const { return _engines[0]->scale(); }
    Type type() const { return LatinXLFD; }

    Qt::HANDLE handle() const { return ((QFontEngineXLFD *) _engines[0])->handle(); }
    QFontEngine *engine(int i) { return _engines[i]; }

private:
    void findEngine(const QChar &ch);

    QFontEngine **_engines;
    int _count;

    glyph_t   glyphIndices [0x200];
    qReal glyphAdvances[0x200];
    glyph_t euroIndex;
    qReal euroAdvance;
};


#elif defined(Q_WS_MAC)
#include <private/qt_mac_p.h>
#include <qmap.h>
#include <qcache.h>

struct QATSUStyle;
class QFontEngineMac : public QFontEngine
{
    mutable ATSUTextLayout mTextLayout;
    mutable QATSUStyle *internal_fi;
    enum { widthCacheSize = 0x500 };
    mutable int widthCache[widthCacheSize];
    QATSUStyle *getFontStyle() const;

public:
    ATSFontFamilyRef familyref;
    QFontEngineMac();
    ~QFontEngineMac();

    bool stringToCMap(const QChar *str, int len, QGlyphLayout *glyphs, int *nglyphs, QTextEngine::ShaperFlags flags) const;

    void draw(QPaintEngine *p, int x, int y, const QTextItem &si);
    void addOutlineToPath(qReal x, qReal y, const QGlyphLayout *glyphs, int numGlyphs, QPainterPath *path);

    glyph_metrics_t boundingBox(const QGlyphLayout *glyphs,  int numGlyphs);
    glyph_metrics_t boundingBox(glyph_t glyph);

    qReal ascent() const;
    qReal descent() const;
    qReal leading() const;
    qReal maxCharWidth() const;

    const char *name() const { return "ATSUI"; }

    bool canRender(const QChar *string,  int len);

    Type type() const { return QFontEngine::Mac; }

    void calculateCost();

    FECaps capabilites() const { return FullTransformations; }

    enum { WIDTH=0x01, DRAW=0x02, EXISTS=0x04 };
    int doTextTask(const QChar *s, int pos, int use_len, int len, uchar task, int =-1, int y=-1,
                   QPaintEngine *p=NULL) const;
};

#elif defined(Q_WS_WIN)

class QFontEngineWin : public QFontEngine
{
public:
    QFontEngineWin(const QString &name, HFONT, bool, LOGFONT);

    FECaps capabilites() const;

    bool stringToCMap(const QChar *str, int len, QGlyphLayout *glyphs, int *nglyphs, QTextEngine::ShaperFlags flags) const;

    void addOutlineToPath(qReal x, qReal y, const QGlyphLayout *glyphs, int numGlyphs, QPainterPath *path);


    glyph_metrics_t boundingBox(const QGlyphLayout *glyphs,  int numGlyphs);
    glyph_metrics_t boundingBox(glyph_t glyph);

    qReal ascent() const;
    qReal descent() const;
    qReal leading() const;
    qReal maxCharWidth() const;
    qReal minLeftBearing() const;
    qReal minRightBearing() const;

    const char *name() const;

    bool canRender(const QChar *string,  int len);

    Type type() const;

    enum { widthCacheSize = 0x800, cmapCacheSize = 0x500 };
    mutable unsigned char widthCache[widthCacheSize];
};

#endif // Q_WS_WIN

class QTestFontEngine : public QFontEngineBox
{
public:
    QTestFontEngine(int size) : QFontEngineBox(size) {}
    Type type() const { return TestFontEngine; }
};

#endif // QFONTENGINE_P_H
