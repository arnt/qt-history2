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
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "qatomic.h"
#include "qglobal.h"
#include "qtextengine_p.h"
#include "qfont_p.h"

#ifdef Q_WS_WIN
#include "qt_windows.h"
#endif

struct glyph_metrics_t;
class QChar;
typedef unsigned int glyph_t;
class QOpenType;
class QPainterPath;

class QTextEngine;
struct QGlyphLayout;

class QFontEngine
{
public:
    enum Type {
        Box,
        Multi,

        // X11 types
        XLFD,

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
#if defined(Q_WS_WIN)
        script_cache = 0;
#endif
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
    virtual void doKerning(int , QGlyphLayout *, QTextEngine::ShaperFlags) const {}

#if !defined(Q_WS_X11) && !defined(Q_WS_WIN)
    virtual void draw(QPaintEngine *p, int x, int y, const QTextItemInt &si) = 0;
#endif
    virtual void addOutlineToPath(qreal, qreal, const QGlyphLayout *, int, QPainterPath *);
    virtual void addBitmapFontToPath(qreal x, qreal y, const QGlyphLayout *, int, QPainterPath *);

    virtual glyph_metrics_t boundingBox(const QGlyphLayout *glyphs,  int numGlyphs) = 0;
    virtual glyph_metrics_t boundingBox(glyph_t glyph) = 0;

    virtual qreal ascent() const = 0;
    virtual qreal descent() const = 0;
    virtual qreal leading() const = 0;

    virtual qreal lineThickness() const;
    virtual qreal underlinePosition() const;

    virtual qreal maxCharWidth() const = 0;
    virtual qreal minLeftBearing() const { return qreal(); }
    virtual qreal minRightBearing() const { return qreal(); }

    virtual const char *name() const = 0;

    virtual bool canRender(const QChar *string,  int len) = 0;

    virtual Type type() const = 0;

    QAtomic     ref;
    QFontDef fontDef;
    uint cache_cost; // amount of mem used in kb by the font
    int cache_count;

#ifdef Q_WS_WIN
    int getGlyphIndexes(const QChar *ch, int numChars, QGlyphLayout *glyphs, bool mirrored) const;
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
    qreal lbearing;
    qreal rbearing;
    struct KernPair {
        uint left_right;
        float adjust;
    };
    QVector<KernPair> kerning_pairs;
    float designToDevice;
    int unitsPerEm;
#endif // Q_WS_WIN
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

    void draw(QPaintEngine *p, int x, int y, const QTextItemInt &si);
    void addOutlineToPath(qreal x, qreal y, const QGlyphLayout *glyphs, int numGlyphs, QPainterPath *path);
    void doKerning(int , QGlyphLayout *, QTextEngine::ShaperFlags) const;

    glyph_metrics_t boundingBox(const QGlyphLayout *glyphs,  int numGlyphs);
    glyph_metrics_t boundingBox(glyph_t glyph);

    qreal ascent() const;
    qreal descent() const;
    qreal leading() const;
    qreal maxCharWidth() const;
    qreal minLeftBearing() const;
    qreal minRightBearing() const;
    qreal underlinePosition() const;
    qreal lineThickness() const;

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

    void draw(QPaintEngine *p, int x, int y, const QTextItemInt &si);

    glyph_metrics_t boundingBox(const QGlyphLayout *glyphs,  int numGlyphs);
    glyph_metrics_t boundingBox(glyph_t glyph);

    qreal ascent() const;
    qreal descent() const;
    qreal leading() const;
    qreal maxCharWidth() const;
    qreal minLeftBearing() const;
    qreal minRightBearing() const;
    qreal underlinePosition() const;
    qreal lineThickness() const;

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
    void draw(QPaintEngine *p, int x, int y, const QTextItemInt &si);
#endif

    glyph_metrics_t boundingBox(const QGlyphLayout *glyphs,  int numGlyphs);
    glyph_metrics_t boundingBox(glyph_t glyph);

    qreal ascent() const;
    qreal descent() const;
    qreal leading() const;
    qreal maxCharWidth() const;
    qreal minLeftBearing() const { return 0; }
    qreal minRightBearing() const { return 0; }

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

#if defined(Q_WS_MAC)
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

    void draw(QPaintEngine *p, int x, int y, const QTextItemInt &si);
    void addOutlineToPath(qreal x, qreal y, const QGlyphLayout *glyphs, int numGlyphs, QPainterPath *path);

    glyph_metrics_t boundingBox(const QGlyphLayout *glyphs,  int numGlyphs);
    glyph_metrics_t boundingBox(glyph_t glyph);

    qreal ascent() const;
    qreal descent() const;
    qreal leading() const;
    qreal maxCharWidth() const;

    const char *name() const { return "ATSUI"; }

    bool canRender(const QChar *string,  int len);

    Type type() const { return QFontEngine::Mac; }

    void calculateCost();

    FECaps capabilites() const { return FullTransformations; }

    enum { WIDTH=0x01, DRAW=0x02, EXISTS=0x04 };
    int doTextTask(const QChar *s, int pos, int use_len, int len, uchar task, int =-1, int y=-1,
                   QPaintEngine *p=NULL) const;
};

#endif


class Q_GUI_EXPORT QFontEngineMulti : public QFontEngine
{
public:
    explicit QFontEngineMulti(int engineCount);
    ~QFontEngineMulti();

    FECaps capabilites() const;

    bool stringToCMap(const QChar *str, int len, QGlyphLayout *glyphs, int *nglyphs,
                      QTextEngine::ShaperFlags flags) const;

    glyph_metrics_t boundingBox(const QGlyphLayout *glyphs,  int numGlyphs);
    glyph_metrics_t boundingBox(glyph_t glyph);

    void recalcAdvances(int , QGlyphLayout *, QTextEngine::ShaperFlags) const;
    void doKerning(int , QGlyphLayout *, QTextEngine::ShaperFlags) const;
    void addOutlineToPath(qreal, qreal, const QGlyphLayout *, int, QPainterPath *);

    qreal ascent() const;
    qreal descent() const;
    qreal leading() const;
    qreal maxCharWidth() const;
    qreal minLeftBearing() const;
    qreal minRightBearing() const;

    inline Type type() const
    { return QFontEngine::Multi; }

    bool canRender(const QChar *string,  int len);
    inline const char *name() const
    { return "Multi"; }

    QFontEngine *engine(int at) const;

protected:
    virtual void loadEngine(int at) = 0;
    QVector<QFontEngine *> engines;
};


#if defined(Q_WS_X11)
#  include "qfontengine_x11_p.h"
#elif defined(Q_WS_WIN)
#  include "qfontengine_win_p.h"
#endif

class QTestFontEngine : public QFontEngineBox
{
public:
    QTestFontEngine(int size) : QFontEngineBox(size) {}
    Type type() const { return TestFontEngine; }
};

#endif // QFONTENGINE_P_H
