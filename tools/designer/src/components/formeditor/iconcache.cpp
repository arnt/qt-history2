#include <QtCore/qdebug.h>
#include <QtCore/QFile>
#include <QtGui/QImage>

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
        ResourceFile rf(qrcPath);
        if (rf.load()) {
            real_path = rf.resolvePath(filePath);
        } else {
            qWarning("IconCache::nameToIcon(): failed to open \"%s\": %s",
                        qrcPath.toLatin1().constData(),
                        rf.errorMessage().toLatin1().constData());
        }
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

