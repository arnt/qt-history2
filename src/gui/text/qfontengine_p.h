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

#ifndef QFONTENGINE_P_H
#define QFONTENGINE_P_H

#ifndef QT_H
#include "qglobal.h"
#include "qshared.h"
#include "qtextengine_p.h"
#include "qfontdata_p.h"
#endif // QT_H

#ifdef Q_WS_WIN
#include "qt_windows.h"
#endif

class QPaintDevice;

struct glyph_metrics_t;
class QChar;
typedef unsigned short glyph_t;
class QOpenType;

class QTextEngine;
struct QGlyphLayout;

class QFontEngine : public QShared
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
	Freetype
    };

    enum Capabilities {
	NoTransformations = 0x00,
	Scale = 0x01,
	Rotate = 0x02,
	RotScale = 0x03,
	Shear = 0x04,
	FullTransformations = 0x0f
    };
    Q_DECLARE_FLAGS(FECaps, Capabilities);

    QFontEngine() {
	count = 0; cache_count = 0;
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

    virtual void draw(QPaintEngine *p, int x, int y, const QTextItem &si, int textFlags) = 0;

    virtual glyph_metrics_t boundingBox(const QGlyphLayout *glyphs,  int numGlyphs) = 0;
    virtual glyph_metrics_t boundingBox(glyph_t glyph) = 0;

    virtual Q26Dot6 ascent() const = 0;
    virtual Q26Dot6 descent() const = 0;
    virtual Q26Dot6 leading() const = 0;

    virtual Q26Dot6 lineThickness() const;
    virtual Q26Dot6 underlinePosition() const;

    virtual Q26Dot6 maxCharWidth() const = 0;
    virtual Q26Dot6 minLeftBearing() const { return Q26Dot6(); }
    virtual Q26Dot6 minRightBearing() const { return Q26Dot6(); }

    virtual const char *name() const = 0;

    virtual bool canRender(const QChar *string,  int len) = 0;

    virtual void setScale(double s) { _scale = Q26Dot6(s); }
    virtual double scale() const { return _scale.toDouble(); }

    virtual Type type() const = 0;

    QFontDef fontDef;
    uint cache_cost; // amount of mem used in kb by the font
    int cache_count;

#ifdef Q_WS_WIN
    HDC dc() const;
    void getGlyphIndexes(const QChar *ch, int numChars, QGlyphLayout *glyphs, bool mirrored) const;
    void getCMap();

    QString	_name;
    HDC		hdc;
    HFONT	hfont;
    LOGFONT     logfont;
    uint	stockFont   : 1;
    uint	paintDevice : 1;
    uint        useTextOutA : 1;
    uint        ttf         : 1;
    uint        symbol      : 1;
    union {
	TEXTMETRICW	w;
	TEXTMETRICA	a;
    } tm;
    int		lw;
    unsigned char *cmap;
    void *script_cache;
    short lbearing;
    short rbearing;
#endif // Q_WS_WIN
    Q26Dot6 _scale;
};

#if defined(Q_WS_QWS)

#include <ft2build.h>
#include FT_FREETYPE_H

class QGlyph;

class QFontEngineFT : public QFontEngine
{
public:
    QFontEngineFT(const QFontDef&, const QPaintDevice *, FT_Face face);
   ~QFontEngineFT();
    FT_Face handle() const;

    FECaps capabilites() const;

    QOpenType *openType() const;
    void recalcAdvances(int len, QGlyphLayout *glyphs, QTextEngine::ShaperFlags flags) const;

    /* returns 0 as glyph index for non existant glyphs */
    bool stringToCMap(const QChar *str, int len, QGlyphLayout *glyphs, int *nglyphs, QTextEngine::ShaperFlags flags) const;

    void draw(QPaintEngine *p, int x, int y, const QTextItem &si, int textFlags);

    glyph_metrics_t boundingBox(const QGlyphLayout *glyphs,  int numGlyphs);
    glyph_metrics_t boundingBox(glyph_t glyph);

    Q26Dot6 ascent() const;
    Q26Dot6 descent() const;
    Q26Dot6 leading() const;
    Q26Dot6 maxCharWidth() const;
    Q26Dot6 minLeftBearing() const;
    Q26Dot6 minRightBearing() const;
    Q26Dot6 underlinePosition() const;
    Q26Dot6 lineThickness() const;

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
#endif // QWS


class QFontEngineBox : public QFontEngine
{
public:
    QFontEngineBox(int size);
    ~QFontEngineBox();

    FECaps capabilites() const;

    bool stringToCMap(const QChar *str, int len, QGlyphLayout *glyphs, int *nglyphs, QTextEngine::ShaperFlags flags) const;

    void draw(QPaintEngine *p, int x, int y, const QTextItem &si, int textFlags);

    glyph_metrics_t boundingBox(const QGlyphLayout *glyphs,  int numGlyphs);
    glyph_metrics_t boundingBox(glyph_t glyph);

    Q26Dot6 ascent() const;
    Q26Dot6 descent() const;
    Q26Dot6 leading() const;
    Q26Dot6 maxCharWidth() const;
    Q26Dot6 minLeftBearing() const { return 0; }
    Q26Dot6 minRightBearing() const { return 0; }

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

#ifndef QT_NO_XFTFREETYPE
#include <ft2build.h>
#include FT_FREETYPE_H

struct TransformedFont
{
    float xx;
    float xy;
    float yx;
    float yy;
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

    void draw(QPaintEngine *p, int x, int y, const QTextItem &si, int textFlags);

    glyph_metrics_t boundingBox(const QGlyphLayout *glyphs,  int numGlyphs);
    glyph_metrics_t boundingBox(glyph_t glyph);

    Q26Dot6 ascent() const;
    Q26Dot6 descent() const;
    Q26Dot6 leading() const;
    Q26Dot6 lineThickness() const;
    Q26Dot6 underlinePosition() const;
    Q26Dot6 maxCharWidth() const;
    Q26Dot6 minLeftBearing() const;
    Q26Dot6 minRightBearing() const;

    int cmap() const;
    const char *name() const;

    bool canRender(const QChar *string,  int len);

    Type type() const;
    XftPattern *pattern() const { return _pattern; }

    void recalcAdvances(int len, QGlyphLayout *glyphs, QTextEngine::ShaperFlags flags) const;

    FT_Face freetypeFace() const { return _face; }
private:
    friend class QFontPrivate;
    XftFont *_font;
    XftPattern *_pattern;
    FT_Face _face;
    QOpenType *_openType;
    int _cmap;
    short lbearing;
    short rbearing;
    enum { widthCacheSize = 0x800, cmapCacheSize = 0x500 };
    mutable struct { Q26Dot6 x; Q26Dot6 y; } widthCache[widthCacheSize];
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

    void draw(QPaintEngine *p, int x, int y, const QTextItem &si, int textFlags);

    glyph_metrics_t boundingBox(const QGlyphLayout *glyphs,  int numGlyphs);
    glyph_metrics_t boundingBox(glyph_t glyph);

    Q26Dot6 ascent() const;
    Q26Dot6 descent() const;
    Q26Dot6 leading() const;
    Q26Dot6 maxCharWidth() const;
    Q26Dot6 minLeftBearing() const;
    Q26Dot6 minRightBearing() const;

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
    short lbearing;
    short rbearing;

    friend class QFontEngineLatinXLFD;
};

class QFontEngineLatinXLFD : public QFontEngine
{
public:
    QFontEngineLatinXLFD(XFontStruct *xfs, const char *name, int cmap);
    ~QFontEngineLatinXLFD();

    FECaps capabilites() const;

    bool stringToCMap(const QChar *str, int len, QGlyphLayout *glyphs, int *nglyphs, QTextEngine::ShaperFlags flags) const;

    void draw(QPaintEngine *p, int x, int y, const QTextItem &si, int textFlags);

    glyph_metrics_t boundingBox(const QGlyphLayout *glyphs,  int numGlyphs);
    glyph_metrics_t boundingBox(glyph_t glyph);

    Q26Dot6 ascent() const;
    Q26Dot6 descent() const;
    Q26Dot6 leading() const;
    Q26Dot6 maxCharWidth() const;
    Q26Dot6 minLeftBearing() const;
    Q26Dot6 minRightBearing() const;

    int cmap() const { return -1; } // ###
    const char *name() const;

    bool canRender(const QChar *string,  int len);

    void setScale(double scale);
    double scale() const { return _engines[0]->scale(); }
    Type type() const { return LatinXLFD; }

    Qt::HANDLE handle() const { return ((QFontEngineXLFD *) _engines[0])->handle(); }

private:
    void findEngine(const QChar &ch);

    QFontEngine **_engines;
    int _count;

    glyph_t   glyphIndices [0x200];
    Q26Dot6 glyphAdvances[0x200];
    glyph_t euroIndex;
    Q26Dot6 euroAdvance;
};


#elif defined(Q_WS_MAC)
#include "qt_mac.h"
#include <qmap.h>
#include <qcache.h>

struct QATSUStyle;
class QFontEngineMac : public QFontEngine
{
    mutable ATSUTextLayout mTextLayout;
    mutable QATSUStyle *internal_fi;
    enum { widthCacheSize = 0x500 };
    mutable unsigned char widthCache[widthCacheSize];
    QATSUStyle *getFontStyle() const;

public:
    ATSFontRef fontref;
    QFontEngineMac();
    ~QFontEngineMac();

    bool stringToCMap(const QChar *str, int len, QGlyphLayout *glyphs, int *nglyphs, QTextEngine::ShaperFlags flags) const;

    void draw(QPaintEngine *p, int x, int y, const QTextItem &si, int textFlags);

    glyph_metrics_t boundingBox(const QGlyphLayout *glyphs,  int numGlyphs);
    glyph_metrics_t boundingBox(glyph_t glyph);

    Q26Dot6 ascent() const;
    Q26Dot6 descent() const;
    Q26Dot6 leading() const;
    Q26Dot6 maxCharWidth() const;

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
    QFontEngineWin(const QString &name, HDC, HFONT, bool, LOGFONT);

    FECaps capabilites() const;

    bool stringToCMap(const QChar *str, int len, QGlyphLayout *glyphs, int *nglyphs, QTextEngine::ShaperFlags flags) const;

    void draw(QPaintEngine *p, int x, int y, const QTextItem &si, int textFlags);

    glyph_metrics_t boundingBox(const QGlyphLayout *glyphs,  int numGlyphs);
    glyph_metrics_t boundingBox(glyph_t glyph);

    Q26Dot6 ascent() const;
    Q26Dot6 descent() const;
    Q26Dot6 leading() const;
    Q26Dot6 maxCharWidth() const;
    Q26Dot6 minLeftBearing() const;
    Q26Dot6 minRightBearing() const;

    const char *name() const;

    bool canRender(const QChar *string,  int len);

    Type type() const;

    enum { widthCacheSize = 0x800, cmapCacheSize = 0x500 };
    mutable unsigned char widthCache[widthCacheSize];
};

#endif // Q_WS_WIN

#endif
