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

#include <QtCore/qbytearray.h>
#include <QtCore/qlist.h>
#include <QtCore/qsize.h>

class QIODevice;
class QImage;
class QImageIOHandler;
class QImageIOPrivate;
class QRect;
class QString;
class QStringList;

class Q_GUI_EXPORT QImageIO
{
public:
    QImageIO();
    explicit QImageIO(QIODevice *device, const QByteArray &format = QByteArray());
    explicit QImageIO(const QString &fileName, const QByteArray &format = QByteArray());
    ~QImageIO();

    void reset();

    void setImage(const QImage &image);
    QImage image() const;

    void setFormat(const QByteArray &format);
    QByteArray format() const;

    void setDevice(QIODevice *device);
    QIODevice *device() const;

    void setFileName(const QString &fileName);
    QString fileName() const;

    void setQuality(int quality);
    int quality() const;

    void setResolution(const QSize &resolution);
    QSize resolution() const;

    void setRegion(const QRect &region);
    QRect region() const;

    void setDescription(const QString &description);
    QString description() const;

    void setParameters(const QByteArray &parameters);
    QByteArray parameters() const;

    void setGamma(float gamma);
    float gamma() const;

    void setSize(const QSize &size);
    QSize size() const;

    bool load();
    bool save();

    int frameCount() const;
    int nextFrameDelay() const;
    int currentFrameNumber() const;
    int loopCount() const;
    bool hasNextFrame() const;

    enum ImageFormatError {
        UnknownError,
        FileNotFound,
        DeviceError,
        UnsupportedImageFormat,
        InvalidImageData
    };
    ImageFormatError error() const;
    QString errorString() const;

    static QByteArray imageFormatForFileName(const QString &fileName);
    static QByteArray imageFormatForDevice(QIODevice *);
    static QList<QByteArray> inputFormats();
    static QList<QByteArray> outputFormats();

private:
    Q_DISABLE_COPY(QImageIO)
    QImageIOPrivate *d;
};

#endif // QIMAGEIO_H
