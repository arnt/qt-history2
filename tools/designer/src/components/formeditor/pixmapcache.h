#ifndef PIXMAPCACHE_H
#define PIXMAPCACHE_H

#include <QMap>
#include <QPixmap>
#include <QString>

#include <abstractpixmapcache.h>
#include "formeditor_global.h"

class QT_FORMEDITOR_EXPORT PixmapCache : public AbstractPixmapCache
{
    Q_OBJECT
public:
    PixmapCache(QObject *parent);
    virtual QPixmap nameToPixmap(const QString &name);
    virtual QString pixmapToName(const QPixmap &pm);
private:
    typedef QMap<QString, QPixmap> NameToPixmapMap;
    typedef QMap<int, QString> SerialNumberToNameMap;
    NameToPixmapMap m_name_to_pixmap;
    SerialNumberToNameMap m_serial_to_name;
};

#endif // PIXMAPCACHE_H
