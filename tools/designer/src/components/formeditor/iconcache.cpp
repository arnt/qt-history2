#include <QImage>
#include <QFile>

#include <resourcefile.h>

#include "iconcache.h"

IconCache::IconCache(QObject *parent)
    : AbstractIconCache(parent)
{
}

QIcon IconCache::nameToIcon(const QString &filePath, const QString &qrcPath)
{
    Key key = qMakePair(filePath, qrcPath);
    NameToIconMap::const_iterator it = m_name_to_icon.find(key);
    if (it != m_name_to_icon.end())
        return *it;

    QString real_path;
    if (!qrcPath.isEmpty()) {
        QFile file(qrcPath);
        ResourceFile rf;
        if (file.open(QIODevice::ReadOnly) && rf.load(file))
            real_path = rf.resolvePath(filePath);
    } else {
       real_path = filePath;
    }
        
    if (real_path.isEmpty())
        return QIcon();

    QIcon icon(real_path);
    if (icon.isNull())
        return QIcon();
    m_name_to_icon.insert(key, icon);
    m_serial_to_name.insert(icon.serialNumber(), key);
    
    return icon;
}

QString IconCache::iconToFilePath(const QIcon &pm)
{
    SerialNumberToNameMap::const_iterator it = m_serial_to_name.find(pm.serialNumber());
    if (it != m_serial_to_name.end())
        return (*it).first;
    return QString();
}

QString IconCache::iconToQrcPath(const QIcon &pm)
{
    SerialNumberToNameMap::const_iterator it = m_serial_to_name.find(pm.serialNumber());
    if (it != m_serial_to_name.end())
        return (*it).second;
    return QString();
}

