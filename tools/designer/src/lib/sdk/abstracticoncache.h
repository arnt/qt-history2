#ifndef ABSTRACTICONCACHE_H
#define ABSTRACTICONCACHE_H

#include <QObject>
#include "sdk_global.h"

class QIcon;
class QPixmap;
class QString;

class QT_SDK_EXPORT AbstractIconCache : public QObject
{
    Q_OBJECT
public:
    AbstractIconCache(QObject *parent)
        : QObject(parent) {}
    virtual QIcon nameToIcon(const QString &filePath, const QString &qrcPath = QString()) = 0;
    virtual QString iconToFilePath(const QIcon &pm) const = 0;
    virtual QString iconToQrcPath(const QIcon &pm) const = 0;
    virtual QPixmap nameToPixmap(const QString &filePath, const QString &qrcPath = QString()) = 0;
    virtual QString pixmapToFilePath(const QPixmap &pm) const = 0;
    virtual QString pixmapToQrcPath(const QPixmap &pm) const = 0;

    virtual QList<QPixmap> pixmapList() const = 0;
    virtual QList<QIcon> iconList() const = 0;
};

#endif // ABSTRACTICONCACHE_H
