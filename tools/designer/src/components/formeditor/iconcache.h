#ifndef ICONCACHE_H
#define ICONCACHE_H

#include <QMap>
#include <QIcon>
#include <QString>

#include <abstracticoncache.h>
#include "formeditor_global.h"

class QT_FORMEDITOR_EXPORT IconCache : public AbstractIconCache
{
    Q_OBJECT
public:
    IconCache(QObject *parent);
    virtual QIcon nameToIcon(const QString &name);
    virtual QString iconToName(const QIcon &pm);
private:
    typedef QMap<QString, QIcon> NameToIconMap;
    typedef QMap<int, QString> SerialNumberToNameMap;
    NameToIconMap m_name_to_icon;
    SerialNumberToNameMap m_serial_to_name;
};

#endif // ICONCACHE_H
