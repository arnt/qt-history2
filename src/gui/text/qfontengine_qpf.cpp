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

#include "qfontengine_qpf_p.h"
#include "private/qpaintengine_raster_p.h"
#include <QtCore/qlibraryinfo.h>
#include <QtCore/qfileinfo.h>

#include <QtCore/qfile.h>
#if !defined(QT_NO_FREETYPE)
#include "private/qfontengine_ft_p.h"
#include "qopentype_p.h"
#endif

// for mmap
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>

#include "qpfutil.cpp"

//#define DEBUG_QPF

#if defined(DEBUG_QPF)
# define DEBUG_VERIFY qDebug
#else
# define DEBUG_VERIFY if (0) qDebug
#endif

#define READ_VERIFY(type, variable) \
    if (tagPtr + sizeof(type) >= endPtr) { \
        DEBUG_VERIFY() << "read verify failed in line" << __LINE__; \
        return 0; \
    } \
    variable = qFromBigEndian<type>(tagPtr); \
    DEBUG_VERIFY() << "read value" << variable << "of type " #type; \
    tagPtr += sizeof(type)

template <typename T>
T readValue(const uchar *&data)
{
    T value = qFromBigEndian<T>(data);
    data += sizeof(T);
    return value;
}

#define VERIFY(condition) \
    if (!(condition)) { \
        DEBUG_VERIFY() << "condition " #condition " failed in line" << __LINE__; \
        return 0; \
    }

#define VERIFY_TAG(condition) \
    if (!(condition)) { \
        DEBUG_VERIFY() << "verifying tag condition " #condition " failed in line" << __LINE__ << "with tag" << tag; \
        return 0; \
    }

static inline const uchar *verifyTag(const uchar *tagPtr, const uchar *endPtr)
{
    quint16 tag, length;
    READ_VERIFY(quint16, tag);
    READ_VERIFY(quint16, length);
    if (tag == QFontEngineQPF::Tag_EndOfHeader)
        return endPtr;
    if (tag < QFontEngineQPF::NumTags) {
        switch (tagTypes[tag]) {
            case QFontEngineQPF::StringType:
                // can't do anything...
                break;
            case QFontEngineQPF::UInt32Type:
                VERIFY_TAG(length == sizeof(quint32));
                break;
            case QFontEngineQPF::FixedType:
                VERIFY_TAG(length == sizeof(quint32));
                break;
            case QFontEngineQPF::UInt8Type:
                VERIFY_TAG(length == sizeof(quint8));
                break;
        }
#if defined(DEBUG_QPF)
        if (length == 1)
            qDebug() << "tag data" << hex << *tagPtr;
        else if (length == 4)
            qDebug() << "tag data" << hex << tagPtr[0] << tagPtr[1] << tagPtr[2] << tagPtr[3];
#endif
    }
    return tagPtr + length;
}

const QFontEngineQPF::Glyph *QFontEngineQPF::findGlyph(glyph_t g) const
{
    if (g >= glyphMapEntries)
        return 0;
    quint32 glyphPos = qFromBigEndian<quint32>(glyphMap + g * sizeof(quint32));
    if (glyphPos > glyphDataSize)
        return 0;
    return reinterpret_cast<const Glyph *>(glyphData + glyphPos);
}

bool QFontEngineQPF::verifyHeader(const uchar *data, int size)
{
    VERIFY(size >= int(sizeof(Header)));
    const Header *header = reinterpret_cast<const Header *>(data);
    if (header->magic[0] != 'Q'
        || header->magic[1] != 'P'
        || header->magic[2] != 'F'
        || header->magic[3] != '2')
        return false;

    VERIFY(header->majorVersion <= CurrentMajorVersion);
    const quint16 dataSize = qFromBigEndian<quint16>(header->dataSize);
    VERIFY(size >= int(sizeof(Header)) + dataSize);

    const uchar *tagPtr = data + sizeof(Header);
    const uchar *tagEndPtr = tagPtr + dataSize;
    while (tagPtr < tagEndPtr - 3) {
        tagPtr = verifyTag(tagPtr, tagEndPtr);
        VERIFY(tagPtr);
    }

    VERIFY(tagPtr <= tagEndPtr);

#if 0
    const uchar *blockPtr = tagEndPtr;
    const uchar *endPtr = data + size;
    while (blockPtr < endPtr) {
        blockPtr = verifyBlock(blockPtr);
        if (!blockPtr)
            return false;
    }

    return blockPtr == endPtr;
#endif
    return true;
}

QVariant QFontEngineQPF::extractHeaderField(const uchar *data, HeaderTag requestedTag)
{
    const Header *header = reinterpret_cast<const Header *>(data);
    const uchar *tagPtr = data + sizeof(Header);
    const uchar *endPtr = tagPtr + qFromBigEndian<quint16>(header->dataSize);
    while (tagPtr < endPtr - 3) {
        quint16 tag = readValue<quint16>(tagPtr);
        quint16 length = readValue<quint16>(tagPtr);
        if (tag == requestedTag) {
            switch (tagTypes[requestedTag]) {
                case StringType:
                    return QVariant(QString::fromUtf8(reinterpret_cast<const char *>(tagPtr), length));
                case UInt32Type:
                    return QVariant(readValue<quint32>(tagPtr));
                case UInt8Type:
                    return QVariant(uint(*tagPtr));
                case FixedType:
                    return QVariant(QFixed::fromFixed(readValue<quint32>(tagPtr)).toReal());
            }
            return QVariant();
        } else if (tag == Tag_EndOfHeader) {
            break;
        }
        tagPtr += length;
    }

    return QVariant();
}

static inline unsigned int getChar(const QChar *str, int &i, const int len)
{
    unsigned int uc = str[i].unicode();
    if (uc >= 0xd800 && uc < 0xdc00 && i < len-1) {
        uint low = str[i+1].unicode();
       if (low >= 0xdc00 && low < 0xe000) {
            uc = (uc - 0xd800)*0x400 + (low - 0xdc00) + 0x10000;
            ++i;
        }
    }
    return uc;
}

QFontEngineQPF::QFontEngineQPF(const QFontDef &def, const uchar *fontData, int dataSize)
    : fontData(fontData), dataSize(dataSize)
{
    fontDef = def;
    cache_cost = 100;
    freetype = 0;
    _openType = 0;
    cmap = 0;
    cmapSize = 0;
    glyphMap = 0;
    glyphMapEntries = 0;
    glyphData = 0;
    glyphDataSize = 0;

    const Header *header = reinterpret_cast<const Header *>(fontData);
    const uchar *data = fontData + sizeof(Header) + qFromBigEndian<quint16>(header->dataSize);
    const uchar *endPtr = fontData + dataSize;
    while (data < endPtr) {
        if (data >= endPtr - 8)
            break;
        quint16 blockTag = readValue<quint16>(data);
        data += 2; // skip padding
        quint32 blockSize = readValue<quint32>(data);

        if (blockTag == CMapBlock) {
            cmap = data;
            cmapSize = blockSize;
        } else if (blockTag == GMapBlock) {
            glyphMap = data;
            glyphMapEntries = blockSize / 4;
        } else if (blockTag == GlyphBlock) {
            glyphData = data;
            glyphDataSize = blockSize;
        }

        data += blockSize;
    }

    face_id.filename = QFile::encodeName(extractHeaderField(fontData, Tag_FileName).toString());
    if (!QFile::exists(face_id.filename)) {
        QString newPath = QLibraryInfo::location(QLibraryInfo::LibrariesPath)
                          + QLatin1String("/fonts/")
                          + QFileInfo(face_id.filename).fileName();
        face_id.filename = QFile::encodeName(newPath);
    }
    face_id.index = extractHeaderField(fontData, Tag_FileIndex).toInt();
#if !defined(QT_NO_FREETYPE)
    freetype = QFreetypeFace::getFace(face_id);
    if (freetype) {
        const quint32 qpfTtfRevision = extractHeaderField(fontData, Tag_FontRevision).toUInt();
        const QByteArray head = freetype->getSfntTable(MAKE_TAG('h', 'e', 'a', 'd'));
        if (head.size() < 8
            || qFromBigEndian<quint32>(reinterpret_cast<const uchar *>(head.constData()) + 4) != qpfTtfRevision) {
            freetype->release(face_id);
            freetype = 0;
        }
    }
    if (!cmap && freetype) {
        freetypeCMapTable = freetype->getSfntTable(MAKE_TAG('c', 'm', 'a', 'p'));
        cmap = reinterpret_cast<const uchar *>(freetypeCMapTable.constData());
        cmapSize = freetypeCMapTable.size();
    }
#endif

    // get the real cmap
    if (cmap) {
        int tableSize = cmapSize;
        cmap = getCMap(cmap, tableSize, &symbol, &cmapSize);
    }

    // verify all the positions in the glyphMap
    if (glyphMap) {
        for (uint i = 0; i < glyphMapEntries; ++i) {
            quint32 glyphDataPos = qFromBigEndian<quint32>(glyphMap + i * sizeof(quint32));
            if (glyphDataPos == 0xffffffff)
                continue;
            if (glyphDataPos >= glyphDataSize) {
                // error
                glyphMap = 0;
                glyphMapEntries = 0;
                break;
            }
        }
    }

//    qDebug() << "fontData" <<  fontData << "dataSize" << dataSize << "cmap" << cmap << "glyphMap" << glyphMap << "glyphData" << glyphData;
//    qDebug() << "QPF engine is valid?" << isValid();
}

QFontEngineQPF::~QFontEngineQPF()
{
    munmap((void *)fontData, dataSize);
#if !defined(QT_NO_FREETYPE)
    delete _openType;
    _openType = 0;
    if (freetype)
        freetype->release(face_id);
#endif
}

QByteArray QFontEngineQPF::getSfntTable(uint tag) const
{
#if !defined(QT_NO_FREETYPE)
    if (freetype)
        return freetype->getSfntTable(tag);
#else
    Q_UNUSED(tag);
#endif
    return QByteArray();
}

bool QFontEngineQPF::stringToCMap(const QChar *str, int len, QGlyphLayout *glyphs, int *nglyphs, QTextEngine::ShaperFlags flags) const
{
    if (*nglyphs < len) {
        *nglyphs = len;
        return false;
    }

    int glyph_pos = 0;
    if (symbol) {
        for (int i = 0; i < len; ++i) {
            unsigned int uc = getChar(str, i, len);
            glyphs[glyph_pos].glyph = getTrueTypeGlyphIndex(cmap, uc);
            if(!glyphs[glyph_pos].glyph && uc < 0x100)
                glyphs[glyph_pos].glyph = getTrueTypeGlyphIndex(cmap, uc + 0xf000);
            ++glyph_pos;
        }
    } else {
        for (int i = 0; i < len; ++i) {
            unsigned int uc = getChar(str, i, len);
            glyphs[glyph_pos].glyph = getTrueTypeGlyphIndex(cmap, uc);
            ++glyph_pos;
        }
    }

    *nglyphs = glyph_pos;
    recalcAdvances(*nglyphs, glyphs, flags);
    return true;
}

void QFontEngineQPF::recalcAdvances(int len, QGlyphLayout *glyphs, QTextEngine::ShaperFlags) const
{
    for (int i = 0; i < len; ++i) {
        const Glyph *g = findGlyph(glyphs[i].glyph);
        if (!g) {
            glyphs[i].glyph = 0;
            continue;
        }
        glyphs[i].advance.x = g->advance;
        glyphs[i].advance.y = 0;
    }
}

void QFontEngineQPF::draw(QPaintEngine *p, qreal _x, qreal _y, const QTextItemInt &si)
{
    QPaintEngineState *pState = p->state;
    QRasterPaintEngine *paintEngine = static_cast<QRasterPaintEngine*>(p);

    QTransform matrix = pState->transform();
    matrix.translate(_x, _y);
    QFixed x = QFixed::fromReal(matrix.dx());
    QFixed y = QFixed::fromReal(matrix.dy());

    QVarLengthArray<QFixedPoint> positions;
    QVarLengthArray<glyph_t> glyphs;
    getGlyphPositions(si.glyphs, si.num_glyphs, matrix, si.flags, glyphs, positions);
    if (glyphs.size() == 0)
        return;

    for(int i = 0; i < glyphs.size(); i++) {
        const Glyph *glyph = findGlyph(glyphs[i]);
        if (!glyph)
            continue;

        const bool mono = false; // ###

        paintEngine->alphaPenBlt(reinterpret_cast<const uchar *>(glyph) + sizeof(Glyph), glyph->bytesPerLine, mono,
                                     qRound(positions[i].x) + glyph->x,
                                     qRound(positions[i].y) + glyph->y,
                                     glyph->width, glyph->height);
    }
}

void QFontEngineQPF::addOutlineToPath(qreal x, qreal y, const QGlyphLayout *glyphs, int numGlyphs, QPainterPath *path, QTextItem::RenderFlags flags)
{
    addBitmapFontToPath(x, y, glyphs, numGlyphs, path, flags);
}

glyph_metrics_t QFontEngineQPF::boundingBox(const QGlyphLayout *glyphs, int numGlyphs)
{
    glyph_metrics_t overall;
    // initialize with line height, we get the same behaviour on all platforms
    overall.y = -ascent();
    overall.height = ascent() + descent() + 1;

    QFixed ymax = 0;
    QFixed xmax = 0;
    for (int i = 0; i < numGlyphs; i++) {
        const Glyph *g = findGlyph(glyphs[i].glyph);
        if (!g)
            continue;

        QFixed x = overall.xoff + glyphs[i].offset.x + g->x;
        QFixed y = overall.yoff + glyphs[i].offset.y + g->y;
        overall.x = qMin(overall.x, x);
        overall.y = qMin(overall.y, y);
        xmax = qMax(xmax, x + g->width);
        ymax = qMax(ymax, y + g->height);
        overall.xoff += g->advance;
    }
    overall.height = qMax(overall.height, ymax - overall.y);
    overall.width = xmax - overall.x;

    return overall;
}

glyph_metrics_t QFontEngineQPF::boundingBox(glyph_t glyph)
{
    glyph_metrics_t overall;
    const Glyph *g = findGlyph(glyph);
    if (!g)
        return overall;
    overall.x = g->x;
    overall.y = g->y;
    overall.width = g->width;
    overall.height = g->height;
    overall.xoff = g->advance;
    return overall;
}

QFixed QFontEngineQPF::ascent() const
{
    return QFixed::fromReal(extractHeaderField(fontData, Tag_Ascent).value<qreal>());
}

QFixed QFontEngineQPF::descent() const
{
    return QFixed::fromReal(extractHeaderField(fontData, Tag_Descent).value<qreal>());
}

QFixed QFontEngineQPF::leading() const
{
    return QFixed::fromReal(extractHeaderField(fontData, Tag_Leading).value<qreal>());
}

qreal QFontEngineQPF::maxCharWidth() const
{
    return extractHeaderField(fontData, Tag_MaxCharWidth).value<qreal>();
}

qreal QFontEngineQPF::minLeftBearing() const
{
    return extractHeaderField(fontData, Tag_MinLeftBearing).value<qreal>();
}

qreal QFontEngineQPF::minRightBearing() const
{
    return extractHeaderField(fontData, Tag_MinRightBearing).value<qreal>();
}

QFixed QFontEngineQPF::underlinePosition() const
{
    return QFixed::fromReal(extractHeaderField(fontData, Tag_UnderlinePosition).value<qreal>());
}

QFixed QFontEngineQPF::lineThickness() const
{
    return QFixed::fromReal(extractHeaderField(fontData, Tag_LineThickness).value<qreal>());
}

QFontEngine::Type QFontEngineQPF::type() const
{
    return QFontEngine::QPF2;
}

bool QFontEngineQPF::canRender(const QChar *string, int len)
{
    if (symbol) {
        for (int i = 0; i < len; ++i) {
            unsigned int uc = getChar(string, i, len);
            glyph_t g = getTrueTypeGlyphIndex(cmap, uc);
            if(!g && uc < 0x100)
                g = getTrueTypeGlyphIndex(cmap, uc + 0xf000);
            if (!g)
                return false;
        }
    } else {
        for (int i = 0; i < len; ++i) {
            unsigned int uc = getChar(string, i, len);
            if (!getTrueTypeGlyphIndex(cmap, uc))
                return false;
        }
    }
    return true;
}

#if !defined(QT_NO_FREETYPE)
FT_Face QFontEngineQPF::lockFace() const
{
    Q_ASSERT(freetype);
    freetype->lock();
    FT_Face face = freetype->face;

    // ### not perfect
    const int ysize = fontDef.pixelSize << 6;
    const int xsize = ysize;

    if (freetype->xsize != xsize || freetype->ysize != ysize) {
        FT_Set_Char_Size(face, xsize, ysize, 0, 0);
        freetype->xsize = xsize;
        freetype->ysize = ysize;
    }
    FT_Matrix identityMatrix;
    identityMatrix.xx = 0x10000;
    identityMatrix.yy = 0x10000;
    identityMatrix.xy = 0;
    identityMatrix.yx = 0;
    if (freetype->matrix.xx != identityMatrix.xx ||
        freetype->matrix.yy != identityMatrix.yy ||
        freetype->matrix.xy != identityMatrix.xy ||
        freetype->matrix.yx != identityMatrix.yx) {
        freetype->matrix = identityMatrix;
        FT_Set_Transform(face, &freetype->matrix, 0);
    }
    return face;
}

void QFontEngineQPF::unlockFace() const
{
    freetype->unlock();
}

QOpenType *QFontEngineQPF::openType() const
{
    if (!freetype)
        return 0;
    if (_openType)
         return _openType;

    FT_Face face = lockFace();
    if (!face || !FT_IS_SFNT(face)) {
        unlockFace();
        return 0;
    }

    _openType = new QOpenType(const_cast<QFontEngineQPF *>(this), face);
    unlockFace();
    return _openType;
}

void QFontEngineQPF::doKerning(int num_glyphs, QGlyphLayout *g, QTextEngine::ShaperFlags flags) const
{
    if (!kerning_pairs_loaded) {
        kerning_pairs_loaded = true;
        if (freetype && freetype->face->size->metrics.x_ppem != 0) {
            lockFace();
            QFixed scalingFactor(freetype->face->units_per_EM/freetype->face->size->metrics.x_ppem);
            unlockFace();
            const_cast<QFontEngineQPF *>(this)->loadKerningPairs(scalingFactor);
        }
    }
    QFontEngine::doKerning(num_glyphs, g, flags);
}

#endif
