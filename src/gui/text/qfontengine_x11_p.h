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

class QFontEngineMultiXLFD : public QFontEngineMulti
{
public:
    QFontEngineMultiXLFD(const QFontDef &r, const QList<int> &l, int s);
    ~QFontEngineMultiXLFD();

    void loadEngine(int at);

private:
    QList<int> encodings;
    int screen;
};

class QFontEngineXLFD : public QFontEngine
{
public:
    QFontEngineXLFD(XFontStruct *f, const char *name, int mib);
    ~QFontEngineXLFD();

    FECaps capabilites() const;

    bool stringToCMap(const QChar *str, int len, QGlyphLayout *glyphs, int *nglyphs,
                      QTextEngine::ShaperFlags flags) const;

    glyph_metrics_t boundingBox(const QGlyphLayout *glyphs, int numGlyphs);
    glyph_metrics_t boundingBox(glyph_t glyph);

    qreal ascent() const;
    qreal descent() const;
    qreal leading() const;
    qreal maxCharWidth() const;
    qreal minLeftBearing() const;
    qreal minRightBearing() const;

    inline Type type() const
    { return QFontEngine::XLFD; }

    bool canRender(const QChar *string, int len);
    const char *name() const;

    inline XFontStruct *fontStruct() const
    { return _fs; }
    int cmap() const;

private:
    XFontStruct *_fs;
    QByteArray _name;
    QTextCodec *_codec;
    int _cmap;
    int lbearing, rbearing;
};

#ifndef QT_NO_FONTCONFIG

#include <fontconfig/fontconfig.h>
#include <ft2build.h>
#include FT_FREETYPE_H

class Q_GUI_EXPORT QFontEngineMultiFT : public QFontEngineMulti
{
public:
    QFontEngineMultiFT(FcFontSet *fs, int s);
    ~QFontEngineMultiFT();

    void loadEngine(int at);

private:
    FcFontSet *fontSet;
    int screen;
};

struct QFreetypeFaceId {
    QByteArray filename;
    int index;
};

struct QFreetypeFace {
    QAtomic ref;
    QAtomic lock;
    FT_Face face;
    FcCharSet *charset;
    int xsize; // 26.6
    int ysize; // 26.6

    enum { cmapCacheSize = 0x200 };
    glyph_t cmapCache[cmapCacheSize];
};

inline bool operator ==(const QFreetypeFaceId &f1, const QFreetypeFaceId &f2)
{
    return f1.index == f2.index && f1.filename == f2.filename;
}

inline uint qHash(const QFreetypeFaceId &f)
{
    return qHash(f.index) + qHash(f.filename);
}

class Q_GUI_EXPORT QFontEngineFT : public QFontEngine
{
public:
    explicit QFontEngineFT(FcPattern *pattern, const QFontDef &fd, int screen);
    ~QFontEngineFT();

    FECaps capabilites() const { return 0; }

    bool stringToCMap(const QChar *str, int len, QGlyphLayout *glyphs, int *nglyphs,
                      QTextEngine::ShaperFlags flags) const;

    glyph_metrics_t boundingBox(const QGlyphLayout *glyphs, int numGlyphs);
    glyph_metrics_t boundingBox(glyph_t glyph);

    qreal ascent() const;
    qreal descent() const;
    qreal leading() const;
    qreal maxCharWidth() const;
    qreal minLeftBearing() const;
    qreal minRightBearing() const;
    qreal lineThickness() const;
    qreal underlinePosition() const;

    inline Type type() const
    { return QFontEngine::Freetype; }

    bool canRender(const QChar *string, int len);
    inline const char *name() const
    { return "freetype"; }

    void recalcAdvances(int len, QGlyphLayout *glyphs, QTextEngine::ShaperFlags flags) const;
    void doKerning(int , QGlyphLayout *, QTextEngine::ShaperFlags) const;
    void addOutlineToPath(qreal x, qreal y, const QGlyphLayout *glyphs, int numGlyphs, QPainterPath *path, QTextItem::RenderFlags flags);

    FcPattern *pattern() const { return _pattern; }
    QOpenType *openType() const;

    FT_Face lockFace() const;
    void unlockFace() const;

    FT_Face non_locked_face() const { return freetype->face; }
    bool drawAsOutline() const { return outline_drawing; }
    bool invalid() const { return xsize == 0 && ysize == 0; }
private:
    void computeSize();

    static QHash<QFreetypeFaceId, QFreetypeFace *> *freetypeFaces;
    QFreetypeFace *freetype;

    mutable qreal lbearing;
    mutable qreal rbearing;
    qreal line_thickness;
    qreal underline_position;
    FcPattern *_pattern;
    int xsize;
    int ysize;
    bool antialias;
    bool outline_drawing;
    int subpixel;

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

    GlyphSet glyphSet;
private:
    mutable QOpenType *_openType;
    FT_Size_Metrics metrics;
    mutable QHash<int, Glyph *> glyph_data; // maps from glyph index to glyph data
};

#endif // QT_NO_FONTCONFIG

#endif // QFONTENGINE_X11_P_H
