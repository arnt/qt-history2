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
#ifndef QFONTENGINE_FT_P_H
#define QFONTENGINE_FT_P_H
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

#include "qfontengine_p.h"

#ifndef QT_NO_FREETYPE

#include <ft2build.h>
#include FT_FREETYPE_H

#if defined(Q_WS_X11)
#include <private/qt_x11_p.h>
#endif

#include <unistd.h>

#ifndef QT_NO_FONTCONFIG
#include <fontconfig/fontconfig.h>
#endif

struct QFreetypeFace
{
    void computeSize(const QFontDef &fontDef, int *xsize, int *ysize, bool *outline_drawing);
    QFontEngine::Properties properties() const;
    QByteArray getSfntTable(uint tag) const;

    static QFreetypeFace *getFace(const QFontEngine::FaceId &face_id);
    void release(const QFontEngine::FaceId &face_id);

    void lock() {
        Q_ASSERT(_lock == 0);
        while (!_lock.testAndSet(0, 1))
            usleep(100);
    }
    void unlock() {
        if (!_lock.testAndSet(1, 0))
            Q_ASSERT(false);
    }

    FT_Face face;
#ifndef QT_NO_FONTCONFIG
    FcCharSet *charset;
#endif
    int xsize; // 26.6
    int ysize; // 26.6
    FT_Matrix matrix;
    FT_CharMap unicode_map;
    FT_CharMap symbol_map;

    enum { cmapCacheSize = 0x200 };
    glyph_t cmapCache[cmapCacheSize];

    int fsType() const;

    static void addGlyphToPath(FT_GlyphSlot g, const QFixedPoint &point, QPainterPath *path, bool no_scale = false);
    static void addBitmapToPath(FT_GlyphSlot slot, const QFixedPoint &point, QPainterPath *path, bool = false);

private:
    QFreetypeFace() {}
    ~QFreetypeFace() {}
    QAtomic ref;
    QAtomic _lock;
    QByteArray fontData;
};

class Q_GUI_EXPORT QFontEngineFT : public QFontEngine
{
public:
    enum GlyphFormat {
        Format_None,
        Format_Render = Format_None,
        Format_Mono,
        Format_A8,
        Format_A32
    };

    /* we don't cache glyphs that are too large anyway, so we can make this struct rather small */
    struct Glyph {
        ~Glyph();
        short linearAdvance;
        unsigned char width;
        unsigned char height;
        signed char x;
        signed char y;
        signed char advance;
        signed char format;
        uchar *data;
        unsigned int uploadedToServer : 1;
    };

    enum SubpixelAntialiasingType {
        Subpixel_None,
        Subpixel_RGB,
        Subpixel_BGR,
        Subpixel_VRGB,
        Subpixel_VBGR
    };

#if defined(Q_WS_X11) && !defined(QT_NO_XRENDER)
    typedef XGlyphInfo GlyphInfo;
#else
    struct GlyphInfo {
        unsigned short  width;
        unsigned short  height;
        short           x;
        short           y;
        short           xOff;
        short           yOff;
    };
#endif

    struct QGlyphSet
    {
        QGlyphSet();
        ~QGlyphSet();
        FT_Matrix transformationMatrix;
        unsigned long id; // server sided id, GlyphSet for X11
        mutable QHash<int, Glyph *> glyph_data; // maps from glyph index to glyph data
    };

    QFontEngine::FaceId faceId() const;
    QFontEngine::Properties properties() const;

    QByteArray getSfntTable(uint tag) const;
    int synthesized() const;

    QFixed ascent() const;
    QFixed descent() const;
    QFixed leading() const;
    QFixed xHeight() const;
    QFixed averageCharWidth() const;

    qreal maxCharWidth() const;
    qreal minLeftBearing() const;
    qreal minRightBearing() const;
    QFixed lineThickness() const;
    QFixed underlinePosition() const;

    void doKerning(int , QGlyphLayout *, QTextEngine::ShaperFlags) const;

    inline Type type() const
    { return QFontEngine::Freetype; }
    inline const char *name() const
    { return "freetype"; }

    void getUnscaledGlyph(glyph_t glyph, QPainterPath *path, glyph_metrics_t *metrics);

    bool canRender(const QChar *string, int len);

    void addGlyphsToPath(glyph_t *glyphs, QFixedPoint *positions, int nglyphs,
                         QPainterPath *path, QTextItem::RenderFlags flags);
    void addOutlineToPath(qreal x, qreal y, const QGlyphLayout *glyphs, int numGlyphs,
                          QPainterPath *path, QTextItem::RenderFlags flags);

    bool stringToCMap(const QChar *str, int len, QGlyphLayout *glyphs, int *nglyphs,
                      QTextEngine::ShaperFlags flags) const;

    glyph_metrics_t boundingBox(const QGlyphLayout *glyphs, int numGlyphs);
    glyph_metrics_t boundingBox(glyph_t glyph);

    void recalcAdvances(int len, QGlyphLayout *glyphs, QTextEngine::ShaperFlags flags) const;
    virtual QImage alphaMapForGlyph(glyph_t);

    virtual int glyphCount() const;

    FT_Face lockFace() const;
    void unlockFace() const;

    FT_Face non_locked_face() const;

    inline bool drawAsOutline() const { return outline_drawing; }
    inline bool drawAntialiased() const { return antialias; }
    inline bool invalid() const { return xsize == 0 && ysize == 0; }

    QOpenType *openType() const;

    inline Glyph *loadGlyph(uint glyph, GlyphFormat format = Format_None) const
    { return loadGlyph(&defaultGlyphSet, glyph, format); }
    Glyph *loadGlyph(QGlyphSet *set, uint glyph, GlyphFormat = Format_None) const;

    QGlyphSet *defaultGlyphs() { return &defaultGlyphSet; }

    inline Glyph *cachedGlyph(glyph_t g) const { return defaultGlyphSet.glyph_data.value(g); }

    QGlyphSet *loadTransformedGlyphSet(glyph_t *glyphs, int num_glyphs, const QTransform &matrix,
                                       GlyphFormat format = Format_Render);

#if defined(Q_WS_QWS)
    virtual void draw(QPaintEngine * /*p*/, qreal /*x*/, qreal /*y*/, const QTextItemInt & /*si*/) {}
#endif

    QFontEngineFT(const QFontDef &fd);
    virtual ~QFontEngineFT();

    bool init(FaceId faceId, bool antiaalias);
protected:

    void freeGlyphSets();

    virtual bool uploadGlyphToServer(QGlyphSet *set, uint glyphid, Glyph *g, GlyphInfo *info, int glyphDataSize) const;
    virtual unsigned long allocateServerGlyphSet();
    virtual void freeServerGlyphSet(unsigned long id);

    QFreetypeFace *freetype;
    int default_load_flags;

    bool antialias;
    bool outline_drawing;
    bool transform;
    SubpixelAntialiasingType subpixelType;
    GlyphFormat preferredServerGlyphFormat;

private:
    FT_Matrix matrix;

    QList<QGlyphSet> transformedGlyphSets;
    mutable QGlyphSet defaultGlyphSet;

    QFontEngine::FaceId face_id;

    int xsize;
    int ysize;

    mutable QFixed lbearing;
    mutable QFixed rbearing;
    QFixed line_thickness;
    QFixed underline_position;

    mutable QOpenType *_openType;
    FT_Size_Metrics metrics;
    mutable bool kerning_pairs_loaded;
};

#endif // QT_NO_FREETYPE
#endif // QFONTENGINE_FT_P_H
