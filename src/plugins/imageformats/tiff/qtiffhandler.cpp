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

#include "qtiffhandler.h"
#include <qvariant.h>
#include <qdebug.h>
#include <qimage.h>
extern "C" {
#include "tiffio.h"
}

tsize_t qtiffReadProc(thandle_t fd, tdata_t buf, tsize_t size)
{
    QIODevice* device = static_cast<QTiffHandler*>(fd)->device();
    return device->isReadable() ? device->read(static_cast<char *>(buf), size) : -1;
}

tsize_t qtiffWriteProc(thandle_t fd, tdata_t buf, tsize_t size)
{
    return static_cast<QTiffHandler*>(fd)->device()->write(static_cast<char *>(buf), size);
}

toff_t qtiffSeekProc(thandle_t fd, toff_t off, int whence)
{
    QIODevice *device = static_cast<QTiffHandler*>(fd)->device();
    switch (whence) {
    case SEEK_SET:
        device->seek(off);
        break;
    case SEEK_CUR:
        device->seek(device->pos() + off);
        break;
    case SEEK_END:
        device->seek(device->size() + off);
        break;
    }

    return device->pos();
}

int qtiffCloseProc(thandle_t /*fd*/)
{
    return 0;
}

toff_t qtiffSizeProc(thandle_t fd)
{
    return static_cast<QTiffHandler*>(fd)->device()->size();
}

int qtiffMapProc(thandle_t /*fd*/, tdata_t* /*pbase*/, toff_t* /*psize*/)
{
    return 0;
}

void qtiffUnmapProc(thandle_t /*fd*/, tdata_t /*base*/, toff_t /*size*/)
{
}

QTiffHandler::QTiffHandler() : QImageIOHandler()
{
    compression = NoCompression;
}

bool QTiffHandler::canRead() const
{
    if (canRead(device())) {
        setFormat("tiff");
        return true;
    }
    return false;
}

bool QTiffHandler::canRead(QIODevice *device)
{
    if (!device) {
        qWarning("QTiffHandler::canRead() called with no device");
        return false;
    }

    // current implementation uses TIFFClientOpen which needs to be
    // able to seek, so sequential devices are not supported
    return !device->isSequential()
        && (device->peek(4) == "\x49\x49\x2A\x00" || device->peek(4) == "\x4D\x4D\x00\x2A");
}

bool QTiffHandler::read(QImage *image)
{
    if (!canRead())
        return false;

    TIFF *tiff = TIFFClientOpen("foo",
                                "r",
                                this,
                                qtiffReadProc,
                                qtiffWriteProc,
                                qtiffSeekProc,
                                qtiffCloseProc,
                                qtiffSizeProc,
                                qtiffMapProc,
                                qtiffUnmapProc);
    QImage tiffImage;
    if (tiff) {
        uint32 width = 0;
        uint32 height = 0;
        TIFFGetField(tiff, TIFFTAG_IMAGEWIDTH, &width);
        TIFFGetField(tiff, TIFFTAG_IMAGELENGTH, &height);
        tiffImage = QImage(width, height, QImage::Format_ARGB32);
        size_t npixels = width * height;
        uint32 *raster = reinterpret_cast<uint32*>(_TIFFmalloc(npixels * sizeof(uint32)));
        if (raster != 0) {
            if (TIFFReadRGBAImage(tiff, width, height, raster, 0)) {
                for (uint32 y=0; y<height; ++y)
                    convert32BitOrder(&raster[(height-y-1)*width], tiffImage.scanLine(y), width);
            }
            _TIFFfree(raster);
        }
        TIFFClose(tiff);
    }

    if (tiffImage.isNull())
        return false;

    *image = tiffImage;
    return true;
}

bool QTiffHandler::write(const QImage &image)
{
    if (!device()->isWritable())
        return false;

    QImage convertedImage = image.convertToFormat(QImage::Format_ARGB32);

    TIFF *tiff = TIFFClientOpen("foo",
                                "w",
                                this,
                                qtiffReadProc,
                                qtiffWriteProc,
                                qtiffSeekProc,
                                qtiffCloseProc,
                                qtiffSizeProc,
                                qtiffMapProc,
                                qtiffUnmapProc);

    if (tiff) {
        int width = convertedImage.width();
        int height = convertedImage.height();
        int depth = convertedImage.depth();
        int bytesPerLine = convertedImage.bytesPerLine();

        if (!TIFFSetField(tiff, TIFFTAG_IMAGEWIDTH, width)
                || !TIFFSetField(tiff, TIFFTAG_IMAGELENGTH, height)
                || !TIFFSetField(tiff, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB)
                || !TIFFSetField(tiff, TIFFTAG_COMPRESSION, compression == NoCompression ? COMPRESSION_NONE : COMPRESSION_LZW)
                || !TIFFSetField(tiff, TIFFTAG_SAMPLESPERPIXEL, depth/8)
                || !TIFFSetField(tiff, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG)
                || !TIFFSetField(tiff, TIFFTAG_BITSPERSAMPLE, 8)) {
            TIFFClose(tiff);
            return false;
        }

        uint32 *bytes = reinterpret_cast<uint32*>(_TIFFmalloc(bytesPerLine));
        for (int y=0; y < height; ++y) {
            convert32BitOrder(convertedImage.scanLine(y), bytes, width);
            if (TIFFWriteScanline(tiff, bytes, y) != 1) {
                _TIFFfree(bytes);
                TIFFClose(tiff);
                return false;
            }
        }
        _TIFFfree(bytes);
        TIFFClose(tiff);
    } else {
        return false;
    }
    return true;
}

QByteArray QTiffHandler::name() const
{
    return "tiff";
}

QVariant QTiffHandler::option(ImageOption option) const
{
    if (option == Size && canRead()) {
        QSize imageSize;
        qint64 pos = device()->pos();
        TIFF *tiff = TIFFClientOpen("foo",
                                    "r",
                                    const_cast<QTiffHandler*>(this),
                                    qtiffReadProc,
                                    qtiffWriteProc,
                                    qtiffSeekProc,
                                    qtiffCloseProc,
                                    qtiffSizeProc,
                                    qtiffMapProc,
                                    qtiffUnmapProc);

        if (tiff) {
            uint32 width = 0;
            uint32 height = 0;
            TIFFGetField(tiff, TIFFTAG_IMAGEWIDTH, &width);
            TIFFGetField(tiff, TIFFTAG_IMAGELENGTH, &height);
            imageSize = QSize(width, height);
        }
        device()->seek(pos);
        if (imageSize.isValid())
            return imageSize;
    } else if (option == CompressionRatio) {
        return compression;
    }
    return QVariant();
}

void QTiffHandler::setOption(ImageOption option, const QVariant &value)
{
    if (option == CompressionRatio && value.type() == QVariant::Int)
        compression = value.toInt();
}

bool QTiffHandler::supportsOption(ImageOption option) const
{
    return (option == Size) || (option == CompressionRatio);
}

void QTiffHandler::convert32BitOrder(const void *source, void *destination, int width)
{
    const uint32 *src = reinterpret_cast<const uint32 *>(source);
    uint32 *target = reinterpret_cast<uint32 *>(destination);
    for (int32 x=0; x<width; ++x) {
        uint32 p = src[x];
        // convert between ARGB and ABGR
        target[x] = (p & 0xff000000)
                    | ((p & 0x00ff0000) >> 16)
                    | (p & 0x0000ff00)
                    | ((p & 0x000000ff) << 16);
    }
}

