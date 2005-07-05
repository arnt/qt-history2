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

#include "private/qxbmhandler_p.h"

#ifndef QT_NO_IMAGEFORMAT_XBM

#include <qimage.h>
#include <qiodevice.h>
#include <qvariant.h>

#include <stdio.h>
#include <ctype.h>


/*****************************************************************************
  X bitmap image read/write functions
 *****************************************************************************/

static inline int hex2byte(register char *p)
{
    return ((isdigit((uchar) *p) ? *p - '0' : toupper((uchar) *p) - 'A' + 10) << 4) |
           (isdigit((uchar) *(p+1)) ? *(p+1) - '0' : toupper((uchar) *(p+1)) - 'A' + 10);
}

static bool read_xbm_image(QIODevice *device, QImage *outImage)
{
    const int buflen = 300;
    char buf[buflen + 1];
    QRegExp r1(QLatin1String("^#define[ \t]+[a-zA-Z0-9._]+[ \t]+"));
    QRegExp r2(QLatin1String("[0-9]+"));
    int w = -1, h = -1;
    QImage image;

    qint64 readBytes = 0;

    // "#define .._width <num>"
    readBytes = device->readLine(buf, buflen);
    if (readBytes == -1)
	return false;
    buf[readBytes] = '\0';

    // skip initial comment, if any
    while (buf[0] != '#' && (readBytes = device->readLine( buf, buflen )) > 0);

    if (readBytes == -1)
	return false;
    buf[readBytes] = '\0';
    QString sbuf;
    sbuf = QString::fromLatin1(buf);

    if (r1.indexIn(sbuf) == 0 &&
         r2.indexIn(sbuf, r1.matchedLength()) == r1.matchedLength())
        w = QString(&buf[r1.matchedLength()]).toInt();

    // "#define .._height <num>"
    readBytes = device->readLine(buf, buflen);
    if (readBytes == -1)
	return false;
    buf[readBytes] = '\0';

    sbuf = QString::fromLatin1(buf);

    if (r1.indexIn(sbuf) == 0 &&
         r2.indexIn(sbuf, r1.matchedLength()) == r1.matchedLength())
        h = QString(&buf[r1.matchedLength()]).toInt();

    // format error
    if (w <= 0 || w > 32767 || h <= 0 || h > 32767)
        return false;

    // scan for database
    for (;;) {
        if ((readBytes = device->readLine(buf, buflen)) <= 0) {
            // end of file
            return false;
        }

        buf[readBytes] = '\0';
        if (QByteArray::fromRawData(buf, readBytes).contains("0x"))
            break;
    }

    image = QImage(w, h, QImage::Format_MonoLSB);
    if (image.isNull())
        return false;

    image.setNumColors(2);
    image.setColor(0, qRgb(255,255,255));        // white
    image.setColor(1, qRgb(0,0,0));                // black

    int           x = 0, y = 0;
    uchar *b = image.scanLine(0);
    char  *p = buf + QByteArray::fromRawData(buf, readBytes).indexOf("0x");
    w = (w+7)/8;                                // byte width

    while (y < h) {                                // for all encoded bytes...
        if (p) {                                // p = "0x.."
            *b++ = hex2byte(p+2);
            p += 2;
            if (++x == w && ++y < h) {
                b = image.scanLine(y);
                x = 0;
            }
            p = strstr(p, "0x");
        } else {                                // read another line
            if ((readBytes = device->readLine(buf,buflen)) <= 0)        // EOF ==> truncated image
                break;
            p = buf + QByteArray::fromRawData(buf, readBytes).indexOf("0x");
        }
    }

    *outImage = image;
    return true;
}

static bool write_xbm_image(const QImage &sourceImage, QIODevice *device, const QString &fileName)
{
    QImage image = sourceImage;
    int	       w = image.width();
    int	       h = image.height();
    int	       i;
    QString    s = fileName; // get file base name
    char *buf = new char[s.length() + 100];

    sprintf(buf, "#define %s_width %d\n", s.toAscii().data(), w);
    device->write(buf, qstrlen(buf));
    sprintf(buf, "#define %s_height %d\n", s.toAscii().data(), h);
    device->write(buf, qstrlen(buf));
    sprintf(buf, "static char %s_bits[] = {\n ", s.toAscii().data());
    device->write(buf, qstrlen(buf));

    if (image.format() != QImage::Format_MonoLSB)
        image = image.convertToFormat(QImage::Format_MonoLSB);

    bool invert = qGray(image.color(0)) < qGray(image.color(1));
    char hexrep[16];
    for (i=0; i<10; i++)
	hexrep[i] = '0' + i;
    for (i=10; i<16; i++)
	hexrep[i] = 'a' -10 + i;
    if (invert) {
	char t;
	for (i=0; i<8; i++) {
	    t = hexrep[15-i];
	    hexrep[15-i] = hexrep[i];
	    hexrep[i] = t;
	}
    }
    int bcnt = 0;
    register char *p = buf;
    int bpl = (w+7)/8;
    for (int y = 0; y < h; ++y) {
        uchar *b = image.scanLine(y);
        for (i = 0; i < bpl; ++i) {
            *p++ = '0'; *p++ = 'x';
            *p++ = hexrep[*b >> 4];
            *p++ = hexrep[*b++ & 0xf];

            if (i < bpl - 1 || y < h - 1) {
                *p++ = ',';
                if (++bcnt > 14) {
                    *p++ = '\n';
                    *p++ = ' ';
                    *p   = '\0';
                    if ((int)qstrlen(buf) != device->write(buf, qstrlen(buf))) {
                        delete [] buf;
                        return false;
                    }
                    p = buf;
                    bcnt = 0;
                }
            }
        }
    }
    strcpy(p, " };\n");
    if ((int)qstrlen(buf) != device->write(buf, qstrlen(buf))) {
        delete [] buf;
        return false;
    }

    delete [] buf;
    return true;
}

bool QXbmHandler::canRead() const
{
    return canRead(device());
}

bool QXbmHandler::canRead(QIODevice *device)
{
    QImage image;

    // it's impossible to tell whether we can load an XBM or not when
    // it's from a sequential device, as the only way to do it is to
    // attempt to parse the whole image.
    if (device->isSequential())
        return false;

    qint64 oldPos = device->pos();
    bool success = read_xbm_image(device, &image);
    device->seek(oldPos);

    return success;
}

bool QXbmHandler::read(QImage *image)
{
    return read_xbm_image(device(), image);
}

bool QXbmHandler::write(const QImage &image)
{
    return write_xbm_image(image, device(), fileName);
}

bool QXbmHandler::supportsOption(ImageOption option) const
{
    return option == Name;
}

QVariant QXbmHandler::option(ImageOption option) const
{
    return option == Name ? fileName : QString();
}

void QXbmHandler::setOption(ImageOption option, const QVariant &value)
{
    if (option == Name)
        fileName = value.toString();
}

QByteArray QXbmHandler::name() const
{
    return "xbm";
}

#endif // QT_NO_IMAGEFORMAT_XBM
