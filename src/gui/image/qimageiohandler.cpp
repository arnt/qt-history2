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

#include "qimageiohandler.h"

#include <qbytearray.h>
#include <qimage.h>
#include <qvariant.h>

class QIODevice;

class QImageIOHandlerPrivate
{
    Q_DECLARE_PUBLIC(QImageIOHandler)
public:
    QImageIOHandlerPrivate(QImageIOHandler *q);
    virtual ~QImageIOHandlerPrivate();

    QIODevice *device;
    QByteArray format;

    QImageIOHandler *q_ptr;
};

QImageIOHandlerPrivate::QImageIOHandlerPrivate(QImageIOHandler *q)
{
    device = 0;
    q_ptr = q;
}

QImageIOHandlerPrivate::~QImageIOHandlerPrivate()
{
}

QImageIOHandler::QImageIOHandler()
    : d_ptr(new QImageIOHandlerPrivate(this))
{
}

QImageIOHandler::QImageIOHandler(QImageIOHandlerPrivate &dd)
    : d_ptr(&dd)
{
}

QImageIOHandler::~QImageIOHandler()
{
}

void QImageIOHandler::setDevice(QIODevice *device)
{
    Q_D(QImageIOHandler);
    d->device = device;
}

QIODevice *QImageIOHandler::device() const
{
    Q_D(const QImageIOHandler);
    return d->device;
}

void QImageIOHandler::setFormat(const QByteArray &format)
{
    Q_D(QImageIOHandler);
    d->format = format;
}

QByteArray QImageIOHandler::format() const
{
    Q_D(const QImageIOHandler);
    return d->format;
}

bool QImageIOHandler::canLoadImage() const
{
    return false;
}

bool QImageIOHandler::save(const QImage &)
{
    return false;
}

QVariant QImageIOHandler::property(ImageProperty /* property */) const
{
    return QVariant();
}

void QImageIOHandler::setProperty(ImageProperty /* property */, const QVariant & /* value */)
{
}

bool QImageIOHandler::supportsProperty(ImageProperty /* property */) const
{
    return false;
}

int QImageIOHandler::currentFrameNumber() const
{
    return 0;
}

int QImageIOHandler::frameCount() const
{
    return 0;
}

int QImageIOHandler::loopCount() const
{
    return 0;
}

int QImageIOHandler::nextFrameDelay() const
{
    return 0;
}

QImageIOPlugin::QImageIOPlugin(QObject *parent)
    : QObject(parent)
{
}
