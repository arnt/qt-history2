
#include "qtextimagehandler_p.h"

#include <qtextformat.h>
#include <qpainter.h>
#include <qpixmapcache.h>
#include <qdragobject.h>
#include <qdebug.h>
#include <private/qtextengine_p.h>
#include <qpalette.h>

static QImage getImage(const QString &name)
{
    QImage img;
    const QMimeSource *source = QMimeSourceFactory::defaultFactory()->data(name);
    if (!source)
        return img;
    QImageDrag::decode(source, img);
    return img;
}

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

        QImage img = getImage(format.name());

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

void QTextImageHandler::layoutObject(QTextObject item, const QTextFormat &format)
{
    if (item.width() >= 0)
        return;

    QTextImageFormat imageFormat = format.toImageFormat();

    QPixmap pixmap = getPixmap(imageFormat);
    if (pixmap.isNull())
        return;

    item.setWidth(pixmap.width());
    item.setAscent(pixmap.height() / 2);
    item.setDescent(pixmap.height() / 2);
}

void QTextImageHandler::drawObject(QPainter *p, const QPoint &position, QTextObject item, const QTextFormat &format, QTextLayout::SelectionType selType)
{
    QTextImageFormat imageFormat = format.toImageFormat();
    QPixmap pixmap = getPixmap(imageFormat);

    QPoint adjustedPos(position.x(), position.y() - item.ascent());

    p->drawPixmap(adjustedPos, pixmap);

    if (selType == QTextLayout::Highlight && item.engine()->pal) {
        QRect rect(adjustedPos, pixmap.size());
        QBrush brush(item.engine()->pal->highlight(), QBrush::Dense4Pattern);

        p->fillRect(rect, brush);
    }
}

