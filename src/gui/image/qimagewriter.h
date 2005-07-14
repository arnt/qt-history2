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

#ifndef QIMAGEWRITER_H
#define QIMAGEWRITER_H

#include <QtCore/qbytearray.h>
#include <QtCore/qlist.h>

QT_MODULE(Gui)

class QIODevice;
class QImage;

class QImageWriterPrivate;
class Q_GUI_EXPORT QImageWriter
{
public:
    enum ImageWriterError {
        UnknownError,
        DeviceError,
        UnsupportedFormatError
    };

    QImageWriter();
    explicit QImageWriter(QIODevice *device, const QByteArray &format);
    explicit QImageWriter(const QString &fileName, const QByteArray &format = QByteArray());
    ~QImageWriter();

    void setFormat(const QByteArray &format);
    QByteArray format() const;

    void setDevice(QIODevice *device);
    QIODevice *device() const;

    void setFileName(const QString &fileName);
    QString fileName() const;

    void setQuality(int quality);
    int quality() const;

    void setGamma(float gamma);
    float gamma() const;

    void setDescription(const QString &description);
    QString description() const;

    bool canWrite() const;
    bool write(const QImage &image);

    ImageWriterError error() const;
    QString errorString() const;

    static QList<QByteArray> supportedImageFormats();

private:
    Q_DISABLE_COPY(QImageWriter)
    QImageWriterPrivate *d;
};

#endif
