#ifndef ABSTRACTPIXMAPCACHE_H
#define ABSTRACTPIXMAPCACHE_H

#include <QObject>
#include "sdk_global.h"

class QPixmap;
class QString;

class QT_SDK_EXPORT AbstractPixmapCache : public QObject
{
    Q_OBJECT
public:
    AbstractPixmapCache(QObject *parent)
        : QObject(parent) {}
    virtual QPixmap nameToPixmap(const QString &name) = 0;
    virtual QString pixmapToName(const QPixmap &pm) = 0;
};
    
#endif // ABSTRACTPIXMAPCACHE_H
