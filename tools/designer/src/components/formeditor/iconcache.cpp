#include <QImage>
#include <QSet>
#include <QFile>
#include <QDomDocument>
#include "iconcache.h"

class QrcParser
{
public:
    QrcParser(const QString &qrcPath);
    QString resolvePath(const QString &path) const;
private:
    typedef QMap<QString, QSet<QString> > ResourceMap;
    ResourceMap m_resource_map;
};

QrcParser::QrcParser(const QString &qrcPath)
{
    QDomDocument doc;
    QFile file(qrcPath);

    if (!file.open(QIODevice::ReadOnly)) {
        qWarning("QrcParser::QrcParser(): failed to open '%s'",
                    qrcPath.toLatin1().constData());
        return;
    }

    QString error_msg;
    int error_line, error_col;
    if (!doc.setContent(&file, &error_msg, &error_line, &error_col)) {
        qWarning("QrcParser::QrcParser(): failed to parse qrc file:\n%s %d:%d:%s",
                    qrcPath.toLatin1().constData(), error_line, error_col,
                    error_msg.toLatin1().constData());
        return;
    } 

    QDomElement root = doc.firstChildElement(QLatin1String("RCC"));
    if (root.isNull()) {
        qWarning("QrcParser::QrcParser(): bad root element in '%s'",
                    qrcPath.toLatin1().constData());
        return;
    }

    QDomElement relt = root.firstChildElement(QLatin1String("qresource"));
    for (; !relt.isNull(); relt = relt.nextSiblingElement(QLatin1String("qresource"))) {
        QSet<QString> file_set;
        QDomElement felt = relt.firstChildElement(QLatin1String("file"));
        for (; !felt.isNull(); felt = felt.nextSiblingElement(QLatin1String("file")))
            file_set.insert(felt.text());
        m_resource_map.insert(relt.attribute(QLatin1String("prefix")), file_set);
    }
}

QString QrcParser::resolvePath(const QString &_path) const
{
    QString path = _path;
    
    if (!path.startsWith(QLatin1String(":")))
        return QString();
    path = path.mid(1);

    ResourceMap::const_iterator it = m_resource_map.begin();
    for (; it != m_resource_map.end(); ++it) {
        if (!path.startsWith(it.key()))
            continue;
            
        QString result = path.mid(it.key().size() + 1);
        if (it.value().contains(result))
            return result;
        else
            return QString();
    }

    return QString();
}

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

    QString real_path = filePath;

    if (!qrcPath.isEmpty()) {
        QrcParser parser(qrcPath);
        real_path = parser.resolvePath(filePath);
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

