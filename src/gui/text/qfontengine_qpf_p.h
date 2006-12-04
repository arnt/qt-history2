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

#ifndef QFONTENGINE_QPF_P_H
#define QFONTENGINE_QPF_P_H

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
#include <qendian.h>

#if !defined(QT_NO_FREETYPE)
#include "qfontengine_ft_p.h"
#endif

class QOpenType;
class QFontEngine;
class QFreetypeFace;

class Q_GUI_EXPORT QFontEngineQPF : public QFontEngine
{
public:
    // if you add new tags please make sure to update the tables in
    // qfontengine_qpf.cpp and tools/makeqpf2/qpf2.cpp
    enum HeaderTag {
        Tag_FontName,          // 0 string
        Tag_FileName,          // 1 string
        Tag_FileIndex,         // 2 quint32
        Tag_FontRevision,      // 3 quint32
        Tag_FreeText,          // 4 string
        Tag_Ascent,            // 5 QFixed
        Tag_Descent,           // 6 QFixed
        Tag_Leading,           // 7 QFixed
        Tag_XHeight,           // 8 QFixed
        Tag_AverageCharWidth,  // 9 QFixed
        Tag_MaxCharWidth,      // 10 QFixed
        Tag_LineThickness,     // 11 QFixed
        Tag_MinLeftBearing,    // 12 QFixed
        Tag_MinRightBearing,   // 13 QFixed
        Tag_UnderlinePosition, // 14 QFixed
        Tag_GlyphFormat,       // 15 quint8
        Tag_PixelSize,         // 16 quint8
        Tag_Weight,            // 17 quint8
        Tag_Style,             // 18 quint8
        Tag_EndOfHeader,       // 19 string

        NumTags
    };

    enum TagType {
        StringType,
        FixedType,
        UInt8Type,
        UInt32Type
    };

    struct Tag
    {
        quint16 tag;
        quint16 size;
    };

    enum GlyphFormat {
        BitmapGlyphs = 1,
        AlphamapGlyphs = 8
    };

    enum {
        CurrentMajorVersion = 2,
        CurrentMinorVersion = 0
    };

    // The CMap is identical to the TrueType CMap table format
    // The GMap table is a normal array with the total number of
    // covered glyphs in the TrueType font
    enum BlockTag {
        CMapBlock,
        GMapBlock,
        GlyphBlock
    };

    struct Q_PACKED Header
    {
        char magic[4]; // 'QPF2'
        quint32 lock;  // values: 0 = unlocked, 1 = locked, 0xffffffff = read-only
        quint8 majorVersion;
        quint8 minorVersion;
        quint16 dataSize;
    };

    struct Q_PACKED Block
    {
        quint16 tag;
        quint16 pad;
        quint32 dataSize;
    };

    struct Q_PACKED Glyph
    {
        quint8 width;
        quint8 height;
        quint8 bytesPerLine;
        qint8 x;
        qint8 y;
        qint8 advance;
    };

    QFontEngineQPF(const QFontDef &def, const uchar *fontData, int dataSize);
    ~QFontEngineQPF();

    FaceId faceId() const { return face_id; }
    QByteArray getSfntTable(uint tag) const;

    bool stringToCMap(const QChar *str, int len, QGlyphLayout *glyphs, int *nglyphs, QTextEngine::ShaperFlags flags) const;
    void recalcAdvances(int , QGlyphLayout *, QTextEngine::ShaperFlags) const;

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
    inline const char *name() const { return "QPF2"; }

    inline bool isValid() const
    { return fontData && dataSize && cmap && glyphMap && glyphData && glyphMapEntries > 0; }

    const Glyph *findGlyph(glyph_t g) const;

    static bool verifyHeader(const uchar *data, int size);
    static QVariant extractHeaderField(const uchar *data, HeaderTag tag);

#if !defined(QT_NO_FREETYPE)
    FT_Face lockFace() const;
    void unlockFace() const;
    QOpenType *openType() const;
    void doKerning(int num_glyphs, QGlyphLayout *g, QTextEngine::ShaperFlags flags) const;
#endif

private:
    const uchar *fontData;
    int dataSize;
    const uchar *cmap;
    int cmapSize;
    const uchar *glyphMap;
    quint32 glyphMapEntries;
    const uchar *glyphData;
    quint32 glyphDataSize;

    QFreetypeFace *freetype;
    FaceId face_id;
    QByteArray freetypeCMapTable;
    mutable QOpenType *_openType;
    mutable bool kerning_pairs_loaded;
};

#endif // QFONTENGINE_QPF_P_H
