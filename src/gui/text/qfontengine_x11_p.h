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

    glyph_metrics_t boundingBox(const QGlyphLayout *glyphs,  int numGlyphs);
    glyph_metrics_t boundingBox(glyph_t glyph);

    qreal ascent() const;
    qreal descent() const;
    qreal leading() const;
    qreal maxCharWidth() const;
    qreal minLeftBearing() const;
    qreal minRightBearing() const;

    inline Type type() const
    { return QFontEngine::XLFD; }

    bool canRender(const QChar *string,  int len);
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

#ifndef QT_NO_XFT

class QFontEngineMultiXft : public QFontEngineMulti
{
public:
    QFontEngineMultiXft(FcFontSet *fs, int s);
    ~QFontEngineMultiXft();

    void loadEngine(int at);

private:
    FcFontSet *fontSet;
    int screen;
};

struct TransformedFont
{
    qreal xx;
    qreal xy;
    qreal yx;
    qreal yy;
    union {
        XftFont *xft_font;
    };
    TransformedFont *next;
};

class QFontEngineXft : public QFontEngine
{
public:
    explicit QFontEngineXft(XftFont *f);
    ~QFontEngineXft();

    FECaps capabilites() const;

    bool stringToCMap(const QChar *str, int len, QGlyphLayout *glyphs, int *nglyphs,
                      QTextEngine::ShaperFlags flags) const;

    glyph_metrics_t boundingBox(const QGlyphLayout *glyphs,  int numGlyphs);
    glyph_metrics_t boundingBox(glyph_t glyph);

    qreal ascent() const;
    qreal descent() const;
    qreal leading() const;
    qreal maxCharWidth() const;
    qreal minLeftBearing() const;
    qreal minRightBearing() const;
    qreal lineThickness() const;

    inline Type type() const
    { return QFontEngine::Xft; }

    bool canRender(const QChar *string,  int len);
    inline const char *name() const
    { return "xft"; }

    void recalcAdvances(int len, QGlyphLayout *glyphs, QTextEngine::ShaperFlags flags) const;
    void doKerning(int , QGlyphLayout *, QTextEngine::ShaperFlags) const;
    void addOutlineToPath(qreal x, qreal y, const QGlyphLayout *glyphs, int numGlyphs,
                          QPainterPath *path);

    inline FT_Face freetypeFace() const
    { return _face; }
    inline XftFont *xftFont() const
    { return _font; }
    inline FcPattern *pattern() const
    { return _font->pattern; }
    QOpenType *openType() const;
    inline int cmap() const
    { return _cmap; }
    XftFont *transformedFont(const QMatrix &matrix);

private:
    FT_Face _face;
    XftFont *_font;
    QOpenType *_openType;
    int _cmap;
    TransformedFont *transformed_fonts;
    qreal lbearing;
    qreal rbearing;

    enum { cmapCacheSize = 0x500 };
    mutable uint advanceCacheSize;
    mutable float *advanceCache;
    mutable uint designAdvanceCacheSize;
    mutable float *designAdvanceCache;
    glyph_t cmapCache[cmapCacheSize];
};

#endif // QT_NO_XFT

#endif // QFONTENGINE_X11_P_H
