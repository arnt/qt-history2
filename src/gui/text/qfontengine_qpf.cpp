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
#include <QtCore/qdir.h>
#include <QtCore/qbuffer.h>
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

#if defined(Q_WS_QWS)
#include "private/qwscommand_qws_p.h"
#include "qwsdisplay_qws.h"
#include "qabstractfontengine_p.h"
#endif
#include "qplatformdefs.h"

#ifdef QT_LSB

#include <dlfcn.h>

#  ifdef QT_NO_MREMAP
#    undef QT_NO_MREMAP
#  endif
#  define MREMAP_MAYMOVE 1
#  define RTLD_DEFAULT   ((void *) 0)

// LSB doesn't standardize mremap - resolve it dynamically at runtime
static void *mremap (void *__addr, size_t __old_len, size_t __new_len, int __flags)
{
    typedef void *(*RemapAddress)(void*, size_t, size_t, int, ...);
    static RemapAddress remapAddr = 0;
    if (!remapAddr) {
        *(void **)(&remapAddr) = ::dlsym(RTLD_DEFAULT, "mremap");
        if (!remapAddr)
            qFatal("Unable to call mremap");
    }

    return remapAddr(__addr, __old_len, __new_len, __flags);
}
#endif

//#define DEBUG_HEADER
//#define DEBUG_FONTENGINE

#if defined(DEBUG_HEADER)
# define DEBUG_VERIFY qDebug
#else
# define DEBUG_VERIFY if (0) qDebug
#endif

#define READ_VERIFY(type, variable) \
    if (tagPtr + sizeof(type) > endPtr) { \
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
            case QFontEngineQPF::BitFieldType:
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
#if defined(DEBUG_HEADER)
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
    if (!g || g >= glyphMapEntries)
        return 0;
    const quint32 *gmapPtr = reinterpret_cast<const quint32 *>(fontData + glyphMapOffset);
    quint32 glyphPos = qFromBigEndian<quint32>(gmapPtr[g]);
    if (glyphPos > glyphDataSize) {
        if (glyphPos == 0xffffffff)
            return 0;
#if defined(DEBUG_FONTENGINE)
        qDebug() << "glyph" << g << "outside of glyphData, remapping font file";
#endif
#ifndef QT_NO_FREETYPE
        const_cast<QFontEngineQPF *>(this)->remapFontData();
#endif
        if (glyphPos > glyphDataSize)
            return 0;
    }
    return reinterpret_cast<const Glyph *>(fontData + glyphDataOffset + glyphPos);
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
                case BitFieldType:
                    return QVariant(QByteArray(reinterpret_cast<const char *>(tagPtr), length));
            }
            return QVariant();
        } else if (tag == Tag_EndOfHeader) {
            break;
        }
        tagPtr += length;
    }

    return QVariant();
}

QString qws_fontCacheDir()
{
    static QString dir;
    if (!dir.isEmpty())
        return dir;
#if defined(Q_WS_QWS)
    extern QString qws_dataDir();
    dir = qws_dataDir();
#else
    dir = QDir::tempPath();
#endif
    dir.append(QLatin1String("/fonts/"));
    QDir qd(dir);
    if (!qd.exists() && !qd.mkpath(dir))
        dir = QDir::tempPath();
    return dir;
}

QList<QByteArray> QFontEngineQPF::cleanUpAfterClientCrash(const QList<int> &crashedClientIds)
{
    QList<QByteArray> removedFonts;
    QDir dir(qws_fontCacheDir(), QLatin1String("*.qsf"));
    for (int i = 0; i < int(dir.count()); ++i) {
        const QByteArray fileName = QFile::encodeName(dir.absoluteFilePath(dir[i]));

        int fd = ::open(fileName.constData(), O_RDONLY);
        if (fd >= 0) {
            void *header = ::mmap(0, sizeof(QFontEngineQPF::Header), PROT_READ, MAP_SHARED, fd, 0);
            if (header && header != MAP_FAILED) {
                quint32 lockValue = reinterpret_cast<QFontEngineQPF::Header *>(header)->lock;

                if (lockValue && crashedClientIds.contains(lockValue)) {
                    removedFonts.append(fileName);
                    QFile::remove(QFile::decodeName(fileName));
                }

                ::munmap(header, sizeof(QFontEngineQPF::Header));
            }
            ::close(fd);
        }
    }
    if (!removedFonts.isEmpty())
        qDebug() << "list of corrupted and removed fonts:" << removedFonts;
    return removedFonts;
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

QFontEngineQPF::QFontEngineQPF(const QFontDef &def, int fileDescriptor, QFontEngine *fontEngine)
    : fd(fileDescriptor), renderingFontEngine(fontEngine)
{
    fontData = 0;
    dataSize = 0;
    fontDef = def;
    cache_cost = 100;
    freetype = 0;
    _openType = 0;
    externalCMap = 0;
    cmapOffset = 0;
    cmapSize = 0;
    glyphMapOffset = 0;
    glyphMapEntries = 0;
    glyphDataOffset = 0;
    glyphDataSize = 0;
    kerning_pairs_loaded = false;
    readOnly = true;

#if defined(DEBUG_FONTENGINE)
    qDebug() << "QFontEngineQPF::QFontEngineQPF( fd =" << fd << ", renderingFontEngine =" << renderingFontEngine << ")";
#endif

    if (fd < 0) {
        if (!renderingFontEngine)
            return;

        fileName = fontDef.family.toLower() + QLatin1String("_")
                   + QString::number(fontDef.pixelSize)
                   + QLatin1String("_") + QString::number(fontDef.weight)
                   + (fontDef.style != QFont::StyleNormal ?
                      QLatin1String("_italic") : QLatin1String(""))
                   + QLatin1String(".qsf");
        fileName.replace(QLatin1Char(' '), QLatin1Char('_'));
        fileName.prepend(qws_fontCacheDir());
        fd = ::open(QFile::encodeName(fileName), O_RDWR | O_EXCL | O_CREAT, 0644);
        if (fd >= 0) {
#if defined(DEBUG_FONTENGINE)
            qDebug() << "creating qpf on the fly:" << fileName;
#endif
            QBuffer buffer;
            buffer.open(QIODevice::ReadWrite);
            QPFGenerator generator(&buffer, renderingFontEngine);
            generator.generate();
            buffer.close();
            const QByteArray &data = buffer.data();
            ::write(fd, data.constData(), data.size());
        } else {
#if defined(DEBUG_FONTENGINE)
            qDebug() << "found existing qpf:" << fileName;
#endif
            fd = ::open(QFile::encodeName(fileName), O_RDWR);
        }
    }

    QT_STATBUF st;
    if (QT_FSTAT(fd, &st)) {
#if defined(DEBUG_FONTENGINE)
        qDebug() << "stat failed!";
#endif
        return;
    }
    dataSize = st.st_size;

    fontData = (const uchar *)::mmap(0, st.st_size, PROT_READ | (renderingFontEngine ? PROT_WRITE : 0), MAP_SHARED, fd, 0);
    if (!fontData || fontData == (const uchar *)MAP_FAILED) {
#if defined(DEBUG_FONTENGINE)
        perror("mmap failed");
#endif
        fontData = 0;
        return;
    }

    if (!verifyHeader(fontData, st.st_size)) {
#if defined(DEBUG_FONTENGINE)
        qDebug() << "verifyHeader failed!";
#endif
        return;
    }

    const Header *header = reinterpret_cast<const Header *>(fontData);

    readOnly = (header->lock == 0xffffffff);

    const uchar *data = fontData + sizeof(Header) + qFromBigEndian<quint16>(header->dataSize);
    const uchar *endPtr = fontData + dataSize;
    while (data <= endPtr - 8) {
        quint16 blockTag = readValue<quint16>(data);
        data += 2; // skip padding
        quint32 blockSize = readValue<quint32>(data);

        if (blockTag == CMapBlock) {
            cmapOffset = data - fontData;
            cmapSize = blockSize;
        } else if (blockTag == GMapBlock) {
            glyphMapOffset = data - fontData;
            glyphMapEntries = blockSize / 4;
        } else if (blockTag == GlyphBlock) {
            glyphDataOffset = data - fontData;
            glyphDataSize = blockSize;
        }

        data += blockSize;
    }

    face_id.filename = QFile::encodeName(extractHeaderField(fontData, Tag_FileName).toString());
    face_id.index = extractHeaderField(fontData, Tag_FileIndex).toInt();
#if !defined(QT_NO_FREETYPE)
    freetype = QFreetypeFace::getFace(face_id);
    if (!freetype) {
        QString newPath =
#ifndef QT_NO_SETTINGS
            QLibraryInfo::location(QLibraryInfo::LibrariesPath) +
#endif
                          QLatin1String("/fonts/") +
                          QFileInfo(QFile::decodeName(face_id.filename)).fileName();
        face_id.filename = QFile::encodeName(newPath);
        freetype = QFreetypeFace::getFace(face_id);
    }
    if (freetype) {
        const quint32 qpfTtfRevision = extractHeaderField(fontData, Tag_FontRevision).toUInt();
        const QByteArray head = freetype->getSfntTable(MAKE_TAG('h', 'e', 'a', 'd'));
        if (head.size() < 8
            || qFromBigEndian<quint32>(reinterpret_cast<const uchar *>(head.constData()) + 4) != qpfTtfRevision) {
            freetype->release(face_id);
            freetype = 0;
        }
    }
    if (!cmapOffset && freetype) {
        freetypeCMapTable = freetype->getSfntTable(MAKE_TAG('c', 'm', 'a', 'p'));
        externalCMap = reinterpret_cast<const uchar *>(freetypeCMapTable.constData());
        cmapSize = freetypeCMapTable.size();
    }
#endif

    // get the real cmap
    if (cmapOffset) {
        int tableSize = cmapSize;
        const uchar *cmapPtr = getCMap(fontData + cmapOffset, tableSize, &symbol, &cmapSize);
        if (cmapPtr)
            cmapOffset = cmapPtr - fontData;
        else
            cmapOffset = 0;
    } else if (externalCMap) {
        int tableSize = cmapSize;
        externalCMap = getCMap(externalCMap, tableSize, &symbol, &cmapSize);
    }

    // verify all the positions in the glyphMap
    if (glyphMapOffset) {
        const quint32 *gmapPtr = reinterpret_cast<const quint32 *>(fontData + glyphMapOffset);
        for (uint i = 0; i < glyphMapEntries; ++i) {
            quint32 glyphDataPos = qFromBigEndian<quint32>(gmapPtr[i]);
            if (glyphDataPos == 0xffffffff)
                continue;
            if (glyphDataPos >= glyphDataSize) {
                // error
                glyphMapOffset = 0;
                glyphMapEntries = 0;
                break;
            }
        }
    }

#if defined(DEBUG_FONTENGINE)
    if (!isValid())
        qDebug() << "fontData" <<  fontData << "dataSize" << dataSize
                 << "externalCMap" << externalCMap << "cmapOffset" << cmapOffset
                 << "glyphMapOffset" << glyphMapOffset << "glyphDataOffset" << glyphDataOffset
                 << "fd" << fd << "glyphDataSize" << glyphDataSize;
#endif
#if defined(Q_WS_QWS)
    if (isValid() && renderingFontEngine)
        qt_fbdpy->sendFontCommand(QWSFontCommand::StartedUsingFont, QFile::encodeName(fileName));
#endif
}

QFontEngineQPF::~QFontEngineQPF()
{
#if defined(Q_WS_QWS)
    if (isValid() && renderingFontEngine)
        qt_fbdpy->sendFontCommand(QWSFontCommand::StoppedUsingFont, QFile::encodeName(fileName));
#endif
    delete renderingFontEngine;
    if (fontData)
        munmap((void *)fontData, dataSize);
    if (fd != -1)
        ::close(fd);
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
    if (!externalCMap && !cmapOffset && renderingFontEngine) {
        if (!renderingFontEngine->stringToCMap(str, len, glyphs, nglyphs, flags))
            return false;
#ifndef QT_NO_FREETYPE
        const_cast<QFontEngineQPF *>(this)->ensureGlyphsLoaded(glyphs, *nglyphs);
#endif
        return true;
    }

    if (*nglyphs < len) {
        *nglyphs = len;
        return false;
    }

#if defined(DEBUG_FONTENGINE)
    QSet<QChar> seenGlyphs;
#endif

    const uchar *cmap = externalCMap ? externalCMap : (fontData + cmapOffset);

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
#if 0 && defined(DEBUG_FONTENGINE)
            QChar c(uc);
            if (!findGlyph(glyphs[glyph_pos].glyph) && !seenGlyphs.contains(c))
                qDebug() << "glyph for character" << c << "/" << hex << uc << "is" << dec << glyphs[glyph_pos].glyph;

            seenGlyphs.insert(c);
#endif
            ++glyph_pos;
        }
    }

    *nglyphs = glyph_pos;
    recalcAdvances(*nglyphs, glyphs, flags);
    return true;
}

void QFontEngineQPF::recalcAdvances(int len, QGlyphLayout *glyphs, QTextEngine::ShaperFlags) const
{
#ifndef QT_NO_FREETYPE
    const_cast<QFontEngineQPF *>(this)->ensureGlyphsLoaded(glyphs, len);
#endif
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
    if (renderingFontEngine &&
        (renderingFontEngine->type() != QFontEngine::Proxy
         || static_cast<QProxyFontEngine *>(renderingFontEngine)->capabilities() & QAbstractFontEngine::CanOutlineGlyphs)) {
        renderingFontEngine->addOutlineToPath(x, y, glyphs, numGlyphs, path, flags);
        return;
    }
    addBitmapFontToPath(x, y, glyphs, numGlyphs, path, flags);
}

glyph_metrics_t QFontEngineQPF::boundingBox(const QGlyphLayout *glyphs, int numGlyphs)
{
#ifndef QT_NO_FREETYPE
    const_cast<QFontEngineQPF *>(this)->ensureGlyphsLoaded(glyphs, numGlyphs);
#endif

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
#ifndef QT_NO_FREETYPE
    {
        QGlyphLayout tmp;
        tmp.glyph = glyph;
        const_cast<QFontEngineQPF *>(this)->ensureGlyphsLoaded(&tmp, 1);
    }
#endif
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
    const uchar *cmap = externalCMap ? externalCMap : (fontData + cmapOffset);

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

bool QFontEngineQPF::isValid() const
{
    return fontData && dataSize && (cmapOffset || externalCMap || renderingFontEngine)
           && glyphMapOffset && glyphDataOffset && (fd >= 0 || glyphDataSize > 0);
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

void QFontEngineQPF::ensureGlyphsLoaded(const QGlyphLayout *glyphs, int len)
{
    if (readOnly)
        return;
    bool locked = false;
    for (int i = 0; i < len; ++i) {
        if (!glyphs[i].glyph)
            continue;
        const Glyph *g = findGlyph(glyphs[i].glyph);
        if (g)
            continue;
        if (!locked) {
            if (!lockFile())
                return;
            locked = true;
            g = findGlyph(glyphs[i].glyph);
            if (g)
                continue;
        }
        loadGlyph(glyphs[i].glyph);
    }
    if (locked) {
        unlockFile();
#if defined(DEBUG_FONTENGINE)
        qDebug() << "Finished rendering glyphs\n";
#endif
    }
}

void QFontEngineQPF::loadGlyph(glyph_t glyph)
{
    quint32 glyphPos = ~0;

    if (!renderingFontEngine)
        return;

    QImage img = renderingFontEngine->alphaMapForGlyph(glyph).convertToFormat(QImage::Format_Indexed8);
    glyph_metrics_t metrics = renderingFontEngine->boundingBox(glyph);
    renderingFontEngine->removeGlyphFromCache(glyph);

    off_t oldSize = ::lseek(fd, 0, SEEK_END);
    if (oldSize == (off_t)-1)
        return;

    Glyph g;
    g.width = img.width();
    g.height = img.height();
    g.bytesPerLine = img.bytesPerLine();
    g.x = qRound(metrics.x);
    g.y = qRound(metrics.y);
    g.advance = qRound(metrics.xoff);

    ::write(fd, &g, sizeof(g));
    ::write(fd, img.bits(), img.numBytes());

    glyphPos = oldSize - glyphDataOffset;
#if 0 && defined(DEBUG_FONTENGINE)
    qDebug() << "glyphPos for new glyph" << glyph << "is" << glyphPos << "oldSize" << oldSize << "glyphDataOffset" << glyphDataOffset;
#endif

    quint32 *gmap = (quint32 *)(fontData + glyphMapOffset);
    gmap[glyph] = qToBigEndian(glyphPos);

    glyphDataSize = glyphPos + sizeof(g) + img.numBytes();
    quint32 *blockSizePtr = (quint32 *)(fontData + glyphDataOffset - 4);
    *blockSizePtr = qToBigEndian(glyphDataSize);
}

bool QFontEngineQPF::lockFile()
{
    // #### this does not handle the case when the process holding the
    // lock hangs for some reason
    struct flock lock;
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0; // lock the whole file
    while (fcntl(fd, F_SETLKW, &lock) != 0) {
        if (errno == EINTR)
            continue;
        perror("locking qpf");
        return false;
    }
    Header *header = (Header *)fontData;
    if (header->lock) {
        lock.l_type = F_UNLCK;
        if (fcntl(fd, F_SETLK, &lock) != 0)
            perror("unlocking possibly corrupt qpf");
        return false;
    }
#if defined(Q_WS_QWS)
    extern int qws_client_id;
    // qws_client_id == 0 means we're the server. in this case we just
    // set the id to 1
    header->lock = qws_client_id ? qws_client_id : 1;
#else
    header->lock = 1;
#endif
    return true;
}

void QFontEngineQPF::unlockFile()
{
    ((Header *)fontData)->lock = 0;

    struct flock lock;
    lock.l_type = F_UNLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0; // lock the whole file
    if (fcntl(fd, F_SETLK, &lock) != 0) {
        perror("unlocking qpf");
    }

    remapFontData();
}

void QFontEngineQPF::remapFontData()
{
    off_t newFileSize = ::lseek(fd, 0, SEEK_END);
    if (newFileSize == (off_t)-1) {
#ifdef DEBUG_FONTENGINE
        perror("QFontEngineQPF::remapFontData: lseek failed");
#endif
        fontData = 0;
        return;
    }

#ifndef QT_NO_MREMAP
    fontData = static_cast<uchar *>(::mremap(const_cast<uchar *>(fontData), dataSize, newFileSize, MREMAP_MAYMOVE));
    if (!fontData || fontData == (const uchar *)MAP_FAILED) {
#  if defined(DEBUG_FONTENGINE)
        perror("QFontEngineQPF::remapFontData(): mremap failed");
#  endif
        fontData = 0;
    }

    if (!fontData)
#endif // QT_NO_MREMAP
    {
        int status = ::munmap((void *)fontData, dataSize);
        if (status != 0)
            qErrnoWarning(status, "QFontEngineQPF::remapFomrData: munmap failed!");

        fontData = (const uchar *)::mmap(0, newFileSize, PROT_READ | (renderingFontEngine ? PROT_WRITE : 0),
                                         MAP_SHARED, fd, 0);
        if (!fontData || fontData == (const uchar *)MAP_FAILED) {
#  if defined(DEBUG_FONTENGINE)
            perror("mmap failed");
#  endif
            fontData = 0;
            return;
        }
    }

    dataSize = newFileSize;
    glyphDataSize = newFileSize - glyphDataOffset;
#if defined(DEBUG_FONTENGINE)
    qDebug() << "remapped the font file to" << newFileSize << "bytes";
#endif
}

#endif // QT_NO_FREETYPE

void QPFGenerator::generate()
{
    writeHeader();
    writeGMap();
    writeBlock(QFontEngineQPF::GlyphBlock, QByteArray());

    dev->seek(4); // position of header.lock
    writeUInt32(0);
}

void QPFGenerator::writeHeader()
{
    QFontEngineQPF::Header header;

    header.magic[0] = 'Q';
    header.magic[1] = 'P';
    header.magic[2] = 'F';
    header.magic[3] = '2';
    header.lock = 1;
    header.majorVersion = QFontEngineQPF::CurrentMajorVersion;
    header.minorVersion = QFontEngineQPF::CurrentMinorVersion;
    header.dataSize = 0;
    dev->write((const char *)&header, sizeof(header));

    writeTaggedString(QFontEngineQPF::Tag_FontName, fe->fontDef.family.toUtf8());

    QFontEngine::FaceId face = fe->faceId();
    writeTaggedString(QFontEngineQPF::Tag_FileName, face.filename);
    writeTaggedUInt32(QFontEngineQPF::Tag_FileIndex, face.index);

    {
        const QByteArray head = fe->getSfntTable(MAKE_TAG('h', 'e', 'a', 'd'));
        const quint32 revision = qFromBigEndian<quint32>(reinterpret_cast<const uchar *>(head.constData()) + 4);
        writeTaggedUInt32(QFontEngineQPF::Tag_FontRevision, revision);
    }

    writeTaggedQFixed(QFontEngineQPF::Tag_Ascent, fe->ascent());
    writeTaggedQFixed(QFontEngineQPF::Tag_Descent, fe->descent());
    writeTaggedQFixed(QFontEngineQPF::Tag_Leading, fe->leading());
    writeTaggedQFixed(QFontEngineQPF::Tag_XHeight, fe->xHeight());
    writeTaggedQFixed(QFontEngineQPF::Tag_AverageCharWidth, fe->averageCharWidth());
    writeTaggedQFixed(QFontEngineQPF::Tag_MaxCharWidth, QFixed::fromReal(fe->maxCharWidth()));
    writeTaggedQFixed(QFontEngineQPF::Tag_LineThickness, fe->lineThickness());
    writeTaggedQFixed(QFontEngineQPF::Tag_MinLeftBearing, QFixed::fromReal(fe->minLeftBearing()));
    writeTaggedQFixed(QFontEngineQPF::Tag_MinRightBearing, QFixed::fromReal(fe->minRightBearing()));
    writeTaggedQFixed(QFontEngineQPF::Tag_UnderlinePosition, fe->underlinePosition());
    writeTaggedUInt8(QFontEngineQPF::Tag_PixelSize, fe->fontDef.pixelSize);
    writeTaggedUInt8(QFontEngineQPF::Tag_Weight, fe->fontDef.weight);
    writeTaggedUInt8(QFontEngineQPF::Tag_Style, fe->fontDef.style);

    writeTaggedUInt8(QFontEngineQPF::Tag_GlyphFormat, QFontEngineQPF::AlphamapGlyphs);

    writeTaggedString(QFontEngineQPF::Tag_EndOfHeader, QByteArray());
    align4();

    const quint64 size = dev->pos();
    header.dataSize = qToBigEndian<quint16>(size - sizeof(header));
    dev->seek(0);
    dev->write((const char *)&header, sizeof(header));
    dev->seek(size);
}

void QPFGenerator::writeGMap()
{
    const quint16 glyphCount = fe->glyphCount();

    writeUInt16(QFontEngineQPF::GMapBlock);
    writeUInt16(0); // padding
    writeUInt32(glyphCount * 4);

    const quint32 emptyGlyph = 0xffffffff;
    for (quint16 i = 0; i < glyphCount; ++i)
        dev->write((const char *)&emptyGlyph, sizeof(emptyGlyph));
}

void QPFGenerator::writeBlock(QFontEngineQPF::BlockTag tag, const QByteArray &data)
{
    writeUInt16(tag);
    writeUInt16(0); // padding
    const int padSize = ((data.size() + 3) / 4) * 4 - data.size();
    writeUInt32(data.size() + padSize);
    dev->write(data);
    for (int i = 0; i < padSize; ++i)
        writeUInt8(0);
}

void QPFGenerator::writeTaggedString(QFontEngineQPF::HeaderTag tag, const QByteArray &string)
{
    writeUInt16(tag);
    writeUInt16(string.length());
    dev->write(string);
}

void QPFGenerator::writeTaggedUInt32(QFontEngineQPF::HeaderTag tag, quint32 value)
{
    writeUInt16(tag);
    writeUInt16(sizeof(value));
    writeUInt32(value);
}

void QPFGenerator::writeTaggedUInt8(QFontEngineQPF::HeaderTag tag, quint8 value)
{
    writeUInt16(tag);
    writeUInt16(sizeof(value));
    writeUInt8(value);
}

void QPFGenerator::writeTaggedQFixed(QFontEngineQPF::HeaderTag tag, QFixed value)
{
    writeUInt16(tag);
    writeUInt16(sizeof(quint32));
    writeUInt32(value.value());
}

QFontEngineMultiQWS::QFontEngineMultiQWS(QFontEngine *fe, int _script, const QStringList &fallbacks)
    : QFontEngineMulti(fallbacks.size() + 1),
      fallbackFamilies(fallbacks), script(_script)
{
    engines[0] = fe;
    fe->ref.ref();
    fontDef = engines[0]->fontDef;
}

void QFontEngineMultiQWS::loadEngine(int at)
{
    Q_ASSERT(at < engines.size());
    Q_ASSERT(engines.at(at) == 0);

    QFontDef request = fontDef;
    request.styleStrategy |= QFont::NoFontMerging;
    request.family = fallbackFamilies.at(at-1);
    engines[at] = QFontDatabase::findFont(script,
                                          /*fontprivate*/0,
                                          request);
    Q_ASSERT(engines[at]);
    engines[at]->ref.ref();
    engines[at]->fontDef = request;
}

void QFontEngineMultiQWS::draw(QPaintEngine */*p*/, qreal /*x*/, qreal /*y*/, const QTextItemInt &/*si*/)
{
    qFatal("QFontEngineMultiQWS::draw should never be called!");
}


