#include "resourcefile.h"

#include <QtCore/QFile>
#include <QtXml/QDomDocument>
#include <QtCore/QTextStream>

ResourceFile::ResourceFile()
{
}

bool ResourceFile::load(QFile &file)
{
    m_resource_map.clear();

    QDomDocument doc;

    QString error_msg;
    int error_line, error_col;
    if (!doc.setContent(&file, &error_msg, &error_line, &error_col)) {
        qWarning("ResourceFile::ResourceFile(): failed to parse qrc file:\n%s %d:%d:%s",
                    file.fileName().toLatin1().constData(), error_line, error_col,
                    error_msg.toLatin1().constData());
        return false;
    } 

    QDomElement root = doc.firstChildElement(QLatin1String("RCC"));
    if (root.isNull()) {
        qWarning("ResourceFile::ResourceFile(): bad root element in '%s'",
                    file.fileName().toLatin1().constData());
        return false;
    }

    QDomElement relt = root.firstChildElement(QLatin1String("qresource"));
    for (; !relt.isNull(); relt = relt.nextSiblingElement(QLatin1String("qresource"))) {
        QStringList file_list;
        QDomElement felt = relt.firstChildElement(QLatin1String("file"));
        for (; !felt.isNull(); felt = felt.nextSiblingElement(QLatin1String("file")))
            file_list.append(felt.text());
        m_resource_map.insert(relt.attribute(QLatin1String("prefix")), file_list);
    }

    return true;
}

void ResourceFile::save(QFile &file)
{
    QDomDocument doc;
    QDomElement root = doc.createElement(QLatin1String("RCC"));
    doc.appendChild(root);

    ResourceMap::const_iterator it = m_resource_map.begin();
    for (; it != m_resource_map.end(); ++it) {
        QDomElement relt = doc.createElement(QLatin1String("qresource"));
        root.appendChild(relt);
        relt.setAttribute(QLatin1String("prefix"), it.key());

        foreach (QString f, it.value()) {
            QDomElement felt = doc.createElement(QLatin1String("file"));
            relt.appendChild(felt);
            QDomText text = doc.createTextNode(f);
            felt.appendChild(text);
        }
    }

    QTextStream stream(&file);
    doc.save(stream, 4);
}

QString ResourceFile::resolvePath(const QString &_path) const
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

QStringList ResourceFile::prefixList() const
{
    return m_resource_map.keys();
}

QStringList ResourceFile::fileList(const QString &prefix)
{
    return m_resource_map.value(prefix);
}

void ResourceFile::addPrefix(const QString &prefix)
{
    m_resource_map.insert(prefix, QStringList());
}

void ResourceFile::addFile(const QString &prefix, const QString &file)
{
    m_resource_map[prefix].append(file);
}

