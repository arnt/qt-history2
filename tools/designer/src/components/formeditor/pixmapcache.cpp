#include <QImage>
#include <qdebug.h>

#include "pixmapcache.h"

PixmapCache::PixmapCache(QObject *parent)
    : AbstractPixmapCache(parent)
{
}

QPixmap PixmapCache::nameToPixmap(const QString &name)
{
    qDebug() << "PixmapCache::nameToPixmap()" << name;

    NameToPixmapMap::const_iterator it = m_name_to_pixmap.find(name);
    if (it != m_name_to_pixmap.end())
        return *it;
    QImage image(name);
    if (image.isNull())
        return QPixmap();
    QPixmap pm;
    pm.fromImage(image);
    if (pm.isNull())
        return QPixmap();
    m_name_to_pixmap.insert(name, pm);
    m_serial_to_name.insert(pm.serialNumber(), name);
    
    qDebug() << "PixmapCache::nameToPixmap() returning" << pm.serialNumber();
    
    return pm;
}

QString PixmapCache::pixmapToName(const QPixmap &pm)
{
    qDebug() << "PixmapCache::pixmapToName()" << pm.serialNumber();
    
    SerialNumberToNameMap::const_iterator it = m_serial_to_name.find(pm.serialNumber());
    if (it != m_serial_to_name.end()) {
        qDebug() << "PixmapCache::pixmapToName()" << *it;
        return *it;
    }
    return QString();
}

