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

#include "private/qbmphandler_p.h"

#include <qimage.h>

static void swapPixel01(QImage *image)        // 1-bpp: swap 0 and 1 pixels
{
    int i;
    if (image->depth() == 1 && image->numColors() == 2) {
        register uint *p = (uint *)image->bits();
        int nbytes = image->numBytes();
        for (i=0; i<nbytes/4; i++) {
            *p = ~*p;
            p++;
        }
        uchar *p2 = (uchar *)p;
        for (i=0; i<(nbytes&3); i++) {
            *p2 = ~*p2;
            p2++;
        }
        QRgb t = image->color(0);                // swap color 0 and 1
        image->setColor(0, image->color(1));
        image->setColor(1, t);
    }
}

/*
    QImageIO::defineIOHandler("BMP", "^BM", 0,
                               read_bmp_image, write_bmp_image);
*/

/*****************************************************************************
  BMP (DIB) image read/write functions
 *****************************************************************************/

const int BMP_FILEHDR_SIZE = 14;                // size of BMP_FILEHDR data

struct BMP_FILEHDR {                                // BMP file header
    char   bfType[2];                                // "BM"
    qint32  bfSize;                                // size of file
    qint16  bfReserved1;
    qint16  bfReserved2;
    qint32  bfOffBits;                                // pointer to the pixmap bits
};

QDataStream &operator>>(QDataStream &s, BMP_FILEHDR &bf)
{                                                // read file header
    s.readRawData(bf.bfType, 2);
    s >> bf.bfSize >> bf.bfReserved1 >> bf.bfReserved2 >> bf.bfOffBits;
    return s;
}

QDataStream &operator<<(QDataStream &s, const BMP_FILEHDR &bf)
{                                                // write file header
    s.writeRawData(bf.bfType, 2);
    s << bf.bfSize << bf.bfReserved1 << bf.bfReserved2 << bf.bfOffBits;
    return s;
}


const int BMP_OLD  = 12;                        // old Windows/OS2 BMP size
const int BMP_WIN  = 40;                        // new Windows BMP size
const int BMP_OS2  = 64;                        // new OS/2 BMP size

const int BMP_RGB  = 0;                                // no compression
const int BMP_RLE8 = 1;                                // run-length encoded, 8 bits
const int BMP_RLE4 = 2;                                // run-length encoded, 4 bits
const int BMP_BITFIELDS = 3;                        // RGB values encoded in data as bit-fields

struct BMP_INFOHDR {                                // BMP information header
    qint32  biSize;                                // size of this struct
    qint32  biWidth;                                // pixmap width
    qint32  biHeight;                                // pixmap height
    qint16  biPlanes;                                // should be 1
    qint16  biBitCount;                        // number of bits per pixel
    qint32  biCompression;                        // compression method
    qint32  biSizeImage;                                // size of image
    qint32  biXPelsPerMeter;                        // horizontal resolution
    qint32  biYPelsPerMeter;                        // vertical resolution
    qint32  biClrUsed;                                // number of colors used
    qint32  biClrImportant;                        // number of important colors
};


QDataStream &operator>>(QDataStream &s, BMP_INFOHDR &bi)
{
    s >> bi.biSize;
    if (bi.biSize == BMP_WIN || bi.biSize == BMP_OS2) {
        s >> bi.biWidth >> bi.biHeight >> bi.biPlanes >> bi.biBitCount;
        s >> bi.biCompression >> bi.biSizeImage;
        s >> bi.biXPelsPerMeter >> bi.biYPelsPerMeter;
        s >> bi.biClrUsed >> bi.biClrImportant;
    }
    else {                                        // probably old Windows format
        qint16 w, h;
        s >> w >> h >> bi.biPlanes >> bi.biBitCount;
        bi.biWidth  = w;
        bi.biHeight = h;
        bi.biCompression = BMP_RGB;                // no compression
        bi.biSizeImage = 0;
        bi.biXPelsPerMeter = bi.biYPelsPerMeter = 0;
        bi.biClrUsed = bi.biClrImportant = 0;
    }
    return s;
}

QDataStream &operator<<(QDataStream &s, const BMP_INFOHDR &bi)
{
    s << bi.biSize;
    s << bi.biWidth << bi.biHeight;
    s << bi.biPlanes;
    s << bi.biBitCount;
    s << bi.biCompression;
    s << bi.biSizeImage;
    s << bi.biXPelsPerMeter << bi.biYPelsPerMeter;
    s << bi.biClrUsed << bi.biClrImportant;
    return s;
}

static int calc_shift(int mask)
{
    int result = 0;
    while (!(mask & 1)) {
        result++;
        mask >>= 1;
    }
    return result;
}

static bool read_dib(QDataStream &s, int offset, int startpos, QImage &image)
{
    BMP_INFOHDR bi;
    QIODevice* d = s.device();

    s >> bi;                                        // read BMP info header
    if (d->atEnd())                                // end of stream/file
        return false;
#if 0
    qDebug("offset...........%d", offset);
    qDebug("startpos.........%d", startpos);
    qDebug("biSize...........%d", bi.biSize);
    qDebug("biWidth..........%d", bi.biWidth);
    qDebug("biHeight.........%d", bi.biHeight);
    qDebug("biPlanes.........%d", bi.biPlanes);
    qDebug("biBitCount.......%d", bi.biBitCount);
    qDebug("biCompression....%d", bi.biCompression);
    qDebug("biSizeImage......%d", bi.biSizeImage);
    qDebug("biXPelsPerMeter..%d", bi.biXPelsPerMeter);
    qDebug("biYPelsPerMeter..%d", bi.biYPelsPerMeter);
    qDebug("biClrUsed........%d", bi.biClrUsed);
    qDebug("biClrImportant...%d", bi.biClrImportant);
#endif
    int w = bi.biWidth,         h = bi.biHeight,  nbits = bi.biBitCount;
    int t = bi.biSize,         comp = bi.biCompression;
    int red_mask, green_mask, blue_mask;
    int red_shift = 0;
    int green_shift = 0;
    int blue_shift = 0;
    int red_scale = 0;
    int green_scale = 0;
    int blue_scale = 0;

    if (!(nbits == 1 || nbits == 4 || nbits == 8 || nbits == 16 || nbits == 24 || nbits == 32) ||
        bi.biPlanes != 1 || comp > BMP_BITFIELDS)
        return false;                                        // weird BMP image
    if (!(comp == BMP_RGB || (nbits == 4 && comp == BMP_RLE4) ||
        (nbits == 8 && comp == BMP_RLE8) || ((nbits == 16 || nbits == 32) && comp == BMP_BITFIELDS)))
         return false;                                // weird compression type

    int ncols;
    int depth;
    switch (nbits) {
        case 32:
        case 24:
        case 16:
            depth = 32;
            break;
        case 8:
        case 4:
            depth = 8;
            break;
        default:
            depth = 1;
    }
    if (depth == 32)                                // there's no colormap
        ncols = 0;
    else                                        // # colors used
        ncols = bi.biClrUsed ? bi.biClrUsed : 1 << nbits;

    image = QImage(w, h, depth, ncols, nbits == 1 ?
                   QImage::BigEndian : QImage::IgnoreEndian);
    if (image.isNull())                        // could not create image
        return false;

    image.setDotsPerMeterX(bi.biXPelsPerMeter);
    image.setDotsPerMeterY(bi.biYPelsPerMeter);

    d->seek(startpos + BMP_FILEHDR_SIZE + bi.biSize); // goto start of colormap

    if (ncols > 0) {                                // read color table
        uchar rgb[4];
        int   rgb_len = t == BMP_OLD ? 3 : 4;
        for (int i=0; i<ncols; i++) {
            if (d->read((char *)rgb, rgb_len) != rgb_len)
                return false;
            image.setColor(i, qRgb(rgb[2],rgb[1],rgb[0]));
            if (d->atEnd())                        // truncated file
                return false;
        }
    } else if (comp == BMP_BITFIELDS && (nbits == 16 || nbits == 32)) {
        if (d->read((char *)&red_mask, sizeof(red_mask)) != sizeof(red_mask))
            return false;
        if (d->read((char *)&green_mask, sizeof(green_mask)) != sizeof(green_mask))
            return false;
        if (d->read((char *)&blue_mask, sizeof(blue_mask)) != sizeof(blue_mask))
            return false;
        red_shift = calc_shift(red_mask);
        red_scale = 256 / ((red_mask >> red_shift) + 1);
        green_shift = calc_shift(green_mask);
        green_scale = 256 / ((green_mask >> green_shift) + 1);
        blue_shift = calc_shift(blue_mask);
        blue_scale = 256 / ((blue_mask >> blue_shift) + 1);
    } else if (comp == BMP_RGB && (nbits == 24 || nbits == 32)) {
        blue_mask = 0x000000ff;
        green_mask = 0x0000ff00;
        red_mask = 0x00ff0000;
        blue_shift = 0;
        green_shift = 8;
        red_shift = 16;
        blue_scale = green_scale = red_scale = 1;
    } else if (comp == BMP_RGB && nbits == 16)  // don't support RGB values for 15/16 bpp
        return false;

    // offset can be bogus, be careful
    if (offset>=0 && startpos + offset > d->pos())
        d->seek(startpos + offset);                // start of image data

    int             bpl = image.bytesPerLine();
#ifdef Q_WS_QWS
    //
    // Guess the number of bytes-per-line if we don't know how much
    // image data is in the file (bogus image ?).
    //
    int bmpbpl = bi.biSizeImage > 0 ?
        bi.biSizeImage / bi.biHeight :
        (d->size() - offset) / bi.biHeight;
    int pad = bmpbpl-bpl;
#endif
    uchar **line = image.jumpTable();

    if (nbits == 1) {                                // 1 bit BMP image
        while (--h >= 0) {
            if (d->read((char*)line[h],bpl) != bpl)
                break;
#ifdef Q_WS_QWS
            if (pad > 0)
                d->seek(d->pos()+pad);
#endif
        }
        if (ncols == 2 && qGray(image.color(0)) < qGray(image.color(1)))
            swapPixel01(&image);                // pixel 0 is white!
    }

    else if (nbits == 4) {                        // 4 bit BMP image
        int    buflen = ((w+7)/8)*4;
        uchar *buf    = new uchar[buflen];
        if (comp == BMP_RLE4) {                // run length compression
            int x=0, y=0, c, i;
            char b;
            register uchar *p = line[h-1];
            const uchar *endp = line[h-1]+w;
            while (y < h) {
                if (!d->getChar(&b))
                    break;
                if (b == 0) {                        // escape code
                    if (!d->getChar(&b) || b == 1) {
                        y = h;                // exit loop
                    } else switch (b) {
                        case 0:                        // end of line
                            x = 0;
                            y++;
                            p = line[h-y-1];
                            break;
                        case 2:                        // delta (jump)
                        {
                            char tmp;
                            d->getChar(&tmp);
                            x += tmp;
                            d->getChar(&tmp);
                            y += tmp;
                        }

                            // Protection
                            if ((uint)x >= (uint)w)
                                x = w-1;
                            if ((uint)y >= (uint)h)
                                y = h-1;

                            p = line[h-y-1] + x;
                            break;
                        default:                // absolute mode
                            // Protection
                            if (p + b > endp)
                                b = endp-p;

                            i = (c = b)/2;
                            while (i--) {
                                d->getChar(&b);
                                *p++ = b >> 4;
                                *p++ = b & 0x0f;
                            }
                            if (c & 1) {
                                char tmp;
                                d->getChar(&tmp);
                                *p++ = tmp >> 4;
                            }
                            if ((((c & 3) + 1) & 2) == 2)
                                d->getChar(0);        // align on word boundary
                            x += c;
                    }
                } else {                        // encoded mode
                    // Protection
                    if (p + b > endp)
                        b = endp-p;

                    i = (c = b)/2;
                    d->getChar(&b);                // 2 pixels to be repeated
                    while (i--) {
                        *p++ = b >> 4;
                        *p++ = b & 0x0f;
                    }
                    if (c & 1)
                        *p++ = b >> 4;
                    x += c;
                }
            }
        } else if (comp == BMP_RGB) {                // no compression
            while (--h >= 0) {
                if (d->read((char*)buf,buflen) != buflen)
                    break;
                register uchar *p = line[h];
                uchar *b = buf;
                for (int i=0; i<w/2; i++) {        // convert nibbles to bytes
                    *p++ = *b >> 4;
                    *p++ = *b++ & 0x0f;
                }
                if (w & 1)                        // the last nibble
                    *p = *b >> 4;
            }
        }
        delete [] buf;
    }

    else if (nbits == 8) {                        // 8 bit BMP image
        if (comp == BMP_RLE8) {                // run length compression
            int x=0, y=0;
            char b;
            register uchar *p = line[h-1];
            const uchar *endp = line[h-1]+w;
            while (y < h) {
                if (!d->getChar(&b))
                    break;
                if (b == 0) {                        // escape code
                    if (!d->getChar(&b) || b == 1) {
                            y = h;                // exit loop
                    } else switch (b) {
                        case 0:                        // end of line
                            x = 0;
                            y++;
                            p = line[h-y-1];
                            break;
                        case 2:                        // delta (jump)
                            // Protection
                            if ((uint)x >= (uint)w)
                                x = w-1;
                            if ((uint)y >= (uint)h)
                                y = h-1;

                            {
                                char tmp;
                                d->getChar(&tmp);
                                x += tmp;
                                d->getChar(&tmp);
                                y += tmp;
                            }
                            p = line[h-y-1] + x;
                            break;
                        default:                // absolute mode
                            // Protection
                            if (p + b > endp)
                                b = endp-p;

                            if (d->read((char *)p, b) != b)
                                return false;
                            if ((b & 1) == 1)
                                d->getChar(0);        // align on word boundary
                            x += b;
                            p += b;
                    }
                } else {                        // encoded mode
                    // Protection
                    if (p + b > endp)
                        b = endp-p;

                    char tmp;
                    d->getChar(&tmp);
                    memset(p, tmp, b); // repeat pixel
                    x += b;
                    p += b;
                }
            }
        } else if (comp == BMP_RGB) {                // uncompressed
            while (--h >= 0) {
                if (d->read((char *)line[h],bpl) != bpl)
                    break;
#ifdef Q_WS_QWS
                if (pad > 0)
                    d->seek(d->pos()+pad);
#endif
            }
        }
    }

    else if (nbits == 16 || nbits == 24 || nbits == 32) { // 16,24,32 bit BMP image
        register QRgb *p;
        QRgb  *end;
        uchar *buf24 = new uchar[bpl];
        int    bpl24 = ((w*nbits+31)/32)*4;
        uchar *b;
        int c;

        while (--h >= 0) {
            p = (QRgb *)line[h];
            end = p + w;
            if (d->read((char *)buf24,bpl24) != bpl24)
                break;
            b = buf24;
            while (p < end) {
                c = *(uchar*)b | (*(uchar*)(b+1)<<8);
                if (nbits != 16)
                    c |= *(uchar*)(b+2)<<16;
                *p++ = qRgb(((c & red_mask) >> red_shift) * red_scale,
                                        ((c & green_mask) >> green_shift) * green_scale,
                                        ((c & blue_mask) >> blue_shift) * blue_scale);
                b += nbits/8;
            }
        }
        delete[] buf24;
    }

    return true;
}

// this is also used in qmime_win.cpp
bool Q_GUI_EXPORT qt_write_dib(QDataStream &s, QImage image)
{
    int        nbits;
    int        bpl_bmp;
    int        bpl = image.bytesPerLine();

    QIODevice* d = s.device();

    if (image.depth() == 8 && image.numColors() <= 16) {
        bpl_bmp = (((bpl+1)/2+3)/4)*4;
        nbits = 4;
    } else if (image.depth() == 32) {
        bpl_bmp = ((image.width()*24+31)/32)*4;
        nbits = 24;
#ifdef Q_WS_QWS
    } else if (image.depth() == 1 || image.depth() == 8) {
        // Qt/E doesn't word align.
        bpl_bmp = ((image.width()*image.depth()+31)/32)*4;
        nbits = image.depth();
#endif
    } else {
        bpl_bmp = bpl;
        nbits = image.depth();
    }

    BMP_INFOHDR bi;
    bi.biSize               = BMP_WIN;                // build info header
    bi.biWidth               = image.width();
    bi.biHeight               = image.height();
    bi.biPlanes               = 1;
    bi.biBitCount      = nbits;
    bi.biCompression   = BMP_RGB;
    bi.biSizeImage     = bpl_bmp*image.height();
    bi.biXPelsPerMeter = image.dotsPerMeterX() ? image.dotsPerMeterX()
                                                : 2834; // 72 dpi default
    bi.biYPelsPerMeter = image.dotsPerMeterY() ? image.dotsPerMeterY() : 2834;
    bi.biClrUsed       = image.numColors();
    bi.biClrImportant  = image.numColors();
    s << bi;                                        // write info header

    if (image.depth() != 32) {                // write color table
        uchar *color_table = new uchar[4*image.numColors()];
        uchar *rgb = color_table;
        QVector<QRgb> c = image.colorTable();
        for (int i=0; i<image.numColors(); i++) {
            *rgb++ = qBlue (c[i]);
            *rgb++ = qGreen(c[i]);
            *rgb++ = qRed  (c[i]);
            *rgb++ = 0;
        }
        d->write((char *)color_table, 4*image.numColors());
        delete [] color_table;
    }

    if (image.depth() == 1 && image.bitOrder() != QImage::BigEndian)
        image = image.convertBitOrder(QImage::BigEndian);

    int         y;

    if (nbits == 1 || nbits == 8) {                // direct output
#ifdef Q_WS_QWS
        // Qt/E doesn't word align.
        int pad = bpl_bmp - bpl;
        char padding[4];
#endif
        for (y=image.height()-1; y>=0; y--) {
            d->write((char*)image.scanLine(y), bpl);
#ifdef Q_WS_QWS
            d->write(padding, pad);
#endif
        }
        return true;
    }

    uchar *buf        = new uchar[bpl_bmp];
    uchar *b, *end;
    register uchar *p;

    memset(buf, 0, bpl_bmp);
    for (y=image.height()-1; y>=0; y--) {        // write the image bits
        if (nbits == 4) {                        // convert 8 -> 4 bits
            p = image.scanLine(y);
            b = buf;
            end = b + image.width()/2;
            while (b < end) {
                *b++ = (*p << 4) | (*(p+1) & 0x0f);
                p += 2;
            }
            if (image.width() & 1)
                *b = *p << 4;
        } else {                                // 32 bits
            QRgb *p   = (QRgb *)image.scanLine(y);
            QRgb *end = p + image.width();
            b = buf;
            while (p < end) {
                *b++ = qBlue(*p);
                *b++ = qGreen(*p);
                *b++ = qRed(*p);
                p++;
            }
        }
        if (bpl_bmp != d->write((char*)buf, bpl_bmp)) {
            delete[] buf;
            return false;
        }
    }
    delete[] buf;
    return true;
}

// this is also used in qmime_win.cpp
bool Q_GUI_EXPORT qt_read_dib(QDataStream &s, QImage &image)
{
    return read_dib(s,-1,-BMP_FILEHDR_SIZE,image);
}

bool QBmpHandler::canRead() const
{
    return canRead(device());
}

bool QBmpHandler::canRead(QIODevice *device)
{
    if (!device) {
        qWarning("QBmpHandler::canRead() called with 0 pointer");
        return false;
    }

    qint64 oldPos = device->pos();

    char head[2];
    qint64 readBytes = device->read(head, sizeof(head));
    if (readBytes != sizeof(head)) {
        if (device->isSequential()) {
            while (readBytes > 0)
                device->ungetChar(head[readBytes-- - 1]);
        } else {
            device->seek(oldPos);
        }
        return false;
    }

    if (device->isSequential()) {
        while (readBytes > 0)
            device->ungetChar(head[readBytes-- - 1]);
    } else {
        device->seek(oldPos);
    }

    return (qstrncmp(head, "BM", 2) == 0);
}

bool QBmpHandler::read(QImage *image)
{
    QIODevice *d = device();
    QDataStream s(d);
    BMP_FILEHDR bf;
    int startpos = d->pos();

    // Intel byte order
    s.setByteOrder(QDataStream::LittleEndian);

    // read BMP file header
    s >> bf;

    // check header
    if (qstrncmp(bf.bfType,"BM",2) != 0)
        return false;

    // read image
    QImage tmpImage;
    if (!read_dib(s, bf.bfOffBits, startpos, tmpImage))
        return false;

    if (image)
        *image = tmpImage;
    return true;
}

bool QBmpHandler::write(const QImage &image)
{
    QIODevice *d = device();
    QDataStream s(d);
    BMP_FILEHDR bf;
    int bpl_bmp;
    int bpl = image.bytesPerLine();

    // Code partially repeated in qt_write_dib
    if (image.depth() == 8 && image.numColors() <= 16) {
        bpl_bmp = (((bpl+1)/2+3)/4)*4;
    } else if (image.depth() == 32) {
        bpl_bmp = ((image.width()*24+31)/32)*4;
    } else {
        bpl_bmp = bpl;
    }

    // Intel byte order
    s.setByteOrder(QDataStream::LittleEndian);

    // build file header
    strncpy(bf.bfType, "BM", 2);

    // write file header
    bf.bfReserved1 = 0;
    bf.bfReserved2 = 0;
    bf.bfOffBits = BMP_FILEHDR_SIZE + BMP_WIN + image.numColors() * 4;
    bf.bfSize = bf.bfOffBits + bpl_bmp*image.height();
    s << bf;

    // write image
    return qt_write_dib(s, image);
}

QByteArray QBmpHandler::name() const
{
    return "bmp";
}
