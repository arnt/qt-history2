#include <QImage>
#include "pixmapcache.h"

PixmapCache::PixmapCache(QObject *parent)
    : AbstractPixmapCache(parent)
{
}

QPixmap PixmapCache::nameToPixmap(const QString &name)
{
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
    return pm;
}

QString PixmapCache::pixmapToName(const QPixmap &pm)
{
    SerialNumberToNameMap::const_iterator it = m_serial_to_name.find(pm.serialNumber());
    if (it != m_serial_to_name.end())
        return *it;
    return QString();
}

