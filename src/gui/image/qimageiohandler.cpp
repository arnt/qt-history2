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

bool QImageIOHandler::canRead() const
{
    return false;
}

bool QImageIOHandler::write(const QImage &)
{
    return false;
}

QVariant QImageIOHandler::option(ImageOption /* option */) const
{
    return QVariant();
}

void QImageIOHandler::setOption(ImageOption /* option */, const QVariant & /* value */)
{
}

bool QImageIOHandler::supportsOption(ImageOption /* option */) const
{
    return false;
}

int QImageIOHandler::currentImageNumber() const
{
    return 0;
}

int QImageIOHandler::imageCount() const
{
    return 0;
}

int QImageIOHandler::loopCount() const
{
    return 0;
}

int QImageIOHandler::nextImageDelay() const
{
    return 0;
}

QImageIOPlugin::QImageIOPlugin(QObject *parent)
    : QObject(parent)
{
}
