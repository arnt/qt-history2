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

#ifndef QASYNCIMAGEIO_H
#define QASYNCIMAGEIO_H

#include "QtGui/qimage.h"

#ifndef QT_NO_ASYNC_IMAGE_IO


class Q_GUI_EXPORT QImageConsumer {
public:
    virtual void end()=0;

    // Change transfer type 1.
    virtual void changed(const QRect&) = 0;
    virtual void frameDone() = 0;

    // Change transfer type 2.
    virtual void frameDone(const QPoint&, const QRect&) = 0;

    virtual void setLooping(int) = 0;
    virtual void setFramePeriod(int) = 0;
    virtual void setSize(int, int) = 0;
};

class Q_GUI_EXPORT QImageFormat {
public:
    virtual ~QImageFormat();
    virtual int decode(QImage& img, QImageConsumer* consumer,
                        const uchar* buffer, int length) = 0;
};

class Q_GUI_EXPORT QImageFormatType {
public:
    virtual ~QImageFormatType();
    virtual QImageFormat* decoderFor(const uchar* buffer, int length) = 0;
    virtual QByteArray formatName() const = 0;
protected:
    QImageFormatType();
};

class QImageDecoderPrivate;
class Q_GUI_EXPORT QImageDecoder {
public:
    QImageDecoder(QImageConsumer* c);
    ~QImageDecoder();

    const QImage& image() { return img; }
    int decode(const uchar* buffer, int length);

    static QByteArray formatName(const uchar* buffer, int length);
    static QImageFormatType* format(QByteArray name); // direct use - no decode()

    static QList<QByteArray> inputFormats();
    static void registerDecoderFactory(QImageFormatType*);
    static void unregisterDecoderFactory(QImageFormatType*);

private:
    QImageFormat* actual_decoder;
    QImageConsumer* consumer;
    QImage img;
    QImageDecoderPrivate *d;
};

#endif // QT_NO_ASYNC_IMAGE_IO

#endif // QASYNCIMAGEIO_H
