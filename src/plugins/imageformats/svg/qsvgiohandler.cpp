#include "qsvgiohandler.h"
#include "qsvgrenderer.h"
#include "qimage.h"
#include "qpixmap.h"
#include "qpainter.h"
#include "qvariant.h"

class QSvgIOHandlerPrivate
{
public:
    QSvgRenderer *r;
    QSize         defaultSize;
    QSize         currentSize;
};

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
    QByteArray contents = device()->readAll();
    device()->seek(0);

    return contents.contains("<svg");
}


QByteArray QSvgIOHandler::name() const
{
    return "svg";
}


bool QSvgIOHandler::read(QImage *image)
{

    d->r->load(device()->readAll());
    d->defaultSize = QSize(d->r->viewBox().width(), d->r->viewBox().height());
    d->currentSize = d->defaultSize;
    if (!d->r->isValid())
        return false;
    *image = QImage(d->currentSize, QImage::Format_ARGB32_Premultiplied);
    QPainter p(image);
    d->r->render(&p);
    p.end();
    return true;
}


QVariant QSvgIOHandler::option(ImageOption option) const
{
    switch(option) {
    case Size:
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
    QByteArray contents = device->readAll();
    device->seek(0);

    return contents.contains("<svg");
}
