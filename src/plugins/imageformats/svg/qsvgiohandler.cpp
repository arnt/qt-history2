/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qsvgiohandler.h"
#include "qsvgrenderer.h"
#include "qimage.h"
#include "qpixmap.h"
#include "qpainter.h"
#include "qvariant.h"
#include "qdebug.h"

class QSvgIOHandlerPrivate
{
public:
    QSvgIOHandlerPrivate()
        : r(new QSvgRenderer()), loaded(false)
    {}
    ~QSvgIOHandlerPrivate()
    {
        delete r;
    }

    bool load(QIODevice *device);

    QSvgRenderer *r;
    QSize         defaultSize;
    QSize         currentSize;
    bool          loaded;
};

bool QSvgIOHandlerPrivate::load(QIODevice *device)
{
    if (loaded)
        return true;

    if (r->load(device->readAll())) {
        defaultSize = QSize(r->viewBox().width(), r->viewBox().height());
        if (currentSize.isEmpty())
            currentSize = defaultSize;
    }
    loaded = r->isValid();

    return loaded;
}

QSvgIOHandler::QSvgIOHandler()
    : d(new QSvgIOHandlerPrivate())
{

}


QSvgIOHandler::~QSvgIOHandler()
{
    delete d;
}


bool QSvgIOHandler::canRead() const
{
    QByteArray contents = device()->peek(80);

    return contents.contains("<svg");
}


QByteArray QSvgIOHandler::name() const
{
    return "svg";
}


bool QSvgIOHandler::read(QImage *image)
{
    if (d->load(device())) {
        *image = QImage(d->currentSize, QImage::Format_ARGB32_Premultiplied);
        image->fill(0x00000000);
        QPainter p(image);
        d->r->render(&p);
        p.end();
        return true;
    }

    return false;
}


QVariant QSvgIOHandler::option(ImageOption option) const
{
    switch(option) {
    case Size:
        d->load(device());
        return d->defaultSize;
        break;
    case ScaledSize:
        return d->currentSize;
        break;
    default:
        break;
    }
    return QVariant();
}


void QSvgIOHandler::setOption(ImageOption option, const QVariant & value)
{
    switch(option) {
    case Size:
        d->defaultSize = value.toSize();
        d->currentSize = value.toSize();
        break;
    case ScaledSize:
        d->currentSize = value.toSize();
        break;
    default:
        break;
    }
}


bool QSvgIOHandler::supportsOption(ImageOption option) const
{
    switch(option)
    {
    case Size:
    case ScaledSize:
        return true;
    default:
        break;
    }
    return false;
}

bool QSvgIOHandler::canRead(QIODevice *device)
{
    QByteArray contents = device->peek(80);
    return contents.contains("<svg");
}
