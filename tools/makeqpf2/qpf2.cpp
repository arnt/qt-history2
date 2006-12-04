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

QByteArray QPF::generate(QFontEngine *fontEngine, bool includeCMap)
{
    QPF font;

    font.addHeader(fontEngine);
    if (includeCMap)
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
    header->lock = qToBigEndian<quint32>(0xffffffff);
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
    gmap.fill(0xff);
    //qDebug() << "glyphCount" << glyphCount;

    QByteArray glyphs;
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
    const uchar *data = reinterpret_cast<const uchar *>(qpf.constData());
    const uchar *endPtr = reinterpret_cast<const uchar *>(qpf.constData() + qpf.size());
    data = dumpHeader(data);
    while (data < endPtr) {
        data = dumpBlock(data);
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

const uchar *QPF::dumpBlock(const uchar *data)
{
    const QFontEngineQPF::Block *block = reinterpret_cast<const QFontEngineQPF::Block *>(data);
    int blockSize = qFromBigEndian(block->dataSize);
    qDebug() << "Block: Tag =" << qFromBigEndian(block->tag) << "; Size =" << blockSize << "; Pointer =" << static_cast<const void *>(data);

    data += sizeof(QFontEngineQPF::Block) + blockSize;
    return data;
}

