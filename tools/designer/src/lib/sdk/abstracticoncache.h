#ifndef ABSTRACTICONCACHE_H
#define ABSTRACTICONCACHE_H

#include <QObject>
#include "sdk_global.h"

class QIcon;
class QString;

class QT_SDK_EXPORT AbstractIconCache : public QObject
{
    Q_OBJECT
public:
    AbstractIconCache(QObject *parent)
        : QObject(parent) {}
    virtual QIcon nameToIcon(const QString &filePath, const QString &qrcPath = QString()) = 0;
    virtual QString iconToFilePath(const QIcon &pm) = 0;
    virtual QString iconToQrcPath(const QIcon &pm) = 0;
};

#endif // ABSTRACTICONCACHE_H
