#include <QImage>
#include "iconcache.h"

IconCache::IconCache(QObject *parent)
    : AbstractIconCache(parent)
{
}

QIcon IconCache::nameToIcon(const QString &name)
{
    NameToIconMap::const_iterator it = m_name_to_icon.find(name);
    if (it != m_name_to_icon.end())
        return *it;
    QIcon icon(name);
    if (icon.isNull())
        return QIcon();
    m_name_to_icon.insert(name, icon);
    m_serial_to_name.insert(icon.serialNumber(), name);
    return icon;
}

QString IconCache::iconToName(const QIcon &pm)
{
    SerialNumberToNameMap::const_iterator it = m_serial_to_name.find(pm.serialNumber());
    if (it != m_serial_to_name.end())
        return *it;
    return QString();
}

