
#include "qtextimagehandler_p.h"

#include <qtextformat.h>
#include <qpainter.h>
#include <qpixmapcache.h>
#include <qdragobject.h>
#include <qdebug.h>
#include <private/qtextengine_p.h>
#include <qpalette.h>

static QPixmap getPixmap(const QTextImageFormat &format)
{
    QPixmap pm;

    QSize size;

    if (format.hasProperty(QTextFormat::ImageWidth))
        size.setWidth(format.width());

    if (format.hasProperty(QTextFormat::ImageHeight))
        size.setHeight(format.height());

    QString key = QString("$qt_rt_%1_%2_%3").arg(format.name()).arg(size.width()).arg(size.height());
    if (!QPixmapCache::find(key, pm)) {

        QImage img = qFromMimeSource_helper(format.name());

        if (img.isNull())
            return pm;

        if (size.isValid() && img.size() != size)
            img = img.smoothScale(size);

        pm.convertFromImage(img);
        QPixmapCache::insert(key, pm);
    }
    return pm;
}

QTextImageHandler::QTextImageHandler(QObject *parent)
    : QObject(parent)
{
}

QSize QTextImageHandler::intrinsicSize(const QTextFormat &format)
{
    QTextImageFormat imageFormat = format.toImageFormat();

    QPixmap pixmap = getPixmap(imageFormat);
    return pixmap.size();
}

void QTextImageHandler::drawObject(QPainter *p, const QRect &rect, const QTextFormat &format)
{
    QTextImageFormat imageFormat = format.toImageFormat();
    QPixmap pixmap = getPixmap(imageFormat);

    p->drawPixmap(rect, pixmap);
}

