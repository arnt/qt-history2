#ifndef ICONCACHE_H
#define ICONCACHE_H

#include <QMap>
#include <QIcon>
#include <QString>
#include <QPair>

#include <abstracticoncache.h>
#include "formeditor_global.h"

class QT_FORMEDITOR_EXPORT IconCache : public AbstractIconCache
{
    Q_OBJECT
public:
    IconCache(QObject *parent);
    virtual QIcon nameToIcon(const QString &path, const QString &resourcePath = QString());
    virtual QString iconToFilePath(const QIcon &pm);
    virtual QString iconToQrcPath(const QIcon &pm);
private:
    typedef QPair<QString, QString> Key;
    typedef QMap<Key, QIcon> NameToIconMap;
    typedef QMap<int, Key> SerialNumberToNameMap;
    NameToIconMap m_name_to_icon;
    SerialNumberToNameMap m_serial_to_name;
};

#endif // ICONCACHE_H
