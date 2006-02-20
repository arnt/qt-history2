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

#ifndef QFONTENGINE_X11_P_H
#define QFONTENGINE_X11_P_H

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
#include <private/qt_x11_p.h>
#ifndef QT_NO_FONTCONFIG
#include <fontconfig/fontconfig.h>
#endif
#ifndef QT_NO_FREETYPE
#include <ft2build.h>
#include FT_FREETYPE_H
#endif

struct QFreetypeFace;

// --------------------------------------------------------------------------

class QFontEngineMultiXLFD : public QFontEngineMulti
{
public:
    QFontEngineMultiXLFD(const QFontDef &r, const QList<int> &l, int s);
    ~QFontEngineMultiXLFD();

    void loadEngine(int at);

private:
    QList<int> encodings;
    int screen;
    QFontDef request;
};

class QFontEngineXLFD : public QFontEngine
{
public:
    QFontEngineXLFD(XFontStruct *f, const QByteArray &name, int mib);
    ~QFontEngineXLFD();

    QFontEngine::FaceId faceId() const;
    QFontEngine::Properties properties() const;
#ifndef QT_NO_FREETYPE
    void getUnscaledGlyph(glyph_t glyph, QPainterPath *path, glyph_metrics_t *metrics);
#endif
    QByteArray getSfntTable(uint tag) const;
    int synthesized() const;
    
    bool stringToCMap(const QChar *str, int len, QGlyphLayout *glyphs, int *nglyphs,
                      QTextEngine::ShaperFlags flags) const;

    glyph_metrics_t boundingBox(const QGlyphLayout *glyphs, int numGlyphs);
    glyph_metrics_t boundingBox(glyph_t glyph);

    void addOutlineToPath(qreal x, qreal y, const QGlyphLayout *glyphs, int numGlyphs, QPainterPath *path, QTextItem::RenderFlags);
    QFixed ascent() const;
    QFixed descent() const;
    QFixed leading() const;
    qreal maxCharWidth() const;
    qreal minLeftBearing() const;
    qreal minRightBearing() const;

    inline Type type() const
    { return QFontEngine::XLFD; }

    bool canRender(const QChar *string, int len);
    const char *name() const;

    inline XFontStruct *fontStruct() const
    { return _fs; }

#ifndef QT_NO_FREETYPE
    FT_Face non_locked_face() const;
    glyph_t glyphIndexToFreetypeGlyphIndex(glyph_t g) const;
#endif

private:
    XFontStruct *_fs;
    QByteArray _name;
    QTextCodec *_codec;
    int _cmap;
    int lbearing, rbearing;
    mutable QFontEngine::FaceId face_id;
    mutable QFreetypeFace *freetype;
    mutable int synth;
};

#ifndef QT_NO_FONTCONFIG

class Q_GUI_EXPORT QFontEngineMultiFT : public QFontEngineMulti
{
public:
    QFontEngineMultiFT(FcFontSet *fs, int s, const QFontDef &request);
    ~QFontEngineMultiFT();

    void loadEngine(int at);

private:
    FcFontSet *fontSet;
    int screen;
};

class Q_GUI_EXPORT QFontEngineFT : public QFontEngine
{
public:
    explicit QFontEngineFT(FcPattern *pattern, const QFontDef &fd, int screen);
    ~QFontEngineFT();

    QFontEngine::FaceId faceId() const { return face_id; }
    QFontEngine::Properties properties() const;
    void getUnscaledGlyph(glyph_t glyph, QPainterPath *path, glyph_metrics_t *metrics);
    QByteArray getSfntTable(uint tag) const;
    int synthesized() const;
    
    bool stringToCMap(const QChar *str, int len, QGlyphLayout *glyphs, int *nglyphs,
                      QTextEngine::ShaperFlags flags) const;

    glyph_metrics_t boundingBox(const QGlyphLayout *glyphs, int numGlyphs);
    glyph_metrics_t boundingBox(glyph_t glyph);

    QFixed ascent() const;
    QFixed descent() const;
    QFixed leading() const;
    QFixed xHeight() const;

    qreal maxCharWidth() const;
    qreal minLeftBearing() const;
    qreal minRightBearing() const;
    QFixed lineThickness() const;
    QFixed underlinePosition() const;

    inline Type type() const
    { return QFontEngine::Freetype; }

    bool canRender(const QChar *string, int len);
    inline const char *name() const
    { return "freetype"; }

    void recalcAdvances(int len, QGlyphLayout *glyphs, QTextEngine::ShaperFlags flags) const;
    void doKerning(int , QGlyphLayout *, QTextEngine::ShaperFlags) const;
    void addGlyphsToPath(glyph_t *glyphs, QFixedPoint *positions, int nglyphs,
                         QPainterPath *path, QTextItem::RenderFlags flags);
    void addOutlineToPath(qreal x, qreal y, const QGlyphLayout *glyphs, int numGlyphs,
                          QPainterPath *path, QTextItem::RenderFlags flags);

    FcPattern *pattern() const { return _pattern; }
    QOpenType *openType() const;

    FT_Face lockFace() const;
    void unlockFace() const;

    FT_Face non_locked_face() const;
    bool drawAsOutline() const { return outline_drawing; }
    bool invalid() const { return xsize == 0 && ysize == 0; }
private:
    QFreetypeFace *freetype;
    QFontEngine::FaceId face_id;

    mutable QFixed lbearing;
    mutable QFixed rbearing;
    QFixed line_thickness;
    QFixed underline_position;
    FcPattern *_pattern;
    int xsize;
    int ysize;
    bool antialias;
    bool outline_drawing;
    int subpixel;
    bool transform;
    int hint_style;
    mutable FT_Matrix matrix; // need mutable because the freetype API doesn't use const

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
    };
    Glyph *loadGlyph(uint glyph, GlyphFormat = Format_None) const;
#ifndef QT_NO_XRENDER
    int xglyph_format;
    GlyphSet glyphSet;
#endif
    inline Glyph *cachedGlyph(glyph_t g) const { return glyph_data.value(g); }
private:
    mutable QOpenType *_openType;
    FT_Size_Metrics metrics;
    mutable QHash<int, Glyph *> glyph_data; // maps from glyph index to glyph data
};

#endif // QT_NO_FONTCONFIG

#endif // QFONTENGINE_X11_P_H
