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

#include "qpf2.h"

#include <private/qfontengine_p.h>

#include "../../src/gui/text/qpfutil.cpp"

int QPF::debugVerbosity = 0;

static const char *headerTagNames[QFontEngineQPF::NumTags] = {
    "FontName",
    "FileName",
    "FileIndex",
    "FontRevision",
    "FreeText",
    "Ascent",
    "Descent",
    "Leading",
    "XHeight",
    "AverageCharWidth",
    "MaxCharWidth",
    "LineThickness",
    "MinLeftBearing",
    "MinRightBearing",
    "UnderlinePosition",
    "GlyphFormat",
    "PixelSize",
    "Weight",
    "Style",
    "EndOfHeader"
};

QByteArray QPF::generate(QFontEngine *fontEngine, int options)
{
    QPF font;

    font.options = options;
    font.addHeader(fontEngine);
    if (options & IncludeCMap)
        font.addCMap(fontEngine);
    font.addGlyphs(fontEngine);

    return font.qpf;
}

void QPF::addHeader(QFontEngine *fontEngine)
{
    QFontEngineQPF::Header *header = reinterpret_cast<QFontEngineQPF::Header *>(addBytes(sizeof(QFontEngineQPF::Header)));

    header->magic[0] = 'Q';
    header->magic[1] = 'P';
    header->magic[2] = 'F';
    header->magic[3] = '2';
    if (options & RenderGlyphs)
        header->lock = 0xffffffff;
    else
        header->lock = 0;
    header->majorVersion = QFontEngineQPF::CurrentMajorVersion;
    header->minorVersion = QFontEngineQPF::CurrentMinorVersion;
    header->dataSize = 0;
    int oldSize = qpf.size();

    addTaggedString(QFontEngineQPF::Tag_FontName, fontEngine->fontDef.family.toUtf8());

    QFontEngine::FaceId face = fontEngine->faceId();
    addTaggedString(QFontEngineQPF::Tag_FileName, face.filename);
    addTaggedUInt32(QFontEngineQPF::Tag_FileIndex, face.index);

    {
        const QByteArray head = fontEngine->getSfntTable(MAKE_TAG('h', 'e', 'a', 'd'));
        const quint32 revision = qFromBigEndian<quint32>(reinterpret_cast<const uchar *>(head.constData()) + 4);
        addTaggedUInt32(QFontEngineQPF::Tag_FontRevision, revision);
    }

    addTaggedQFixed(QFontEngineQPF::Tag_Ascent, fontEngine->ascent());
    addTaggedQFixed(QFontEngineQPF::Tag_Descent, fontEngine->descent());
    addTaggedQFixed(QFontEngineQPF::Tag_Leading, fontEngine->leading());
    addTaggedQFixed(QFontEngineQPF::Tag_XHeight, fontEngine->xHeight());
    addTaggedQFixed(QFontEngineQPF::Tag_AverageCharWidth, fontEngine->averageCharWidth());
    addTaggedQFixed(QFontEngineQPF::Tag_MaxCharWidth, QFixed::fromReal(fontEngine->maxCharWidth()));
    addTaggedQFixed(QFontEngineQPF::Tag_LineThickness, fontEngine->lineThickness());
    addTaggedQFixed(QFontEngineQPF::Tag_MinLeftBearing, QFixed::fromReal(fontEngine->minLeftBearing()));
    addTaggedQFixed(QFontEngineQPF::Tag_MinRightBearing, QFixed::fromReal(fontEngine->minRightBearing()));
    addTaggedQFixed(QFontEngineQPF::Tag_UnderlinePosition, fontEngine->underlinePosition());
    addTaggedUInt8(QFontEngineQPF::Tag_PixelSize, fontEngine->fontDef.pixelSize);
    addTaggedUInt8(QFontEngineQPF::Tag_Weight, fontEngine->fontDef.weight);
    addTaggedUInt8(QFontEngineQPF::Tag_Style, fontEngine->fontDef.style);

    addTaggedUInt8(QFontEngineQPF::Tag_GlyphFormat, QFontEngineQPF::AlphamapGlyphs);

    addTaggedString(QFontEngineQPF::Tag_EndOfHeader, QByteArray());
    align4();
    header = reinterpret_cast<QFontEngineQPF::Header *>(qpf.data());
    header->dataSize = qToBigEndian<quint16>(qpf.size() - oldSize);
}

void QPF::addCMap(QFontEngine *fontEngine)
{
    QByteArray cmapTable = fontEngine->getSfntTable(MAKE_TAG('c', 'm', 'a', 'p'));
    addBlock(QFontEngineQPF::CMapBlock, cmapTable);
}

void QPF::addGlyphs(QFontEngine *fe)
{
    QByteArray maxpTable = fe->getSfntTable(MAKE_TAG('m', 'a', 'x', 'p'));
    const quint16 glyphCount = qFromBigEndian<quint16>(reinterpret_cast<const uchar *>(maxpTable.constData() + 4));

    QByteArray gmap;
    gmap.resize(glyphCount * sizeof(quint32));
    gmap.fill(char(0xff));
    //qDebug() << "glyphCount" << glyphCount;

    QByteArray glyphs;
    if (options & RenderGlyphs) {
        // this is only a rough estimation
        glyphs.reserve(glyphCount 
                * (sizeof(QFontEngineQPF::Glyph) 
                    + qRound(fe->maxCharWidth() * (fe->ascent() + fe->descent()).toReal())));

        QGlyphLayout layout[10];
        for (uint uc = 0; uc < 0x10000; ++uc) {
            QChar ch(uc);
            int nglyphs = 10;
            if (!fe->stringToCMap(&ch, 1, &layout[0], &nglyphs, /*flags*/ 0))
                continue;

            if (nglyphs != 1)
                continue;

            const quint32 glyphIndex = layout[0].glyph;

            if (!glyphIndex)
                continue;

            Q_ASSERT(glyphIndex < glyphCount);

            QImage img = fe->alphaMapForGlyph(glyphIndex).convertToFormat(QImage::Format_Indexed8);
            glyph_metrics_t metrics = fe->boundingBox(glyphIndex);

            const quint32 oldSize = glyphs.size();
            glyphs.resize(glyphs.size() + sizeof(QFontEngineQPF::Glyph) + img.numBytes());
            uchar *data = reinterpret_cast<uchar *>(glyphs.data() + oldSize);

            uchar *gmapPtr = reinterpret_cast<uchar *>(gmap.data() + glyphIndex * sizeof(quint32));
            qToBigEndian(oldSize, gmapPtr);

            QFontEngineQPF::Glyph *glyph = reinterpret_cast<QFontEngineQPF::Glyph *>(data);
            glyph->width = img.width();
            glyph->height = img.height();
            glyph->bytesPerLine = img.bytesPerLine();
            glyph->x = qRound(metrics.x);
            glyph->y = qRound(metrics.y);
            glyph->advance = qRound(metrics.xoff);
            data += sizeof(QFontEngineQPF::Glyph);

            if (debugVerbosity && uc >= 'A' && uc <= 'z' || debugVerbosity > 1) {
                qDebug() << "adding glyph with index" << glyphIndex << " uc =" << char(uc) << ":\n"
                    << "    glyph->x =" << glyph->x << "rounded from" << metrics.x << "\n"
                    << "    glyph->y =" << glyph->y << "rounded from" << metrics.y << "\n"
                    << "    width =" << glyph->width << "height =" << glyph->height
                    << "    advance =" << glyph->advance << "rounded from" << metrics.xoff
                    ;
            }

            qMemCopy(data, img.bits(), img.numBytes());
        }
    }

    addBlock(QFontEngineQPF::GMapBlock, gmap);
    addBlock(QFontEngineQPF::GlyphBlock, glyphs);
}

void QPF::addBlock(QFontEngineQPF::BlockTag tag, const QByteArray &blockData)
{
    addUInt16(tag);
    addUInt16(0); // padding
    const int padSize = ((blockData.size() + 3) / 4) * 4 - blockData.size();
    addUInt32(blockData.size() + padSize);
    addByteArray(blockData);
    for (int i = 0; i < padSize; ++i)
        addUInt8(0);
}

#define ADD_TAGGED_DATA(tag, qtype, type, value) \
    addUInt16(tag); \
    addUInt16(sizeof(qtype)); \
    add##type(value)

void QPF::addTaggedString(QFontEngineQPF::HeaderTag tag, const QByteArray &string)
{
    addUInt16(tag);
    addUInt16(string.length());
    addByteArray(string);
}

void QPF::addTaggedQFixed(QFontEngineQPF::HeaderTag tag, QFixed value)
{
    ADD_TAGGED_DATA(tag, quint32, UInt32, value.value());
}

void QPF::addTaggedUInt8(QFontEngineQPF::HeaderTag tag, quint8 value)
{
    ADD_TAGGED_DATA(tag, quint8, UInt8, value);
}

void QPF::addTaggedInt8(QFontEngineQPF::HeaderTag tag, qint8 value)
{
    ADD_TAGGED_DATA(tag, qint8, Int8, value);
}

void QPF::addTaggedUInt16(QFontEngineQPF::HeaderTag tag, quint16 value)
{
    ADD_TAGGED_DATA(tag, quint16, UInt16, value);
}

void QPF::addTaggedUInt32(QFontEngineQPF::HeaderTag tag, quint32 value)
{
    ADD_TAGGED_DATA(tag, quint32, UInt32, value);
}

void QPF::dump(const QByteArray &qpf)
{
    QPF font;
    font.qpf = qpf;

    const uchar *data = reinterpret_cast<const uchar *>(qpf.constData());
    const uchar *endPtr = reinterpret_cast<const uchar *>(qpf.constData() + qpf.size());
    data = font.dumpHeader(data);

    const quint32 *gmap = 0;
    quint32 glyphCount = 0;

    while (data < endPtr) {
        const QFontEngineQPF::Block *block = reinterpret_cast<const QFontEngineQPF::Block *>(data);
        quint32 tag = qFromBigEndian(block->tag);
        quint32 blockSize = qFromBigEndian(block->dataSize);
        qDebug() << "Block: Tag =" << qFromBigEndian(block->tag) << "; Size =" << blockSize << "; Offset =" << hex << data - reinterpret_cast<const uchar *>(qpf.constData());
        data += sizeof(QFontEngineQPF::Block);

        if (debugVerbosity) {
            if (tag == QFontEngineQPF::GMapBlock) {
                gmap = reinterpret_cast<const quint32 *>(data);
                glyphCount = blockSize / 4;
                font.dumpGMapBlock(gmap, glyphCount);
            } else if (tag == QFontEngineQPF::GlyphBlock
                       && gmap && debugVerbosity > 1) {
                font.dumpGlyphBlock(gmap, glyphCount, data, data + blockSize);
            }
        }

        data += blockSize;
    }
}

const uchar *QPF::dumpHeader(const uchar *data)
{
    const QFontEngineQPF::Header *header = reinterpret_cast<const QFontEngineQPF::Header *>(data);
    qDebug() << "Header:";
    qDebug() << "magic =" 
             << header->magic[0]
             << header->magic[1]
             << header->magic[2]
             << header->magic[3];
    qDebug() << "lock =" << qFromBigEndian(header->lock);
    qDebug() << "majorVersion =" << header->majorVersion;
    qDebug() << "minorVersion =" << header->minorVersion;
    qDebug() << "dataSize =" << qFromBigEndian(header->dataSize);

    data += sizeof(QFontEngineQPF::Header);

    const uchar *endPtr = data + qFromBigEndian(header->dataSize);

    while (data && data < endPtr) {
        data = dumpHeaderTag(data);
    }

    return endPtr;
}

const uchar *QPF::dumpHeaderTag(const uchar *data)
{
    const QFontEngineQPF::Tag *tagPtr = reinterpret_cast<const QFontEngineQPF::Tag *>(data);
    quint16 tag = qFromBigEndian(tagPtr->tag);
    quint16 size = qFromBigEndian(tagPtr->size);

    qDebug() << "Tag =" << tag << headerTagNames[tag];
    qDebug() << "Size =" << size;

    if (tag == QFontEngineQPF::Tag_EndOfHeader)
        return 0;

    data += sizeof(QFontEngineQPF::Tag);

    Q_ASSERT(tag < QFontEngineQPF::NumTags);

    switch (tagTypes[tag]) {
        case QFontEngineQPF::StringType:
            qDebug() << "Payload =" << QString::fromUtf8(QByteArray(reinterpret_cast<const char *>(data), size));
            break;
        case QFontEngineQPF::FixedType:
            Q_ASSERT(size == sizeof(quint32));
            qDebug() << "Payload =" << QFixed::fromFixed(qFromBigEndian<quint32>(data)).toReal();
            break;
        case QFontEngineQPF::UInt8Type:
            Q_ASSERT(size == sizeof(quint8));
            qDebug() << "Payload =" << *data;
            break;
        case QFontEngineQPF::UInt32Type:
            Q_ASSERT(size == sizeof(quint32));
            qDebug() << "Payload =" << qFromBigEndian<quint32>(data);
            break;
    }

    data += size;
    return data;
}

void QPF::dumpGMapBlock(const quint32 *gmap, int glyphCount)
{
    qDebug() << "glyphCount =" << glyphCount;
    int renderedGlyphs = 0;
    for (int i = 0; i < glyphCount; ++i) {
        if (gmap[i] != 0xffffffff) {
            const quint32 glyphPos = qFromBigEndian(gmap[i]);
            qDebug("gmap[%d] = 0x%x / %u", i, glyphPos, glyphPos);
            ++renderedGlyphs;
        }
    }
    qDebug() << "Glyphs rendered:" << renderedGlyphs << "; Glyphs missing from the font:" << glyphCount - renderedGlyphs;
}

void QPF::dumpGlyphBlock(const quint32 *gmap, int glyphCount, const uchar *data, const uchar *endPtr)
{
    // glyphPos -> glyphIndex
    QMap<quint32, quint32> reverseGlyphMap;
    for (int i = 0; i < glyphCount; ++i) {
        if (gmap[i] == 0xffffffff)
            continue;
        const quint32 glyphPos = qFromBigEndian(gmap[i]);
        reverseGlyphMap[glyphPos] = i;
    }

    const uchar *glyphBlockBegin = data;
    while (data < endPtr) {
        const QFontEngineQPF::Glyph *g = reinterpret_cast<const QFontEngineQPF::Glyph *>(data);

        const quint64 glyphOffset = data - glyphBlockBegin;
        const quint32 glyphIndex = reverseGlyphMap.value(glyphOffset, 0xffffffff);

        if (glyphIndex == 0xffffffff)
            qDebug() << "############: Glyph present in glyph block is not listed in glyph map!";
        qDebug("glyph at offset 0x%x glyphIndex = %u", quint32(glyphOffset), glyphIndex);
        qDebug() << "    width =" << g->width << "height =" << g->height << "x =" << g->x << "y =" << g->y;
        qDebug() << "    advance =" << g->advance << "bytesPerLine =" << g->bytesPerLine;

        data += sizeof(*g);
        if (glyphIndex == 0xffffffff || debugVerbosity > 4) {
            dumpGlyph(data, g);
        }

        data += g->height * g->bytesPerLine;
    }
}

void QPF::dumpGlyph(const uchar *data, const QFontEngineQPF::Glyph *glyph)
{
    fprintf(stderr, "---- glyph data:\n");
    const char *alphas = " .o#";
    for (int y = 0; y < glyph->height; ++y) {
        for (int x = 0; x < glyph->width; ++x) {
            const uchar value = data[y * glyph->bytesPerLine + x];
            fprintf(stderr, "%c", alphas[value >> 6]);
        }
        fprintf(stderr, "\n");
    }
    fprintf(stderr, "----\n");
}

