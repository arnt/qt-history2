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

#include "private/qppmhandler_p.h"
#include <qimage.h>
#include <qvariant.h>
#include <qvector.h>
#include <ctype.h>

/*****************************************************************************
  PBM/PGM/PPM (ASCII and RAW) image read/write functions
 *****************************************************************************/

static int read_pbm_int(QIODevice *d)
{
    char c;
    int          val = -1;
    bool  digit;
    const int buflen = 100;
    char  buf[buflen];
    for (;;) {
        if (!d->getChar(&c))                // end of file
            break;
        digit = isdigit((uchar) c);
        if (val != -1) {
            if (digit) {
                val = 10*val + c - '0';
                continue;
            } else {
                if (c == '#')                        // comment
                    d->readLine(buf, buflen);
                break;
            }
        }
        if (digit)                                // first digit
            val = c - '0';
        else if (isspace((uchar) c))
            continue;
        else if (c == '#')
            d->readLine(buf, buflen);
        else
            break;
    }
    return val;
}

static bool read_pbm_image(QIODevice *device, QImage *outImage)        // read PBM image data
{
    const int        buflen = 300;
    char        buf[buflen];
    int                w, h, nbits, mcc, y;
    int                pbm_bpl;
    char        type;
    bool        raw;
    QImage        image;

    if (device->read(buf, 3) != 3)                        // read P[1-6]<white-space>
        return false;
    if (!(buf[0] == 'P' && isdigit((uchar) buf[1]) && isspace((uchar) buf[2])))
        return false;
    QImage::Format format;
    switch ((type=buf[1])) {
        case '1':                                // ascii PBM
        case '4':                                // raw PBM
            nbits = 1;
            format = QImage::Format_Mono;
            break;
        case '2':                                // ascii PGM
        case '5':                                // raw PGM
            nbits = 8;
            format = QImage::Format_Indexed8;
            break;
        case '3':                                // ascii PPM
        case '6':                                // raw PPM
            nbits = 32;
            format = QImage::Format_RGB32;
            break;
        default:
            return false;
    }
    raw = type >= '4';
    w = read_pbm_int(device);                        // get image width
    h = read_pbm_int(device);                        // get image height
    if (nbits == 1)
        mcc = 1;                                // ignore max color component
    else
        mcc = read_pbm_int(device);                // get max color component
    if (w <= 0 || w > 32767 || h <= 0 || h > 32767 || mcc <= 0)
        return false;                                        // weird P.M image

    int maxc = mcc;
    if (maxc > 255)
        maxc = 255;
    image = QImage(w, h, format);
    if (image.isNull())
        return false;

    pbm_bpl = (nbits*w+7)/8;                        // bytes per scanline in PBM

    if (raw) {                                // read raw data
        if (nbits == 32) {                        // type 6
            pbm_bpl = 3*w;
            uchar *buf24 = new uchar[pbm_bpl], *b;
            QRgb  *p;
            QRgb  *end;
            for (y=0; y<h; y++) {
                if (device->read((char *)buf24, pbm_bpl) != pbm_bpl) {
                    delete[] buf24;
                    return false;
                }
                p = (QRgb *)image.scanLine(y);
                end = p + w;
                b = buf24;
                while (p < end) {
                    *p++ = qRgb(b[0],b[1],b[2]);
                    b += 3;
                }
            }
            delete[] buf24;
        } else {                                // type 4,5
            for (y=0; y<h; y++) {
                if (device->read((char *)image.scanLine(y), pbm_bpl)
                        != pbm_bpl)
                    return false;
            }
        }
    } else {                                        // read ascii data
        register uchar *p;
        int n;
        for (y=0; y<h; y++) {
            p = image.scanLine(y);
            n = pbm_bpl;
            if (nbits == 1) {
                int b;
                int bitsLeft = w;
                while (n--) {
                    b = 0;
                    for (int i=0; i<8; i++) {
                        if (i < bitsLeft)
                            b = (b << 1) | (read_pbm_int(device) & 1);
                        else
                            b = (b << 1) | (0 & 1); // pad it our self if we need to
                    }
                    bitsLeft -= 8;
                    *p++ = b;
                }
            } else if (nbits == 8) {
                if (mcc == maxc) {
                    while (n--) {
                        *p++ = read_pbm_int(device);
                    }
                } else {
                    while (n--) {
                        *p++ = read_pbm_int(device) * maxc / mcc;
                    }
                }
            } else {                                // 32 bits
                n /= 4;
                int r, g, b;
                if (mcc == maxc) {
                    while (n--) {
                        r = read_pbm_int(device);
                        g = read_pbm_int(device);
                        b = read_pbm_int(device);
                        *((QRgb*)p) = qRgb(r, g, b);
                        p += 4;
                    }
                } else {
                    while (n--) {
                        r = read_pbm_int(device) * maxc / mcc;
                        g = read_pbm_int(device) * maxc / mcc;
                        b = read_pbm_int(device) * maxc / mcc;
                        *((QRgb*)p) = qRgb(r, g, b);
                        p += 4;
                    }
                }
            }
        }
    }

    if (nbits == 1) {                                // bitmap
        image.setNumColors(2);
        image.setColor(0, qRgb(255,255,255)); // white
        image.setColor(1, qRgb(0,0,0));        // black
    } else if (nbits == 8) {                        // graymap
        image.setNumColors(maxc+1);
        for (int i=0; i<=maxc; i++)
            image.setColor(i, qRgb(i*255/maxc,i*255/maxc,i*255/maxc));
    }

    *outImage = image;
    return true;
}

static bool write_pbm_image(QIODevice *out, const QImage &sourceImage, const QByteArray &sourceFormat)
{
    QByteArray str;
    QImage image = sourceImage;
    QByteArray format = sourceFormat;

    format = format.left(3);                        // ignore RAW part
    bool gray = format == "PGM";

    if (format == "PBM") {
        image = image.convertDepth(1);
    } else if (image.depth() == 1) {
        image = image.convertDepth(8);
    }

    if (image.depth() == 1 && image.numColors() == 2) {
        if (qGray(image.color(0)) < qGray(image.color(1))) {
            // 0=dark/black, 1=light/white - invert
            image.detach();
            for (int y=0; y<image.height(); y++) {
                uchar *p = image.scanLine(y);
                uchar *end = p + image.bytesPerLine();
                while (p < end)
                    *p++ ^= 0xff;
            }
        }
    }

    uint w = image.width();
    uint h = image.height();

    str = "P\n";
    str += QByteArray::number(w);
    str += ' ';
    str += QByteArray::number(h);
    str += '\n';

    switch (image.depth()) {
        case 1: {
            str.insert(1, '4');
            if (out->write(str, str.length()) != str.length())
                return false;
            w = (w+7)/8;
            for (uint y=0; y<h; y++) {
                uchar* line = image.scanLine(y);
                if (w != (uint)out->write((char*)line, w))
                    return false;
            }
            }
            break;

        case 8: {
            str.insert(1, gray ? '5' : '6');
            str.append("255\n");
            if (out->write(str, str.length()) != str.length())
                return false;
            QVector<QRgb> color = image.colorTable();
            uint bpl = w*(gray ? 1 : 3);
            uchar *buf   = new uchar[bpl];
            for (uint y=0; y<h; y++) {
                uchar *b = image.scanLine(y);
                uchar *p = buf;
                uchar *end = buf+bpl;
                if (gray) {
                    while (p < end) {
                        uchar g = (uchar)qGray(color[*b++]);
                        *p++ = g;
                    }
                } else {
                    while (p < end) {
                        QRgb rgb = color[*b++];
                        *p++ = qRed(rgb);
                        *p++ = qGreen(rgb);
                        *p++ = qBlue(rgb);
                    }
                }
                if (bpl != (uint)out->write((char*)buf, bpl))
                    return false;
            }
            delete [] buf;
            }
            break;

        case 32: {
            str.insert(1, gray ? '5' : '6');
            str.append("255\n");
            if (out->write(str, str.length()) != str.length())
                return false;
            uint bpl = w*(gray ? 1 : 3);
            uchar *buf = new uchar[bpl];
            for (uint y=0; y<h; y++) {
                QRgb  *b = (QRgb*)image.scanLine(y);
                uchar *p = buf;
                uchar *end = buf+bpl;
                if (gray) {
                    while (p < end) {
                        uchar g = (uchar)qGray(*b++);
                        *p++ = g;
                    }
                } else {
                    while (p < end) {
                        QRgb rgb = *b++;
                        *p++ = qRed(rgb);
                        *p++ = qGreen(rgb);
                        *p++ = qBlue(rgb);
                    }
                }
                if (bpl != (uint)out->write((char*)buf, bpl))
                    return false;
            }
            delete [] buf;
            }
    }

    return true;
}

bool QPpmHandler::canRead() const
{
    return canRead(device(), &subType);
}

bool QPpmHandler::canRead(QIODevice *device, QByteArray *subType)
{
    if (!device) {
        qWarning("QPpmHandler::canRead() called with no device");
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

    if (head[0] != 'P')
        return false;

    if (head[1] == '1' || head[1] == '4') {
        if (subType)
            *subType = "pbm";
    } else if (head[1] == '2' || head[1] == '5') {
        if (subType)
            *subType = "pgm";
    } else if (head[1] == '3' || head[1] == '6') {
        if (subType)
            *subType = "ppm";
    } else {
        return false;
    }
    return true;
}

bool QPpmHandler::read(QImage *image)
{
    return read_pbm_image(device(), image);
}

bool QPpmHandler::write(const QImage &image)
{
    return write_pbm_image(device(), image, subType);
}

bool QPpmHandler::supportsOption(ImageOption option) const
{
    return option == SubType;
}

QVariant QPpmHandler::option(ImageOption option) const
{
    return option == SubType ? subType : QByteArray();
}

void QPpmHandler::setOption(ImageOption option, const QVariant &value)
{
    if (option == SubType)
        subType = value.toByteArray().toLower();
}

QByteArray QPpmHandler::name() const
{
    return subType.isEmpty() ? QByteArray("ppm") : subType;
}
