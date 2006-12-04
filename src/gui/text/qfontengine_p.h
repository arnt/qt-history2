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

#define MAKE_TAG(ch1, ch2, ch3, ch4) (\
    (((quint32)(ch1)) << 24) | \
    (((quint32)(ch2)) << 16) | \
    (((quint32)(ch3)) << 8) | \
    ((quint32)(ch4)) \
   )

class Q_GUI_EXPORT QFontEngine : public QObject
{
    Q_OBJECT
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
        QPF1,
        QPF2,
        TestFontEngine = 0x1000
    };

    QFontEngine();
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
    virtual void doKerning(int , QGlyphLayout *, QTextEngine::ShaperFlags) const;

#if !defined(Q_WS_X11) && !defined(Q_WS_WIN) && !defined(Q_WS_MAC)
    virtual void draw(QPaintEngine *p, qreal x, qreal y, const QTextItemInt &si) = 0;
#endif
    virtual void addGlyphsToPath(glyph_t *glyphs, QFixedPoint *positions, int nglyphs,
                                 QPainterPath *path, QTextItem::RenderFlags flags);
    void getGlyphPositions(const QGlyphLayout *glyphs, int nglyphs, const QTransform &matrix, QTextItem::RenderFlags flags,
                           QVarLengthArray<glyph_t> &glyphs_out, QVarLengthArray<QFixedPoint> &positions);

    virtual void addOutlineToPath(qreal, qreal, const QGlyphLayout *, int, QPainterPath *, QTextItem::RenderFlags flags);
    void addBitmapFontToPath(qreal x, qreal y, const QGlyphLayout *, int, QPainterPath *, QTextItem::RenderFlags);
    virtual QImage alphaMapForGlyph(glyph_t);

    virtual glyph_metrics_t boundingBox(const QGlyphLayout *glyphs, int numGlyphs) = 0;
    virtual glyph_metrics_t boundingBox(glyph_t glyph) = 0;

    virtual QFixed ascent() const = 0;
    virtual QFixed descent() const = 0;
    virtual QFixed leading() const = 0;
    virtual QFixed xHeight() const;
    virtual QFixed averageCharWidth() const;

    virtual QFixed lineThickness() const;
    virtual QFixed underlinePosition() const;

    virtual qreal maxCharWidth() const = 0;
    virtual qreal minLeftBearing() const { return qreal(); }
    virtual qreal minRightBearing() const { return qreal(); }

    virtual const char *name() const = 0;

    virtual bool canRender(const QChar *string, int len) = 0;

    virtual Type type() const = 0;

    static const uchar *getCMap(const uchar *table, uint tableSize, bool *isSymbolFont, int *cmapSize);
    static quint32 getTrueTypeGlyphIndex(const uchar *cmap, uint unicode);

    QAtomic     ref;
    QFontDef fontDef;
    uint cache_cost; // amount of mem used in kb by the font
    int cache_count;
    uint fsType : 16;
    bool symbol;

#ifdef Q_WS_WIN
    int getGlyphIndexes(const QChar *ch, int numChars, QGlyphLayout *glyphs, bool mirrored) const;
    void getCMap();

    QString        _name;
    HFONT        hfont;
    LOGFONT     logfont;
    uint        stockFont   : 1;
    uint        useTextOutA : 1;
    uint        ttf         : 1;
    union {
        TEXTMETRICW        w;
        TEXTMETRICA        a;
    } tm;
    int                lw;
    unsigned char *cmap;
    void *script_cache;
    qreal lbearing;
    qreal rbearing;
    QFixed designToDevice;
    int unitsPerEm;
    QFixed x_height;
    FaceId _faceId;
    mutable int synthesized_flags;
    mutable QFixed lineWidth;
#endif // Q_WS_WIN
#if defined(Q_WS_WIN) || defined(Q_WS_X11) || defined(Q_WS_QWS)
    struct KernPair {
        uint left_right;
        QFixed adjust;
    };
    QVector<KernPair> kerning_pairs;
    void loadKerningPairs(QFixed scalingFactor);
#endif
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

#ifndef QT_NO_QWS_QPF

class QFontEngineQPF1Data;

class QFontEngineQPF1 : public QFontEngine
{
public:
    QFontEngineQPF1(const QFontDef&, const QString &fn);
   ~QFontEngineQPF1();

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


    QFontEngineQPF1Data *d;
};
#endif // QT_NO_QWS_QPF

#endif // QWS


class QFontEngineBox : public QFontEngine
{
public:
    QFontEngineBox(int size);
    ~QFontEngineBox();

    bool stringToCMap(const QChar *str, int len, QGlyphLayout *glyphs, int *nglyphs, QTextEngine::ShaperFlags flags) const;

#if !defined(Q_WS_X11) && !defined(Q_WS_WIN) && !defined(Q_WS_MAC)
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
    QFixed averageCharWidth() const;

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

#if defined(Q_WS_MAC)
#include "private/qt_mac_p.h"
#include "QtCore/qmap.h"
#include "QtCore/qcache.h"

#include "private/qcore_mac_p.h"

struct QShaperItem;
class QFontEngineMacMulti;

class QFontEngineMac : public QFontEngine
{
    friend class QFontEngineMacMulti;
public:
    QFontEngineMac(ATSUStyle baseStyle, ATSUFontID fontID, const QFontDef &def, QFontEngineMacMulti *multiEngine = 0);
    virtual ~QFontEngineMac();

    virtual bool stringToCMap(const QChar *str, int len, QGlyphLayout *glyphs, int *nglyphs, QTextEngine::ShaperFlags flags) const;

    virtual glyph_metrics_t boundingBox(const QGlyphLayout *glyphs, int numGlyphs);
    virtual glyph_metrics_t boundingBox(glyph_t glyph);

    virtual QFixed ascent() const;
    virtual QFixed descent() const;
    virtual QFixed leading() const;
    virtual QFixed xHeight() const;
    virtual qreal maxCharWidth() const;
    virtual QFixed averageCharWidth() const;

    virtual void addGlyphsToPath(glyph_t *glyphs, QFixedPoint *positions, int numGlyphs,
                                 QPainterPath *path, QTextItem::RenderFlags);

    virtual const char *name() const { return "QFontEngineMac"; }

    virtual bool canRender(const QChar *string, int len);

    virtual int synthesized() const { return synthesisFlags; }

    virtual Type type() const { return QFontEngine::Mac; }

    void draw(CGContextRef ctx, qreal x, qreal y, const QTextItemInt &ti, int paintDeviceHeight);

    virtual FaceId faceId() const;
    virtual QByteArray getSfntTable(uint tag) const;
    virtual Properties properties() const;
    virtual void getUnscaledGlyph(glyph_t glyph, QPainterPath *path, glyph_metrics_t *metrics);
    virtual QImage alphaMapForGlyph(glyph_t);

private:
    ATSUFontID fontID;
    QCFType<CGFontRef> cgFont;
    ATSUStyle style;
    int synthesisFlags;
    mutable QGlyphLayout kashidaGlyph;
    QFontEngineMacMulti *multiEngine;
    mutable const unsigned char *cmap;
    mutable bool symbolCMap;
    mutable QByteArray cmapTable;
};

class QFontEngineMacMulti : public QFontEngineMulti
{
    friend class QFontEngineMac;
public:
    QFontEngineMacMulti(const ATSUFontID &fontID, const QFontDef &fontDef, bool kerning);
    virtual ~QFontEngineMacMulti();

    virtual bool stringToCMap(const QChar *str, int len, QGlyphLayout *glyphs, int *nglyphs, QTextEngine::ShaperFlags flags) const;
    bool stringToCMap(const QChar *str, int len, QGlyphLayout *glyphs, int *nglyphs, QTextEngine::ShaperFlags flags,
                      QShaperItem *shaperItem) const;

    virtual void recalcAdvances(int , QGlyphLayout *, QTextEngine::ShaperFlags) const;
    virtual void doKerning(int , QGlyphLayout *, QTextEngine::ShaperFlags) const;

    virtual const char *name() const { return "ATSUI"; }

    bool canRender(const QChar *string, int len);

    inline ATSUFontID macFontID() const { return fontID; }

protected:
    virtual void loadEngine(int at);

private:
    inline const QFontEngineMac *engineAt(int i) const
    { return static_cast<const QFontEngineMac *>(engines.at(i)); }

    bool stringToCMapInternal(const QChar *str, int len, QGlyphLayout *glyphs, int *nglyphs, QTextEngine::ShaperFlags flags,
                              QShaperItem *shaperItem) const;

    int fontIndexForFontID(ATSUFontID id) const;

    ATSUFontID fontID;
    uint kerning : 1;

    mutable ATSUTextLayout textLayout;
    mutable ATSUStyle style;
};

#endif

#if defined(Q_WS_WIN)
#  include "private/qfontengine_win_p.h"
#endif

class QTestFontEngine : public QFontEngineBox
{
public:
    QTestFontEngine(int size) : QFontEngineBox(size) {}
    Type type() const { return TestFontEngine; }
};

#endif // QFONTENGINE_P_H
