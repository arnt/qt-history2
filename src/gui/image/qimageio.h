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

#ifndef QIMAGEIO_H
#define QIMAGEIO_H

#include <QtCore/qglobal.h>
#include <QtGui/qimage.h>
#include <QtCore/qstring.h>
#include <QtCore/qstringlist.h>

#ifndef QT_NO_IMAGEIO

class QImageIO;
class QIODevice;
struct QImageIOData;
typedef void (*image_io_handler)(QImageIO *); // image IO handler


class Q_GUI_EXPORT QImageIO
{
public:
    QImageIO();
    QImageIO(QIODevice *ioDevice, const char *format);
    QImageIO(const QString &fileName, const char* format);
    ~QImageIO();

    const QImage &image() const { return im; }
    int status() const { return iostat; }
    const char *format() const { return frmt; }
    QIODevice *ioDevice() const { return iodev; }
    QString fileName() const { return fname; }
    int quality() const;
    QString description() const { return descr; }
    const char *parameters() const;
    float gamma() const;

    void setImage(const QImage &);
    void setStatus(int);
    void setFormat(const char *);
    void setIODevice(QIODevice *);
    void setFileName(const QString &);
    void setQuality(int);
    void setDescription(const QString &);
    void setParameters(const char *);
    void setGamma(float);

    bool read();
    bool write();

    static QByteArray imageFormat(const QString &fileName);
    static QByteArray imageFormat(QIODevice *);
    static QList<QByteArray> inputFormats();
    static QList<QByteArray> outputFormats();

    inline static void defineIOHandler(const char *format,
                                       const char *header,
                                       const char *flags,
                                       image_io_handler read_image,
                                       image_io_handler write_image) {
        defineIOHandler(format, header, flags, QStringList(QString(format).toLower()), 
                        read_image, write_image);
    }
    static void defineIOHandler(const char *format,
                                const char *header,
                                const char *flags,
                                const QStringList &extensions,
                                image_io_handler read_image,
                                image_io_handler write_image);


private:
    Q_DISABLE_COPY(QImageIO)

    void init();

    QImage im;               // image
    int iostat;              // IO status
    QByteArray frmt;         // image format
    QIODevice *iodev;        // IO device
    QString fname;           // file name
    char *params;            // image parameters
    QString descr;           // image description
    QImageIOData *d;
};

#endif // QT_NO_IMAGEIO

#endif // QIMAGEIO_H
