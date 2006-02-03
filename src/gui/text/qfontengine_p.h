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

#include "QtCore/qglobal.h"
#include "QtCore/qatomic.h"
#include <QtCore/qvarlengtharray.h>
#include "private/qtextengine_p.h"
#include "private/qfont_p.h"

#ifdef Q_WS_WIN
#include "QtCore/qt_windows.h"
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

        // Apple Mac OS types
        Mac,

        // Trolltech QWS types
        Freetype,
        QPF,
        TestFontEngine = 0x1000
    };

    inline QFontEngine() {
        ref = 0;
        cache_count = 0;
        fsType = 0;
#if defined(Q_WS_WIN)
        script_cache = 0;
        cmap = 0;
#endif
    }
    virtual ~QFontEngine();

    // all of these are in unscaled metrics if the engine supports uncsaled metrics,
    // otherwise in design metrics
    struct Properties {
        QByteArray postscriptName;
        QByteArray copyright;
        QRectF boundingBox;
        QFixed emSquare;
        QFixed ascent;
        QFixed descent;
        QFixed leading;
        QFixed italicAngle;
        QFixed capHeight;
        QFixed lineWidth;
    };
    virtual Properties properties() const;
    virtual void getUnscaledGlyph(glyph_t glyph, QPainterPath *path, glyph_metrics_t *metrics);
    virtual QByteArray getSfntTable(uint /*tag*/) const { return QByteArray(); }
    
    struct FaceId {
        FaceId() : index(0), encoding(0) {}
        QByteArray filename;
        int index;
        int encoding;
    };
    virtual FaceId faceId() const { return FaceId(); }
    enum SynthesizedFlags {
        SynthesizedItalic = 0x1,
        SynthesizedBold = 0x2,
        SynthesizedStretch = 0x4
    };
    virtual int synthesized() const { return 0; }

    /* returns 0 as glyph index for non existant glyphs */
    virtual bool stringToCMap(const QChar *str, int len, QGlyphLayout *glyphs, int *nglyphs, QTextEngine::ShaperFlags flags) const = 0;

    virtual QOpenType *openType() const { return 0; }
    virtual void recalcAdvances(int , QGlyphLayout *, QTextEngine::ShaperFlags) const {}
    virtual void doKerning(int , QGlyphLayout *, QTextEngine::ShaperFlags) const {}

#if !defined(Q_WS_X11) && !defined(Q_WS_WIN) && !defined(Q_NEW_MAC_FONTENGINE)
    virtual void draw(QPaintEngine *p, qreal x, qreal y, const QTextItemInt &si) = 0;
#endif
    virtual void addGlyphsToPath(glyph_t *glyphs, QFixedPoint *positions, int nglyphs,
                                 QPainterPath *path, QTextItem::RenderFlags flags);
    void getGlyphPositions(const QGlyphLayout *glyphs, int nglyphs, const QMatrix &matrix, QTextItem::RenderFlags flags, 
                           QVarLengthArray<glyph_t> &glyphs_out, QVarLengthArray<QFixedPoint> &positions);
    
    virtual void addOutlineToPath(qreal, qreal, const QGlyphLayout *, int, QPainterPath *, QTextItem::RenderFlags flags);
    void addBitmapFontToPath(qreal x, qreal y, const QGlyphLayout *, int, QPainterPath *, QTextItem::RenderFlags);

    virtual glyph_metrics_t boundingBox(const QGlyphLayout *glyphs, int numGlyphs) = 0;
    virtual glyph_metrics_t boundingBox(glyph_t glyph) = 0;

    virtual QFixed ascent() const = 0;
    virtual QFixed descent() const = 0;
    virtual QFixed leading() const = 0;
    virtual QFixed xHeight() const;

    virtual QFixed lineThickness() const;
    virtual QFixed underlinePosition() const;

    virtual qreal maxCharWidth() const = 0;
    virtual qreal minLeftBearing() const { return qreal(); }
    virtual qreal minRightBearing() const { return qreal(); }

    virtual const char *name() const = 0;

    virtual bool canRender(const QChar *string, int len) = 0;

    virtual Type type() const = 0;

    QAtomic     ref;
    QFontDef fontDef;
    uint cache_cost; // amount of mem used in kb by the font
    int cache_count;
    uint fsType;

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
        QFixed adjust;
    };
    QVector<KernPair> kerning_pairs;
    QFixed designToDevice;
    int unitsPerEm;
    FaceId _faceId;
    mutable int synthesized_flags;
    mutable QFixed lineWidth;
#elif defined(Q_WS_MAC)
    uint kerning : 1;
#endif // Q_WS_WIN
};

inline bool operator ==(const QFontEngine::FaceId &f1, const QFontEngine::FaceId &f2)
{
    return (f1.index == f2.index) && (f1.encoding == f2.encoding) && (f1.filename == f2.filename);
}

inline uint qHash(const QFontEngine::FaceId &f)
{
    return qHash((f.index << 16) + f.encoding) + qHash(f.filename);
}


class QGlyph;

#if defined(Q_WS_QWS)

#ifndef QT_NO_FREETYPE
#include <ft2build.h>
#include FT_FREETYPE_H

class QFontEngineFT : public QFontEngine
{
public:
    QFontEngineFT(const QFontDef&, FT_Face face);
   ~QFontEngineFT();
    FT_Face handle() const;

    QFontEngine::FaceId faceId() const { return face_id; }
    QFontEngine::Properties properties() const;
    void getUnscaledGlyph(glyph_t glyph, QPainterPath *path, glyph_metrics_t *metrics);
    QByteArray getSfntTable(uint tag) const;
    int synthesized() const;
    
    QOpenType *openType() const;
    void recalcAdvances(int len, QGlyphLayout *glyphs, QTextEngine::ShaperFlags flags) const;

    /* returns 0 as glyph index for non existant glyphs */
    bool stringToCMap(const QChar *str, int len, QGlyphLayout *glyphs, int *nglyphs, QTextEngine::ShaperFlags flags) const;

    void draw(QPaintEngine *p, qreal x, qreal y, const QTextItemInt &si);
    void addOutlineToPath(qreal x, qreal y, const QGlyphLayout *glyphs, int numGlyphs, QPainterPath *path, QTextItem::RenderFlags flags);
    void addGlyphsToPath(glyph_t *glyphs, QFixedPoint *positions, int numGlyphs,
                         QPainterPath *path, QTextItem::RenderFlags flags);
    void doKerning(int , QGlyphLayout *, QTextEngine::ShaperFlags) const;

    glyph_metrics_t boundingBox(const QGlyphLayout *glyphs, int numGlyphs);
    glyph_metrics_t boundingBox(glyph_t glyph);

    QFixed ascent() const;
    QFixed descent() const;
    QFixed leading() const;
    QFixed xHeight() const;
    
    qreal maxCharWidth() const;
    qreal minLeftBearing() const;
    qreal minRightBearing() const;
    QFixed underlinePosition() const;
    QFixed lineThickness() const;

    Type type() const;

    bool canRender(const QChar *string, int len);
    inline const char *name() const { return 0; }

    FT_Face face;
    bool smooth;
    QGlyph **rendered_glyphs;
    QOpenType *_openType;
    enum { cmapCacheSize = 0x200 };
    mutable glyph_t cmapCache[cmapCacheSize];

    FaceId face_id;
    friend class QFontDatabase;
    static FT_Library ft_library;
};
#endif // QT_NO_FREETYPE

#ifndef QT_NO_QWS_QPF

class QFontEngineQPFData;

class QFontEngineQPF : public QFontEngine
{
public:
    QFontEngineQPF(const QFontDef&, const QString &fn);
   ~QFontEngineQPF();

    bool stringToCMap(const QChar *str, int len, QGlyphLayout *glyphs, int *nglyphs, QTextEngine::ShaperFlags flags) const;

    void draw(QPaintEngine *p, qreal x, qreal y, const QTextItemInt &si);
    void addOutlineToPath(qreal x, qreal y, const QGlyphLayout *glyphs, int numGlyphs, QPainterPath *path, QTextItem::RenderFlags flags);

    glyph_metrics_t boundingBox(const QGlyphLayout *glyphs, int numGlyphs);
    glyph_metrics_t boundingBox(glyph_t glyph);

    QFixed ascent() const;
    QFixed descent() const;
    QFixed leading() const;
    qreal maxCharWidth() const;
    qreal minLeftBearing() const;
    qreal minRightBearing() const;
    QFixed underlinePosition() const;
    QFixed lineThickness() const;

    Type type() const;

    bool canRender(const QChar *string, int len);
    inline const char *name() const { return 0; }


    QFontEngineQPFData *d;
};
#endif // QT_NO_QWS_QPF

#endif // QWS


class QFontEngineBox : public QFontEngine
{
public:
    QFontEngineBox(int size);
    ~QFontEngineBox();

    bool stringToCMap(const QChar *str, int len, QGlyphLayout *glyphs, int *nglyphs, QTextEngine::ShaperFlags flags) const;

#if !defined(Q_WS_X11) && !defined(Q_WS_WIN) && !defined(Q_NEW_MAC_FONTENGINE)
    void draw(QPaintEngine *p, qreal x, qreal y, const QTextItemInt &si);
#endif
    void addOutlineToPath(qreal x, qreal y, const QGlyphLayout *glyphs, int numGlyphs, QPainterPath *path, QTextItem::RenderFlags flags);

    glyph_metrics_t boundingBox(const QGlyphLayout *glyphs, int numGlyphs);
    glyph_metrics_t boundingBox(glyph_t glyph);

    QFixed ascent() const;
    QFixed descent() const;
    QFixed leading() const;
    qreal maxCharWidth() const;
    qreal minLeftBearing() const { return 0; }
    qreal minRightBearing() const { return 0; }

#ifdef Q_WS_X11
    int cmap() const;
#endif
    const char *name() const;

    bool canRender(const QChar *string, int len);

    Type type() const;
    inline int size() const { return _size; }

private:
    friend class QFontPrivate;
    int _size;
};

#if defined(Q_WS_MAC)
#include "private/qt_mac_p.h"
#include "QtCore/qmap.h"
#include "QtCore/qcache.h"

#if defined(Q_NEW_MAC_FONTENGINE)

class QFontEngineMac : public QFontEngine
{
public:
    // ####
    ATSFontFamilyRef familyref;

    QFontEngineMac();
    virtual ~QFontEngineMac();

    void init();

    virtual bool stringToCMap(const QChar *str, int len, QGlyphLayout *glyphs, int *nglyphs, QTextEngine::ShaperFlags flags) const;
    virtual void recalcAdvances(int , QGlyphLayout *, QTextEngine::ShaperFlags) const;
    virtual void doKerning(int , QGlyphLayout *, QTextEngine::ShaperFlags) const;

    virtual void addGlyphsToPath(glyph_t *glyphs, QFixedPoint *positions, int numGlyphs,
                                 QPainterPath *path, QTextItem::RenderFlags);

    virtual glyph_metrics_t boundingBox(const QGlyphLayout *glyphs, int numGlyphs);
    virtual glyph_metrics_t boundingBox(glyph_t glyph);

    virtual QFixed ascent() const;
    virtual QFixed descent() const;
    virtual QFixed leading() const;
    virtual QFixed xHeight() const;
    virtual qreal maxCharWidth() const;

    virtual const char *name() const { return "ATSUI"; }

    bool canRender(const QChar *string, int len);

    virtual Type type() const { return QFontEngine::Mac; }

    virtual int synthesized() const { return synthesisFlags; }

    void draw(CGContextRef ctx, qreal x, qreal y, const QTextItemInt &ti, int paintDeviceHeight);

private:
    int fontIndexForFMFont(FMFont font) const;
    ATSUStyle styleForFont(int fontIndex) const;

    mutable ATSUTextLayout textLayout;
    mutable ATSUStyle style;
    int synthesisFlags;
    mutable QVector<FMFont> fonts;
};

#else

struct QATSUStyle;
struct QATSUGlyphInfo;
class QFontEngineMac : public QFontEngine
{
    mutable ATSUTextLayout mTextLayout;
    mutable QATSUStyle *internal_fi;
    enum { widthCacheSize = 0x500 };
    mutable int widthCache[widthCacheSize];
    QATSUStyle *getFontStyle() const;
    mutable QCache<QString, QATSUGlyphInfo> glyphCache;

public:
    ATSFontFamilyRef familyref;
    QFontEngineMac();
    ~QFontEngineMac();

    bool stringToCMap(const QChar *str, int len, QGlyphLayout *glyphs, int *nglyphs, QTextEngine::ShaperFlags flags) const;
    void recalcAdvances(int , QGlyphLayout *, QTextEngine::ShaperFlags) const;
    void doKerning(int , QGlyphLayout *, QTextEngine::ShaperFlags) const;


    void draw(QPaintEngine *p, qreal x, qreal y, const QTextItemInt &si);
    void addOutlineToPath(qreal x, qreal y, const QGlyphLayout *glyphs, int numGlyphs, QPainterPath *path, QTextItem::RenderFlags flags);

    glyph_metrics_t boundingBox(const QGlyphLayout *glyphs, int numGlyphs);
    glyph_metrics_t boundingBox(glyph_t glyph);

    QFixed ascent() const;
    QFixed descent() const;
    QFixed leading() const;
    qreal maxCharWidth() const;

    const char *name() const { return "ATSUI"; }

    bool canRender(const QChar *string, int len);

    Type type() const { return QFontEngine::Mac; }

    void calculateCost();

    enum { WIDTH=0x01, DRAW=0x02, EXISTS=0x04, ADVANCES=0x08 };
    int doTextTask(const QChar *s, int pos, int use_len, int len, uchar task, QFixed x =-1, QFixed y=-1,
                   QPaintEngine *p=0, void **data=0) const;
};

#endif

#endif


class Q_GUI_EXPORT QFontEngineMulti : public QFontEngine
{
public:
    explicit QFontEngineMulti(int engineCount);
    ~QFontEngineMulti();

    bool stringToCMap(const QChar *str, int len, QGlyphLayout *glyphs, int *nglyphs,
                      QTextEngine::ShaperFlags flags) const;

    glyph_metrics_t boundingBox(const QGlyphLayout *glyphs, int numGlyphs);
    glyph_metrics_t boundingBox(glyph_t glyph);

    void recalcAdvances(int , QGlyphLayout *, QTextEngine::ShaperFlags) const;
    void doKerning(int , QGlyphLayout *, QTextEngine::ShaperFlags) const;
    void addOutlineToPath(qreal, qreal, const QGlyphLayout *, int, QPainterPath *, QTextItem::RenderFlags flags);

    QFixed ascent() const;
    QFixed descent() const;
    QFixed leading() const;
    QFixed xHeight() const;
    
    QFixed lineThickness() const;
    QFixed underlinePosition() const;
    qreal maxCharWidth() const;
    qreal minLeftBearing() const;
    qreal minRightBearing() const;

    inline Type type() const
    { return QFontEngine::Multi; }

    bool canRender(const QChar *string, int len);
    inline const char *name() const
    { return "Multi"; }

    QFontEngine *engine(int at) const;

protected:
    friend class QPSPrintEnginePrivate;
    friend class QPSPrintEngineFontMulti;
    virtual void loadEngine(int at) = 0;
    QVector<QFontEngine *> engines;
};


#if defined(Q_WS_X11)
#  include "private/qfontengine_x11_p.h"
#elif defined(Q_WS_WIN)
#  include "private/qfontengine_win_p.h"
#endif

class QTestFontEngine : public QFontEngineBox
{
public:
    QTestFontEngine(int size) : QFontEngineBox(size) {}
    Type type() const { return TestFontEngine; }
};

#endif // QFONTENGINE_P_H
