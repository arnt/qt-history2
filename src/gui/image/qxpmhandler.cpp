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

#include "private/qxpmhandler_p.h"

#include <private/qcolor_p.h>
#include <qimage.h>
#include <qmap.h>
#include <qtextstream.h>
#include <qvariant.h>

/*****************************************************************************
  Misc. utility functions
 *****************************************************************************/
#if !defined(QT_NO_IMAGEIO_XPM) || !defined(QT_NO_IMAGEIO_XBM)
static QString fbname(const QString &fileName) // get file basename (sort of)
{
    QString s = fileName;
    if (!s.isEmpty()) {
        int i;
        if ((i = s.lastIndexOf('/')) >= 0)
            s = s.mid(i);
        if ((i = s.lastIndexOf('\\')) >= 0)
            s = s.mid(i);
        QRegExp r(QLatin1String("[a-zA-Z][a-zA-Z0-9_]*"));
        int p = r.indexIn(s);
        if (p == -1)
            s.clear();
        else
            s = s.mid(p, r.matchedLength());
    }
    if (s.isEmpty())
        s = QString::fromLatin1("dummy");
    return s;
}
#endif

// Skip until ", read until the next ", return the rest in *buf
// Returns false on error, true on success

static bool read_xpm_string(QByteArray &buf, QIODevice *d, const char * const *source, int &index)
{
    if (source) {
        buf = source[index++];
        return true;
    }

    buf = "";
    char c;
    do {
        if (!d->getChar(&c))
            return false;
    } while (c != '"');

    do {
        if (!d->getChar(&c))
            return false;
        if (c != '"')
            buf.append(c);
    } while (c != '"');

    return true;
}


//
// INTERNAL
//
// Reads an .xpm from either the QImageIO or from the QString *.
// One of the two HAS to be 0, the other one is used.
//

bool qt_read_xpm_image_or_array(QIODevice *device, const char * const * source, QImage &image)
{
    QByteArray buf(200, 0);

    int i, cpp, ncols, w, h, index = 0;

    if (device) {
        // "/* XPM */"
        int readBytes;
        if ((readBytes = device->readLine(buf.data(), buf.size())) < 0)
            return false;

        if (buf.indexOf("/* XPM") != 0) {
            while (readBytes > 0) {
                device->ungetChar(buf.at(readBytes - 1));
                --readBytes;
            }
            return false;
        }// bad magic
    }

    if (!read_xpm_string(buf, device, source, index))
        return false;

    if (sscanf(buf, "%d %d %d %d", &w, &h, &ncols, &cpp) < 4)
        return false;                                        // < 4 numbers parsed

    if (cpp > 15)
        return false;

    if (ncols > 256) {
        image.create(w, h, 32);
    } else {
        image.create(w, h, 8, ncols);
    }

    if (image.isNull())
        return false;

    QMap<QString, int> colorMap;
    int currentColor;

    for(currentColor=0; currentColor < ncols; ++currentColor) {
        if (!read_xpm_string(buf, device, source, index)) {
            qWarning("QImage: XPM color specification missing");
            return false;
        }
        QString index;
        index = buf.left(cpp);
        buf = buf.mid(cpp).simplified().toLower();
        buf.prepend(" ");
        i = buf.indexOf(" c ");
        if (i < 0)
            i = buf.indexOf(" g ");
        if (i < 0)
            i = buf.indexOf(" g4 ");
        if (i < 0)
            i = buf.indexOf(" m ");
        if (i < 0) {
            qWarning("QImage: XPM color specification is missing: %s", buf.constData());
            return false;        // no c/g/g4/m specification at all
        }
        buf = buf.mid(i+3);
        // Strip any other colorspec
        int end = buf.indexOf(' ', 4);
        if (end >= 0)
            buf.truncate(end);
        buf = buf.trimmed();
        if (buf == "none") {
            image.setAlphaBuffer(true);
            int transparentColor = currentColor;
            if (image.depth() == 8) {
                image.setColor(transparentColor,
                                RGB_MASK & qRgb(198,198,198));
                colorMap.insert(index, transparentColor);
            } else {
                QRgb rgb = RGB_MASK & qRgb(198,198,198);
                colorMap.insert(index, rgb);
            }
        } else {
            QRgb c_rgb;
            if (((buf.length()-1) % 3) && (buf[0] == '#')) {
                buf.truncate(((buf.length()-1) / 4 * 3) + 1); // remove alpha channel left by imagemagick
            }
            if (buf[0] == '#') {
                qt_get_hex_rgb(buf, &c_rgb);
            } else {
                qt_get_named_rgb(buf, &c_rgb);
            }
            if (image.depth() == 8) {
                image.setColor(currentColor, 0xff000000 | c_rgb);
                colorMap.insert(index, currentColor);
            } else {
                colorMap.insert(index, 0xff000000 | c_rgb);
            }
        }
    }

    // Read pixels
    for(int y=0; y<h; y++) {
        if (!read_xpm_string(buf, device, source, index)) {
            qWarning("QImage: XPM pixels missing on image line %d", y);
            return false;
        }
        if (image.depth() == 8) {
            uchar *p = image.scanLine(y);
            uchar *d = (uchar *)buf.data();
            uchar *end = d + buf.length();
            int x;
            if (cpp == 1) {
                char b[2];
                b[1] = '\0';
                for (x=0; x<w && d<end; x++) {
                    b[0] = *d++;
                    *p++ = (uchar)colorMap[b];
                }
            } else {
                char b[16];
                b[cpp] = '\0';
                for (x=0; x<w && d<end; x++) {
                    strncpy(b, (char *)d, cpp);
                    *p++ = (uchar)colorMap[b];
                    d += cpp;
                }
            }
        } else {
            QRgb *p = (QRgb*)image.scanLine(y);
            uchar *d = (uchar *)buf.data();
            uchar *end = d + buf.length();
            int x;
            char b[16];
            b[cpp] = '\0';
            for (x=0; x<w && d<end; x++) {
                strncpy(b, (char *)d, cpp);
                *p++ = (QRgb)colorMap[b];
                d += cpp;
            }
        }
    }
    return true;
}

static const char* xpm_color_name(int cpp, int index)
{
    static char returnable[5];
    static const char code[] = ".#abcdefghijklmnopqrstuvwxyzABCD"
                               "EFGHIJKLMNOPQRSTUVWXYZ0123456789";
    // cpp is limited to 4 and index is limited to 64^cpp
    if (cpp > 1) {
        if (cpp > 2) {
            if (cpp > 3) {
                returnable[3] = code[index % 64];
                index /= 64;
            } else
                returnable[3] = '\0';
            returnable[2] = code[index % 64];
            index /= 64;
        } else
            returnable[2] = '\0';
        // the following 4 lines are a joke!
        if (index == 0)
            index = 64*44+21;
        else if (index == 64*44+21)
            index = 0;
        returnable[1] = code[index % 64];
        index /= 64;
    } else
        returnable[1] = '\0';
    returnable[0] = code[index];

    return returnable;
}


// write XPM image data
static void write_xpm_image(const QImage &sourceImage, QIODevice *device, const QString &fileName)
{
    QImage image;
    if (sourceImage.depth() != 32)
        image = sourceImage.convertDepth(32);
    else
        image = sourceImage;

    QMap<QRgb, int> colorMap;

    int w = image.width(), h = image.height(), ncolors = 0;
    int x, y;

    // build color table
    for(y=0; y<h; y++) {
        QRgb * yp = (QRgb *)image.scanLine(y);
        for(x=0; x<w; x++) {
            QRgb color = *(yp + x);
            if (!colorMap.contains(color))
                colorMap.insert(color, ncolors++);
        }
    }

    // number of 64-bit characters per pixel needed to encode all colors
    int cpp = 1;
    for (int k = 64; ncolors > k; k *= 64) {
        ++cpp;
        // limit to 4 characters per pixel
        // 64^4 colors is enough for a 4096x4096 image
         if (cpp > 4)
            break;
    }

    QString line;

    // write header
    QTextStream s(device);
    s << "/* XPM */" << endl
      << "static char *" << fbname(fileName) << "[]={" << endl
      << "\"" << w << " " << h << " " << ncolors << " " << cpp << "\"";

    // write palette
    QMap<QRgb, int>::Iterator c = colorMap.begin();
    while (c != colorMap.end()) {
        QRgb color = c.key();
        if (image.hasAlphaBuffer() && color == (color & RGB_MASK))
            line.sprintf("\"%s c None\"",
                          xpm_color_name(cpp, *c));
        else
            line.sprintf("\"%s c #%02x%02x%02x\"",
                          xpm_color_name(cpp, *c),
                          qRed(color),
                          qGreen(color),
                          qBlue(color));
        ++c;
        s << "," << endl << line;
    }

    // write pixels, limit to 4 characters per pixel
    line.truncate(cpp*w);
    for(y=0; y<h; y++) {
        QRgb * yp = (QRgb *) image.scanLine(y);
        int cc = 0;
        for(x=0; x<w; x++) {
            int color = (int)(*(yp + x));
            QByteArray chars(xpm_color_name(cpp, colorMap[color]));
            line[cc++] = chars[0];
            if (cpp > 1) {
                line[cc++] = chars[1];
                if (cpp > 2) {
                    line[cc++] = chars[2];
                    if (cpp > 3) {
                        line[cc++] = chars[3];
                    }
                }
            }
        }
        s << "," << endl << "\"" << line << "\"";
    }
    s << "};" << endl;
}

bool QXpmHandler::canLoadImage() const
{
    return canLoadImage(device());
}

bool QXpmHandler::canLoadImage(QIODevice *device)
{
    if (!device) {
        qWarning("QXpmHandler::canLoadImage() called with no device");
        return false;
    }

    qint64 oldPos = device->pos();

    char head[6];
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

    return qstrncmp(head, "/* XPM", 6) == 0;
}

bool QXpmHandler::load(QImage *image)
{
    return qt_read_xpm_image_or_array(device(), 0, *image);
}

bool QXpmHandler::save(const QImage &image)
{
    write_xpm_image(image, device(), fileName);
    return !image.isNull();
}

bool QXpmHandler::supportsProperty(ImageProperty property) const
{
    return property == Name;
}

QVariant QXpmHandler::property(ImageProperty property) const
{
    return property == Name ? fileName : QString();
}

void QXpmHandler::setProperty(ImageProperty property, const QVariant &value)
{
    if (property == Name)
        fileName = value.toString();
}

QByteArray QXpmHandler::name() const
{
    return "xpm";
}
